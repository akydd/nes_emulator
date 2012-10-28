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

#define MEM_SIZE 2 * 1024

/* memory */
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

void adc_imm()
{
	PC++;
	uint16_t addr = PC;
	uint8_t val = memory[addr];
	val += (P & C_FLAG);
	uint8_t sum = A + val;

	add_set_c_flag(A, val);
	add_set_v_flag(A, val, sum);

	A = sum;

	PC++;
}

void adc_zero()
{
	PC++;
	uint16_t addr = memory[PC];
	uint8_t val = memory[addr];
	val += (P & C_FLAG);
	uint8_t sum = A + val;

	add_set_c_flag(A, val);
	add_set_v_flag(A, val, sum);

	A = sum;

	PC++;
}

void adc_zero_x()
{
	PC++;
	uint16_t addr = memory[PC] + X;
	uint8_t val = memory[addr];
	val += (P & C_FLAG);
	uint8_t sum = A + val;

	add_set_c_flag(A, val);
	add_set_v_flag(A, val, sum);

	A = sum;

	PC++;
}

void adc_abs()
{
	PC++;
	uint16_t addr = memory[PC];
	addr = addr << 8;
	PC++;
	addr |= memory[PC];

	uint8_t val = memory[addr];
	val += (P & C_FLAG);
	uint8_t sum = A + val;

	add_set_c_flag(A, val);
	add_set_v_flag(A, val, sum);

	A = sum;

	PC++;
}

void adc_abs_x()
{
	PC++;
	uint16_t addr = memory[PC];
	addr = addr << 8;
	PC++;
	addr |= memory[PC];

	uint8_t val = memory[addr + X];
	val += (P & C_FLAG);
	uint8_t sum = A + val;

	add_set_c_flag(A, val);
	add_set_v_flag(A, val, sum);

	A = sum;

	PC++;
}


/*
 * Determine if the carry flag should be set when adding a and b.
 */
void add_set_c_flag(uint8_t a, uint8_t b)
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
void add_set_v_flag(uint8_t a, uint8_t b, uint8_t sum)
{
	int overflow = 0;
	if((((a|b) ^ sum) & 0x80) == 0x80) {
		overflow = 1;
	}

	if((overflow == 1) ^ ((P & C_FLAG) == C_FLAG)) {
		P |= V_FLAG;
	}
}
