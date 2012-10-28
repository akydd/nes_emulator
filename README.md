# nes_emulator

My attempt at writing a NES emulator

## High Level Design

As far as I know right now, the major NES components are:
* An 8-bit processor based on the 6502, without a decimal mode
* 2 kilobytes of onboard RAM
* A custom Picture Processing Unit (PPU)
* A 5-channel Audio Processing Unit (APU)

## 6502

The 6502 processor has the following registers.
* An 8-bit accumulator register (A)
* Two 8-bit index registers (X and Y)
* An 8-bit processor status register (P).  Contains the flags
  * Carry (C)
  * Zero (Z)
  * Interrupt Disable
  * Decimal Mode
  * Break Command
  * Overflow (V)
  * Negative (N)
* An 8-bit stack pointer (S)
* A 16-bit program counter (PC)

### Assembly Language

There are a total of 56 instructions.  You can find them
[here](http://www.obelisk.demon.co.uk/6502/instructions.html), with complete
descriptions [here](http://www.obelisk.demon.co.uk/6502/reference.html), or with
descriptions and opcodes [here](http://6502.org/tutorials/6502opcodes.html).

There is a very nice article on how the opcodes are organized
[here](http://www.llx.com/~nparker/a2/opcodes.html). I hope that this will make
the instruction logic easier to implement in the code.

#### Addressing Modes

The processor has 13 addressing modes.  You can find them
[here](http://www.obelisk.demon.co.uk/6502/addressing.html).

#### Handling the flags

I found good explanations of how to determine when the overflow and carry flags
should be set at [stackoverflow](http://stackoverflow.com/q/8034566).
