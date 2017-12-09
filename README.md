## cs6960 final project
#### Elijah Grubb u0894728

For my final project I implemented the [bonus hw](http://panda.moyix.net/~moyix/cs3224/fall16/bonus_hw/bonus_hw.html) of Brendan Dolan-Gavitt's
OS class at NYU.

I modified the `Makefile`, `console.c`, `defs.h`, `main.c`, `trap.c` and `traps.h`.

I also added `mouse.c`. The bulk of my code exists in `console.c` and `mouse.c`.

This project runs with a standard `make qemu`. `make qemu-nox` doesn't provide
the needed interface for using the mouse driver.

