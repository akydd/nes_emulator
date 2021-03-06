/*
 * =====================================================================================
 *
 *       Filename:  test_ppu_mem.c
 *
 *    Description:  Tests for PPU memory
 *
 *        Version:  1.0
 *        Created:  14-01-15 21:14:42
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alan Kydd (), akydd@ualberta.net
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <stdio.h>

#include "ppu_memory.c"


#define mu_assert(message, test) do { if (!(test)) return message; } while (0)
#define mu_run_test(test) do { char *message = test(); tests_run++; \
	if (message) return message; } while (0)

int tests_run = 0;

struct ppu_memory *memory;

static char *test_PPU_MEM_init()
{
	memory = PPU_MEM_init();
	mu_assert("memory is NULL!", memory != NULL);
	PPU_MEM_delete(&memory);
	mu_assert("memory is not NULL!", memory == NULL);

	return 0;
}

static char *test_PPU_MEM_write_mirrored_palette_0x3F00()
{
	memory = PPU_MEM_init();

	PPU_MEM_write(memory, 0x3F00, 123);
	PPU_MEM_write(memory, 0x3F01, 234);

	mu_assert("0x3F00 not set", memory->memory[0x3F00] == 123);
	mu_assert("0x3F00 not mirrored", memory->memory[0x3F00 + 1 * 32] == 123);
	mu_assert("0x3F00 not mirrored", memory->memory[0x3F00 + 2 * 32] == 123);
	mu_assert("0x3F00 not mirrored", memory->memory[0x3F00 + 3 * 32] == 123);
	mu_assert("0x3F00 not mirrored", memory->memory[0x3F00 + 4 * 32] == 123);

	mu_assert("0x3F01 not set", memory->memory[0x3F01] == 234);
	mu_assert("0x3F01 not mirrored", memory->memory[0x3F01 + 1 * 32] == 234);
	mu_assert("0x3F01 not mirrored", memory->memory[0x3F01 + 2 * 32] == 234);
	mu_assert("0x3F01 not mirrored", memory->memory[0x3F01 + 3 * 32] == 234);
	mu_assert("0x3F01 not mirrored", memory->memory[0x3F01 + 4 * 32] == 234);

	PPU_MEM_delete(&memory);
	return 0;
}

static char *test_PPU_MEM_write_mirrored_palette_0x3F00_to_0x3F10()
{
	memory = PPU_MEM_init();

	PPU_MEM_write(memory, 0x3F00, 123);

	mu_assert("0x3F10 not set", memory->memory[0x3F10 + 0 * 32] == 123);
	mu_assert("0x3F10 not mirrored", memory->memory[0x3F10 + 1 * 32] == 123);
	mu_assert("0x3F10 not mirrored", memory->memory[0x3F10 + 2 * 32] == 123);
	mu_assert("0x3F10 not mirrored", memory->memory[0x3F10 + 3 * 32] == 123);
	mu_assert("0x3F10 not mirrored", memory->memory[0x3F10 + 4 * 32] == 123);

	PPU_MEM_delete(&memory);
	return 0;
}

static char *test_PPU_MEM_write_mirrored_palette_0x3F04_to_0x3F14()
{
	memory = PPU_MEM_init();

	PPU_MEM_write(memory, 0x3F04, 123);

	mu_assert("0x3F14 not set", memory->memory[0x3F14 + 0 * 32] == 123);
	mu_assert("0x3F14 not mirrored", memory->memory[0x3F14 + 1 * 32] == 123);
	mu_assert("0x3F14 not mirrored", memory->memory[0x3F14 + 2 * 32] == 123);
	mu_assert("0x3F14 not mirrored", memory->memory[0x3F14 + 3 * 32] == 123);
	mu_assert("0x3F14 not mirrored", memory->memory[0x3F14 + 4 * 32] == 123);

	PPU_MEM_delete(&memory);
	return 0;
}

static char *test_PPU_MEM_write_mirrored_palette_0x3F08_to_0x3F18()
{
	memory = PPU_MEM_init();

	PPU_MEM_write(memory, 0x3F08, 123);

	mu_assert("0x3F18 not set", memory->memory[0x3F18 + 0 * 32] == 123);
	mu_assert("0x3F18 not mirrored", memory->memory[0x3F18 + 1 * 32] == 123);
	mu_assert("0x3F18 not mirrored", memory->memory[0x3F18 + 2 * 32] == 123);
	mu_assert("0x3F18 not mirrored", memory->memory[0x3F18 + 3 * 32] == 123);
	mu_assert("0x3F18 not mirrored", memory->memory[0x3F18 + 4 * 32] == 123);

	PPU_MEM_delete(&memory);
	return 0;
}

static char *test_PPU_MEM_write_mirrored_palette_0x3F0C_to_0x3F1C()
{
	memory = PPU_MEM_init();

	PPU_MEM_write(memory, 0x3F1C, 123);

	mu_assert("0x3F1C not set", memory->memory[0x3F1C + 0 * 32] == 123);
	mu_assert("0x3F1C not mirrored", memory->memory[0x3F1C + 1 * 32] == 123);
	mu_assert("0x3F1C not mirrored", memory->memory[0x3F1C + 2 * 32] == 123);
	mu_assert("0x3F1C not mirrored", memory->memory[0x3F1C + 3 * 32] == 123);
	mu_assert("0x3F1C not mirrored", memory->memory[0x3F1C + 4 * 32] == 123);

	PPU_MEM_delete(&memory);
	return 0;
}

static char *test_PPU_MEM_nametable0_horizontal_mirroring()
{
	memory = PPU_MEM_init();

	PPU_MEM_set_mirroring(memory, 0);
	PPU_MEM_write(memory, 0x2000, 123);

	mu_assert("Nametable 0 not set", memory->memory[0x2000] == 123);
	mu_assert("Nametable 0 not mirrored in nametable 1", memory->memory[0x2400] == 123);
	mu_assert("Nametable 0 not mirrored at 0x3000", memory->memory[0x3000] == 123);
	mu_assert("Nametable 0 not mirrored at 0x3400", memory->memory[0x3400] == 123);

	PPU_MEM_delete(&memory);
	return 0;
}

static char *test_PPU_MEM_attributetable0_horizontal_mirroring()
{
	memory = PPU_MEM_init();

	PPU_MEM_set_mirroring(memory, 0);
	PPU_MEM_write(memory, 0x23C0, 123);

	mu_assert("Attrib table 0 not set", memory->memory[0x23C0] == 123);
	mu_assert("Attrib table 0 not mirrored in Attrib table 1", memory->memory[0x27C0] == 123);
	mu_assert("Attrib table 0 not mirrored 0x33C0", memory->memory[0x33C0] == 123);
	mu_assert("Attrib table 0 not mirrored 0x37C0", memory->memory[0x37C0] == 123);

	PPU_MEM_delete(&memory);
	return 0;
}

static char *test_PPU_MEM_nametable1_horizontal_mirroring()
{
	memory = PPU_MEM_init();

	PPU_MEM_set_mirroring(memory, 0);
	PPU_MEM_write(memory, 0x2400, 123);

	mu_assert("Nametable 1 not set", memory->memory[0x2400] == 123);
	mu_assert("Nametable 1 not mirrored in nametable 0", memory->memory[0x2000] == 123);
	mu_assert("Nametable 1 not mirrored at 0x3400", memory->memory[0x3400] == 123);
	mu_assert("Nametable 1 not mirrored at 0x3000", memory->memory[0x3000] == 123);

	PPU_MEM_delete(&memory);
	return 0;
}

static char *test_PPU_MEM_attributetable1_horizontal_mirroring()
{
	memory = PPU_MEM_init();

	PPU_MEM_set_mirroring(memory, 0);
	PPU_MEM_write(memory, 0x27C0, 123);

	mu_assert("Attrib table 1 not set", memory->memory[0x27C0] == 123);
	mu_assert("Attrib table 1 not mirrored in Attrib table 0", memory->memory[0x23C0] == 123);
	mu_assert("Attrib table 1 not mirrored at 0x37C0", memory->memory[0x37C0] == 123);
	mu_assert("Attrib table 1 not mirrored at 0x33C0", memory->memory[0x33C0] == 123);

	PPU_MEM_delete(&memory);
	return 0;
}

static char *test_PPU_MEM_nametable2_horizontal_mirroring()
{
	memory = PPU_MEM_init();

	PPU_MEM_set_mirroring(memory, 0);
	PPU_MEM_write(memory, 0x2800, 123);

	mu_assert("Nametable 2 not set", memory->memory[0x2800] == 123);
	mu_assert("Nametable 2 not mirrored in nametable 3", memory->memory[0x2C00] == 123);
	mu_assert("Nametable 2 not mirrored 0x3800", memory->memory[0x3800] == 123);
	mu_assert("Nametable 2 not mirrored 0x3C00", memory->memory[0x3C00] == 123);

	PPU_MEM_delete(&memory);
	return 0;
}

static char *test_PPU_MEM_attributetable2_horizontal_mirroring()
{
	memory = PPU_MEM_init();

	PPU_MEM_set_mirroring(memory, 0);
	PPU_MEM_write(memory, 0x2BC0, 123);

	mu_assert("Attrib table 2 not set", memory->memory[0x2BC0] == 123);
	mu_assert("Attrib table 2 not mirrored in Attrib table 3", memory->memory[0x2FC0] == 123);
	mu_assert("Attrib table 2 not mirrored at 0x3BC0", memory->memory[0x3BC0] == 123);
	mu_assert("Attrib table 2 mirrored at 0x3FC0", memory->memory[0x3FC0] != 123);

	PPU_MEM_delete(&memory);
	return 0;
}

static char *test_PPU_MEM_nametable3_horizontal_mirroring()
{
	memory = PPU_MEM_init();

	PPU_MEM_set_mirroring(memory, 0);
	PPU_MEM_write(memory, 0x2C00, 123);

	mu_assert("Nametable 3 not set", memory->memory[0x2C00] == 123);
	mu_assert("Nametable 3 not mirrored in nametable 2", memory->memory[0x2800] == 123);
	mu_assert("Nametable 3 not mirrored at 0x3C00", memory->memory[0x3C00] == 123);
	mu_assert("Nametable 3 not mirrored at 0x3800", memory->memory[0x3800] == 123);

	PPU_MEM_delete(&memory);
	return 0;
}

static char *test_PPU_MEM_attributetable3_horizontal_mirroring()
{
	memory = PPU_MEM_init();

	PPU_MEM_set_mirroring(memory, 0);
	PPU_MEM_write(memory, 0x2FC0, 123);

	mu_assert("Attrib table 3 not set", memory->memory[0x2FC0] == 123);
	mu_assert("Attrib table 3 not mirrored in Attrib table 2", memory->memory[0x2BC0] == 123);
	mu_assert("Attrib table 3 not mirrored at 0x3BC0", memory->memory[0x3BC0] == 123);
	mu_assert("Attrib table 3 mirrored at 0x3FC0", memory->memory[0x3FC0] != 123);

	PPU_MEM_delete(&memory);
	return 0;
}

static char *test_PPU_MEM_nametable0_vertical_mirroring()
{
	memory = PPU_MEM_init();

	PPU_MEM_set_mirroring(memory, 1);
	PPU_MEM_write(memory, 0x2000, 123);

	mu_assert("Nametable 0 not set", memory->memory[0x2000] == 123);
	mu_assert("Nametable 0 not v-mirrored in nametable 2", memory->memory[0x2800] == 123);
	mu_assert("Nametable 0 not v-mirrored at 0x3000", memory->memory[0x3000] == 123);
	mu_assert("Nametable 0 not v-mirrored at 0x3800", memory->memory[0x3800] == 123);

	PPU_MEM_delete(&memory);
	return 0;
}

static char *test_PPU_MEM_attributetable0_vertical_mirroring()
{
	memory = PPU_MEM_init();

	PPU_MEM_set_mirroring(memory, 1);
	PPU_MEM_write(memory, 0x23C0, 123);

	mu_assert("Attrib table 0 not set", memory->memory[0x23C0] == 123);
	mu_assert("Attrib table 0 not v-mirrored in Attrib table 2", memory->memory[0x2BC0] == 123);
	mu_assert("Attrib table 0 not v-mirrored at 0x33C0", memory->memory[0x33C0] == 123);
	mu_assert("Attrib table 0 not v-mirrored at 0x3BC0", memory->memory[0x3BC0] == 123);

	PPU_MEM_delete(&memory);
	return 0;
}

static char *test_PPU_MEM_nametable1_vertical_mirroring()
{
	memory = PPU_MEM_init();

	PPU_MEM_set_mirroring(memory, 1);
	PPU_MEM_write(memory, 0x2400, 123);

	mu_assert("Nametable 1 not set", memory->memory[0x2400] == 123);
	mu_assert("Nametable 1 not v-mirrored in nametable 3", memory->memory[0x2C00] == 123);
	mu_assert("Nametable 1 not v-mirrored at 0x3400", memory->memory[0x3400] == 123);
	mu_assert("Nametable 1 not v-mirrored at 0x3C00", memory->memory[0x3C00] == 123);

	PPU_MEM_delete(&memory);
	return 0;
}

static char *test_PPU_MEM_attributetable1_vertical_mirroring()
{
	memory = PPU_MEM_init();

	PPU_MEM_set_mirroring(memory, 1);
	PPU_MEM_write(memory, 0x27C0, 123);

	mu_assert("Attrib table 1 not set", memory->memory[0x27C0] == 123);
	mu_assert("Attrib table 1 not v-mirrored in Attrib table 3", memory->memory[0x2FC0] == 123);
	mu_assert("Attrib table 1 not v-mirrored at 0x37C0", memory->memory[0x37C0] == 123);
	mu_assert("Attrib table 1 v-mirrored at 0x3FC0", memory->memory[0x3FC0] != 123);

	PPU_MEM_delete(&memory);
	return 0;
}

static char *test_PPU_MEM_nametable2_vertical_mirroring()
{
	memory = PPU_MEM_init();

	PPU_MEM_set_mirroring(memory, 1);
	PPU_MEM_write(memory, 0x2800, 123);

	mu_assert("Nametable 2 not set", memory->memory[0x2800] == 123);
	mu_assert("Nametable 2 not v-mirrored in nametable 0", memory->memory[0x2000] == 123);
	mu_assert("Nametable 2 not v-mirrored at 0x3800", memory->memory[0x3800] == 123);
	mu_assert("Nametable 2 not v-mirrored at 0x3000", memory->memory[0x3000] == 123);

	PPU_MEM_delete(&memory);
	return 0;
}

static char *test_PPU_MEM_attributetable2_vertical_mirroring()
{
	memory = PPU_MEM_init();

	PPU_MEM_set_mirroring(memory, 1);
	PPU_MEM_write(memory, 0x2BC0, 123);

	mu_assert("Attrib table 2 not set", memory->memory[0x2BC0] == 123);
	mu_assert("Attrib table 2 not v-mirrored in Attrib table 0", memory->memory[0x23C0] == 123);
	mu_assert("Attrib table 2 not v-mirrored at 0x3BC0", memory->memory[0x3BC0] == 123);
	mu_assert("Attrib table 2 not v-mirrored at 0x33C0", memory->memory[0x33C0] == 123);

	PPU_MEM_delete(&memory);
	return 0;
}

static char *test_PPU_MEM_nametable3_vertical_mirroring()
{
	memory = PPU_MEM_init();

	PPU_MEM_set_mirroring(memory, 1);
	PPU_MEM_write(memory, 0x2C00, 123);

	mu_assert("Nametable 3 not set", memory->memory[0x2C00] == 123);
	mu_assert("Nametable 3 not v-mirrored in nametable 1", memory->memory[0x2400] == 123);
	mu_assert("Nametable 3 not v-mirrored 0x3C00", memory->memory[0x3C00] == 123);
	mu_assert("Nametable 3 not v-mirrored 0x3400", memory->memory[0x3400] == 123);

	PPU_MEM_delete(&memory);
	return 0;
}

static char *test_PPU_MEM_attributetable3_vertical_mirroring()
{
	memory = PPU_MEM_init();

	PPU_MEM_set_mirroring(memory, 1);
	PPU_MEM_write(memory, 0x2FC0, 123);

	mu_assert("Attrib table 3 not set", memory->memory[0x2FC0] == 123);
	mu_assert("Attrib table 3 not v-mirrored in Attrib table 1", memory->memory[0x27C0] == 123);
	mu_assert("Attrib table 3 v-mirrored at 0x3FC0", memory->memory[0x3FC0] != 123);
	mu_assert("Attrib table 3 not v-mirrored at 0x37C0", memory->memory[0x37C0] == 123);

	PPU_MEM_delete(&memory);
	return 0;
}

static char *test_PPU_MEM_0x2F00_not_mirrored()
{
	memory = PPU_MEM_init();

	PPU_MEM_set_mirroring(memory, 0);
	PPU_MEM_write(memory, 0x2F00, 123);
	mu_assert("0x2F00 should not be h-mirrored to 0x3F00", memory->memory[0x3F00] != 123);


	PPU_MEM_set_mirroring(memory, 1);
	PPU_MEM_write(memory, 0x2F00, 123);
	mu_assert("0x2F00 should not be v-mirrored to 0x3F00", memory->memory[0x3F00] != 123);

	PPU_MEM_delete(&memory);
	return 0;
}

static char *test_PPU_MEM_load_vrom()
{
	return 0;
}

static char *all_tests()
{
	mu_run_test(test_PPU_MEM_init);

	mu_run_test(test_PPU_MEM_write_mirrored_palette_0x3F00);
	mu_run_test(test_PPU_MEM_write_mirrored_palette_0x3F00_to_0x3F10);
	mu_run_test(test_PPU_MEM_write_mirrored_palette_0x3F04_to_0x3F14);
	mu_run_test(test_PPU_MEM_write_mirrored_palette_0x3F08_to_0x3F18);
	mu_run_test(test_PPU_MEM_write_mirrored_palette_0x3F0C_to_0x3F1C);

	mu_run_test(test_PPU_MEM_nametable0_horizontal_mirroring);
	mu_run_test(test_PPU_MEM_nametable1_horizontal_mirroring);
	mu_run_test(test_PPU_MEM_nametable2_horizontal_mirroring);
	mu_run_test(test_PPU_MEM_nametable3_horizontal_mirroring);

	mu_run_test(test_PPU_MEM_attributetable0_horizontal_mirroring);
	mu_run_test(test_PPU_MEM_attributetable1_horizontal_mirroring);
	mu_run_test(test_PPU_MEM_attributetable2_horizontal_mirroring);
	mu_run_test(test_PPU_MEM_attributetable3_horizontal_mirroring);

	mu_run_test(test_PPU_MEM_nametable0_vertical_mirroring);
	mu_run_test(test_PPU_MEM_nametable1_vertical_mirroring);
	mu_run_test(test_PPU_MEM_nametable2_vertical_mirroring);
	mu_run_test(test_PPU_MEM_nametable3_vertical_mirroring);

	mu_run_test(test_PPU_MEM_attributetable0_vertical_mirroring);
	mu_run_test(test_PPU_MEM_attributetable1_vertical_mirroring);
	mu_run_test(test_PPU_MEM_attributetable2_vertical_mirroring);
	mu_run_test(test_PPU_MEM_attributetable3_vertical_mirroring);

	mu_run_test(test_PPU_MEM_0x2F00_not_mirrored);

	//mu_run_test(test_PPU_MEM_load_vrom);

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
