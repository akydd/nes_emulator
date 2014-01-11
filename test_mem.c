/*
 * =====================================================================================
 *
 *       Filename:  test_mem.c
 *
 *    Description:  Tests for memory
 *
 *        Version:  1.0
 *        Created:  13-07-29 10:51:42 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alan Kydd (), akydd@ualberta.net
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <stdio.h>

#include "memory.c"


#define mu_assert(message, test) do { if (!(test)) return message; } while (0)
#define mu_run_test(test) do { char *message = test(); tests_run++; \
	if (message) return message; } while (0)

int tests_run = 0;

struct memory *memory;

static char *test_MEM_init()
{
	memory = MEM_init();
	mu_assert("memory is NULL!", memory != NULL);
	MEM_delete(&memory);
	mu_assert("memory is not NULL!", memory == NULL);

	return 0;
}

static char *test_MEM_write_non_mirrored()
{
	memory = MEM_init();

	MEM_write(memory, 0x6000, 123);
	MEM_write(memory, 0x7FFF, 99);

	mu_assert("Wrong value at 0x6000", memory->memory[0x6000] == 123);
	mu_assert("Wrong value at 0x7FFF", memory->memory[0x7FFF] == 99);

	MEM_delete(&memory);
	return 0;
}

static char *test_MEM_write_mirrored()
{
	memory = MEM_init();

	MEM_write(memory, 0x0200, 123);

	mu_assert("No mirrow at 0x0200 + 1*0x0800", memory->memory[0x0200 + 0x0800] == 123);
	mu_assert("No mirrow at 0x0200 + 2*0x0800", memory->memory[0x0200 + 2*0x0800] == 123);
	mu_assert("No mirrow at 0x0200 + 3*0x0800", memory->memory[0x0200 + 3*0x0800] == 123);

	MEM_delete(&memory);
	return 0;
}

static char *test_VRAM_registers_are_mirrored()
{
	memory = MEM_init();

	/* write to all PPU registers that are mapped into main memory */
	MEM_write(memory, VRAM_REG_ADDR + 0, 8);
	MEM_write(memory, VRAM_REG_ADDR + 1, 7);
	MEM_write(memory, VRAM_REG_ADDR + 2, 6);
	MEM_write(memory, VRAM_REG_ADDR + 3, 5);
	MEM_write(memory, VRAM_REG_ADDR + 4, 4);
	MEM_write(memory, VRAM_REG_ADDR + 5, 3);
	MEM_write(memory, VRAM_REG_ADDR + 6, 2);
	MEM_write(memory, VRAM_REG_ADDR + 7, 1);

	/* Ensure writes are correct and are mapped another 1023 times */
	int i;
	for(i = 0; i < 1024; i++) {
		mu_assert("0x2000 not mirrored!", memory->memory[VRAM_REG_ADDR + 0 + i * VRAM_REG_MIRROR_SIZE] == 8);
		mu_assert("0x2001 not mirrored!", memory->memory[VRAM_REG_ADDR + 1 + i * VRAM_REG_MIRROR_SIZE] == 7);
		mu_assert("0x2002 not mirrored!", memory->memory[VRAM_REG_ADDR + 2 + i * VRAM_REG_MIRROR_SIZE] == 6);
		/* 0x2003 will be incremented due to write to 0x2004 */
		mu_assert("0x2003 not mirrored!", memory->memory[VRAM_REG_ADDR + 3 + i * VRAM_REG_MIRROR_SIZE] == 6);
		mu_assert("0x2004 not mirrored!", memory->memory[VRAM_REG_ADDR + 4 + i * VRAM_REG_MIRROR_SIZE] == 4);
		mu_assert("0x2005 not mirrored!", memory->memory[VRAM_REG_ADDR + 5 + i * VRAM_REG_MIRROR_SIZE] == 3);
		/* 0x2006 will be incremented due to 0x2000:2 == 0 and previous
		 * write to ox2007 */
		mu_assert("0x2006 not mirrored!", memory->memory[VRAM_REG_ADDR + 6 + i * VRAM_REG_MIRROR_SIZE] == 3);
		mu_assert("0x2007 not mirrored!", memory->memory[VRAM_REG_ADDR + 7 + i * VRAM_REG_MIRROR_SIZE] == 1);
	}

	MEM_delete(&memory);
	return 0;
}

static char *test_write_to_PPU_OAMDATA_REG_increments_PPU_OAMADDR_REG()
{
	memory = MEM_init();

	MEM_write(memory,MEM_PPU_OAMADDR_REG_ADDR, 0); 
	
	MEM_write(memory,MEM_PPU_OAMDATA_REG_ADDR, 0); 
	mu_assert("PPU_OAMADDR_REG not incremented!", memory->memory[MEM_PPU_OAMADDR_REG_ADDR] == 1);

	MEM_write(memory,MEM_PPU_OAMDATA_REG_ADDR, 0); 
	mu_assert("PPU_OAMADDR_REG not incremented!", memory->memory[MEM_PPU_OAMADDR_REG_ADDR] == 2);

	MEM_delete(&memory);
	return 0;
}

static char *test_write_to_PPU_DATA_increments_PPU_ADDR_by_1()
{
	memory = MEM_init();

	MEM_write(memory, MEM_PPU_STATUS_REG_ADDR, 0);
	MEM_write(memory, MEM_PPU_ADDR_REG_ADDR, 0);

	MEM_write(memory, MEM_PPU_DATA_REG_ADDR, 0);
	mu_assert("PPU_ADDR_REG not incremented by 1", memory->memory[MEM_PPU_ADDR_REG_ADDR] == 1);

	MEM_write(memory, MEM_PPU_DATA_REG_ADDR, 0);
	mu_assert("PPU_ADDR_REG not incremented by 1", memory->memory[MEM_PPU_ADDR_REG_ADDR] == 2);

	MEM_delete(&memory);
	return 0;
}

static char *test_write_to_PPU_DATA_increments_PPU_ADDR_by_32()
{
	memory = MEM_init();

	MEM_write(memory, MEM_PPU_STATUS_REG_ADDR, 4);
	MEM_write(memory, MEM_PPU_ADDR_REG_ADDR, 0);

	MEM_write(memory, MEM_PPU_DATA_REG_ADDR, 0);
	mu_assert("PPU_ADDR_REG not incremented by 32", memory->memory[MEM_PPU_ADDR_REG_ADDR] == 32);

	MEM_write(memory, MEM_PPU_DATA_REG_ADDR, 0);
	mu_assert("PPU_ADDR_REG not incremented by 32", memory->memory[MEM_PPU_ADDR_REG_ADDR] == 64);

	MEM_delete(&memory);
	return 0;
}

static char *all_tests()
{
	mu_run_test(test_MEM_init);
	mu_run_test(test_MEM_write_mirrored);
	mu_run_test(test_MEM_write_non_mirrored);
	mu_run_test(test_VRAM_registers_are_mirrored);
	mu_run_test(test_write_to_PPU_OAMDATA_REG_increments_PPU_OAMADDR_REG);
	mu_run_test(test_write_to_PPU_DATA_increments_PPU_ADDR_by_1);
	mu_run_test(test_write_to_PPU_DATA_increments_PPU_ADDR_by_32);

	return 0;
}

int main()
{
	char *result = all_tests();
	if (result != 0) {
		(void) printf("%s\n", result);
	} else {
		(void) printf("All tests passed!\n");
	}
	(void) printf("Tests run: %d\n", tests_run);

	return result != 0;
}
