/*
 * =====================================================================================
 *
 *       Filename:  ppu_registers.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  14-01-23 10:35:01 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <stdint.h>

#include "ppu_registers.h"

struct ppu_registers {
	/* holds the vram address */
	uint16_t vram_addr;
	uint16_t temp_vram_addr;

	/* Toggle for writes to PPUSCROLL and PPUADDR */
	uint8_t toggle;

	uint8_t fine_x_scroll;
};

struct ppu_registers *PPU_Registers_init()
{
	struct ppu_registers *reg = malloc(sizeof(struct ppu_registers));
	reg->toggle = 0;

	return reg;
}

inline void write_toggle(struct ppu_registers *reg)
{
	if (reg->toggle == 0) {
		reg->toggle = 1;
	} else {
		reg->toggle = 0;
	}
}

inline void PPU_Registers_write_address(struct ppu_registers *reg, struct memory *mem, uint8_t val)
{
	if (reg->toggle == 0) {
		reg->temp_vram_addr = (uint16_t)val << 8;
	} else {
		reg->temp_vram_addr |= val;
		reg->vram_addr = reg->temp_vram_addr;
	}

	write_toggle(reg);

	MEM_write(mem, PPUADDR_ADDR, val);
}


inline void PPU_Registers_write_scroll(struct ppu_registers *reg, struct memory *mem, uint8_t val)
{
	uint8_t low_3 = val & 7;
	uint8_t hi_5 = val & 504;

	if (reg->toggle == 0) {
		reg->fine_x_scroll = low_3;
		reg->temp_vram_addr = hi_5 >> 3;
	} else {
		reg->temp_vram_addr |= ((uint16_t)low_3 << 12) & ((uint16_t)hi_5 << 2);
	}

	write_toggle(reg);
	
	MEM_write(mem, PPUSCROLL_ADDR, val);
}
