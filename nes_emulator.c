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

/* func declarations */
/* setting flags */
void add_set_carry_flag(uint8_t, uint8_t);
void add_set_overflow_flag(uint8_t, uint8_t, uint8_t);
void set_zero_flag(uint8_t);
void set_negative_flag(uint8_t);
/* processing instruction sets */
void process_code(uint8_t);
void process_00_code(uint8_t);
void process_01_code(uint8_t);
void process_10_code(uint8_t);
/* instructions */
void nul(uint8_t);
void ora(uint8_t);
void and(uint8_t);
void eor(uint8_t);
void adc(uint8_t);
void sta(uint8_t);
void lda(uint8_t);
void cmp(uint8_t);
void sbc(uint8_t);
/* addressing modes */
uint8_t get_zero_page_indexed_indirect(void);
uint8_t get_zero(void);
uint8_t get_immediate(void);
uint8_t get_absolute(void);
uint8_t get_zero_page_indirect_index(void);
uint8_t get_zero_x(void);
uint8_t get_absolute_x(void);
uint8_t get_absolute_y(void);

#define MEM_SIZE 2 * 1024

/* 2 kilobytes of memory */
uint8_t memory[MEM_SIZE];

/* registers, all unsigned */
uint16_t PC;	/* program counter */
uint8_t S = 0;	/* stack pounter */
uint8_t A = 0;	/* accumulator */
uint8_t X = 0;	/* X index */
uint8_t Y = 0;	/* Y index */
uint8_t P = 0;	/* processor status flags */

/* flags, carry, zero, overflow, negative */
#define C_FLAG 1<<0
#define Z_FLAG 1<<1
#define V_FLAG 1<<6
#define N_FLAG 1<<7

#define N_TEST 1<<7

int main(void)
{
	return 0;
}

void process_code(uint8_t code)
{
	/* opcodes are formatted as aaabbbcc, where aaa and cc determine
	 * the instruction, and bbb determines the addressing mode. */

	static void (* const pf[]) (uint8_t) = {
		&process_00_code, &process_01_code, &process_10_code
	};

	/* determine the instruction set.  Cases for cc = 00, 01, or 10 */
	uint8_t cc = code & 0x03;
	if(cc < sizeof(pf) / sizeof(*pf)) {
		pf[cc](code);
	}
}

void process_00_code(uint8_t code)
{
	static void (* const pf[]) (uint8_t) = {
		&nul, &bit, &jmp, &jmp_abs, &sty, &ldy, &cpy, &cpx
	};

	/* determine the instruction and addressing mode. */
	uint8_t aaa = (code>>5) & 0x07;
	uint8_t bbb = (code>>2) & 0x07;

	if(aaa < sizeof(pf) / sizeof(*pf)) {
		pf[aaa](bbb);
	}
}

void process_01_code(uint8_t code)
{
	static void (* const pf[]) (uint8_t) = {
		&ora, &and, &eor, &adc, &sta, &lda, &cmp, &sbc
	};

	/* determine the instruction and addressing mode. */
	uint8_t aaa = (code>>5) & 0x07;
	uint8_t bbb = (code>>2) & 0x07;

	if(aaa < sizeof(pf) / sizeof(*pf)) {
		pf[aaa](bbb);
	}
}

void process_10_code(uint8_t code)
{
	static void (* const pf[]) (uint8_t) = {
		&asl, &rol, &lsr, &ror, &stx, &ldx, &dec, &inc
	};

	/* determine the instruction and addressing mode. */
	uint8_t aaa = (code>>5) & 0x07;
	uint8_t bbb = (code>>2) & 0x07;

	if(aaa < sizeof(pf) / sizeof(*pf)) {
		pf[aaa](bbb);
	}
}
/* 
 * common memory access functions for addressing modes.
 * This mode determines which value gets used in the computation, so these
 * functions can be reused for all instructions.
 */

/*
 * Accumulator mode return the value in the accumulator
 */
uint8_t get_accumulator()
{
	return A;
}

/*
 * Immediate mode returns the operand after the instr
 */
uint8_t get_immediate()
{
	return memory[PC];
}

/*
 * Absolute mode returns the value at the 16-bit address at the PC
 */
uint8_t get_absolute()
{
	uint16_t addr = memory[PC];
	addr = addr << 8;
	PC++;
	addr |= memory[PC];

	return memory[addr];
}

/*
 * Zero Page mode returns the byte at the address given by the operand
 */
uint8_t get_zero()
{
	uint8_t addr = memory[PC];
	return memory[addr];
}

/*
 * Zero Page index with X mode returns the value at the address given by the sum
 * of the operand and X.
 */
uint8_t get_zero_x()
{
	uint8_t addr = memory[PC] + X;
	return memory[addr];
}

/*
 * Zero Page index with Y mode returns the value at the address given by the sum
 * of the operand and Y.
 */
uint8_t get_zero_y()
{
	uint8_t addr = memory[PC] + Y;
	return memory[addr];
}

/*
 * Absolute index with X mode returns the value at the address given by the
 * sum of X and the operand.
 */
uint8_t get_absolute_x()
{
	uint16_t addr = memory[PC] + X;
	return memory[addr];
}

/*
 * Absolute index with Y mode returns the value at the address given by the
 * sum of Y and the operand.
 */
uint8_t get_absolute_y()
{
	uint16_t addr = memory[PC] + Y;
	return memory[addr];
}

/*
 * Zero Page indexed indirect mode returns the value at the 2-byte address given
 * by the values of the memory at addresses (operand + X) and (operand + X + 1).
 */
uint8_t get_zero_page_indexed_indirect()
{
	uint16_t addr = memory[PC + X];
	addr = addr << 8;
	addr |= memory[PC + X + 1];
	return memory[addr];
}

/*
 * Zero Page indirect indexed with Y mode returns the value at the 2-byte
 * address given by the ((value at the address of the operand) + Y) and ((value
 * at the address of the operand) + Y) + 1.
 */
uint8_t get_zero_page_indirect_index_y()
{
	uint16_t addr = memory[PC] + Y;
	addr = addr << 8;
	addr |= memory[PC] + Y + 1;
	return memory[addr];
}

uint8_t get_addr_mode_01_value(mode)
{
	static uint8_t (* const pf[]) (void) = {
		&get_zero_page_indexed_indirect,
		&get_zero,
		&get_immediate,
		&get_absolute,
		&get_zero_page_indirect_index_y,
		&get_zero_x,
		&get_absolute_x,
		&get_absolute_y
	};

	if(mode < sizeof(pf) / sizeof(*pf)) {
		return pf[mode]();
	}
	return -1;
}

void nul(uint8_t mode)
{
	(void)printf("No such code!\n");
}

void adc(uint8_t mode)
{
	PC++;

	uint8_t val = get_addr_mode_01_value(mode);
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
 * Determine if the carry flag should be set when adding a and b.
 */
void add_set_carry_flag(uint8_t a, uint8_t b)
{
	/* if carry flag is zero, check if a + b > 0xff.
	 * Otherwise, check if a + b >= 0xff. */
	if ((P & C_FLAG) == 0) {
		if(a > 0xff - b) {
			P |= C_FLAG;
		}
	} else {
		if (a >= 0xff - b) {
			P |= C_FLAG;
		}
	}
}

/*
 * Determine if the overflow flag should be set after adding a and b.
 */
void add_set_overflow_flag(uint8_t a, uint8_t b, uint8_t sum)
{
	int overflow = 0;
	if((((a|b) ^ sum) & 0x80) == 0x80) {
		overflow = 1;
	}

	if((overflow == 1) ^ ((P & C_FLAG) == C_FLAG)) {
		P |= V_FLAG;
	}
}

void set_zero_flag(uint8_t a)
{
	if (a == 0) {
		P |= Z_FLAG;
	}
}

void set_negative_flag(uint8_t a)
{
	if ((a & N_TEST) == N_TEST) {
		P |= N_FLAG;
	}
}
