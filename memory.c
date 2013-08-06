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
	mem->memory[addr] = val;

	/* write to mirrored RAM */
	if (addr < MIRROR_ADDR) {
		uint16_t base_addr = addr % MIRROR_SIZE; 
		mem->memory[base_addr] = val;
		mem->memory[base_addr + 1 * MIRROR_SIZE] = val;
		mem->memory[base_addr + 2 * MIRROR_SIZE] = val;
		mem->memory[base_addr + 3 * MIRROR_SIZE] = val;
	}

	/* write to mirrored VRAM */
	if ((addr >= VRAM_REG_ADDR) && (addr < IO_REG_ADDR)) {
		uint16_t base_addr = (addr % VRAM_REG_MIRROR_SIZE) + VRAM_REG_ADDR;
		int i;
		for(i = 0; i < 1024; i++) {
			mem->memory[base_addr + i * VRAM_REG_MIRROR_SIZE] = val;
		}
	}
}

int MEM_load_file(struct memory *mem, char *filename)
{
	FILE *nes_file = fopen(filename, "r");

	if (nes_file == NULL) {
		(void)printf("Input file not found!\n");
		return 0;
	}

	/* Get the file header */
	uint8_t header[16];
	if (fread(header, sizeof(uint8_t), 16, nes_file) != 16) {
		(void)printf("Could not read header!\n");
		(void)fclose(nes_file);
		return 0;
	}
	
	/* Ensure proper file format */
	if(strncmp((char *)header, "NES", 3) != 0) {
		(void)printf("File is not an NES file!\n");	
		(void)fclose(nes_file);
		return 0;
	}

	/* Number of memory banks */
	uint8_t num_16kb_rom_banks = header[4];
	uint8_t num_8kb_vrom_banks = header[5];
	uint8_t num_8kb_vram_banks = header[8];

	(void)printf("Number of 16 kb rom banks: %d\n", num_16kb_rom_banks);
	(void)printf("Number of 8 kb vrom banks: %d\n", num_8kb_vrom_banks);
	(void)printf("Number of 8 kb vram banks: %d\n", num_8kb_vram_banks);

	/* Battery backed RAM? */
	uint8_t battery_backed_ram_present = ((header[6] & (1<<1)) != 0);
	(void)printf("Battery backed ram present: %d\n", battery_backed_ram_present);

	/* Trainer present? */
	uint8_t trainer_present = ((header[6] & (1<<2)) != 0);
	(void)printf("Trainer present: %d\n", trainer_present);

	/* memory mapper type */
	uint8_t mapper = (header[7] &= ~15) | (header[6]>>4);
	(void)printf("Memory mapper type: %d\n", mapper);

	/* File read and load operations begin.  File is read as a sequence of unsigned 8 bit ints */
	uint8_t data;
	uint32_t mem_addr;

	/* Load 512 byte trainer into memory 0x7000 - 0x71ff, if present in file */
	if(trainer_present != 0) {
		mem_addr = 0x7000;
		while((fread(&data, sizeof(uint8_t), 1, nes_file) != 0) && (mem_addr < 0x7200)) {
			MEM_write(mem, mem_addr, data);
			mem_addr++;
		}
	}

	/* Load 2*16 kb ROM banks into shared memory*/
	mem_addr = MEM_ROM_LOW_BANK_ADDR;
	while ((fread(&data, sizeof(uint8_t), 1, nes_file) != 0) && (mem_addr <= MEM_SIZE)) {
		MEM_write(mem, mem_addr, data);
		if(mem_addr % 1024 == 0) {
			(void)printf("Loading data into %#x\n", mem_addr);
		}
		mem_addr++;
	}

	/* Load 8 kb VROM banks into PPU memory */

	(void)fclose(nes_file);
	return 1;
}
