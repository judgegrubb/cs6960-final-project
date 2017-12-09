#include "types.h"
#include "x86.h"
#include "defs.h"
#include "traps.h"

// Constants used for talking to the PS/2 controller
#define PS2CTRL             0x64    // PS/2 controller status port
#define PS2DATA             0x60    // PS/2 controller data port
#define PS2DIB              0x01    // PS/2 data in buffer
#define MOUSE_ENABLEIRQ     0x02    // The enable IRQ 12 bit in the Compaq
                                    // status byte

// Constants in the mouse movement status byte
#define MOUSE_LEFT      1           // Bit indicating the left mouse button
#define MOUSE_RIGHT     2           // Bit indicating the right mouse button
#define MOUSE_MIDDLE    4           // Bit indicating the middle mouse button
#define MOUSE_XSIGN     (1 << 4)    // Sign bit for the x-axis
#define MOUSE_YSIGN     (1 << 5)    // Sign bit for the y-axis
#define MOUSE_ALWAYS_SET 0xC0       // This bit is always set to 1

static unsigned char count = 0;
static char mouse_data[3];

static void
mousewait(int is_read)
{
  uint tries = 100000;
  while (tries--) {
    if (is_read) {
      if (inb(PS2CTRL) & 1) return;
    }
    else {
      if ((inb(PS2CTRL) & 2) == 0) return;
    }
  }
  cprintf("Tries expired, couldn't read mouse data.\n");
}

static void
mousewait_recv(void)
{
  mousewait(1);
}

static void
mousewait_send(void)
{
  mousewait(0);
}

void
mousecmd(uchar cmd)
{
  uchar ack;
  // Sending a command
  mousewait_send();
  outb(PS2CTRL, 0xD4);
  mousewait_send();
  outb(PS2DATA, cmd);
  mousewait_recv();
  do {
    ack = inb(PS2DATA);
  } while (ack != 0xFA);
}

void
mouseinit(void)
{
  mousewait_send();
  outb(PS2CTRL, 0xA8); // enable secondary ps/2 device
  mousewait_send();
  outb(PS2CTRL, 0x20); // tell it we want the compaq status byte
  unsigned char status_byte;
  mousewait_recv();
  status_byte = (inb(PS2DATA) | 2); // get status byte and set interrupts enabled bit to 1
  mousewait_send();
  outb(PS2CTRL, PS2DATA); // we're going to send the modified status byte
  mousewait_send();
  outb(PS2DATA, status_byte); // send the status byte
  mousecmd(0xF6); // default mouse settings
  mousecmd(0xF4); // activate mouse
  ioapicenable(IRQ_MOUSE, 0); // enable ioapic for mouse interrupts
}

static int leftpressed = 0;
static int rightpressed = 0;

void
mouseintr(void)
{
  unsigned char status_byte;
  mousewait_recv();
  status_byte = (inb(PS2CTRL) & PS2DIB);
  if (status_byte) {
    mousewait_recv();
    mouse_data[count++] = inb(PS2DATA);
    if (count == 3) {
      count = 0;
      if ((mouse_data[0] & 0x80) || (mouse_data[0] & 0x40)) {
        // x and y overflow, if either bit is set just ignore movement
        return;
      }
      if (mouse_data[0] & MOUSE_LEFT) {
        leftpressed = 1;
      } else {
        if (leftpressed) {
          leftpressed = 0;
          mouse_leftclick();
          return;
        }
      }
      if (mouse_data[0] & MOUSE_RIGHT) {
        rightpressed = 1;
      } else {
        if (rightpressed) {
          rightpressed = 0;
          mouse_rightclick();
          return;
        }
      }
      int xmove = (int) mouse_data[1];
      int ymove = (int) mouse_data[2];
      movecursor(xmove, ymove);
    }
  }
}
