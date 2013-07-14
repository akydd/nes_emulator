/*
 * =====================================================================================
 *
 *       Filename:  memory.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  13-07-13 03:41:55 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alan Kydd (), akydd@ualberta.net
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include "memory.h"

void MEM_init(struct memory *mem)
{
	/* clear the cpu mem */
	uint8_t *mem_ptr = NULL;
	for(mem_ptr = mem->memory; mem_ptr < mem->memory + MEM_SIZE; mem_ptr++)
	{
		*mem_ptr = 0;
	}
}

uint8_t MEM_read(struct memory *mem, const uint16_t addr)
{
	return mem->memory[addr];
}

void MEM_write(struct memory *mem, const uint16_t addr, const uint8_t val)
{
	mem->memory[addr] = val;

	/* write to RAM plus 3 mirrors */
	if (addr < MIRROR_ADDR) {
		mem->memory[addr + MIRROR_ADDR] = val;
		mem->memory[addr + 2 * MIRROR_ADDR] = val;
		mem->memory[addr + 3 * MIRROR_ADDR] = val;
	}

	/* write to VRAM plus 1023 mirrors */
	if ((addr >= VRAM_REG_START) &&
			(addr < ((VRAM_REG_START) + (VRAM_REG_SIZE)))) {
		int i;
		for(i = 1; i < 1024; i++) {
			mem->memory[addr + i * VRAM_REG_SIZE] = val;
		}
	}
}
