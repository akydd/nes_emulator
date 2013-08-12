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
 *   Organization:  
 *
 * =============================================================================
 */
#include <stdlib.h>

#include "ppu.h"

struct ppu *PPU_init(struct memory *mem, struct ppu_memory *ppu_mem)
{
	struct ppu *ppu = malloc(sizeof(struct ppu));

	ppu->mem = mem;
	ppu->ppu_mem = ppu_mem;

	/* Initialize values as of power-on */
	MEM_write(ppu->mem, PPUCTRL_ADDR, 0x00);
	MEM_write(ppu->mem, PPUMASK_ADDR, 0x00);
	MEM_write(ppu->mem, PPUSTATUS_ADDR, 0xa0);

	return ppu;
}

void PPU_delete(struct ppu **ppu)
{
	free(*ppu);
}

uint8_t PPU_step(struct ppu *ppu, uint8_t cycle)
{
	return 1;
}
