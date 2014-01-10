/*
 * =============================================================================
 *
 *       Filename:  ppu.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  13-08-09 10:33:58 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alan Kydd (), akydd@ualberta.net
 *
 * =============================================================================
 */
#include <stdlib.h>

#include "ppu.h"

struct ppu {
	/* 2 registers for generating NMIs */
	unsigned int NMI_occured;
	unsigned int NMI_output;
	/* Other registers are stored in shared memory (?) */
};

struct ppu *PPU_init(struct memory *mem)
{
	struct ppu *ppu = malloc(sizeof(struct ppu));

	/* Initialize values as of power-on */
	MEM_write(mem, PPUCTRL_ADDR, 0x00);
	MEM_write(mem, PPUMASK_ADDR, 0x00);
	MEM_write(mem, PPUSTATUS_ADDR, 0xa0);

	return ppu;
}

void PPU_delete(struct ppu **ppu)
{
	free(*ppu);
}

uint8_t vblank_is_enabled(struct memory *mem)
{
	uint8_t ctrl = MEM_read(mem, PPUCTRL_ADDR);
	return ctrl & (1<<7);
}

inline void set_vblank_flag(struct memory *mem)
{
	uint8_t status = MEM_read_no_set(mem, PPUSTATUS_ADDR);
	status |= (1<<7);
	MEM_write(mem, PPUSTATUS_ADDR, status);
	(void)printf("VBLANK set\n");
}

inline void clear_vblank_flag(struct memory *mem)
{
	uint8_t status = MEM_read_no_set(mem, PPUSTATUS_ADDR);
	status &= ~(1<<7);
	MEM_write(mem, PPUSTATUS_ADDR, status);
	(void)printf("VBLANK cleared\n");
}

uint8_t PPU_step(struct ppu *ppu, struct memory *mem, int cycle)
{
	//(void)printf("PPU is in cycle %d\n", cycle);
	/* VBLANK flag is set at the 2nd cycle of scanline 241.  This is the
	 * start of the VBLANKing interval. */
	if (cycle == 241 * 341 + 1) {
		set_vblank_flag(mem);

		/* This return indicates an NMI to the CPU */
		if (vblank_is_enabled(mem) != 0) {
			(void)printf("Executing VBLANK\n");
			return 0;
		}
	}

	/* VBLANK flag is cleared at the 2nd cycle of scanline 261 */
	if (cycle == 261 * 341 + 1) {
		clear_vblank_flag(mem);
	}

	return 1;
}
