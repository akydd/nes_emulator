/*
 * =============================================================================
 *
 *       Filename:  cpu.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  13-03-28 09:01:46 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alan Kydd (), akydd@ualberta.net
 *   Organization:  
 *
 * =============================================================================
 */
#include <stdint.h>
#include <stdlib.h>

#include "cpu.h"

void CPU_init(struct cpu *cpu, struct memory *mem)
{
	cpu->PC = 0;
	/* Stack grows down from 0x1FF, or 511 */
	cpu->S = MEM_STACK_START;
	cpu->A = 0;
	cpu->X = 0;
	cpu->Y = 0;
	cpu->P = 0;
	cpu->mem = mem;
}



/* Stack manipulation */
void CPU_push16_stack(struct cpu *cpu, uint16_t val)
{
	/* push high byte, then low */
	MEM_write(cpu->mem, cpu->S, val>>8);
	cpu->S--;
	MEM_write(cpu->mem, cpu->S, val);
	cpu->S--;
}

void CPU_push8_stack(struct cpu *cpu, uint8_t val)
{
	MEM_write(cpu->mem, cpu->S, val);
	cpu->S--;
}

uint8_t CPU_pop8_stack(struct cpu *cpu)
{
	cpu->S++;
	return MEM_read(cpu->mem, cpu->S);
}

uint16_t CPU_pop16_stack(struct cpu *cpu)
{
	cpu->S++;
	uint16_t low = MEM_read(cpu->mem, cpu->S);
	cpu->S++;
	uint16_t high = MEM_read(cpu->mem , cpu->S);
	return (high<<8) | low;
}


/* Memory manipulation */
uint16_t CPU_pop16_mem(struct cpu *cpu)
{
	uint16_t low = MEM_read(cpu->mem, cpu->PC);
	cpu->PC++;
	uint16_t high = MEM_read(cpu->mem, cpu->PC);
	cpu->PC++;
	return (high<<8) | low;
}

uint8_t CPU_pop8_mem(struct cpu *cpu)
{
	uint8_t val = MEM_read(cpu->mem, cpu->PC);
	cpu->PC++;
	return val;
}



/*
 * Internal functions for setting/clearing/testing status flags
 */
inline uint8_t status_flag_is_set(const struct cpu *cpu, const uint8_t flag)
{
	if ((cpu->P & flag) == flag)
	{
		return 1;
	}
	return 0;
}

inline void set_status_flag(struct cpu *cpu, const uint8_t flag)
{
	cpu->P |= flag;
}

inline void clear_status_flag(struct cpu *cpu, const uint8_t flag)
{
	cpu->P &= ~(flag);
}

/* Carry flag */
uint8_t CPU_carry_flag_is_set(const struct cpu *cpu)
{
	return status_flag_is_set(cpu, C_FLAG);
}

void CPU_set_carry_flag(struct cpu *cpu)
{
	set_status_flag(cpu, C_FLAG);
}

void CPU_clear_carry_flag(struct cpu *cpu)
{
	clear_status_flag(cpu, C_FLAG);
}

/*
 * Manipulate the carry flag based on an ADC operation.
 */
void CPU_set_carry_flag_on_add(struct cpu *cpu, const uint8_t a, const uint8_t b)
{
	/* if carry flag is zero, check if a + b > 0xff.
	 * Otherwise, check if a + b >= 0xff. */
	if (CPU_carry_flag_is_set(cpu) == 0) {
		if (a > 0xff - b) {
			CPU_set_carry_flag(cpu);
		} else {
			CPU_clear_carry_flag(cpu);
		}
	} else {
		if (a >= 0xff - b) {
			CPU_set_carry_flag(cpu);
		} else {
			CPU_clear_carry_flag(cpu);
		}
	}
}

/*
 * Manipulate the carry flag based on an SBC operation
 */
void CPU_set_carry_flag_on_sub(struct cpu *cpu, const uint8_t a, const uint8_t b)
{
	/* If carry flag is set:
	 * 	If a >= b, set the carry flag.
	 *	Otherwise clear the carry flag.
	 * If carry flag is not set:
	 * 	If a > b, set the carry flag.
	 * 	Otherwise clear the carry flag.
	 */
	if (CPU_carry_flag_is_set(cpu) == 1) {
		if (a < b) {
			CPU_clear_carry_flag(cpu);
		}
	} else {
		if (a > b) {
			CPU_set_carry_flag(cpu);
		}
	}
}

/* Overflow flag */
uint8_t CPU_overflow_flag_is_set(const struct cpu *cpu)
{
	return status_flag_is_set(cpu, V_FLAG);
}

void CPU_set_overflow_flag(struct cpu *cpu)
{
	set_status_flag(cpu, V_FLAG);
}

void CPU_clear_overflow_flag(struct cpu *cpu)
{
	clear_status_flag(cpu, V_FLAG);
}

/*
 * Manipulate the overflow flag for an ADC operation.
 * Formula found at
 * http://www.righto.com/2012/12/the-6502-overflow-flag-explained.html
 */
void CPU_set_overflow_flag_for_adc(struct cpu *cpu, const uint8_t a, const uint8_t b, const uint8_t result)
{
	if (((a^result) & (b^result) & 0x80) != 0)
	{
		CPU_set_overflow_flag(cpu);
	}
}

/*
 * Manipulate the overflow flag for an SBC operation.
 * Formula found at
 * http://www.righto.com/2012/12/the-6502-overflow-flag-explained.html
 */
void CPU_set_overflow_flag_for_sbc(struct cpu *cpu, const uint8_t a, const uint8_t b, const uint8_t result)
{
	if (((a^result) & ((0xff-b)^result) & 0x80) != 0)
	{
		CPU_set_overflow_flag(cpu);
	}
}

void CPU_set_overflow_flag_for_value(struct cpu *cpu, const uint8_t a)
{
	if ((a & V_FLAG) == V_FLAG) {
		CPU_set_overflow_flag(cpu);
	}
}

/* Zero flag */
uint8_t CPU_zero_flag_is_set(const struct cpu *cpu)
{
	return status_flag_is_set(cpu, Z_FLAG);
}

void CPU_set_zero_flag(struct cpu *cpu)
{
	set_status_flag(cpu, Z_FLAG);
}

void CPU_clear_zero_flag(struct cpu *cpu)
{
	clear_status_flag(cpu, Z_FLAG);
}

void CPU_set_zero_flag_for_value(struct cpu *cpu, const uint8_t a)
{
	if (a == 0) {
		CPU_set_zero_flag(cpu);
	}
}

/* Negative flag */
uint8_t CPU_negative_flag_is_set(const struct cpu *cpu)
{
	return status_flag_is_set(cpu, N_FLAG);
}

void CPU_set_negative_flag(struct cpu *cpu)
{
	set_status_flag(cpu, N_FLAG);
}

void CPU_clear_negative_flag(struct cpu *cpu)
{
	clear_status_flag(cpu, N_FLAG);
}

void CPU_set_negative_flag_for_value(struct cpu *cpu, const uint8_t a)
{
	if ((a & N_FLAG) == N_FLAG) {
		set_status_flag(cpu, N_FLAG);
	}
}

/* Break flag */
uint8_t CPU_break_flag_is_set(const struct cpu *cpu)
{
	return status_flag_is_set(cpu, B_FLAG);
}

void CPU_set_break_flag(struct cpu *cpu)
{
	set_status_flag(cpu, B_FLAG);
}

void CPU_clear_break_flag(struct cpu *cpu)
{
	clear_status_flag(cpu, B_FLAG);
}

/* Interrupt flag */
uint8_t CPU_interrupt_flag_is_set(const struct cpu *cpu)
{
	return status_flag_is_set(cpu, I_FLAG);
}

void CPU_set_interrupt_flag(struct cpu *cpu)
{
	set_status_flag(cpu, I_FLAG);
}

void CPU_clear_interrupt_flag(struct cpu *cpu)
{
	clear_status_flag(cpu, I_FLAG);
}

void CPU_set_decimal_flag(struct cpu *cpu)
{
	set_status_flag(cpu, D_FLAG);
}

void CPU_clear_decimal_flag(struct cpu *cpu)
{
	clear_status_flag(cpu, D_FLAG);
}
