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

void init(struct cpu *cpu)
{
	cpu->PC = 0;
	cpu->S = STACK_START;	/* stack grows down from 0x1FF, or 511 */
	cpu->A = 0;
	cpu->X = 0;
	cpu->Y = 0;
	cpu->P = 0;

	/* clear the cpu mem */
	uint8_t *mem_ptr = NULL;
	for(mem_ptr = cpu->memory; mem_ptr < cpu->memory + MEM_SIZE; mem_ptr++)
	{
		*mem_ptr = 0;
	}
}



/* Stack manipulation */
void push16_stack(uint16_t val, struct cpu *cpu)
{
	/* push high byte, then low */
	cpu->memory[cpu->S] = val>>8;
	cpu->S--;
	cpu->memory[cpu->S] = val;
	cpu->S--;
}

void push8_stack(uint8_t val, struct cpu *cpu)
{
	cpu->memory[cpu->S] = val;
	cpu->S--;
}

uint8_t pop8_stack(struct cpu *cpu)
{
	cpu->S++;
	return cpu->memory[cpu->S];
}

uint16_t pop16_stack(struct cpu *cpu)
{
	cpu->S++;
	uint16_t low = cpu->memory[cpu->S];
	cpu->S++;
	uint16_t high = cpu->memory[cpu->S];
	return (high<<8) | low;
}


/* Memory manipulation */
uint16_t pop16_mem(struct cpu *cpu)
{
	uint16_t low = cpu->memory[cpu->PC];
	cpu->PC++;
	uint16_t high = cpu->memory[cpu->PC];
	cpu->PC++;
	return (high<<8) | low;
}

uint8_t pop8_mem(struct cpu *cpu)
{
	uint8_t val = cpu->memory[cpu->PC];
	cpu->PC++;
	return val;
}



/*
 * Functions for setting/clearing/testing status flags
 */
inline uint8_t status_flag_is_set(uint8_t flag, struct cpu *cpu)
{
	if ((cpu->P & flag) == flag)
	{
		return 1;
	}
	return 0;
}

inline void set_status_flag(uint8_t flag, struct cpu *cpu)
{
	cpu->P |= flag;
}

inline void clear_status_flag(uint8_t flag, struct cpu *cpu)
{
	cpu->P &= ~(flag);
}

/* Carry flag */
inline uint8_t carry_flag_is_set(struct cpu *cpu)
{
	return status_flag_is_set(C_FLAG, cpu);
}

void set_carry_flag(struct cpu *cpu)
{
	set_status_flag(C_FLAG, cpu);
}

void clear_carry_flag(struct cpu *cpu)
{
	clear_status_flag(C_FLAG, cpu);
}

/*
 * Manipulate the carry flag based on an ADC operation.
 */
void set_carry_flag_on_add(uint8_t a, uint8_t b, struct cpu *cpu)
{
	/* if carry flag is zero, check if a + b > 0xff.
	 * Otherwise, check if a + b >= 0xff. */
	if (carry_flag_is_set(cpu) == 0) {
		if(a > 0xff - b) {
			set_carry_flag(cpu);
		}
	} else {
		if (a >= 0xff - b) {
			set_carry_flag(cpu);
		}
	} else {
		clear_carry_flag(cpu);
	}
}

/* Overflow flag */
uint8_t overflow_flag_is_set(struct cpu *cpu)
{
	return status_flag_is_set(V_FLAG, cpu);
}

void set_overflow_flag(struct cpu *cpu)
{
	set_status_flag(V_FLAG, cpu);
}

void clear_overflow_flag(struct cpu *cpu)
{
	clear_status_flag(V_FLAG, cpu);
}

/*
 * Manipulate the overflow flag for an ADC operation
 */
void set_overflow_flag_for_adc(uint8_t a, uint8_t b, struct cpu *cpu)
{
	/*
	int overflow = 0;
	if((((a|b) ^ sum) & 0x80) == 0x80) {
		overflow = 1;
	}

	if((overflow == 1) ^ (carry_flag_is_set(cpu) == 1)) {
		set_overflow_flag(cpu);
	}*/

	/*
	 * Convert operands to signed 16 bit, sum and check if signed two's
	 * complement result is less than -128 or greater than 127.  If it is,
	 * then set the overflow flag.  Clear it otherwise.
	 */
	int16_t op1 = (int16_t)a;
	int16_t op2 = (int16_t)b;
	int16_t sum = op1 + op2;

	if ((sum < 127) | (sum < -128))
	{
		set_overflow_flag(cpu);
	}
	else
	{
		clear_overflow_flag(cpu);
	}
}

/*
 * Manipulate the overflow flag for an SBC operation
 */
void set_overflow_flag_for_sbc(uint8_t a, uint8_t b, struct cpu *cpu)
{
	/*
	 * Convert operands to signed 16 bit, subtract and check if signed two's
	 * complement result is less than -128 or greater than 127.  If it is,
	 * then set the overflow flag.  Clear it otherwise.
	 */
	int16_t op1 = (int16_t)a;
	int16_t op2 = (int16_t)b;
	int16_t diff = op1 - op2;

	if ((diff < 127) | (diff < -128))
	{
		set_overflow_flag(cpu);
	}
	else
	{
		clear_overflow_flag(cpu);
	}
}

void set_overflow_flag_for_value(uint8_t a, struct cpu *cpu)
{
	if ((a & V_FLAG) == V_FLAG) {
		set_overflow_flag(cpu);
	}
}

/* Zero flag */
uint8_t zero_flag_is_set(struct cpu *cpu)
{
	return status_flag_is_set(Z_FLAG, cpu);
}

void set_zero_flag(struct cpu *cpu)
{
	set_status_flag(Z_FLAG, cpu);
}

void clear_zero_flag(struct cpu *cpu)
{
	clear_status_flag(Z_FLAG, cpu);
}

void set_zero_flag_for_value(uint8_t a, struct cpu *cpu)
{
	if (a == 0) {
		set_zero_flag(cpu);
	}
}

/* Negative flag */
uint8_t negative_flag_is_set(struct cpu *cpu)
{
	return status_flag_is_set(N_FLAG, cpu);
}

void set_negative_flag(struct cpu *cpu)
{
	set_status_flag(N_FLAG, cpu);
}

void clear_negative_flag(struct cpu *cpu)
{
	clear_status_flag(N_FLAG, cpu);
}

void set_negative_flag_for_value(uint8_t a, struct cpu *cpu)
{
	if ((a & N_FLAG) == N_FLAG) {
		set_status_flag(N_FLAG, cpu);
	}
}

/* Break flag */
uint8_t break_flag_is_set(struct cpu *cpu)
{
	return status_flag_is_set(B_FLAG, cpu);
}

void set_break_flag(struct cpu *cpu)
{
	set_status_flag(B_FLAG, cpu);
}

void clear_break_flag(struct cpu *cpu)
{
	clear_status_flag(B_FLAG, cpu);
}

/* Interrupt flag */
uint8_t interrupt_flag_is_set(struct cpu *cpu)
{
	return status_flag_is_set(I_FLAG, cpu);
}

void set_interrupt_flag(struct cpu *cpu)
{
	set_status_flag(I_FLAG, cpu);
}

void clear_interrupt_flag(struct cpu *cpu)
{
	clear_status_flag(I_FLAG, cpu);
}
