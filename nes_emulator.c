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
	S++;
	uint8_t imm = memory[S];
	S++;
}

/*
 * Determine if the carry flag should be set when adding a and b.
 */
void adc_set_c_flag(uint8_t a, uint8_t b)
{
	/* if carry flag is zero, check if a + b > 0xff.
	 * Otherwise, check if a + b >= 0xff. */
	if (P & C_FLAG == 0) {
		if(a > 0Xff - b) {
			P |= C_FLAG;
		}
	} else {
		if (a >= 0xff - b) {
			P |= C_FLAG;
		}
	}
}
