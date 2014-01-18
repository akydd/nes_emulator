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

	/* Use these to keep cycle state */
	unsigned int line;
	unsigned int dot;
};

struct ppu *PPU_init(struct memory *mem)
{
	struct ppu *ppu = malloc(sizeof(struct ppu));

	/* Initialize values as of power-on */
	MEM_write(mem, PPUCTRL_ADDR, 0x00);
	MEM_write(mem, PPUMASK_ADDR, 0x00);
	MEM_write(mem, PPUSTATUS_ADDR, 0xa0);

	/* PPU starts at cycle 0 */
	ppu->line = 0;
	ppu->dot = 0;

	return ppu;
}

void PPU_delete(struct ppu **ppu)
{
	free(*ppu);
}

inline uint8_t vblank_is_enabled(struct memory *mem)
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

inline void increment_cycle(struct ppu *ppu)
{
	if (ppu->dot == 340) { // end of line
		ppu->dot = 0;
		if (ppu->line == 261) { // end of frame
			ppu->line = 0;
		} else {
			ppu->line++;
		}
	} else {
		ppu->dot++;
	}
}

inline void render(struct ppu *ppu, struct memory *mem, struct ppu_memory *ppu_mem)
{
	/* VBLANK flag is cleared at the 2nd cycle of scanline 261 */
	if (ppu->line == 261 && ppu->dot == 1) {
		clear_vblank_flag(mem);
	}
}

uint8_t PPU_step(struct ppu *ppu, struct memory *mem, struct ppu_memory *ppu_mem)
{
	// (void)printf("PPU line: %d, dot: %d\n", ppu->line, ppu->dot);

	/* VBLANK flag is set at the 2nd cycle of scanline 241.  This is the
	 * start of the VBLANKing interval. */
	if (ppu->dot == 1 && ppu->line == 241) {
		set_vblank_flag(mem);

		/* This return indicates an NMI to the CPU */
		if (vblank_is_enabled(mem) != 0) {
			(void)printf("Executing VBLANK\n");
			return 0;
		}
	}

	if (ppu->line < 240 || ppu->line == 261) {
		render(ppu, mem, ppu_mem);	
	}

	increment_cycle(ppu);
	return 1;
}
