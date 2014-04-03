/*
 * =============================================================================
 *
 *       Filename:  memory.h
 *
 *    Description:  Public interface to the NES CPU memory module.
 *
 *        Version:  1.0
 *        Created:  13-03-28 08:39:54 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alan Kydd (), akydd@ualberta.net
 *
 * =============================================================================
 */

#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stdio.h>

#include "controller.h"
#include "ppu.h"

#define MEM_STACK_START 0x01FF
#define MEM_STACK_END 0x0100
#define MEM_STACK_OFFSET 0x0100
#define MEM_NMI_VECTOR 0xFFFA
#define MEM_RESET_VECTOR 0xFFFC
#define MEM_BRK_VECTOR 0xFFFE
#define IO_REG_ADDR 0x4000
#define MEM_CONTROLLER_REG_ADDR 0x4016

struct memory;
/*
 * General
 * =======
 *
 * 64 kilobytes (0x10000 bytes) of memory.  For the NES, this memory is organized as:
 *
 *  _________________
 * | IRQ/BRK vector  |	0xFFFF		64 kb
 * | IRQ/BRK vector  |  0xFFFE
 * | Reset vector    |	0xFFFD
 * | Reset vector    |	0xFFFC
 * | NM Interrupt    |  0xFFFB
 * | NM Interrupt    |  0xFFFA
 * |                 |
 * | Cartridge ROM   |
 * | High bank, 16kB |
 * |_________________|	0xC000		48 kb
 * |                 |	0xBFFF
 * | Cartridge ROM   |
 * | Low bank, 16kB  |
 * |_________________|	0x8000		32 kb
 * |                 |	0x7FFF
 * | Save RAM        |
 * | 8 kB            |
 * |_________________|	0x6000		24 kb
 * |                 |	0x5FFF
 * | Expansion mods  |
 * |_________________|	0x5000		20 kb
 * |                 |	0x4FFF
 * | I/O, 12 kB      |
 * |      ___________|	0x4000
 * |     |           |	0x3FFF
 * |     | Internal  |
 * |     | NES VRAM, |
 * |     | 8 kB      |
 * |_____|___________|	0x2000		8 kb
 * |                 |	0x1FFF
 * | 3 RAM mirrors   |
 * |_________________|	0x0800		2 kb
 * |                 |	0x07FF
 * | RAM             |
 * |      ___________|	0x0200
 * |     |           |	0x01FF
 * |     | Stack     |
 * |     |___________|	0x0100
 * |     |           |	0x00FF
 * |     | Zero Page |
 * |_____|___________|	0x0000
 *
 *  Notes:
 *  - The stack starts at 0X01FF and grows down.
 *  - For cartridges with more than 32 kB ROM or more than 8 kB VRAM (VRAM),
 *    the extra data is paged into the address space using mappers.  TODO.
 *
 *
 * APU Memory
 * ==========
 *
 * APU registers are mapped to 0x4000 to 0x4017 above as:
 *
 * Registers	Channel		Units
 * ------------------------------------------------------------
 * $4017	All		Frame counter
 * $4015	All		Channel enable and length counter status
 * $4010-$4013	DMC		Timer, memory reader, sample buffer, output unit
 * $400C-$400F	Noise		Timer, length counter, envelope, linear feedback shift register
 * $4008-$400B	Triangle	Timer, length counter, linear counter
 * $4004-$4007	Pulse 2		Timer, length counter, envelope, sweep
 * $4000-$4003	Pulse 1		Timer, length counter, envelope, sweep
 *
 * Notes:
 * - For details, see apu.h.
 *
 *
 * PPU Memory
 * ==========
 *
 * PPU registers are mapped to 0x2000 to 0x3FFF above as:
 * 
 * 0x2008 - 0x3FFF - 1023 mirrors of 0x2000 - 0x2007
 * 0x2007 (R/W)    - Data
 * 0x2006 (W)      - Address
 * 0x2005 (W)      - Scroll
 * 0x2004 (R/W)    - OAM Data
 * 0x2003 (W)      - OAM Address
 * 0x2002 (R)      - Status
 * 0x2001 (W)      - Mask
 * 0x2000 (W)      - Control
 *
 * Notes:
 * - For internal details of the registers, see ppu_memory.h.
 *
 *
 * Game Controllers
 * ================
 *
 * I/O for the game controllers is mapped to 0x4016 and 0x4017.
 *
 */

/*
 * Create a new memory struct.
 */
extern struct memory *MEM_init();

/*
 * Attach a controller.  In the NES, the controller is mapped into memory.
 */
extern void MEM_attach_controller(struct memory *, struct controller *);

/*
 * Attach a PPU.  In the NES, the PPU registers are mapped into memory.
 */
extern void MEM_attach_ppu(struct memory *, struct ppu *);

/*
 * Delete a memory struct
 */
extern void MEM_delete(struct memory **);

/*
 * Return the value at given memory location.
 */
extern uint8_t MEM_read(struct memory *, const uint16_t);

/* 
 * Writes to memory during CPU execution should be delegated to this function
 * for proper mirroring.
 */
extern void MEM_write(struct memory *, const uint16_t, const uint8_t);

/* 
 * PPU register functions are not included here, as they are the responsibility of the PPU.
 */

/*
 * Load 512 byte trainer into memory at 0x7000 - 0x71FF
 */
extern void MEM_load_trainer(struct memory *, FILE *);

/*
 * Load ROM data into memory
 */
extern void MEM_load_rom(struct memory *, uint8_t, FILE *);

/* 
 * Print blarggs test output 
 */
extern void MEM_print_test_status(struct memory *);

#endif
