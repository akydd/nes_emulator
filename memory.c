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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memory.h"

struct memory {
	uint8_t memory[MEM_SIZE];
};

struct memory *MEM_init()
{
	/* Allocate memory */
	struct memory *mem = malloc(sizeof(struct memory));

	/* clear the allocated memory */
	uint8_t *mem_ptr = NULL;
	for(mem_ptr = mem->memory; mem_ptr < mem->memory + MEM_SIZE; mem_ptr++)
	{
		*mem_ptr = 0;
	}
	return mem;
}

void MEM_delete(struct memory **mem)
{
	free(*mem);
	*mem = NULL;
}

uint8_t MEM_read(struct memory *mem, const uint16_t addr)
{
	return mem->memory[addr];
}

void MEM_write(struct memory *mem, const uint16_t addr, const uint8_t val)
{
	/* write to mirrored RAM */
	if (addr < MIRROR_ADDR)
	{
		uint16_t base_addr = addr % MIRROR_SIZE; 
		mem->memory[base_addr] = val;
		mem->memory[base_addr + 1 * MIRROR_SIZE] = val;
		mem->memory[base_addr + 2 * MIRROR_SIZE] = val;
		mem->memory[base_addr + 3 * MIRROR_SIZE] = val;
	}
	/* write to mirrored VRAM */
	else if ((addr >= VRAM_REG_ADDR) && (addr < IO_REG_ADDR))
	{
		uint16_t base_addr = (addr % VRAM_REG_MIRROR_SIZE) + VRAM_REG_ADDR;
		int i;
		for(i = 0; i < 1024; i++) {
			mem->memory[base_addr + i * VRAM_REG_MIRROR_SIZE] = val;
		}
	} 
	/* all other writes */
	else
	{
		mem->memory[addr] = val;
	}
}
