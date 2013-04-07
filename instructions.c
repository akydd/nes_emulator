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



/* helpers */
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

inline uint16_t ind(struct cpu *cpu)
{
	uint16_t addr_of_low = pop16_mem(cpu);
	uint16_t low = cpu->memory[addr_of_low];
	uint16_t high = cpu->memory[addr_of_low + 1]<<8;
	return (high | low);
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
	uint16_t addr = (uint16_t)pop8_mem(cpu);
	return addr + cpu->X;
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

inline void eor(uint16_t addr, struct cpu *cpu)
{
	uint8_t val = cpu->memory[addr];
	cpu->A ^= val;
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

	/* shift carry bit into low bit */
	if (carry_flag_is_set(cpu))
	{
		result |= 0x01;
	}

	/* shift bit 7 into carry flag */
	if (high_bit_is_set(val))
	{
		set_carry_flag(cpu);
	} else {
		clear_carry_flag(cpu);
	}

	cpu->memory[addr] = result;
	set_negative_flag_for_value(result, cpu);
	set_zero_flag_for_value(result, cpu);
}

inline void ror(uint16_t addr, struct cpu *cpu)
{
	uint8_t val = cpu->memory[addr];
	uint8_t result = val>>1;

	/* shift carry bit into high bit */
	if (carry_flag_is_set(cpu))
	{
		result |= 0x80;
	}

	/* shift bit 0 into carry flag */
	if (low_bit_is_set(val))
	{
		set_carry_flag(cpu);
	} else {
		clear_carry_flag(cpu);
	}

	cpu->memory[addr] = result;
	set_negative_flag_for_value(result, cpu);
	set_zero_flag_for_value(result, cpu);
}

inline void lsr(uint16_t addr, struct cpu *cpu)
{
	uint8_t val = cpu->memory[addr];
	uint8_t result = val>>1;

	/* shift old bit 0 into the carry flag */
	if(low_bit_is_set(val))
	{
		set_carry_flag(cpu);
	} else {
		clear_carry_flag(cpu);
	}

	cpu->memory[addr] = result;
	set_negative_flag_for_value(result, cpu);
	set_zero_flag_for_value(result, cpu);
}

inline void jmp(uint16_t addr, struct cpu *cpu)
{
	cpu->PC = addr;
}

/*
 * Sum the contents of the accumulator, value at addr, and the carry flag
 */
inline void adc(uint16_t addr, struct cpu *cpu)
{
	uint8_t a = cpu->memory[addr];
	uint8_t b = cpu->A;

	uint8_t sum = a + b;
	if(carry_flag_is_set(cpu))
	{
		sum++;
	}
	cpu->A = sum;

	set_zero_flag_for_value(sum, cpu);
	set_negative_flag_for_value(sum, cpu);
	set_carry_flag_on_add(a, b, cpu);
	set_overflow_flag_for_adc(a, b, cpu);
}

inline void sta(uint16_t addr, struct cpu *cpu)
{
	cpu->memory[addr] = cpu->A;
}

inline void sty(uint16_t addr, struct cpu *cpu)
{
	cpu->memory[addr] = cpu->Y;
}

inline void stx(uint16_t addr, struct cpu *cpu)
{
	cpu->memory[addr] = cpu->X;
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

	if(negative_flag_is_set(cpu) == 0) {
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

void plp(struct cpu *cpu)
{
	cpu->PC++;
	uint8_t val = pop8_stack(cpu);
	cpu->P = val;
}

void and_imm(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = imm(cpu);
	and(addr, cpu);
}

void rol_acc(struct cpu *cpu)
{
	cpu->PC++;

	uint8_t val = cpu->A;
	uint8_t result = val<<1;

	/* shift carry bit into low bit */
	if (carry_flag_is_set(cpu))
	{
		result |= 0x01;
	}

	/* shift bit 7 into carry flag */
	if (high_bit_is_set(val))
	{
		set_carry_flag(cpu);
	} else {
		clear_carry_flag(cpu);
	}

	cpu->A = result;
}

void bit_abs(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu);
	bit(addr, cpu);
}

void and_abs(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu);
	and(addr, cpu);
}

void rol_abs(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu);
	rol(addr, cpu);
}

/*
 * Branch when negative
 */
void bmi_r(struct cpu *cpu)
{
	cpu->PC++;
	uint8_t offset = pop8_mem(cpu);

	if(negative_flag_is_set(cpu) == 1) {
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

void and_ind_y(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = ind_y(cpu);
	and(addr, cpu);
}

void and_zero_pg_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg_x(cpu);
	and(addr, cpu);
}

void rol_zero_pg_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg_x(cpu);
	rol(addr, cpu);
}

void sec(struct cpu *cpu)
{
	cpu->PC++;
	set_carry_flag(cpu);
}

void and_abs_y(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_y(cpu);
	and(addr, cpu);
}

void and_abs_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_x(cpu);
	and(addr, cpu);
}

void rol_abs_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_x(cpu);
	rol(addr, cpu);
}

void rti(struct cpu *cpu)
{
	cpu->PC++;
	cpu->P = pop8_stack(cpu);
	cpu->PC = pop16_stack(cpu);
}

void eor_ind_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = ind_x(cpu);
	eor(addr, cpu);
}

void eor_zero_pg(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu);
	eor(addr, cpu);
}


void lsr_zero_pg(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu);
	lsr(addr, cpu);
}

void pha(struct cpu *cpu)
{
	cpu->PC++;
	push8_stack(cpu->A, cpu);
}

void eor_imm(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = imm(cpu);
	eor(addr, cpu);
}

void lsr_acc(struct cpu *cpu)
{
	cpu->PC++;
	uint8_t val = cpu->A;
	uint8_t result = val>>1;

	/* shift old bit 0 into the carry flag */
	if(low_bit_is_set(val))
	{
		set_carry_flag(cpu);
	} else {
		clear_carry_flag(cpu);
	}

	cpu->A = result;
}

void jmp_abs(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu);
	jmp(addr, cpu);
}

void eor_abs(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu);
	eor(addr, cpu);
}

void lsr_abs(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu);
	lsr(addr, cpu);
}

/*
 * If overflow flag is clear, add relative displacement to cpu->PC
 */
void bvc_r(struct cpu *cpu)
{
	cpu->PC++;
	uint8_t offset = pop8_mem(cpu);

	if(overflow_flag_is_set(cpu) == 0) {
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

void eor_ind_y(struct cpu *cpu)
{
	cpu->PC++;
	uint8_t addr = ind_y(cpu);
	eor(addr, cpu);
}

void eor_zero_pg_x(struct cpu *cpu)
{
	cpu->PC++;
	uint8_t addr = zero_pg_x(cpu);
	eor(addr, cpu);
}

void lsr_zero_pg_x(struct cpu *cpu)
{
	cpu->PC++;
	uint8_t addr = zero_pg_x(cpu);
	lsr(addr, cpu);
}

void cli(struct cpu *cpu)
{
	cpu->PC++;
	clear_interrupt_flag(cpu);
}

void eor_abs_y(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_y(cpu);
	eor(addr, cpu);
}

void eor_abs_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_x(cpu);
	eor(addr, cpu);
}

void lsr_abs_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_x(cpu);
	lsr(addr, cpu);
}

void rts(struct cpu *cpu)
{
	cpu->PC = pop16_stack(cpu);
}

void adc_ind_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = ind_x(cpu);
	adc(addr, cpu);
}

void adc_zero_pg(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu);
	adc(addr, cpu);
}

void ror_zero_pg(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu);
	ror(addr, cpu);
}

void pla(struct cpu *cpu)
{
	cpu->PC++;
	uint8_t val = pop8_stack(cpu);
	cpu->A = val;

	set_zero_flag_for_value(val, cpu);
	set_negative_flag_for_value(val, cpu);
}

void adc_imm(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = imm(cpu);
	adc(addr, cpu);
}

void ror_acc(struct cpu *cpu)
{
	cpu->PC++;
	uint8_t val = cpu->A;
	uint8_t result = val>>1;

	/* shift carry bit into high bit */
	if (carry_flag_is_set(cpu))
	{
		result |= 0x80;
	}

	/* shift bit 0 into carry flag */
	if (low_bit_is_set(val))
	{
		set_carry_flag(cpu);
	} else {
		clear_carry_flag(cpu);
	}

	cpu->A = result;
	set_negative_flag_for_value(result, cpu);
	set_zero_flag_for_value(result, cpu);
}

void jmp_ind(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = ind(cpu);
	jmp(addr, cpu);
}

void adc_abs(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu);
	adc(addr, cpu);
}

void ror_abs(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu);
	ror(addr, cpu);
}

void bvs_r(struct cpu *cpu)
{
	cpu->PC++;
	uint8_t offset = pop8_mem(cpu);

	if(overflow_flag_is_set(cpu) == 1) {
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

void adc_ind_y(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = ind_y(cpu);
	adc(addr, cpu);
}

void adc_zero_pg_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg_x(cpu);
	adc(addr, cpu);
}

void ror_zero_pg_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg_x(cpu);
	ror(addr, cpu);
}

void sei(struct cpu *cpu)
{
	cpu->PC++;
	set_interrupt_flag(cpu);
}

void adc_abs_y(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_y(cpu);
	adc(addr, cpu);
}

void adc_abs_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_x(cpu);
	adc(addr, cpu);
}

void ror_abs_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_x(cpu);
	ror(addr, cpu);
}

void sta_ind_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = ind_x(cpu);
	sta(addr, cpu);
}

void sty_zero_pg(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu);
	sty(addr, cpu);
}

void sta_zero_pg(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu);
	sta(addr, cpu);
}

void stx_zero_pg(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu);
	stx(addr, cpu);
}

void dey(struct cpu *cpu)
{
	cpu->PC++;
	cpu->Y--;

	set_zero_flag_for_value(cpu->Y, cpu);
	set_negative_flag_for_value(cpu->Y, cpu);
}

void txa(struct cpu *cpu)
{
	cpu->PC++;
	cpu->A = cpu->X;

	set_zero_flag_for_value(cpu->A, cpu);
	set_negative_flag_for_value(cpu->A, cpu);
}

void sty_abs(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu);
	sty(addr, cpu);
}

void sta_abs(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu);
	sta(addr, cpu);
}

void stx_abs(struct cpu * cpu)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu);
	stx(addr, cpu);
}

void bcc_r(struct cpu *cpu)
{
	cpu->PC++;
	uint8_t offset = pop8_mem(cpu);

	if(carry_flag_is_set(cpu) == 0) {
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

void sta_ind_y(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = ind_y(cpu);
	sta(addr, cpu);
}

void sty_zero_pg_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg_x(cpu);
	sty(addr, cpu);
}

void sta_zero_pg_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg_x(cpu);
	sta(addr, cpu);
}

void stx_zero_pg_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg_x(cpu);
	stx(addr, cpu);
}

void tya(struct cpu *cpu)
{
	cpu->PC++;
	cpu->A = cpu->Y;

	set_zero_flag_for_value(cpu->A, cpu);
	set_negative_flag_for_value(cpu->A, cpu);
}

void sta_abs_y(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_y(cpu);
	sta(addr, cpu);
}

void txs(struct cpu *cpu)
{
	cpu->PC++;
	push8_stack(cpu->X, cpu);
}

void sta_abs_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_x(cpu);
	sta(addr, cpu);
}
