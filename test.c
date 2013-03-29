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
	brk(&cpu);

	return 0;
}

static char *all_tests() {
	mu_run_test(test_cpu_init);
	mu_run_test(test_brk);
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
