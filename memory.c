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
 *
 * =====================================================================================
 */
#include <stdlib.h>

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
	/* Read from 0x2002 unsets the VBLANK flag at bit 7.  Direct memory
	 * write is ok as 0x2002 is not mirrored. */
	if (addr == MEM_PPU_STATUS_REG_ADDR) {
		mem->memory[addr] &= ~(1<<7);
	}
	return mem->memory[addr];
}

uint8_t MEM_read_no_set(struct memory *mem, const uint16_t addr)
{
	return mem->memory[addr];
}

void write_mirrored_ppu_registers(struct memory *mem, const uint16_t base_addr, const uint8_t val)
{
	int i;
	for(i = 0; i < 1024; i++) {
		mem->memory[base_addr + i * VRAM_REG_MIRROR_SIZE] = val;
	}
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
		/* Calculate the base PPU register address */
		uint16_t base_addr = (addr % VRAM_REG_MIRROR_SIZE) + VRAM_REG_ADDR;

		/* Write to all mirrored addresses for this PPU register */
		write_mirrored_ppu_registers(mem, base_addr, val);


		/* Handle special PPU cases */

		/* Write to 0x2004 autoincrements 0x2003 by 1 */
		if (base_addr == MEM_PPU_OAMDATA_REG_ADDR) {
			uint8_t incremented_val = mem->memory[MEM_PPU_OAMADDR_REG_ADDR] + 1;
			write_mirrored_ppu_registers(mem, MEM_PPU_OAMADDR_REG_ADDR, incremented_val);
		}
	} 
	/* all other writes */
	else
	{
		mem->memory[addr] = val;
	}
}
