/*
 * =============================================================================
 *
 *       Filename:  test.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  13-03-28 11:15:21 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alan Kydd (), akydd@ualberta.net
 *   Organization:  
 *
 * =============================================================================
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "cpu.h"
#include "instructions.h"

#define mu_assert(message, test) do { if (!(test)) return message; } while (0)
#define mu_run_test(test) do { char *message = test(); tests_run++; \
	if (message) return message; } while (0)

int tests_run = 0;

struct cpu cpu;

static char *test_cpu_init()
{
	init(&cpu);

	/* check that mem is all cleared */
	int clear = 1;
	uint8_t *mem = NULL;
	for(mem = cpu.memory; mem < cpu.memory + MEM_SIZE; mem++)
	{
		if (*mem != 0)
		{
			clear = 0;
			break;
		}
	}
	mu_assert("Mem not cleared", clear == 1);

	mu_assert("PC not init", cpu.PC == 0);
	mu_assert("S not init", cpu.S == 511);
	mu_assert("A not init", cpu.A == 0);
	mu_assert("X not init", cpu.X == 0);
	mu_assert("Y not init", cpu.Y == 0);
	mu_assert("P not init", cpu.P == 0);
	return 0;
}

static char *test_brk()
{
	init(&cpu);
	/* setup some state */
	cpu.PC = 0x07D0 - 2; /* 1998 */
	cpu.P = 5;

	brk(&cpu);

	mu_assert("BRK - PC not set", cpu.PC == 0x07D0);
	mu_assert("BRK - S not set", cpu.S == STACK_START - 3);
	mu_assert("BRK - flag not set", (cpu.P & B_FLAG) == B_FLAG);
	mu_assert("BRK - high byte not on S", cpu.memory[STACK_START] == 0x07);
	mu_assert("BRK - low byte not on S", cpu.memory[STACK_START-1] == 0xD0);
	mu_assert("BRK - flags not on S", cpu.memory[STACK_START-2] == (5|B_FLAG));

	return 0;
}

static char *test_ora_ind_x()
{
	init(&cpu);

	cpu.X = 6;
	cpu.A = 5;
	cpu.memory[1] = 0xB4;
	cpu.memory[0xBA] = 0x12;
	cpu.memory[0xBB] = 0x0E;
	cpu.memory[0x0E12] = 10;

	ora_ind_x(&cpu);

	mu_assert("ORA - Wrong val for A", cpu.A == (5|10));
	mu_assert("ORA - PC not incremented", cpu.PC == 2);

	return 0;
}

static char *test_ora_zero_pg()
{
	init(&cpu);

	cpu.A = 5;
	cpu.memory[1] = 0xB4;
	cpu.memory[0xB4] = 10;

	ora_zero_pg(&cpu);

	mu_assert("ORA - Wrong val for A", cpu.A == (5|10));
	mu_assert("ORA - PC not incremented", cpu.PC == 2);

	return 0;
}

static char *test_asl_zero_pg()
{
	init(&cpu);

	cpu.memory[1] = 0xB4;
	cpu.memory[0xB4] = 10;

	asl_zero_pg(&cpu);

	mu_assert("ASL - fail.", cpu.memory[0xB4] == 20);
	mu_assert("ASL - PC not incremented", cpu.PC == 2);

	return 0;
}

static char *test_php()
{
	init(&cpu);

	cpu.P = 17;

	php(&cpu);

	mu_assert("PHP - PC not incr", cpu.PC == 1);
	mu_assert("PHP - S not incr", cpu.S == STACK_START - 1);
	mu_assert("PHP - flags not set", cpu.memory[STACK_START] == 17);

	return 0;
}

static char *test_ora_imm()
{
	init(&cpu);

	cpu.A = 5;
	cpu.memory[1] = 10;

	ora_imm(&cpu);

	mu_assert("ORA - Wrong val for A", cpu.A == (5|10));
	mu_assert("ORA - PC not incremented", cpu.PC == 2);

	return 0;
}

static char *test_asl_A()
{
	init(&cpu);

	cpu.A = 10;

	asl_A(&cpu);

	mu_assert("ASL - fail.", cpu.A == 20);
	mu_assert("ASL - PC not incremented", cpu.PC == 1);

	return 0;
}

static char *test_ora_a()
{
	init(&cpu);

	cpu.A = 5;
	cpu.memory[1] = 0x12; /* low byte is stored first */
	cpu.memory[2] = 0x0E;
	cpu.memory[0x0E12] = 10;

	ora_a(&cpu);

	mu_assert("ora_a- fail.", cpu.A == (5|10));
	mu_assert("ora_a - PC not incremented", cpu.PC == 3);

	return 0;
}

static char *test_asl_a()
{
	init(&cpu);

	cpu.memory[1] = 0x12; /* low byte is stored first */
	cpu.memory[2] = 0x0E;
	cpu.memory[0x0E12] = 10;

	asl_a(&cpu);

	mu_assert("asl_a - fail.", cpu.memory[0x0E12] == 20);
	mu_assert("asl_a - PC not incremented", cpu.PC == 3);

	return 0;
}

static char *test_bpl_r_clear_neg()
{
	init(&cpu);

	cpu.PC = 10;
	cpu.memory[11] = 0xF9; /* -7, when signed */

	bpl_r(&cpu);

	mu_assert("bpl_r - fail when clear", cpu.PC == 5);

	return 0;
}

static char *test_bpl_r_clear_pos()
{
	init(&cpu);

	cpu.PC = 10;
	cpu.memory[11] = 0x07; /* 7, when signed */

	bpl_r(&cpu);

	mu_assert("bpl_r - fail when clear", cpu.PC == 19);

	return 0;
}

static char *test_bpl_r_set()
{
	init(&cpu);

	cpu.PC = 10;
	cpu.P |= N_FLAG;
	cpu.memory[11] = 0xF9; /* -7, when signed */

	bpl_r(&cpu);

	mu_assert("bpl_r - fail when set", cpu.PC == 12);

	return 0;
}

static char *test_ora_ind_y()
{
	init(&cpu);

	cpu.Y = 6;
	cpu.A = 5;
	cpu.memory[1] = 0xB4;
	cpu.memory[0xB4] = 0x12;
	cpu.memory[0xB5] = 0x0E;
	cpu.memory[0x0E12 + 6] = 10;

	ora_ind_y(&cpu);

	mu_assert("ora_ind_y - Wrong val for A", cpu.A == (5|10));
	mu_assert("ora_ind_y - PC not incremented", cpu.PC == 2);

	return 0;
}

static char *test_ora_zero_pg_x()
{
	init(&cpu);

	cpu.X = 6;
	cpu.A = 5;
	cpu.memory[1] = 0xB4;
	cpu.memory[0xB4 + cpu.X] = 10;

	ora_zero_pg_x(&cpu);

	mu_assert("ora_aero_pg_x - Wrong val for A", cpu.A == (5|10));
	mu_assert("ora_zero_pg_x - PC not incremented", cpu.PC == 2);

	return 0;
}

static char *test_asl_zero_pg_x()
{
	init(&cpu);

	cpu.X = 5;
	cpu.memory[1] = 0xB4;
	cpu.memory[0xB4 + cpu.X] = 10;

	asl_zero_pg_x(&cpu);

	mu_assert("asl_zero_pg_x - fail.", cpu.memory[0xB4 + cpu.X] == 20);
	mu_assert("asl_zero_pg_x - PC not incremented", cpu.PC == 2);

	return 0;
}

static char *test_clc()
{
	init(&cpu);
	cpu.P |= C_FLAG;

	clc(&cpu);

	mu_assert("clc - fail.", (cpu.P & C_FLAG) == 0);
	mu_assert("clc - PC not incremented", cpu.PC == 1);

	return 0;
}

static char *test_ora_abs_y()
{
	init(&cpu);

	cpu.Y = 6;
	cpu.A = 5;
	cpu.memory[1] = 0xB4;
	cpu.memory[2] = 0x0E;
	cpu.memory[0x0EB4 + cpu.Y] = 10;

	ora_abs_y(&cpu);

	mu_assert("ora_abs_y - Wrong val for A", cpu.A == (5|10));
	mu_assert("ora_abs_y - PC not incremented", cpu.PC == 3);

	return 0;
}

static char *test_ora_abs_x()
{
	init(&cpu);

	cpu.X = 6;
	cpu.A = 5;
	cpu.memory[1] = 0xB4;
	cpu.memory[2] = 0x0E;
	cpu.memory[0x0EB4 + cpu.X] = 10;

	ora_abs_x(&cpu);

	mu_assert("ora_abs_x - Wrong val for A", cpu.A == (5|10));
	mu_assert("ora_abs_x - PC not incremented", cpu.PC == 3);

	return 0;
}

static char *all_tests()
{
	/*
	 * Checklist for tested addressing modes:
	 *
	 * Mode		Tested
	 * -------------------
	 * ind_x	y
	 * zero_pg	y
	 * implied	y
	 * imm		y
	 * A		y
	 * a		y
	 * r		y
	 * ind_y	y
	 * zero_pg_x	y
	 * abs_y	y
	 * abs_x	y
	 * ind		n
	 */
	mu_run_test(test_cpu_init);
	mu_run_test(test_brk);
	mu_run_test(test_ora_ind_x);
	mu_run_test(test_ora_zero_pg);
	mu_run_test(test_asl_zero_pg);
	mu_run_test(test_php);
	mu_run_test(test_ora_imm);
	mu_run_test(test_asl_A);
	mu_run_test(test_ora_a);
	mu_run_test(test_asl_a);
	mu_run_test(test_bpl_r_clear_neg);
	mu_run_test(test_bpl_r_clear_pos);
	mu_run_test(test_bpl_r_set);
	mu_run_test(test_ora_ind_y);
	mu_run_test(test_ora_zero_pg_x);
	mu_run_test(test_asl_zero_pg_x);
	mu_run_test(test_clc);
	mu_run_test(test_ora_abs_y);
	mu_run_test(test_ora_abs_x);

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
