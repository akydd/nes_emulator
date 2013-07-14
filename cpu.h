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

#ifndef CPU_H
#define CPH_H

#include <stdint.h>
#include "memory.h"

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
	/* registers, all unsigned */
	uint16_t PC;	/* program counter */
	uint16_t S;	/* stack pointer */
	uint8_t A;	/* accumulator */
	uint8_t X;	/* X index */
	uint8_t Y;	/* Y index */
	uint8_t P;	/* processor status flags */
	struct memory *mem;	/* Shared memory */
};

/*
 * Initialize the cpu with starting values
 */
void CPU_init(struct cpu *, struct memory *);



/* Stack manipulation */
void CPU_push16_stack(struct cpu *, uint16_t);
void CPU_push8_stack(struct cpu *, uint8_t);
uint8_t CPU_pop8_stack(struct cpu *);
uint16_t CPU_pop16_stack(struct cpu *);


/* 
 * Memory manipulation: read N bits and move the PC to the follow address.
 * Values aren't really "popped", as they are still accessible if you know
 * the address.
 */
uint16_t CPU_pop16_mem(struct cpu *);
uint8_t CPU_pop8_mem(struct cpu *);



/* Status flag manipulation */

uint8_t CPU_carry_flag_is_set(const struct cpu *);
void CPU_set_carry_flag(struct cpu *);
void CPU_clear_carry_flag(struct cpu *);
/*
 * Determine if the carry flag should be set when adding a and b.
 */
void CPU_set_carry_flag_on_add(struct cpu *, const uint8_t a, const uint8_t b);
/*
 * Determine if the carry flag should be set when subtracting a and b.
 */
void CPU_set_carry_flag_on_sub(struct cpu *, const uint8_t a, const uint8_t b);
uint8_t CPU_overflow_flag_is_set(const struct cpu *);
void CPU_set_overflow_flag(struct cpu *);
void CPU_clear_overflow_flag(struct cpu *);
/*
 * Manipulate the overflow flag for an ADC operation.
 */
void CPU_set_overflow_flag_for_adc(struct cpu *, const uint8_t, const uint8_t, const uint8_t);
/*
 * Manipulate the overflow flag for a SBC operation.
 */
void CPU_set_overflow_flag_for_sbc(struct cpu *, const uint8_t, const uint8_t, const uint8_t);
void CPU_set_overflow_flag_for_value(struct cpu *, const uint8_t);

uint8_t CPU_zero_flag_is_set(const struct cpu *);
void CPU_set_zero_flag(struct cpu *);
void CPU_clear_zero_flag(struct cpu *);
void CPU_set_zero_flag_for_value(struct cpu *, const uint8_t);

uint8_t CPU_negative_flag_is_set(const struct cpu *);
void CPU_set_negative_flag(struct cpu *);
void CPU_clear_negative_flag(struct cpu *);
void CPU_set_negative_flag_for_value(struct cpu *, const uint8_t);

uint8_t CPU_break_flag_is_set(const struct cpu *);
void CPU_set_break_flag(struct cpu *);
void CPU_clear_break_flag(struct cpu *);

uint8_t CPU_interrupt_flag_is_set(const struct cpu *);
void CPU_set_interrupt_flag(struct cpu *);
void CPU_clear_interrupt_flag(struct cpu *);

void CPU_set_decimal_flag(struct cpu *);
void CPU_clear_decimal_flag(struct cpu *);

#endif
