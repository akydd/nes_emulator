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
#include "memory.h"

#define mu_assert(message, test) do { if (!(test)) return message; } while (0)
#define mu_run_test(test) do { char *message = test(); tests_run++; \
	if (message) return message; } while (0)

int tests_run = 0;

struct cpu *cpu;
struct memory *memory;

static char *test_cpu_init()
{
	memory = MEM_init();
	cpu = CPU_init(memory);

	mu_assert("PC not init", cpu->PC == 0);
	mu_assert("S not init", cpu->S == MEM_STACK_START);
	mu_assert("A not init", cpu->A == 0);
	mu_assert("X not init", cpu->X == 0);
	mu_assert("Y not init", cpu->Y == 0);
	mu_assert("P not init", cpu->P == 0);

	CPU_delete(&cpu);
	return 0;
}

static char *test_pc_at_reset_vector_on_init()
{
	memory = MEM_init();

	/* Put something into the reset vector */
	MEM_write(memory, MEM_RESET_VECTOR, 0x0F);
	MEM_write(memory, MEM_RESET_VECTOR + 1, 0x33);

	cpu = CPU_init(memory);

	mu_assert("PC not set to addr in reset vector", cpu->PC == 0x330F);

	CPU_delete(&cpu);
	return 0;
}

static char *test_push8_stack()
{
	memory = MEM_init();
	cpu = CPU_init(memory);

	CPU_push8_stack(cpu, 10);

	mu_assert("push8_stack - S not moved", cpu->S == MEM_STACK_START - 1);
	mu_assert("push8_stack - not pushed", MEM_read(cpu->mem, cpu->S + 1) == 10);

	CPU_delete(&cpu);
	return 0;
}

static char *test_push16_stack()
{
	memory = MEM_init();
	cpu = CPU_init(memory);

	CPU_push16_stack(cpu, 0x0EF4);

	mu_assert("push16_stack - S not moved", cpu->S == MEM_STACK_START - 2);
	mu_assert("push16_stack - low not pushed", MEM_read(cpu->mem, cpu->S+1) == 0xF4);
	mu_assert("push16_stack - high not pushed", MEM_read(cpu->mem, cpu->S+2) == 0x0E);

	CPU_delete(&cpu);
	return 0;
}

static char *test_pop8_mem()
{
	memory = MEM_init();
	cpu = CPU_init(memory);

	MEM_write(cpu->mem, 0, 0xF4);

	uint8_t val = CPU_pop8_mem(cpu);

	mu_assert("pop8_mem - PC not moved", cpu->PC == 1);
	mu_assert("pop8_mem - wrong value", val == 0xF4);

	CPU_delete(&cpu);
	return 0;
}

static char *test_pop16_mem()
{
	memory = MEM_init();
	cpu = CPU_init(memory);

	MEM_write(cpu->mem, 0, 0xF4);
	MEM_write(cpu->mem, 1, 0x0E);

	uint16_t val = CPU_pop16_mem(cpu);

	mu_assert("pop16_mem - PC not moved", cpu->PC == 2);
	mu_assert("pop16_mem - wrong value", val == 0x0EF4);

	CPU_delete(&cpu);
	return 0;
}

static char *all_tests()
{
	mu_run_test(test_cpu_init);
	mu_run_test(test_pc_at_reset_vector_on_init);
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
