/*
 * =============================================================================
 *
 *       Filename:  test_instructions.c
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

#include "memory.h"
#include "cpu.c"

#define mu_assert(message, test) do { if (!(test)) return message; } while (0)
#define mu_run_test(test) do { char *message = test(); tests_run++; \
	if (message) return message; } while (0)

int tests_run = 0;

struct memory *memory;
struct cpu *cpu;

static char *test_brk()
{
	memory = MEM_init();
	cpu = CPU_init(memory);
	/* setup some state */
	cpu->PC = 0x07D0 - 2; /* 1998 */
	cpu->P = 5;
	MEM_write(memory, MEM_BRK_VECTOR, 0x10);
	MEM_write(memory, MEM_BRK_VECTOR + 1, 0x01);

	brk(cpu, memory);

	mu_assert("BRK - PC not correct", cpu->PC == 0x0110);
	mu_assert("BRK - S not set", cpu->S == MEM_STACK_START - 3);
	mu_assert("BRK - flag not set", (cpu->P & B_FLAG) == B_FLAG);
	mu_assert("BRK - high byte not on S", MEM_read(memory, MEM_STACK_START) == 0x07);
	mu_assert("BRK - low byte not on S", MEM_read(memory, MEM_STACK_START-1) == 0xD0);
	mu_assert("BRK - flags not on S", MEM_read(memory, MEM_STACK_START-2) == (5|B_FLAG));

	CPU_delete(&cpu);
	return 0;
}

static char *test_ora_ind_x()
{
	memory = MEM_init();
	cpu = CPU_init(memory);

	cpu->X = 6;
	cpu->A = 5;
	MEM_write(memory, 1, 0xB4);
	MEM_write(memory, 0xBA, 0x12);
	MEM_write(memory, 0xBB, 0x0E);
	MEM_write(memory, 0x0E12, 11);

	ora_ind_x(cpu, memory);

	mu_assert("ora_ind_x - Wrong val for A", cpu->A == (5|11));
	mu_assert("ora_ind_x - PC not incremented", cpu->PC == 2);

	CPU_delete(&cpu);
	return 0;
}

static char *test_ora_zero_pg()
{
	memory = MEM_init();
	cpu = CPU_init(memory);

	cpu->A = 5;
	MEM_write(memory, 1, 0xB4);
	MEM_write(memory, 0xB4, 11);

	ora_zero_pg(cpu, memory);

	mu_assert("ora_zero_pg - Wrong val for A", cpu->A == (5|11));
	mu_assert("ora_zero_pg - PC not incremented", cpu->PC == 2);

	CPU_delete(&cpu);
	return 0;
}

static char *test_asl_zero_pg()
{
	memory = MEM_init();
	cpu = CPU_init(memory);

	MEM_write(memory, 1, 0xB4);
	MEM_write(memory, 0xB4, 10);

	asl_zero_pg(cpu, memory);

	mu_assert("asl_zero_pg - fail.", MEM_read(memory, 0xB4) == 20);
	mu_assert("asl_zero_pg - PC not incremented", cpu->PC == 2);

	CPU_delete(&cpu);
	return 0;
}

static char *test_php()
{
	memory = MEM_init();
	cpu = CPU_init(memory);

	cpu->P = 17;

	php(cpu, memory);

	mu_assert("PHP - PC not incr", cpu->PC == 1);
	mu_assert("PHP - S not incr", cpu->S == MEM_STACK_START - 1);
	mu_assert("PHP - flags not set", MEM_read(memory, MEM_STACK_START) == 17);

	CPU_delete(&cpu);
	return 0;
}

static char *test_ora_imm()
{
	memory = MEM_init();
	cpu = CPU_init(memory);

	cpu->A = 5;
	MEM_write(memory, 1, 11);

	ora_imm(cpu, memory);

	mu_assert("ora_imm - Wrong val for A", cpu->A == (5|11));
	mu_assert("ora_imm - PC not incremented", cpu->PC == 2);

	CPU_delete(&cpu);
	return 0;
}

static char *test_asl_acc()
{
	memory = MEM_init();
	cpu = CPU_init(memory);

	cpu->A = 10;

	asl_acc(cpu, memory);

	mu_assert("asl_acc - fail.", cpu->A == 20);
	mu_assert("asl_acc - PC not incremented", cpu->PC == 1);

	CPU_delete(&cpu);
	return 0;
}

static char *test_ora_abs()
{
	memory = MEM_init();
	cpu = CPU_init(memory);

	cpu->A = 5;
	MEM_write(memory, 1, 0x12); /* low byte is stored first */
	MEM_write(memory, 2, 0x0E);
	MEM_write(memory, 0x0E12, 11);

	ora_abs(cpu, memory);

	mu_assert("ora_abs- fail.", cpu->A == (5|11));
	mu_assert("ora_abs - PC not incremented", cpu->PC == 3);

	CPU_delete(&cpu);
	return 0;
}

static char *test_asl_abs()
{
	memory = MEM_init();
	cpu = CPU_init(memory);

	MEM_write(memory, 1, 0x12); /* low byte is stored first */
	MEM_write(memory, 2, 0x0E);
	MEM_write(memory, 0x0E12, 10);

	asl_abs(cpu, memory);

	mu_assert("asl_abs - fail.", MEM_read(memory, 0x0E12) == 20);
	mu_assert("asl_abs - PC not incremented", cpu->PC == 3);

	CPU_delete(&cpu);
	return 0;
}

static char *test_bpl_r_clear_neg()
{
	memory = MEM_init();
	cpu = CPU_init(memory);

	cpu->PC = 10;
	MEM_write(memory, 11, 0xF9); /* -7, when signed */

	bpl_r(cpu, memory);

	mu_assert("bpl_r - fail when clear", cpu->PC == 5);

	CPU_delete(&cpu);
	return 0;
}

static char *test_bpl_r_clear_pos()
{
	memory = MEM_init();
	cpu = CPU_init(memory);

	cpu->PC = 10;
	MEM_write(memory, 11, 0x07); /* 7, when signed */

	bpl_r(cpu, memory);

	mu_assert("bpl_r - fail when clear", cpu->PC == 19);

	CPU_delete(&cpu);
	return 0;
}

static char *test_bpl_r_set()
{
	memory = MEM_init();
	cpu = CPU_init(memory);

	cpu->PC = 10;
	cpu->P |= N_FLAG;
	MEM_write(memory, 11, 0xF9); /* -7, when signed */

	bpl_r(cpu, memory);

	mu_assert("bpl_r - fail when set", cpu->PC == 12);

	CPU_delete(&cpu);
	return 0;
}

static char *test_ora_ind_y()
{
	memory = MEM_init();
	cpu = CPU_init(memory);

	cpu->Y = 6;
	cpu->A = 5;
	MEM_write(memory, 1, 0xB4);
	MEM_write(memory, 0xB4, 0x12);
	MEM_write(memory, 0xB5, 0x0E);
	MEM_write(memory, 0x0E12 + 6, 11);

	ora_ind_y(cpu, memory);

	mu_assert("ora_ind_y - Wrong val for A", cpu->A == (5|11));
	mu_assert("ora_ind_y - PC not incremented", cpu->PC == 2);

	CPU_delete(&cpu);
	return 0;
}

static char *test_ora_zero_pg_x()
{
	memory = MEM_init();
	cpu = CPU_init(memory);

	cpu->X = 6;
	cpu->A = 5;
	MEM_write(memory, 1, 0xB4);
	MEM_write(memory, 0xB4 + cpu->X, 11);

	ora_zero_pg_x(cpu, memory);

	mu_assert("ora_aero_pg_x - Wrong val for A", cpu->A == (5|11));
	mu_assert("ora_zero_pg_x - PC not incremented", cpu->PC == 2);

	CPU_delete(&cpu);
	return 0;
}

static char *test_asl_zero_pg_x()
{
	memory = MEM_init();
	cpu = CPU_init(memory);

	cpu->X = 5;
	MEM_write(memory, 1, 0xB4);
	MEM_write(memory, 0xB4 + cpu->X, 10);

	asl_zero_pg_x(cpu, memory);

	mu_assert("asl_zero_pg_x - fail.", MEM_read(memory, 0xB4 + cpu->X) == 20);
	mu_assert("asl_zero_pg_x - PC not incremented", cpu->PC == 2);

	CPU_delete(&cpu);
	return 0;
}

static char *test_clc()
{
	memory = MEM_init();
	cpu = CPU_init(memory);
	cpu->P |= C_FLAG;

	clc(cpu, memory);

	mu_assert("clc - fail.", (cpu->P & C_FLAG) == 0);
	mu_assert("clc - PC not incremented", cpu->PC == 1);

	CPU_delete(&cpu);
	return 0;
}

static char *test_ora_abs_y()
{
	memory = MEM_init();
	cpu = CPU_init(memory);

	cpu->Y = 6;
	cpu->A = 5;
	MEM_write(memory, 1, 0xB4);
	MEM_write(memory, 2, 0x0E);
	MEM_write(memory, 0x0EB4 + cpu->Y, 11);

	ora_abs_y(cpu, memory);

	mu_assert("ora_abs_y - Wrong val for A", cpu->A == (5|11));
	mu_assert("ora_abs_y - PC not incremented", cpu->PC == 3);

	CPU_delete(&cpu);
	return 0;
}

static char *test_ora_abs_x()
{
	memory = MEM_init();
	cpu = CPU_init(memory);

	cpu->X = 6;
	cpu->A = 5;
	MEM_write(memory, 1, 0xB4);
	MEM_write(memory, 2, 0x0E);
	MEM_write(memory, 0x0EB4 + cpu->X, 11);

	ora_abs_x(cpu, memory);

	mu_assert("ora_abs_x - Wrong val for A", cpu->A == (5|11));
	mu_assert("ora_abs_x - PC not incremented", cpu->PC == 3);

	CPU_delete(&cpu);
	return 0;
}

static char *test_asl_abs_x()
{
	memory = MEM_init();
	cpu = CPU_init(memory);

	cpu->X = 6;
	MEM_write(memory, 1, 0xB4);
	MEM_write(memory, 2, 0x0E);
	MEM_write(memory, 0x0EB4 + cpu->X, 10);

	asl_abs_x(cpu, memory);

	mu_assert("asl_abs_x - fail.", MEM_read(memory, 0x0EB4 + cpu->X) == 20);
	mu_assert("asl_abs_x - PC not incremented", cpu->PC == 3);

	CPU_delete(&cpu);
	return 0;
}

static char *test_jsr_abs()
{
	memory = MEM_init();
	cpu = CPU_init(memory);

	MEM_write(memory, 1, 0xB4);
	MEM_write(memory, 2, 0x0E);

	jsr_abs(cpu, memory);

	mu_assert("jsr_abs - PC not set", cpu->PC == 0x0EB4);
	mu_assert("jsr_abs - stack high byte wrong", MEM_read(memory, cpu->S+2) == 0);
	mu_assert("jsr_abs - stack low byte wrong", MEM_read(memory, cpu->S+1) == 2);

	CPU_delete(&cpu);
	return 0;
}

static char *test_and_ind_x()
{
	memory = MEM_init();
	cpu = CPU_init(memory);

	cpu->X = 6;
	cpu->A = 5;
	MEM_write(memory, 1, 0xB4);
	MEM_write(memory, 0xBA, 0x12);
	MEM_write(memory, 0xBB, 0x0E);
	MEM_write(memory, 0x0E12, 11);

	and_ind_x(cpu, memory);

	mu_assert("and_ind_x - Wrong val for A", cpu->A == (5&11));
	mu_assert("and_ind_x - PC not incremented", cpu->PC == 2);

	CPU_delete(&cpu);
	return 0;
}

static char *test_bit_zero_pg()
{
	memory = MEM_init();
	cpu = CPU_init(memory);

	MEM_write(memory, 1, 0xB4);
	cpu->A = 0x0F; /* 00001111 */
	MEM_write(memory, 0xB4, 0xF0); /* 11110000 */
	/* P should be 11000010 = 0xC2 */

	bit_zero_pg(cpu, memory);

	mu_assert("bit_zero_pg - Wrong Z_FLAG", (cpu->P & Z_FLAG) == Z_FLAG);
	mu_assert("bit_zero_pg - Wrong N_FLAG", (cpu->P & N_FLAG) == N_FLAG);
	mu_assert("bit_zero_pg - Wrong V_FLAG", (cpu->P & V_FLAG) == V_FLAG);
	mu_assert("bit_zero_pg - Wrong P", cpu->P = 0xC2);
	mu_assert("bit_zero_pg - PC not incremented", cpu->PC == 2);

	CPU_delete(&cpu);
	return 0;
}

static char *test_and_zero_pg()
{
	memory = MEM_init();
	cpu = CPU_init(memory);

	cpu->A = 5;
	MEM_write(memory, 1, 0xB4);
	MEM_write(memory, 0xB4, 11);

	and_zero_pg(cpu, memory);

	mu_assert("and_zero_pg - Wrong val for A", cpu->A == (5&11));
	mu_assert("and_zero_pg - PC not incremented", cpu->PC == 2);

	CPU_delete(&cpu);
	return 0;
}

static char *test_rol_zero_pg_carry_flag_set()
{
	memory = MEM_init();
	cpu = CPU_init(memory);
	CPU_set_carry_flag(cpu);

	MEM_write(memory, 1, 0xB4);
	MEM_write(memory, 0xB4, 6);

	rol_zero_pg(cpu, memory);

	mu_assert("rol_zero_pg - Wrong result", MEM_read(memory, 0xB4) == 6*2 + 1);
	mu_assert("rol_zero_pg - PC not incremented", cpu->PC == 2);
	mu_assert("rol_zero_pg - Wrong C_FLAG", CPU_carry_flag_is_set(cpu) == 0);

	CPU_delete(&cpu);
	return 0;
}

static char *test_rol_zero_pg_high_bit_set()
{
	memory = MEM_init();
	cpu = CPU_init(memory);

	MEM_write(memory, 1, 0xB4);
	MEM_write(memory, 0xB4, 0x86);

	rol_zero_pg(cpu, memory);

	mu_assert("rol_zero_pg - Wrong result", MEM_read(memory, 0xB4) == 6*2);
	mu_assert("rol_zero_pg - PC not incremented", cpu->PC == 2);
	mu_assert("rol_zero_pg - Wrong C_FLAG", CPU_carry_flag_is_set(cpu) == 1);

	CPU_delete(&cpu);
	return 0;
}

static char *test_plp()
{
	memory = MEM_init();
	cpu = CPU_init(memory);

	CPU_push8_stack(cpu, memory, 0xA5);

	plp(cpu, memory);

	mu_assert("plp - PC not incremented", cpu->PC == 1);
	mu_assert("plp - Wrong flags", cpu->P == 0xA5);

	CPU_delete(&cpu);
	return 0;
}

static char *test_and_imm()
{
	memory = MEM_init();
	cpu = CPU_init(memory);

	cpu->A = 5;
	MEM_write(memory, 1, 11);

	and_imm(cpu, memory);

	mu_assert("and_imm - Wrong val for A", cpu->A == (5&11));
	mu_assert("and_imm - PC not incremented", cpu->PC == 2);

	CPU_delete(&cpu);
	return 0;
}

static char *test_rol_acc()
{
	memory = MEM_init();
	cpu = CPU_init(memory);

	cpu->A = 6;

	rol_acc(cpu, memory);

	mu_assert("rol_acc- Wrong result", cpu->A == 6*2);
	mu_assert("rol_acc - PC not incremented", cpu->PC == 1);
	mu_assert("rol_acc - Wrong C_FLAG", CPU_carry_flag_is_set(cpu) == 0);

	CPU_delete(&cpu);
	return 0;
}

static char *test_bit_abs()
{
	memory = MEM_init();
	cpu = CPU_init(memory);

	MEM_write(memory, 1, 0x12); /* low byte is stored first */
	MEM_write(memory, 2, 0x0E);
	cpu->A = 0x0F; /* 00001111 */
	MEM_write(memory, 0x0E12, 0xF0); /* 11110000 */
	/* P should be 11000010 = 0xC2 */

	bit_abs(cpu, memory);

	mu_assert("bit_abs - Wrong Z_FLAG", (cpu->P & Z_FLAG) == Z_FLAG);
	mu_assert("bit_abs - Wrong N_FLAG", (cpu->P & N_FLAG) == N_FLAG);
	mu_assert("bit_abs - Wrong V_FLAG", (cpu->P & V_FLAG) == V_FLAG);
	mu_assert("bit_abs - Wrong P", cpu->P = 0xC2);
	mu_assert("bit_abs - PC not incremented", cpu->PC == 3);

	CPU_delete(&cpu);
	return 0;
}

static char *test_and_abs()
{
	memory = MEM_init();
	cpu = CPU_init(memory);

	cpu->A = 5;
	MEM_write(memory, 1, 0x12); /* low byte is stored first */
	MEM_write(memory, 2, 0x0E);
	MEM_write(memory, 0x0E12, 11);

	and_abs(cpu, memory);

	mu_assert("and_abs- fail.", cpu->A == (5&11));
	mu_assert("and_abs - PC not incremented", cpu->PC == 3);

	CPU_delete(&cpu);
	return 0;
}

static char *test_rol_abs_carry_flag_set()
{
	memory = MEM_init();
	cpu = CPU_init(memory);
	CPU_set_carry_flag(cpu);

	MEM_write(memory, 1, 0x12);
	MEM_write(memory, 2, 0x0E);
	MEM_write(memory, 0x0E12, 6);

	rol_abs(cpu, memory);

	mu_assert("rol_abs - Wrong result", MEM_read(memory, 0x0E12) == 6*2 + 1);
	mu_assert("rol_abs - PC not incremented", cpu->PC == 3);
	mu_assert("rol_abs - Wrong C_FLAG", CPU_carry_flag_is_set(cpu) == 0);

	CPU_delete(&cpu);
	return 0;
}

static char *test_bmi_r_clear()
{
	memory = MEM_init();
	cpu = CPU_init(memory);
	MEM_write(memory, 1, 0xF9); /* -7, when signed */

	bmi_r(cpu, memory);

	mu_assert("bmi_r - fail when clear", cpu->PC == 2);

	CPU_delete(&cpu);
	return 0;
}

static char *test_bmi_r_set()
{
	memory = MEM_init();
	cpu = CPU_init(memory);
	cpu->P |= N_FLAG;
	cpu->PC = 10;
	MEM_write(memory, 11, 0xF9); /* -7, when signed */

	bmi_r(cpu, memory);

	mu_assert("bmi_r - fail when set", cpu->PC == 5);

	CPU_delete(&cpu);
	return 0;
}

static char *test_sec()
{
	memory = MEM_init();
	cpu = CPU_init(memory);

	sec(cpu, memory);

	mu_assert("sec - C_FLAG not set", (cpu->P & C_FLAG) == C_FLAG);
	mu_assert("sec - PC not incremented", cpu->PC == 1);

	CPU_delete(&cpu);
	return 0;
}

static char *test_rti()
{
	memory = MEM_init();
	cpu = CPU_init(memory);
	CPU_push8_stack(cpu, memory, 0x0A);
	CPU_push8_stack(cpu, memory, 0xB4);
	CPU_push8_stack(cpu, memory, 0x16);

	rti(cpu, memory);

	mu_assert("rti - PC wrong", cpu->PC == 0x0AB4);
	mu_assert("rti - status flags wrong", cpu->P == 0x16);

	CPU_delete(&cpu);
	return 0;
}

static char *all_tests()
{
	/*
	 * Checklist for tested addressing modes:
	 *
	 * Mode		Read Tested	Write tesed
	 * ----------------------------------------
	 * ind_x	y
	 * zero_pg	y
	 * implied	y
	 * imm		y
	 * acc		y
	 * abs		y
	 * r		y
	 * ind_y	y
	 * zero_pg_x	y
	 * abs_y	y
	 * abs_x	y
	 * ind		n
	 */
	mu_run_test(test_brk);
	mu_run_test(test_ora_ind_x);
	mu_run_test(test_ora_zero_pg);
	mu_run_test(test_asl_zero_pg);
	mu_run_test(test_php);
	mu_run_test(test_ora_imm);
	mu_run_test(test_asl_acc);
	mu_run_test(test_ora_abs);
	mu_run_test(test_asl_abs);
	mu_run_test(test_bpl_r_clear_neg);
	mu_run_test(test_bpl_r_clear_pos);
	mu_run_test(test_bpl_r_set);
	mu_run_test(test_ora_ind_y);
	mu_run_test(test_ora_zero_pg_x);
	mu_run_test(test_asl_zero_pg_x);
	mu_run_test(test_clc);
	mu_run_test(test_ora_abs_y);
	mu_run_test(test_ora_abs_x);
	mu_run_test(test_asl_abs_x);
	mu_run_test(test_jsr_abs);
	mu_run_test(test_and_ind_x);
	mu_run_test(test_bit_zero_pg);
	mu_run_test(test_and_zero_pg);
	mu_run_test(test_rol_zero_pg_carry_flag_set);
	mu_run_test(test_rol_zero_pg_high_bit_set);
	mu_run_test(test_plp);
	mu_run_test(test_and_imm);
	mu_run_test(test_rol_acc);
	mu_run_test(test_bit_abs);
	mu_run_test(test_and_abs);
	mu_run_test(test_rol_abs_carry_flag_set);
	mu_run_test(test_bmi_r_set);
	mu_run_test(test_bmi_r_clear);
	mu_run_test(test_sec);
	mu_run_test(test_rti);

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
