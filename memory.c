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

#define MEM_SIZE 0xFFFF
#define MEM_ROM_LOW_BANK_ADDR 0x8000
#define MIRROR_ADDR 0x2000
#define MIRROR_SIZE 0x0800
#define VRAM_REG_ADDR 0x2000
#define VRAM_REG_MIRROR_SIZE 8

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

void write_mirrored_ppu_registers(struct memory *mem, const uint16_t base_addr, const uint8_t val)
{
	int i;
	for(i = 0; i < 1024; i++) {
		mem->memory[base_addr + i * VRAM_REG_MIRROR_SIZE] = val;
	}
}

uint8_t MEM_read(struct memory *mem, const uint16_t addr)
{
	uint8_t val;

	/* There are resitrictions on reading VRAM */
	if ((addr >= VRAM_REG_ADDR) && (addr < IO_REG_ADDR))
	{
		/* Calculate the base PPU register address */
		uint16_t base_addr = (addr % VRAM_REG_MIRROR_SIZE) + VRAM_REG_ADDR;
		if ((base_addr == MEM_PPU_OAMADDR_REG_ADDR) || (base_addr == MEM_PPU_SCROLL_REG_ADDR) || (base_addr == MEM_PPU_ADDR_REG_ADDR)) {
			(void)printf("*** Read %#x not allowed here! ***\n", addr);
			exit(0);
		} else {
			val =  mem->memory[addr];
		}
	} else {
		val = mem->memory[addr];
	}
#ifdef DEBUG	
	(void)printf("Read data %#x from address %#x\n", val, addr);
#endif
	return val;
}

uint8_t MEM_get_ppu_ctrl_1(struct memory *mem) {
	return mem->memory[MEM_PPU_CTRL_1_REG_ADDR];
}

uint8_t MEM_get_ppu_ctrl_2(struct memory *mem) {
	return mem->memory[MEM_PPU_CTRL_2_REG_ADDR];
}

void MEM_write(struct memory *mem, const uint16_t addr, const uint8_t val)
{
#ifdef DEBUG
	(void)printf("Writing data %#x into address %#x\n", val, addr);
#endif
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
		
		/* Cannot write to 0x2002 */
		if (base_addr == MEM_PPU_STATUS_REG_ADDR) {
			(void)printf("*** Write to 0x2002 not permitted ***");
			exit(0);
		}

		/* Write to all mirrored addresses for this PPU register */
		write_mirrored_ppu_registers(mem, base_addr, val);
	} 
	/* all other writes */
	else
	{
		mem->memory[addr] = val;
	}
}

void MEM_set_ppu_status(struct memory *mem, const uint8_t val)
{
	write_mirrored_ppu_registers(mem, MEM_PPU_STATUS_REG_ADDR, val);
}

void MEM_load_trainer(struct memory *mem, FILE *nes_file)
{
	uint8_t *mem_ptr = mem->memory + 0x7000;
	uint8_t data;

	int i = 0;
	while((fread(&data, sizeof(uint8_t), 1, nes_file) != 0) && (i < 512)) {
		*mem_ptr = data;
		mem_ptr++;
		i++;
	}
}

void MEM_load_rom(struct memory *mem, FILE *nes_file)
{
	uint8_t data;
	uint32_t mem_addr;

	(void)printf("Loading ROM now!\n");

	/* Load 2*16 kb ROM banks into shared memory */
	mem_addr = MEM_ROM_LOW_BANK_ADDR;
	while ((fread(&data, sizeof(uint8_t), 1, nes_file) != 0) && (mem_addr <= MEM_SIZE)) {
#ifdef DEBUG
		(void)printf("Loading ROM...");
#endif
		MEM_write(mem, mem_addr, data);
#ifdef DEBUG
		(void)printf("OK!\n");
#endif
		mem_addr++;
	}
}

void MEM_print_test_status(struct memory *mem)
{
	(void)printf("%#x: ", mem->memory[0x6000]);
	char *a = &(mem->memory[0x6004]);
	int length = 0;
	while(*(a++) != '\0' || length++ < 10) {
		(void)printf("%c", *a);
	}
	(void)printf("\n");
}
