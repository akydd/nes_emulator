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

/* Functions for flags */
inline void set_status_flag(uint8_t pos, struct cpu *cpu)
{
	cpu->P |= 0x01<<pos;
}

inline void clear_status_flag(uint8_t pos, struct cpu *cpu)
{
	cpu->P &= ~(0x01<<pos);
}

void set_carry_flag(struct cpu *cpu)
{
	set_status_flag(0, cpu);
}

void clear_carry_flag(struct cpu *cpu)
{
	clear_status_flag(0, cpu);
}

/*
 * Determine if the carry flag should be set when adding a and b.
 */
void set_carry_flag_on_add(uint8_t a, uint8_t b, struct cpu *cpu)
{
	/* if carry flag is zero, check if a + b > 0xff.
	 * Otherwise, check if a + b >= 0xff. */
	if ((cpu->P & C_FLAG) == 0) {
		if(a > 0xff - b) {
			cpu->P |= C_FLAG;
		}
	} else {
		if (a >= 0xff - b) {
			cpu->P |= C_FLAG;
		}
	}
}

/*
 * Determine if the overflow flag should be set after adding a and b.
 */
void set_overflow_flag_on_add(uint8_t a, uint8_t b, uint8_t sum, struct cpu *cpu)
{
	int overflow = 0;
	if((((a|b) ^ sum) & 0x80) == 0x80) {
		overflow = 1;
	}

	if((overflow == 1) ^ ((cpu->P & C_FLAG) == C_FLAG)) {
		cpu->P |= V_FLAG;
	}
}

void set_zero_flag_for_value(uint8_t a, struct cpu *cpu)
{
	if (a == 0) {
		cpu->P |= Z_FLAG;
	}
}

void set_negative_flag_for_value(uint8_t a, struct cpu *cpu)
{
	if ((a & N_FLAG) == N_FLAG) {
		cpu->P |= N_FLAG;
	}
}

void set_overflow_flag_for_value(uint8_t a, struct cpu *cpu)
{
	if ((a & V_FLAG) == V_FLAG) {
		cpu->P |= V_FLAG;
	}
}

void set_break_flag(struct cpu *cpu)
{
	cpu->P |= B_FLAG;
}

void set_interrupt_flag(struct cpu *cpu)
{
	cpu->P |= I_FLAG;
}
