# nes_emulator

My attempt at writing a NES emulator

## Design

As far as I know right now, the major NES components are:
* An 8-bit processor based on the 6502
* 2 kilobytes of onboard RAM
* A custom Picture Processing Unit (PPU)
* A 5-channel Audio Processing Unit (APU)

### 6502 Details

The 6502 processor has the following registers.
* An 8-bit accumulator register (A)
* Two 8-bit index registsers (X and Y)
* An 8-bit processor status register (P).  Contains the flags
  * Carry (C)
  * Zero (Z)
  * Interrupt Disable
  * Decimal Mode
  * Break Command
  * Overflow (O)
  * Negative (N)
* An 8-bit stack pointer (S)
* A 16-bit program counter (PC)

There are a total of 56 instructions.  You can find them
[here](http://www.obelisk.demon.co.uk/6502/instructions.html), with complete
descriptions [here](http://www.obelisk.demon.co.uk/6502/reference.html).

The processor has 13 addressing modes.  You can find them
[here](http://www.obelisk.demon.co.uk/6502/addressing.html).


