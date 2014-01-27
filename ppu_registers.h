/*
 * =====================================================================================
 *
 *       Filename:  ppu_registers.h
 *
 *    Description:  PPU registers are externalized as they are accessed from the
 *                  CPU (via reads/writes to main memory) and by the PPU
 *
 *        Version:  1.0
 *        Created:  14-01-23 10:26:46 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alan Kydd
 *
 * =====================================================================================
 */

#ifndef PPU_REGISTERS
#define PPU_REGISTERS

#include "memory.h"

struct ppu_registers;

extern struct ppu_registers *PPU_Registers_init();

extern uint8_t PPU_Registers_read_status(struct ppu_registers *reg, struct memory *mem);

extern void PPU_Registers_write_status(struct ppu_registers *reg, uint8_t val);

extern void PPU_Registers_write_address(struct ppu_registers *reg, struct memory *, uint8_t val);

extern void PPU_Registers_write_OAM_addr(struct ppu_registers *reg, uint8_t val);

extern void PPU_Registers_write_OAM_data(struct ppu_registers *reg, uint8_t val);

extern void PPU_Registers_write_data(struct ppu_registers *reg, uint8_t val);

#endif
