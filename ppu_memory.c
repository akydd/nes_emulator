/*
 * =============================================================================
 *
 *       Filename:  ppu_memory.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  13-08-10 03:31:16 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alan Kydd (), akydd@ualberta.net
 *   Organization:  
 *
 * =============================================================================
 */
#include <stdlib.h>

#include "ppu_memory.h"

struct ppu_memory {
	uint8_t memory[PPU_MEM_SIZE];
};

struct ppu_memory *PPU_MEM_init()
{
	/* Allocate memory */
	struct ppu_memory *ppu_mem = malloc(sizeof(struct ppu_memory));

	/* clear the allocated memory */
	uint8_t *mem_ptr = NULL;
	for(mem_ptr = ppu_mem->memory; mem_ptr < ppu_mem->memory + PPU_MEM_SIZE; mem_ptr++)
	{
		*mem_ptr = 0;
	}

	return ppu_mem;
}

void PPU_MEM_delete(struct ppu_memory **ppu_mem)
{
	free(*ppu_mem);
	*ppu_mem = NULL;
}

void PPU_MEM_write(struct ppu_memory *ppu_mem, const uint16_t addr, const uint8_t val)
{
	ppu_mem->memory[addr] = val;
}
