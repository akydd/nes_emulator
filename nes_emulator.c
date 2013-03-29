/*
 * ============================================================================
 *
 *       Filename:  nes_emulator.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  12-10-27 01:03:51 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alan Kydd (), akydd@ualberta.net
 *   Organization:  
 *
 * ============================================================================
 */
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "cpu.h"
#include "instructions.h"

/* func declarations */

int main(void)
{
	return 0;
}

/*
 * bitwise and of operand and the accumulator, stored in accumulator
 */
void and(uint8_t mode)
{
	PC++;

	uint8_t val = read_addr_mode_01(mode);
	A &= val;

	set_negative_flag(A);
	set_zero_flag(A);

	PC++;
}

/*
 * Exclusive or of operand and the accumulator, stored in accumulator
 */
void eor(uint8_t mode)
{
	PC++;

	uint8_t val = read_addr_mode_01(mode);
	A ^= val;

	set_negative_flag(A);
	set_zero_flag(A);

	PC++;
}

void adc(uint8_t mode)
{
	PC++;

	uint8_t val = read_addr_mode_01(mode);
	val += (P & C_FLAG);
	uint8_t sum = A + val;

	add_set_carry_flag(A, val);
	add_set_overflow_flag(A, val, sum);
	set_negative_flag(sum);
	set_zero_flag(sum);

	A = sum;

	PC++;
}

/*
 * Store the accumulator value to the memory location given by the mode and
 * opcode.
 */
void sta(uint8_t mode)
{
	PC++;

	write_addr_mode_01(mode, A);
}

/*
 * Load a byte of memory into the accumulator
 */
void lda(uint8_t mode)
{
	PC++;

	uint8_t val = read_addr_mode_01(mode);
	A = val;

	set_negative_flag(A);
	set_zero_flag(A);

	PC++;
}

/* 
 * compare Accumulator to value at location of operand:
 * A < val => set negative flag
 * A > val => set carry flag
 * A == val => set zero flag, set carry flag
 */
void cmp(uint8_t mode)
{
	PC++;

	uint8_t val = read_addr_mode_01(mode);
	if(A < val) {
		P |= N_FLAG;
	} else if (A > val) {
		P |= C_FLAG;
	} else {
		P |= Z_FLAG;
		P |= C_FLAG;
	}

	PC++;
}

void sbc(uint8_t mode)
{
	PC++;

	uint8_t val = read_addr_mode_01(mode);
	uint8_t diff = A - val;
	if ((P & C_FLAG) == 0) {
		diff--;
	}

	/* TODO: figure these out 
	   add_set_carry_flag(A, val);
	   add_set_overflow_flag(A, val, sum);
	   */
	set_negative_flag(diff);
	set_zero_flag(diff);

	A = diff;

	PC++;
}

/*
 * Set the flags according to the result of A anded with byte at address.
 * Result is not stored.
 */
void bit(uint8_t mode)
{
	PC++;

	uint8_t val = read_addr_mode_00(mode);
	uint8_t and = A & val;

	set_negative_flag(and);
	set_zero_flag(and);
	set_overflow_flag(and);

	PC++;
}

/*
 * Set the PC as specified by the operand.  Indirect mode.
 */
void jmp(uint8_t mode)
{
	PC++;
	PC = memory[get_absolute()];
}

/*
 * Set the PC as specified by the operand.  Absolute mode.
 */
void jmp_abs(uint8_t mode)
{
	PC++;
	PC = get_absolute();
}

/*
 * Store contents of Y into memory
 */
void sty(uint8_t mode)
{
	PC++;
	write_addr_mode_00(mode, Y);
	PC++;
}

/*
 * Load memory into Y, setting Z and N flags
 */
void ldy(uint8_t mode)
{
	PC++;
	Y = read_addr_mode_00(mode);

	set_negative_flag(Y);
	set_zero_flag(Y);
	PC++;
}

/* 
 * compare Y to value at location of operand:
 * Y < val => set negative flag
 * Y > val => set carry flag
 * Y == val => set zero flag, set carry flag
 */
void cpy(uint8_t mode)
{
	PC++;

	uint8_t val = read_addr_mode_00(mode);
	if(Y < val) {
		P |= N_FLAG;
	} else if (Y > val) {
		P |= C_FLAG;
	} else {
		P |= Z_FLAG;
		P |= C_FLAG;
	}

	PC++;
}

/* 
 * compare X to value at location of operand:
 * X < val => set negative flag
 * X > val => set carry flag
 * X == val => set zero flag, set carry flag
 */
void cpx(uint8_t mode)
{
	PC++;

	uint8_t val = read_addr_mode_00(mode);
	if(X < val) {
		P |= N_FLAG;
	} else if (X > val) {
		P |= C_FLAG;
	} else {
		P |= Z_FLAG;
		P |= C_FLAG;
	}

	PC++;
}

/*
 * Rotate bits to the left, pushing in value of carry flag.
 */
void rol(uint8_t mode)
{
	PC++;

	uint8_t old_val = read_addr_mode_01(mode);
	uint8_t new_val = old_val<<1;
	new_val |= (P & C_FLAG);
	write_addr_mode_01(mode, new_val);

	set_zero_flag(new_val);
	set_negative_flag(new_val);

	if((old_val & N_FLAG) == N_FLAG) {
		P |= C_FLAG;
	}
	PC++;
}

/*
 * Logical shift right.  Pop into the carry flag.
 */
void lsr(uint8_t mode)
{
	PC++;

	uint8_t old_val = read_addr_mode_01(mode);
	uint8_t new_val = old_val>>1;
	write_addr_mode_01(mode, new_val);

	set_zero_flag(new_val);
	set_negative_flag(new_val);

	if((old_val & C_FLAG) == C_FLAG) {
		P |= C_FLAG;
	}
	PC++;
}

/*
 * Rotate bits right, pushing in carry flag value and popping into carry flag.
 */
void ror(uint8_t mode)
{
	PC++;

	uint8_t old_val = read_addr_mode_01(mode);
	uint8_t new_val = old_val>>1;
	if((P & C_FLAG) == C_FLAG) {
		new_val |= N_FLAG;
	}
	write_addr_mode_01(mode, new_val);

	set_zero_flag(new_val);
	set_negative_flag(new_val);

	if((old_val & C_FLAG) == C_FLAG) {
		P |= C_FLAG;
	}
	PC++;
}

/*
 * Store X into memory
 */
void stx(uint8_t mode)
{
	PC++;
	write_addr_mode_01(mode, X);
	PC++;
}

/*
 * Load memory into X, setting Z and N flags
 */
void ldx(uint8_t mode)
{
	PC++;
	X = read_addr_mode_01(mode);

	set_negative_flag(X);
	set_zero_flag(X);
	PC++;
}

/*
 * Subtract 1 from value at memory and stores back.
 */
void dec(uint8_t mode)
{
	PC++;
	uint8_t old_val = read_addr_mode_01(mode);
	uint8_t new_val = old_val - 1;
	write_addr_mode_01(mode, new_val);

	set_negative_flag(new_val);
	set_zero_flag(new_val);
	PC++;
}

/*
 * Add 1 to value at memory and stores back.
 */
void inc(uint8_t mode)
{
	PC++;
	uint8_t old_val = read_addr_mode_01(mode);
	uint8_t new_val = old_val + 1;
	write_addr_mode_01(mode, new_val);

	set_negative_flag(new_val);
	set_zero_flag(new_val);
	PC++;
}

/*
 * Pull processor status from stack
 */
void plp()
{
	PC++;
	S++;
	P = memory[S];
}

/*
 * Set carry flag
 */
void sec()
{
	PC++;
	P |= C_FLAG;
}

/*
 * Push accumulator onto stack
 */
void pha()
{
	PC++;
	memory[S] = A;
	S--;
}

/*
 * Clear the interrupt disable flag
 */
void cli()
{
	PC++;
	P &= ~I_FLAG;
}

/*
 * Pull accumulator status from stack
 */
void pla()
{
	PC++;
	S++;
	A = memory[S];
}

/*
 * Set Interrupt disable flag
 */
void sei()
{
	PC++;
	P |= I_FLAG;
}

/*
 * Decrement Y
 */
void dey()
{
	PC++;
	Y--;
	set_negative_flag(Y);
	set_zero_flag(Y);
}

/*
 * Transfer Y to A
 */
void tya()
{
	PC++;
	A = Y;
	set_negative_flag(A);
	set_zero_flag(A);
}

/*
 * Transfer A to Y
 */
void tay()
{
	PC++;
	Y = A;
	set_negative_flag(Y);
	set_zero_flag(Y);
}

/*
 * Clear the overflow flag
 */
void clv()
{
	PC++;
	P &= ~V_FLAG;
}

/*
 * Increment Y
 */
void iny()
{
	PC++;
	Y++;
	set_negative_flag(Y);
	set_zero_flag(Y);
}

/*
 * Clear the decimal flag
 */
void cld()
{
	PC++;
	P &= ~D_FLAG;
}

/*
 * Increment X
 */
void inx()
{
	PC++;
	X++;
	set_negative_flag(X);
	set_zero_flag(X);
}

/*
 * Set Decimal mode flag
 */
void sed()
{
	PC++;
	P |= D_FLAG;
}

/*
 * If negative flag is set, add relative displacement to PC
 */
void bmi()
{
	PC++;
	if((P & N_FLAG) == N_FLAG) {
		PC = get_relative();
	} else {
		PC++;
	}
}

/*
 * If overflow flag is clear, add relative displacement to PC
 */
void bvc()
{
	PC++;
	if((P & V_FLAG) != V_FLAG) {
		PC = get_relative();
	} else {
		PC++;
	}
}

/*
 * If overflow flag is set, add relative displacement to PC
 */
void bvs()
{
	PC++;
	if((P & V_FLAG) == V_FLAG) {
		PC = get_relative();
	} else {
		PC++;
	}
}

/*
 * If carry flag is clear, add relative displacement to PC
 */
void bcc()
{
	PC++;
	if((P & C_FLAG) != C_FLAG) {
		PC = get_relative();
	} else {
		PC++;
	}
}

/*
 * If carry flag is set, add relative displacement to PC
 */
void bcs()
{
	PC++;
	if((P & C_FLAG) == C_FLAG) {
		PC = get_relative();
	} else {
		PC++;
	}
}

/*
 * If zero flag is clear, add relative displacement to PC
 */
void bne()
{
	PC++;
	if((P & Z_FLAG) != Z_FLAG) {
		PC = get_relative();
	} else {
		PC++;
	}
}

/*
 * If zero flag is set, add relative displacement to PC
 */
void beq()
{
	PC++;
	if((P & Z_FLAG) == Z_FLAG) {
		PC = get_relative();
	} else {
		PC++;
	}
}

/*
 * copy X to accumulator and set flags
 */
void txa()
{
	A = X;
	set_negative_flag(A);
	set_zero_flag(A);
	PC++;
}

/*
 * copy X to the stack pointer
 */
void txs()
{
	S = X;
	PC++;
}

/*
 * Copy accumulator into X register and set flags
 */
void tax()
{
	X = A;
	set_negative_flag(X);
	set_zero_flag(X);
	PC++;
}

/*
 * Copy contents of stack register to X register and set flags
 */
void tsx()
{
	X = S;
	set_negative_flag(X);
	set_zero_flag(X);
	PC++;
}

/*
 * Decrement the X register, set flags as appropriate
 */
void dex()
{
	X--;
	set_negative_flag(X);
	set_zero_flag(X);
	PC++;
}

/*
 * No operation
 */
void nop()
{
	PC++;
}

/* 
 * Pulls the flags and PC from the stack
 * TODO: ensure that I'm pulling the PC from the stack correctly
 */
void rti()
{
	P = memory[S];
	S++;
	PC = memory[S];
	PC<<8;
	S++;
	PC |= memory[S];
	S++;
}

/*
uint16_t get_relative()
{
	int8_t offset = memory[PC];
	PC++;
	return PC + offset;
}

uint8_t get_accumulator()
{
	return A;
}

uint8_t get_immediate()
{
	uint8_t op_1 = memory[PC];
	PC++;
	return op_1;
}

uint8_t get_absolute()
{
	uint16_t low = memory[PC];
	PC++;
	uint16_t high = memory[PC];
	PC++;
	uint16_t addr = ((high<<8) | low);
	return memory[addr];
}

uint8_t get_zero()
{
	uint16_t addr = (uint16_t)(memory[PC] % 0xFF);
	PC++;
	return memory[addr];
}

uint8_t get_zero_x()
{
	uint16_t addr = (uint16_t)((memory[PC] + X) % 0xFF);
	PC++;
	return memory[addr];
}

uint8_t get_zero_y()
{
	uint16_t addr = (uint16_t)((memory[PC] + Y) % 0xFF);
	PC++;
	return memory[addr];
}

uint8_t get_absolute_x()
{
	uint16_t low = memory[PC];
	PC++;
	uint16_t high = memory[PC];
	PC++;
	uint16_t addr = ((high<<8) | low);
	return memory[addr + X];
}

uint8_t get_absolute_y()
{
	uint16_t low = memory[PC];
	PC++;
	uint16_t high = memory[PC];
	PC++;
	uint16_t addr = ((high<<8) | low);
	return memory[addr + Y];
}

uint16_t get_zero_page_indirect_index_y()
{
	uint16_t addr = (uint16_t)memory[PC] + Y;
	addr = addr << 8;
	addr |= memory[PC] + Y + 1;
	return addr;
}*/
