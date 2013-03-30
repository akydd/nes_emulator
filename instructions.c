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
#include <stdio.h>

/* 
 * break: Set the break flag, push high byte of PC onto the stack, push the low
 * byte of PC onto the stack, push the status flags on the stack, then address
 * $FFFE/$FFFF is loaded into the PC. BRK is really a two-byte instruction.
 */
void brk(struct cpu *cpu)
{
	cpu->PC += 2;
	set_break_flag(cpu);
	uint8_t PC_low_byte = cpu->PC;
	cpu->memory[cpu->S] = cpu->PC>>8;
	cpu->S--;
	cpu->memory[cpu->S] = PC_low_byte;
	cpu->S--;
	cpu->memory[cpu->S] = cpu->P;
	cpu->S--;
}

/*
 * bitwise or of operand and the accumulator, stored in accumulator
 */
void ora_ind_x(struct cpu *cpu)
{
	cpu->PC++;
	uint8_t addr_of_low = cpu->memory[cpu->PC] + cpu->X;
	uint16_t low = cpu->memory[addr_of_low];
	uint16_t high = cpu->memory[addr_of_low + 1]<<8;
	cpu->PC++;
	uint16_t addr = high | low;

	uint8_t val = cpu->memory[addr];
	cpu->A |= val;

	set_negative_flag(cpu->A, cpu);
	set_zero_flag(cpu->A, cpu);
}

void ora_zero_pg(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = cpu->memory[cpu->PC];
	cpu->PC++;

	uint8_t val = cpu->memory[addr];
	cpu->A |= val;

	set_negative_flag(cpu->A, cpu);
	set_zero_flag(cpu->A, cpu);
}

/*
 * shift bits to the left, pushing in 0.
 */
void asl_zero_pg(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = cpu->memory[cpu->PC];
	cpu->PC++;

	uint8_t val = cpu->memory[addr];
	uint8_t new_val = val<<1;
	cpu->memory[addr] = new_val;

	set_zero_flag(new_val, cpu);
	set_negative_flag(new_val, cpu);
	if((val & N_FLAG) == N_FLAG) {
		cpu->P |= C_FLAG;
	}
}

/*
 * Push processor status flags onto the stack
 */
void php(struct cpu *cpu)
{
	cpu->PC++;
	cpu->memory[cpu->S] = cpu->P;
	cpu->S--;
}

void ora_imm(struct cpu *cpu)
{
	cpu->PC++;
	uint8_t val = cpu->memory[cpu->PC];
	cpu->PC++;

	cpu->A |= val;
	set_negative_flag(cpu->A, cpu);
	set_zero_flag(cpu->A, cpu);
}

void asl_A(struct cpu *cpu)
{
	cpu->PC++;

	uint8_t val = cpu->A;
	uint8_t new_val = val<<1;
	cpu->A = new_val;

	set_zero_flag(new_val, cpu);
	set_negative_flag(new_val, cpu);
	if((val & N_FLAG) == N_FLAG) {
		cpu->P |= C_FLAG;
	}
}

void ora_a(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t low = cpu->memory[cpu->PC];
	cpu->PC++;
	uint16_t high = cpu->memory[cpu->PC];
	uint16_t addr = (high<<8) | low;
	uint8_t val = cpu->memory[addr];
	cpu->PC++;

	cpu->A |= val;
	set_negative_flag(cpu->A, cpu);
	set_zero_flag(cpu->A, cpu);
}

void asl_a(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t low = cpu->memory[cpu->PC];
	cpu->PC++;
	uint16_t high = cpu->memory[cpu->PC];
	uint16_t addr = (high<<8) | low;
	uint8_t val = cpu->memory[addr];
	cpu->PC++;

	uint8_t new_val = val<<1;
	cpu->memory[addr] = new_val;

	set_zero_flag(new_val, cpu);
	set_negative_flag(new_val, cpu);
	if((val & N_FLAG) == N_FLAG) {
		cpu->P |= C_FLAG;
	}
}

/*
 * If negative flag is clear, add relative displacement to cpu->PC
 */
void bpl_r(struct cpu *cpu)
{
	cpu->PC++;
	uint8_t offset = cpu->memory[cpu->PC];
	cpu->PC++;

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
	uint8_t addr_of_low = cpu->memory[cpu->PC];
	uint16_t low = cpu->memory[addr_of_low];
	uint16_t high = cpu->memory[addr_of_low + 1]<<8;
	cpu->PC++;
	uint16_t addr = (high|low) + cpu->Y;

	uint8_t val = cpu->memory[addr];
	cpu->A |= val;

	set_negative_flag(cpu->A, cpu);
	set_zero_flag(cpu->A, cpu);
}

void ora_zero_pg_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = cpu->memory[cpu->PC] + cpu->X;
	cpu->PC++;
	uint8_t val = cpu->memory[addr];
	cpu->A |= val;

	set_negative_flag(cpu->A, cpu);
	set_zero_flag(cpu->A, cpu);
}

void asl_zero_pg_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = cpu->memory[cpu->PC] + cpu->X;
	cpu->PC++;
	uint8_t val = cpu->memory[addr];

	uint8_t new_val = val<<1;
	cpu->memory[addr] = new_val;

	set_zero_flag(new_val, cpu);
	set_negative_flag(new_val, cpu);
	if((val & N_FLAG) == N_FLAG) {
		cpu->P |= C_FLAG;
	}
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
	uint16_t low = cpu->memory[cpu->PC];
	cpu->PC++;
	uint16_t high = cpu->memory[cpu->PC]<<8;
	cpu->PC++;
	uint16_t addr = (high | low) + cpu->Y;

	uint8_t val = cpu->memory[addr];
	cpu->A |= val;

	set_negative_flag(cpu->A, cpu);
	set_zero_flag(cpu->A, cpu);
}

void ora_abs_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t low = cpu->memory[cpu->PC];
	cpu->PC++;
	uint16_t high = cpu->memory[cpu->PC]<<8;
	cpu->PC++;
	uint16_t addr = (high | low) + cpu->X;

	uint8_t val = cpu->memory[addr];
	cpu->A |= val;

	set_negative_flag(cpu->A, cpu);
	set_zero_flag(cpu->A, cpu);
}

void asl_abs_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = cpu->memory[cpu->PC] + cpu->X;
	cpu->PC++;
	uint8_t val = cpu->memory[addr];

	uint8_t new_val = val<<1;
	cpu->memory[addr] = new_val;

	set_zero_flag(new_val, cpu);
	set_negative_flag(new_val, cpu);
	if((val & N_FLAG) == N_FLAG) {
		cpu->P |= C_FLAG;
	}
}

