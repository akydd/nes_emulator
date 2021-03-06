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
#define MEM_ROM_HIGH_BANK_ADDR 0xC000
#define MIRROR_ADDR 0x2000
#define MIRROR_SIZE 0x0800
#define VRAM_REG_ADDR 0x2000
#define VRAM_REG_MIRROR_SIZE 8

struct memory {
	uint8_t memory[MEM_SIZE];
	struct controller *controller;
	struct ppu *ppu;
};

struct memory *MEM_init()
{
	struct memory *mem = malloc(sizeof(struct memory));

	/* clear the allocated memory */
	uint8_t *mem_ptr = NULL;
	for(mem_ptr = mem->memory; mem_ptr < mem->memory + MEM_SIZE; mem_ptr++)
	{
		*mem_ptr = 0;
	}
	
	mem->controller = NULL;
	mem->ppu = NULL;

	return mem;
}

void MEM_attach_controller(struct memory *mem, struct controller *controller)
{
	mem->controller = controller;
}

void MEM_attach_ppu(struct memory *mem, struct ppu *ppu)
{
	mem->ppu = ppu;
}

void MEM_delete(struct memory **mem)
{
	(*mem)->controller = NULL;
	(*mem)->ppu = NULL;
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

	if ((addr == MEM_CONTROLLER_REG_ADDR) && (mem->controller != NULL)) {
		// special case for reading the address to which the controller is
		// attached
		val = CONTROLLER_read(mem->controller);
	} else if ((addr >= VRAM_REG_ADDR) && (addr < IO_REG_ADDR) && (mem->ppu != NULL)) {
		// Special case for when the PPU is attached.  Reducing the
		// address to (base address + 8) should bypass the mirroring
		// altogether.
		uint16_t base_addr = (addr % VRAM_REG_MIRROR_SIZE) + VRAM_REG_ADDR;
		val = PPU_read_register(mem->ppu, base_addr);
	} else {
		val = mem->memory[addr];
	}
#ifdef DEBUG_MEM
	(void)printf("Read data %#x from address %#x\n", val, addr);
#endif
	return val;
}

void MEM_write(struct memory *mem, const uint16_t addr, const uint8_t val)
{
#ifdef DEBUG_MEM
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

		// If the PPU is attached, update its registers and bypass the
		// mirrored memory altogether.
		if (mem->ppu != NULL) {
			PPU_write_register(mem->ppu, base_addr, val);
		} else {
			write_mirrored_ppu_registers(mem, base_addr, val);
		}
	} 
	/* all other writes */
	else
	{
		mem->memory[addr] = val;

		// Writes to a controller
		if (addr == MEM_CONTROLLER_REG_ADDR && mem->controller != NULL) {
			CONTROLLER_write(mem->controller, val);
		}
	}
}

void MEM_load_trainer(struct memory *mem, FILE *nes_file)
{
	uint8_t *mem_ptr = mem->memory + 0x7000;
	uint8_t data;

	int i = 0;
	while(i < 512) {
		(void)fread(&data, sizeof(uint8_t), 1, nes_file);
		*mem_ptr = data;
		mem_ptr++;
		i++;
	}
}

void MEM_load_rom(struct memory *mem, uint8_t num_banks, FILE *nes_file)
{
	uint8_t data;
	uint32_t mem_addr;
	uint32_t mem_end;

	(void)printf("Loading %d ROM banks\n", num_banks);

	// This is only correct for 1 or 2 memory banks.
	if (num_banks == 2) {
		mem_addr = MEM_ROM_LOW_BANK_ADDR;
	} else {
		mem_addr = MEM_ROM_HIGH_BANK_ADDR;
	}

	mem_end = mem_addr + (num_banks * 0x4000) - 1;
	while (mem_addr <= mem_end) {
		(void)fread(&data, sizeof(uint8_t), 1, nes_file);
		MEM_write(mem, mem_addr, data);
		mem_addr++;
	}
}

void MEM_print_test_status(struct memory *mem)
{
	uint8_t code = mem->memory[0x6000];
	uint8_t *a = &(mem->memory[0x6004]);
	(void)printf("%#x: ", code);
	while(*a != 0) {
		(void)printf("%c", *a);
		a++;
	}
}
