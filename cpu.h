/*
 * =============================================================================
 *
 *       Filename:  cpu.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  13-03-28 08:39:54 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alan Kydd (), akydd@ualberta.net
 *   Organization:  
 *
 * =============================================================================
 */

#define MEM_SIZE 2 * 1024

/* 
 * Flags, from left to right:
 * Negative, oVerflow, Break, Decimal mode, Interrupt disable, Zero, Carry
 * NV_BDIZC
 */
#define N_FLAG 1<<7
#define V_FLAG 1<<6
#define B_FLAG 1<<4
#define D_FLAG 1<<3
#define I_FLAG 1<<2
#define Z_FLAG 1<<1
#define C_FLAG 1<<0

struct cpu {
	/* 2 kilobytes of memory */
	uint8_t memory[MEM_SIZE];	/* zero page: [0..255] */
					/* stack: [256..511] */

	/* registers, all unsigned */
	uint16_t PC;	/* program counter */
	uint16_t S;	/* stack pointer */
	uint8_t A;	/* accumulator */
	uint8_t X;	/* X index */
	uint8_t Y;	/* Y index */
	uint8_t P;	/* processor status flags */
};

void init(struct cpu *);

/*
 * Determine if the carry flag should be set when adding a and b.
 */
void add_set_carry_flag(uint8_t a, uint8_t b, struct cpu *);

/*
 * Determine if the overflow flag should be set after adding a and b.
 */
void add_set_overflow_flag(uint8_t a, uint8_t b, uint8_t sum, struct cpu *);

void set_zero_flag(uint8_t a, struct cpu *);

void set_negative_flag(uint8_t a, struct cpu *);

void set_overflow_flag(uint8_t a, struct cpu *);

void set_break_flag(struct cpu *);

void set_interrupt_flag(struct cpu *);
