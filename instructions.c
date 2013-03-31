/*
 * =============================================================================
 *
 *       Filename:  instructions.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  13-03-28 09:35:20 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alan Kydd (), akydd@ualberta.net
 *   Organization:  
 *
 * =============================================================================
 */
#include <stdlib.h>
#include <stdint.h>
#include "cpu.h"
#include "instructions.h"

/* Common functions for addressing modes */
inline uint16_t imm(struct cpu *cpu)
{
	return cpu->PC++;
}

inline uint16_t zero_pg(struct cpu *cpu)
{
	return (uint16_t)pop8_mem(cpu);
}

inline uint16_t abs_(struct cpu *cpu)
{
	return pop16_mem(cpu);
}

inline uint16_t ind_x(struct cpu *cpu)
{
	uint8_t addr_of_low = pop8_mem(cpu) + cpu->X;
	uint16_t low = cpu->memory[addr_of_low];
	uint16_t high = cpu->memory[addr_of_low + 1]<<8;
	return (high | low);
}

inline uint16_t ind_y(struct cpu *cpu)
{
	uint8_t addr_of_low = pop8_mem(cpu);
	uint16_t low = cpu->memory[addr_of_low];
	uint16_t high = cpu->memory[addr_of_low + 1]<<8;
	return (high|low) + cpu->Y;
}

inline uint16_t abs_y(struct cpu *cpu)
{
	return pop16_mem(cpu) + cpu->Y;
}

inline uint16_t abs_x(struct cpu *cpu)
{
	return pop16_mem(cpu) + cpu->X;
}

inline uint16_t zero_pg_x(struct cpu *cpu)
{
	return (uint16_t)(pop8_mem(cpu) + cpu->X);
}

/* Common functions for executing instructions */
inline void ora(uint16_t addr, struct cpu *cpu)
{
	uint8_t val = cpu->memory[addr];
	cpu->A |= val;
	set_negative_flag_for_value(cpu->A, cpu);
	set_zero_flag_for_value(cpu->A, cpu);
}

inline void and(uint16_t addr, struct cpu *cpu)
{
	uint8_t val = cpu->memory[addr];
	cpu->A &= val;
	set_negative_flag_for_value(cpu->A, cpu);
	set_zero_flag_for_value(cpu->A, cpu);
}

inline void asl(uint16_t addr, struct cpu *cpu)
{
	uint8_t val = cpu->memory[addr];
	uint8_t new_val = val<<1;
	cpu->memory[addr] = new_val;

	set_zero_flag_for_value(new_val, cpu);
	set_negative_flag_for_value(new_val, cpu);
	if((val & N_FLAG) == N_FLAG) {
		cpu->P |= C_FLAG;
	}
}

inline void bit(uint16_t addr, struct cpu *cpu)
{
	uint8_t val = cpu->memory[addr];
	uint8_t result = cpu->A & val;
	set_zero_flag_for_value(result, cpu);
	cpu->P |= (val & N_FLAG);
	cpu->P |= (val & V_FLAG);
}

inline void rol(uint16_t addr, struct cpu *cpu)
{
	uint8_t val = cpu->memory[addr];
	uint8_t result = val<<1;

}

inline uint8_t bit_is_set(uint8_t value, uint8_t pos)
{
	if((value & 0x01<<pos) == 0x01<<pos)
	{
		return 1;
	}
	return 0;
}

inline uint8_t high_bit_is_set(uint8_t value)
{
	return bit_is_set(value, 7);
}

inline uint8_t low_bit_is_set(uint8_t value)
{
	return bit_is_set(value, 0);
}

/* 
 * break: Set the break flag, push PC onto the stack,
 * push the status flags on the stack, then address
 * $FFFE/$FFFF is loaded into the PC. BRK is really a two-byte instruction.
 */
void brk(struct cpu *cpu)
{
	cpu->PC += 2;
	set_break_flag(cpu);
	push16_stack(cpu->PC, cpu);
	push8_stack(cpu->P, cpu);
}

/*
 * bitwise or of operand and the accumulator, stored in accumulator
 */
void ora_ind_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = ind_x(cpu);
	ora(addr, cpu);
}

void ora_zero_pg(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu);
	ora(addr, cpu);
}

/*
 * shift bits to the left, pushing in 0.
 */
void asl_zero_pg(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu);
	asl(addr, cpu);
}

/*
 * Push processor status flags onto the stack
 */
void php(struct cpu *cpu)
{
	cpu->PC++;
	push8_stack(cpu->P, cpu);
}

void ora_imm(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = imm(cpu);
	ora(addr, cpu);
}

void asl_acc(struct cpu *cpu)
{
	cpu->PC++;

	uint8_t val = cpu->A;
	uint8_t new_val = val<<1;
	cpu->A = new_val;

	set_zero_flag_for_value(new_val, cpu);
	set_negative_flag_for_value(new_val, cpu);
	if((val & N_FLAG) == N_FLAG) {
		cpu->P |= C_FLAG;
	}
}

void ora_abs(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu);
	ora(addr, cpu);
}

void asl_abs(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu);
	asl(addr, cpu);
}

/*
 * If negative flag is clear, add relative displacement to cpu->PC
 */
void bpl_r(struct cpu *cpu)
{
	cpu->PC++;
	uint8_t offset = pop8_mem(cpu);

	if((cpu->P & N_FLAG) == 0) {
		if(offset >= 0x80) { 
			/* 
			   Need special conversion when unsigned
			   int would be converted to a negative
			   signed int.
			   */
			offset = 0xFF - offset + 1;
			cpu->PC -= offset;
		} else {
			cpu->PC += offset;
		}
	}
}

void ora_ind_y(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = ind_y(cpu);
	ora(addr, cpu);
}

void ora_zero_pg_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg_x(cpu);
	ora(addr, cpu);
}

void asl_zero_pg_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg_x(cpu);
	asl(addr, cpu);
}

/*
 * Clear carry flag
 */
void clc(struct cpu *cpu)
{
	cpu->P &= ~C_FLAG;
	cpu->PC++;
}

void ora_abs_y(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_y(cpu);
	ora(addr, cpu);
}

void ora_abs_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_x(cpu);
	ora(addr, cpu);
}

void asl_abs_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_x(cpu);
	asl(addr, cpu);
}

void jsr_abs(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t transfer_addr = abs_(cpu);
	uint16_t next_op_addr = cpu->PC-1;
	push16_stack(next_op_addr, cpu);
	cpu->PC = transfer_addr;
}

void and_ind_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = ind_x(cpu);
	and(addr, cpu);
}

void bit_zero_pg(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu);
	bit(addr, cpu);
}

void and_zero_pg(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu);
	and(addr, cpu);
}

void rol_zero_pg(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu);
	rol(addr, cpu);
}
