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
 *
 * =============================================================================
 */
#include <stdlib.h>

#include "ppu_memory.h"

#define PALETTE_RAM_ADDR 0x3F00
#define PALETTE_RAM_SIZE 32
#define NAME_TABLE_0_ADDR 0x2000
#define ALL_TABLE_MIRROR_ADDR 0x3000

struct ppu_memory {
	uint8_t memory[PPU_MEM_SIZE];
	uint8_t mirror_type; // 0 = horizontal mirroring, 1 = vertical mirroring
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

void write_mirrored_palette(struct ppu_memory *ppu_mem, const uint16_t addr, const uint8_t val)
{
	/* These four addresses are mirrored within 0x3F00 - 0x3F1F */
	if (addr == 0x3F00) {
		write_mirrored_palette(ppu_mem, 0x3F10, val);
	}
	if (addr == 0x3F04) {
		write_mirrored_palette(ppu_mem, 0x3F14, val);
	}
	if (addr == 0x3F08) {
		write_mirrored_palette(ppu_mem, 0x3F18, val);
	}
	if (addr == 0x3F0C) {
		write_mirrored_palette(ppu_mem, 0x3F1C, val);
	}

	/* All values are written 5 times, starting at PALETTE_RAM_ADDR */
	uint16_t base_addr = (addr % PALETTE_RAM_SIZE) + PALETTE_RAM_ADDR;
	int i;
	for(i = 0; i < 5; i++) {
		uint16_t new_addr = base_addr + i * PALETTE_RAM_SIZE;
		ppu_mem->memory[new_addr] = val;
		//(void)printf("Writing %#x to %#x\n", val, new_addr);
	}
}

void write_with_mirror_nametables_to_0x3000(struct ppu_memory *ppu_mem, const uint16_t addr, const uint8_t val)
{
	ppu_mem->memory[addr] = val;

	if (addr < 0x2F00) {
		ppu_mem->memory[addr + 0x1000] = val;
	}
}

void write_with_horizontal_mirroring(struct ppu_memory *ppu_mem, const uint16_t addr, const uint8_t val)
{
	write_with_mirror_nametables_to_0x3000(ppu_mem, addr, val);

	if (addr < 0x2800) {
		if (addr < 0x2400) {
			write_with_mirror_nametables_to_0x3000(ppu_mem, addr + 0x0400, val);
		} else {
			write_with_mirror_nametables_to_0x3000(ppu_mem, addr - 0x0400, val);
		}
	} else {
		if (addr < 0x2C00) {
			write_with_mirror_nametables_to_0x3000(ppu_mem, addr + 0x0400, val);
		} else {
			write_with_mirror_nametables_to_0x3000(ppu_mem, addr - 0x0400, val);
		}
	}
}

void write_with_vertical_mirroring(struct ppu_memory *ppu_mem, const uint16_t addr, const uint8_t val)
{
	write_with_mirror_nametables_to_0x3000(ppu_mem, addr, val);

	if (addr < 0x2800) {
		write_with_mirror_nametables_to_0x3000(ppu_mem, addr + 0x0800, val);
	} else {
		write_with_mirror_nametables_to_0x3000(ppu_mem, addr - 0x0800, val);
	}
}

void write_mirrored_nametables(struct ppu_memory *ppu_mem, const uint16_t addr, const uint8_t val)
{
	if (ppu_mem->mirror_type == 0) {
		write_with_horizontal_mirroring(ppu_mem, addr, val);
	} else {
		write_with_vertical_mirroring(ppu_mem, addr, val);
	}
}

void PPU_MEM_write(struct ppu_memory *ppu_mem, const uint16_t addr, const uint8_t val)
{
	if (addr >= PALETTE_RAM_ADDR) {
		write_mirrored_palette(ppu_mem, addr, val);
	} else if ((addr >= NAME_TABLE_0_ADDR) && (addr < ALL_TABLE_MIRROR_ADDR)) {
		write_mirrored_nametables(ppu_mem, addr, val);
	} else {
		ppu_mem->memory[addr] = val;
	}
}

void PPU_MEM_load_vrom(struct ppu_memory *ppu_mem, FILE *nes_file)
{
	uint8_t data;
	uint32_t mem_addr = 0;

	while ((fread(&data, sizeof(uint8_t), 1, nes_file) != 0) && (mem_addr < 0x2000)) {
		ppu_mem->memory[mem_addr] = data;
		if(mem_addr % 1024 == 0) {
			(void)printf("Loading data %#x into VROM %#x\n", data, mem_addr);
		}
		mem_addr++;
	}
}

void PPU_MEM_set_mirroring(struct ppu_memory *ppu_mem, const uint8_t mirror_type)
{
	ppu_mem->mirror_type = mirror_type;
}
