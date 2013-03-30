/*
 * =============================================================================
 *
 *       Filename:  test_cpu.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  13-03-30 01:03:49 PM
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
	mu_assert("S not init", cpu.S == STACK_START);
	mu_assert("A not init", cpu.A == 0);
	mu_assert("X not init", cpu.X == 0);
	mu_assert("Y not init", cpu.Y == 0);
	mu_assert("P not init", cpu.P == 0);
	return 0;
}

static char *test_push8_stack()
{
	init(&cpu);
	push8_stack(10, &cpu);

	mu_assert("push8_stack - S not moved", cpu.S == STACK_START - 1);
	mu_assert("push8_stack - not pushed", cpu.memory[cpu.S + 1] == 10);
	return 0;
}

static char *test_push16_stack()
{
	init(&cpu);
	push16_stack(0x0EF4, &cpu);

	mu_assert("push16_stack - S not moved", cpu.S == STACK_START - 2);
	mu_assert("push16_stack - low not pushed", cpu.memory[cpu.S+1] == 0xF4);
	mu_assert("push16_stack - high not pushed", cpu.memory[cpu.S+2] == 0x0E);
	return 0;
}

static char *test_pop8_mem()
{
	init(&cpu);
	cpu.memory[0] = 0xF4;

	uint8_t val = pop8_mem(&cpu);

	mu_assert("pop8_mem - PC not moved", cpu.PC == 1);
	mu_assert("pop8_mem - wrong value", val == 0xF4);
	return 0;
}

static char *test_pop16_mem()
{
	init(&cpu);
	cpu.memory[0] = 0xF4;
	cpu.memory[1] = 0x0E;

	uint16_t val = pop16_mem(&cpu);

	mu_assert("pop16_mem - PC not moved", cpu.PC == 2);
	mu_assert("pop16_mem - wrong value", val == 0x0EF4);
	return 0;
}

static char *all_tests()
{
	mu_run_test(test_cpu_init);
	mu_run_test(test_push8_stack);
	mu_run_test(test_push16_stack);
	mu_run_test(test_pop8_mem);
	mu_run_test(test_pop16_mem);
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
