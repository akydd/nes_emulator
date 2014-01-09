/*
 * =====================================================================================
 *
 *       Filename:  loader.c
 *
 *    Description:  Implementation for file loader
 *
 *        Version:  1.0
 *        Created:  13-08-09 10:58:26 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alan Kydd (), akydd@ualberta.net
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "loader.h"

int LOADER_load_file(struct memory *mem, struct ppu_memory *ppu_mem, char *filename)
{
	/* Open file in binary mode (needed for Windows) */
	FILE *nes_file = fopen(filename, "rb");

	if (nes_file == NULL) {
		(void)printf("Input file not found!\n");
		return 0;
	}

	/* Get the file header */
	uint8_t header[16];
	int bytes_read;
	if ((bytes_read = fread(header, sizeof(uint8_t), 16, nes_file)) != 16) {
		(void)printf("Only read %d bytes from file header!\n", bytes_read);
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
	uint8_t num_8kb_ram_banks = header[8];

	(void)printf("Number of 16 kb rom banks: %d\n", num_16kb_rom_banks);
	(void)printf("Number of 8 kb vrom banks: %d\n", num_8kb_vrom_banks);
	(void)printf("Number of 8 kb ram banks: %d\n", num_8kb_ram_banks);

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

	/* Load 2*16 kb ROM banks into shared memory */
	mem_addr = MEM_ROM_LOW_BANK_ADDR;
	while ((fread(&data, sizeof(uint8_t), 1, nes_file) != 0) && (mem_addr <= MEM_SIZE)) {
		MEM_write(mem, mem_addr, data);
		if(mem_addr % 1024 == 0) {
			(void)printf("Loading data %#x into %#x\n", data, mem_addr);
		}
		mem_addr++;
	}

	/* Load 8 kb VROM bank into PPU memory, if present */
	if(num_8kb_vrom_banks == 1) {
		mem_addr = 0;
		while ((fread(&data, sizeof(uint8_t), 1, nes_file) != 0) && (mem_addr < 0x2000)) {
			PPU_MEM_write(ppu_mem, mem_addr, data);
			if(mem_addr % 1024 == 0) {
				(void)printf("Loading data %#x into VROM %#x\n", data, mem_addr);
			}
			mem_addr++;
		}
	}

	(void)fclose(nes_file);
	return 1;
}
