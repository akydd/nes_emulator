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
#define STACK_START 511

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

/*
 * Initialize the cpu with starting values
 */
void init(struct cpu *);



/* Stack manipulation */
void push16_stack(uint16_t, struct cpu *);
void push8_stack(uint8_t, struct cpu *);
uint8_t pop8_stack(struct cpu *);
uint16_t pop16_stack(struct cpu *);


/* 
 * Memory manipulation: read N bits and move the PC to the follow address.
 * Values aren't really "popped", as they are still accessible if you know
 * the address.
 */
uint16_t pop16_mem(struct cpu *);
uint8_t pop8_mem(struct cpu *);



/* Status flag manipulation */

uint8_t carry_flag_is_set(struct cpu *);
void set_carry_flag(struct cpu *);
void clear_carry_flag(struct cpu *);
/*
 * Determine if the carry flag should be set when adding a and b.
 */
void set_carry_flag_on_add(uint8_t a, uint8_t b, struct cpu *);
/*
 * Determine if the carry flag should be set when subtracting a and b.
 */
void set_carry_flag_on_sub(uint8_t a, uint8_t b, struct cpu *);
uint8_t overflow_flag_is_set(struct cpu *);
void set_overflow_flag(struct cpu *);
void clear_overflow_flag(struct cpu *);
/*
 * Manipulate the overflow flag for an ADC operation.
 */
void set_overflow_flag_for_adc(uint8_t, uint8_t, struct cpu *);
/*
 * Manipulate the overflow flag for a SBC operation.
 */
void set_overflow_flag_for_sbc(uint8_t, uint8_t, struct cpu *);
void set_overflow_flag_for_value(uint8_t, struct cpu *);

uint8_t zero_flag_is_set(struct cpu *);
void set_zero_flag(struct cpu *);
void clear_zero_flag(struct cpu *);
void set_zero_flag_for_value(uint8_t, struct cpu *);

uint8_t negative_flag_is_set(struct cpu *);
void set_negative_flag(struct cpu *);
void clear_negative_flag(struct cpu *);
void set_negative_flag_for_value(uint8_t, struct cpu *);

uint8_t break_flag_is_set(struct cpu *);
void set_break_flag(struct cpu *);
void clear_break_flag(struct cpu *);

uint8_t interrupt_flag_is_set(struct cpu *);
void set_interrupt_flag(struct cpu *);
void clear_interrupt_flag(struct cpu *);

void set_decimal_flag(struct cpu *);
void clear_decimal_flag(struct cpu *);
