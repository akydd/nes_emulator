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

#include "memory.h"
#include "cpu.h"
#include "instructions.h"



/* helpers */
inline uint8_t bit_is_set(const uint8_t value, const uint8_t pos)
{
	if((value & 0x01<<pos) == 0x01<<pos)
	{
		return 1;
	}
	return 0;
}

inline uint8_t high_bit_is_set(const uint8_t value)
{
	return bit_is_set(value, 7);
}

inline uint8_t low_bit_is_set(const uint8_t value)
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
	return (uint16_t)CPU_pop8_mem(cpu);
}

inline uint16_t abs_(struct cpu *cpu)
{
	return CPU_pop16_mem(cpu);
}

inline uint16_t ind_x(struct cpu *cpu)
{
	uint8_t addr_of_low = CPU_pop8_mem(cpu) + cpu->X;
	uint16_t low = MEM_read(cpu->mem, addr_of_low);
	uint16_t high = MEM_read(cpu->mem, addr_of_low + 1)<<8;
	return (high | low);
}

inline uint16_t ind_y(struct cpu *cpu)
{
	uint8_t addr_of_low = CPU_pop8_mem(cpu);
	uint16_t low = MEM_read(cpu->mem, addr_of_low);
	uint16_t high = MEM_read(cpu->mem, addr_of_low + 1)<<8;
	return (high|low) + cpu->Y;
}

inline uint16_t ind(struct cpu *cpu)
{
	uint16_t addr_of_low = CPU_pop16_mem(cpu);
	uint16_t low = MEM_read(cpu->mem, addr_of_low);
	uint16_t high = MEM_read(cpu->mem, addr_of_low + 1)<<8;
	return (high | low);
}

inline uint16_t abs_y(struct cpu *cpu)
{
	return CPU_pop16_mem(cpu) + cpu->Y;
}

inline uint16_t abs_x(struct cpu *cpu)
{
	return CPU_pop16_mem(cpu) + cpu->X;
}

inline uint16_t zero_pg_x(struct cpu *cpu)
{
	uint16_t addr = (uint16_t)CPU_pop8_mem(cpu);
	return addr + cpu->X;
}

inline uint16_t zero_pg_y(struct cpu *cpu)
{
	uint16_t addr = (uint16_t)CPU_pop8_mem(cpu);
	return addr + cpu->Y;
}

/* Common functions for executing instructions */
inline void ora(uint16_t addr, struct cpu *cpu)
{
	uint8_t val = MEM_read(cpu->mem, addr);
	cpu->A |= val;
	CPU_set_negative_flag_for_value(cpu, cpu->A);
	CPU_set_zero_flag_for_value(cpu, cpu->A);
}

inline void and(uint16_t addr, struct cpu *cpu)
{
	uint8_t val = MEM_read(cpu->mem, addr);
	cpu->A &= val;
	CPU_set_negative_flag_for_value(cpu, cpu->A);
	CPU_set_zero_flag_for_value(cpu, cpu->A);
}

inline void eor(uint16_t addr, struct cpu *cpu)
{
	uint8_t val = MEM_read(cpu->mem, addr);
	cpu->A ^= val;
	CPU_set_negative_flag_for_value(cpu, cpu->A);
	CPU_set_zero_flag_for_value(cpu, cpu->A);
}

inline void asl(uint16_t addr, struct cpu *cpu)
{
	uint8_t val = MEM_read(cpu->mem, addr);
	uint8_t new_val = val<<1;
	MEM_write(cpu->mem, addr, new_val);

	CPU_set_zero_flag_for_value(cpu, new_val);
	CPU_set_negative_flag_for_value(cpu, new_val);
	if((val & N_FLAG) == N_FLAG) {
		cpu->P |= C_FLAG;
	}
}

inline void bit(uint16_t addr, struct cpu *cpu)
{
	uint8_t val = MEM_read(cpu->mem, addr);
	uint8_t result = cpu->A & val;
	CPU_set_zero_flag_for_value(cpu, result);
	cpu->P |= (val & N_FLAG);
	cpu->P |= (val & V_FLAG);
}

inline void rol(uint16_t addr, struct cpu *cpu)
{
	uint8_t val = MEM_read(cpu->mem, addr);
	uint8_t result = val<<1;

	/* shift carry bit into low bit */
	if (CPU_carry_flag_is_set(cpu))
	{
		result |= 0x01;
	}

	/* shift bit 7 into carry flag */
	if (high_bit_is_set(val))
	{
		CPU_set_carry_flag(cpu);
	} else {
		CPU_clear_carry_flag(cpu);
	}

	MEM_write(cpu->mem, addr, result);
	CPU_set_negative_flag_for_value(cpu, result);
	CPU_set_zero_flag_for_value(cpu, result);
}

inline void ror(uint16_t addr, struct cpu *cpu)
{
	uint8_t val = MEM_read(cpu->mem, addr);
	uint8_t result = val>>1;

	/* shift carry bit into high bit */
	if (CPU_carry_flag_is_set(cpu))
	{
		result |= 0x80;
	}

	/* shift bit 0 into carry flag */
	if (low_bit_is_set(val))
	{
		CPU_set_carry_flag(cpu);
	} else {
		CPU_clear_carry_flag(cpu);
	}

	MEM_write(cpu->mem, addr, result);
	CPU_set_negative_flag_for_value(cpu, result);
	CPU_set_zero_flag_for_value(cpu, result);
}

inline void lsr(uint16_t addr, struct cpu *cpu)
{
	uint8_t val = MEM_read(cpu->mem, addr);
	uint8_t result = val>>1;

	/* shift old bit 0 into the carry flag */
	if(low_bit_is_set(val))
	{
		CPU_set_carry_flag(cpu);
	} else {
		CPU_clear_carry_flag(cpu);
	}

	MEM_write(cpu->mem, addr, result);
	CPU_set_negative_flag_for_value(cpu, result);
	CPU_set_zero_flag_for_value(cpu, result);
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
	uint8_t a = MEM_read(cpu->mem, addr);
	uint8_t b = cpu->A;

	uint8_t sum = a + b;
	if(CPU_carry_flag_is_set(cpu))
	{
		sum++;
	}
	cpu->A = sum;

	CPU_set_zero_flag_for_value(cpu, sum);
	CPU_set_negative_flag_for_value(cpu, sum);
	CPU_set_carry_flag_on_add(cpu, a, b);
	CPU_set_overflow_flag_for_adc(cpu, a, b, sum);
}

inline void sbc(uint16_t addr, struct cpu *cpu)
{
	uint8_t a = MEM_read(cpu->mem, addr);
	uint8_t b = cpu->A;

	uint8_t diff = a - b;
	if(CPU_carry_flag_is_set(cpu))
	{
		diff--;
	}
	cpu->A = diff;

	CPU_set_zero_flag_for_value(cpu, diff);
	CPU_set_negative_flag_for_value(cpu, diff);
	CPU_set_carry_flag_on_sub(cpu, a, b);
	CPU_set_overflow_flag_for_sbc(cpu, a, b, diff);
}

inline void sta(uint16_t addr, struct cpu *cpu)
{
	MEM_write(cpu->mem, addr, cpu->A);
}

inline void sty(uint16_t addr, struct cpu *cpu)
{
	MEM_write(cpu->mem, addr, cpu->Y);
}

inline void stx(uint16_t addr, struct cpu *cpu)
{
	MEM_write(cpu->mem, addr, cpu->X);
}

inline void ldy(uint16_t addr, struct cpu *cpu)
{
	cpu->Y = MEM_read(cpu->mem, addr);
	CPU_set_zero_flag_for_value(cpu, cpu->Y);
	CPU_set_negative_flag_for_value(cpu, cpu->Y);
}

inline void ldx(uint16_t addr, struct cpu *cpu)
{
	cpu->X = MEM_read(cpu->mem, addr);
	CPU_set_zero_flag_for_value(cpu, cpu->X);
	CPU_set_negative_flag_for_value(cpu, cpu->X);
}

inline void lda(uint16_t addr, struct cpu *cpu)
{
	cpu->A = MEM_read(cpu->mem, addr);
	CPU_set_zero_flag_for_value(cpu, cpu->A);
	CPU_set_negative_flag_for_value(cpu, cpu->A);
}

inline void compare(uint8_t a, uint8_t b, struct cpu *cpu)
{
	if(a == b) {
		CPU_set_zero_flag(cpu);
		CPU_set_carry_flag(cpu);
	} else if(a > b) {
		CPU_set_carry_flag(cpu);
		CPU_set_negative_flag_for_value(cpu, a-b);
	} else {
		CPU_clear_carry_flag(cpu);
		CPU_set_negative_flag_for_value(cpu, a-b);
	}
}

inline void cpy(uint16_t addr, struct cpu *cpu)
{
	uint8_t a = cpu->Y;
	uint8_t b = MEM_read(cpu->mem, addr);
	compare(a, b, cpu);
}

inline void cpx(uint16_t addr, struct cpu *cpu)
{
	uint8_t a = cpu->X;
	uint8_t b = MEM_read(cpu->mem, addr);
	compare(a, b, cpu);
}

inline void cmp(uint16_t addr, struct cpu *cpu)
{
	uint8_t a = cpu->A;
	uint8_t b = MEM_read(cpu->mem, addr);
	compare(a, b, cpu);
}

inline void dec(uint16_t addr, struct cpu *cpu)
{
	uint8_t val = MEM_read(cpu->mem, addr);
	uint8_t result = val - 1;
	MEM_write(cpu->mem, addr, result);
	CPU_set_zero_flag_for_value(cpu, result);
	CPU_set_negative_flag_for_value(cpu, result);
}

inline void inc(uint16_t addr, struct cpu *cpu)
{
	uint8_t val = MEM_read(cpu->mem, addr);
	uint8_t result = val + 1;
	MEM_write(cpu->mem, addr, result);
	CPU_set_zero_flag_for_value(cpu, result);
	CPU_set_negative_flag_for_value(cpu, result);
}

/* 
 * break: Set the break flag, push PC onto the stack,
 * push the status flags on the stack, then address
 * $FFFE/$FFFF is loaded into the PC. BRK is really a two-byte instruction.
 */
uint8_t brk(struct cpu *cpu)
{
	cpu->PC += 2;
	CPU_set_break_flag(cpu);
	CPU_push16_stack(cpu, cpu->PC);
	CPU_push8_stack(cpu, cpu->P);

	uint16_t low = MEM_read(cpu->mem, MEM_BRK_VECTOR);
	uint16_t high = MEM_read(cpu->mem, MEM_BRK_VECTOR + 1)<<8;
	cpu->PC = (high | low);

	return 7;
}

/*
 * bitwise or of operand and the accumulator, stored in accumulator
 */
uint8_t ora_ind_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = ind_x(cpu);
	ora(addr, cpu);

	return 6;
}

uint8_t ora_zero_pg(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu);
	ora(addr, cpu);

	return 3;
}

/*
 * shift bits to the left, pushing in 0.
 */
uint8_t asl_zero_pg(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu);
	asl(addr, cpu);

	return 5;
}

/*
 * Push processor status flags onto the stack
 */
uint8_t php(struct cpu *cpu)
{
	cpu->PC++;
	CPU_push8_stack(cpu, cpu->P);

	return 3;
}

uint8_t ora_imm(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = imm(cpu);
	ora(addr, cpu);

	return 2;
}

uint8_t asl_acc(struct cpu *cpu)
{
	cpu->PC++;

	uint8_t val = cpu->A;
	uint8_t new_val = val<<1;
	cpu->A = new_val;

	CPU_set_zero_flag_for_value(cpu, new_val);
	CPU_set_negative_flag_for_value(cpu, new_val);
	if((val & N_FLAG) == N_FLAG) {
		cpu->P |= C_FLAG;
	}

	return 2;
}

uint8_t ora_abs(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu);
	ora(addr, cpu);

	return 4;
}

uint8_t asl_abs(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu);
	asl(addr, cpu);

	return 6;
}

/*
 * If negative flag is clear, add relative displacement to cpu->PC
 */
uint8_t bpl_r(struct cpu *cpu)
{
	cpu->PC++;
	/* Offset must be handled as a signed number */
	int8_t offset = (int8_t)CPU_pop8_mem(cpu);

	if(CPU_negative_flag_is_set(cpu) == 0) {
		cpu->PC += offset;
		return 3;
	}
	return 2;
}

uint8_t ora_ind_y(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = ind_y(cpu);
	ora(addr, cpu);

	return 5;
}

uint8_t ora_zero_pg_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg_x(cpu);
	ora(addr, cpu);

	return 4;
}

uint8_t asl_zero_pg_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg_x(cpu);
	asl(addr, cpu);

	return 6;
}

/*
 * Clear carry flag
 */
uint8_t clc(struct cpu *cpu)
{
	cpu->P &= ~C_FLAG;
	cpu->PC++;

	return 2;
}

uint8_t ora_abs_y(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_y(cpu);
	ora(addr, cpu);

	return 4;
}

uint8_t ora_abs_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_x(cpu);
	ora(addr, cpu);

	return 4;
}

uint8_t asl_abs_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_x(cpu);
	asl(addr, cpu);

	return 7;
}

uint8_t jsr_abs(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t transfer_addr = abs_(cpu);
	uint16_t next_op_addr = cpu->PC-1;
	CPU_push16_stack(cpu, next_op_addr);
	cpu->PC = transfer_addr;

	return 6;
}

uint8_t and_ind_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = ind_x(cpu);
	and(addr, cpu);

	return 6;
}

uint8_t bit_zero_pg(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu);
	bit(addr, cpu);

	return 3;
}

uint8_t and_zero_pg(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu);
	and(addr, cpu);

	return 3;
}

uint8_t rol_zero_pg(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu);
	rol(addr, cpu);

	return 5;
}

uint8_t plp(struct cpu *cpu)
{
	cpu->PC++;
	uint8_t val = CPU_pop8_stack(cpu);
	cpu->P = val;

	return 4;
}

uint8_t and_imm(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = imm(cpu);
	and(addr, cpu);

	return 2;
}

uint8_t rol_acc(struct cpu *cpu)
{
	cpu->PC++;

	uint8_t val = cpu->A;
	uint8_t result = val<<1;

	/* shift carry bit into low bit */
	if (CPU_carry_flag_is_set(cpu))
	{
		result |= 0x01;
	}

	/* shift bit 7 into carry flag */
	if (high_bit_is_set(val))
	{
		CPU_set_carry_flag(cpu);
	} else {
		CPU_clear_carry_flag(cpu);
	}

	cpu->A = result;

	return 2;
}

uint8_t bit_abs(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu);
	bit(addr, cpu);

	return 4;
}

uint8_t and_abs(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu);
	and(addr, cpu);

	return 4;
}

uint8_t rol_abs(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu);
	rol(addr, cpu);

	return 6;
}

/*
 * Branch when negative
 */
uint8_t bmi_r(struct cpu *cpu)
{
	cpu->PC++;
	/* Offset must ba handled as a signed number */
	int8_t offset = (int8_t)CPU_pop8_mem(cpu);

	if(CPU_negative_flag_is_set(cpu) == 1) {
		cpu->PC += offset;
		return 3;
	}
	return 2;
}

uint8_t and_ind_y(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = ind_y(cpu);
	and(addr, cpu);

	return 5;
}

uint8_t and_zero_pg_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg_x(cpu);
	and(addr, cpu);

	return 4;
}

uint8_t rol_zero_pg_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg_x(cpu);
	rol(addr, cpu);

	return 6;
}

uint8_t sec(struct cpu *cpu)
{
	cpu->PC++;
	CPU_set_carry_flag(cpu);

	return 2;
}

uint8_t and_abs_y(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_y(cpu);
	and(addr, cpu);

	return 4;
}

uint8_t and_abs_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_x(cpu);
	and(addr, cpu);

	return 4;
}

uint8_t rol_abs_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_x(cpu);
	rol(addr, cpu);

	return 7;
}

uint8_t rti(struct cpu *cpu)
{
	cpu->PC++;
	cpu->P = CPU_pop8_stack(cpu);
	cpu->PC = CPU_pop16_stack(cpu);

	return 6;
}

uint8_t eor_ind_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = ind_x(cpu);
	eor(addr, cpu);

	return 6;
}

uint8_t eor_zero_pg(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu);
	eor(addr, cpu);

	return 3;
}


uint8_t lsr_zero_pg(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu);
	lsr(addr, cpu);

	return 5;
}

uint8_t pha(struct cpu *cpu)
{
	cpu->PC++;
	CPU_push8_stack(cpu, cpu->A);

	return 3;
}

uint8_t eor_imm(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = imm(cpu);
	eor(addr, cpu);

	return 2;
}

uint8_t lsr_acc(struct cpu *cpu)
{
	cpu->PC++;
	uint8_t val = cpu->A;
	uint8_t result = val>>1;

	/* shift old bit 0 into the carry flag */
	if(low_bit_is_set(val))
	{
		CPU_set_carry_flag(cpu);
	} else {
		CPU_clear_carry_flag(cpu);
	}

	cpu->A = result;

	return 2;
}

uint8_t jmp_abs(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu);
	jmp(addr, cpu);

	return 3;
}

uint8_t eor_abs(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu);
	eor(addr, cpu);

	return 4;
}

uint8_t lsr_abs(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu);
	lsr(addr, cpu);

	return 6;
}

/*
 * If overflow flag is clear, add relative displacement to cpu->PC
 */
uint8_t bvc_r(struct cpu *cpu)
{
	cpu->PC++;
	/* Offset must be handled as a signed number */
	int8_t offset = (int8_t)CPU_pop8_mem(cpu);

	if(CPU_overflow_flag_is_set(cpu) == 0) {
		cpu->PC += offset;
		return 3;
	}
	return 2;
}

uint8_t eor_ind_y(struct cpu *cpu)
{
	cpu->PC++;
	uint8_t addr = ind_y(cpu);
	eor(addr, cpu);

	return 5;
}

uint8_t eor_zero_pg_x(struct cpu *cpu)
{
	cpu->PC++;
	uint8_t addr = zero_pg_x(cpu);
	eor(addr, cpu);

	return 4;
}

uint8_t lsr_zero_pg_x(struct cpu *cpu)
{
	cpu->PC++;
	uint8_t addr = zero_pg_x(cpu);
	lsr(addr, cpu);

	return 6;
}

uint8_t cli(struct cpu *cpu)
{
	cpu->PC++;
	CPU_clear_interrupt_flag(cpu);

	return 2;
}

uint8_t eor_abs_y(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_y(cpu);
	eor(addr, cpu);

	return 4;
}

uint8_t eor_abs_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_x(cpu);
	eor(addr, cpu);

	return 4;
}

uint8_t lsr_abs_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_x(cpu);
	lsr(addr, cpu);

	return 7;
}

uint8_t rts(struct cpu *cpu)
{
	cpu->PC = CPU_pop16_stack(cpu) + 1;

	return 6;
}

uint8_t adc_ind_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = ind_x(cpu);
	adc(addr, cpu);

	return 6;
}

uint8_t adc_zero_pg(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu);
	adc(addr, cpu);

	return 3;
}

uint8_t ror_zero_pg(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu);
	ror(addr, cpu);

	return 5;
}

uint8_t pla(struct cpu *cpu)
{
	cpu->PC++;
	uint8_t val = CPU_pop8_stack(cpu);
	cpu->A = val;

	CPU_set_zero_flag_for_value(cpu, val);
	CPU_set_negative_flag_for_value(cpu, val);

	return 4;
}

uint8_t adc_imm(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = imm(cpu);
	adc(addr, cpu);

	return 2;
}

uint8_t ror_acc(struct cpu *cpu)
{
	cpu->PC++;
	uint8_t val = cpu->A;
	uint8_t result = val>>1;

	/* shift carry bit into high bit */
	if (CPU_carry_flag_is_set(cpu))
	{
		result |= 0x80;
	}

	/* shift bit 0 into carry flag */
	if (low_bit_is_set(val))
	{
		CPU_set_carry_flag(cpu);
	} else {
		CPU_clear_carry_flag(cpu);
	}

	cpu->A = result;
	CPU_set_negative_flag_for_value(cpu, result);
	CPU_set_zero_flag_for_value(cpu, result);

	return 2;
}

uint8_t jmp_ind(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = ind(cpu);
	jmp(addr, cpu);

	return 5;
}

uint8_t adc_abs(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu);
	adc(addr, cpu);

	return 4;
}

uint8_t ror_abs(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu);
	ror(addr, cpu);

	return 6;
}

uint8_t bvs_r(struct cpu *cpu)
{
	cpu->PC++;
	/* Offset must be handles as a signed number */
	int8_t offset = (int8_t)CPU_pop8_mem(cpu);

	if(CPU_overflow_flag_is_set(cpu) == 1) {
		cpu->PC += offset;
		return 3;
	}
	return 2;
}

uint8_t adc_ind_y(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = ind_y(cpu);
	adc(addr, cpu);

	return 2;
}

uint8_t adc_zero_pg_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg_x(cpu);
	adc(addr, cpu);

	return 4;
}

uint8_t ror_zero_pg_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg_x(cpu);
	ror(addr, cpu);

	return 6;
}

uint8_t sei(struct cpu *cpu)
{
	cpu->PC++;
	CPU_set_interrupt_flag(cpu);

	return 2;
}

uint8_t adc_abs_y(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_y(cpu);
	adc(addr, cpu);

	return 4;
}

uint8_t adc_abs_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_x(cpu);
	adc(addr, cpu);

	return 4;
}

uint8_t ror_abs_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_x(cpu);
	ror(addr, cpu);

	return 7;
}

uint8_t sta_ind_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = ind_x(cpu);
	sta(addr, cpu);

	return 6;
}

uint8_t sty_zero_pg(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu);
	sty(addr, cpu);

	return 3;
}

uint8_t sta_zero_pg(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu);
	sta(addr, cpu);

	return 3;
}

uint8_t stx_zero_pg(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu);
	stx(addr, cpu);

	return 3;
}

uint8_t dey(struct cpu *cpu)
{
	cpu->PC++;
	cpu->Y--;

	CPU_set_zero_flag_for_value(cpu, cpu->Y);
	CPU_set_negative_flag_for_value(cpu, cpu->Y);

	return 2;
}

uint8_t txa(struct cpu *cpu)
{
	cpu->PC++;
	cpu->A = cpu->X;

	CPU_set_zero_flag_for_value(cpu, cpu->A);
	CPU_set_negative_flag_for_value(cpu, cpu->A);

	return 2;
}

uint8_t sty_abs(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu);
	sty(addr, cpu);

	return 4;
}

uint8_t sta_abs(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu);
	sta(addr, cpu);

	return 4;
}

uint8_t stx_abs(struct cpu * cpu)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu);
	stx(addr, cpu);

	return 4;
}

uint8_t bcc_r(struct cpu *cpu)
{
	cpu->PC++;
	/* Offset must be treated as a signed number */
	int8_t offset = (int8_t)CPU_pop8_mem(cpu);

	if(CPU_carry_flag_is_set(cpu) == 0) {
		cpu->PC += offset;
		return 3;
	}
	return 2;
}

uint8_t sta_ind_y(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = ind_y(cpu);
	sta(addr, cpu);

	return 6;
}

uint8_t sty_zero_pg_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg_x(cpu);
	sty(addr, cpu);

	return 4;
}

uint8_t sta_zero_pg_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg_x(cpu);
	sta(addr, cpu);

	return 4;
}

uint8_t stx_zero_pg_y(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg_y(cpu);
	stx(addr, cpu);

	return 4;
}

uint8_t tya(struct cpu *cpu)
{
	cpu->PC++;
	cpu->A = cpu->Y;

	CPU_set_zero_flag_for_value(cpu, cpu->A);
	CPU_set_negative_flag_for_value(cpu, cpu->A);

	return 2;
}

uint8_t sta_abs_y(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_y(cpu);
	sta(addr, cpu);

	return 5;
}

uint8_t txs(struct cpu *cpu)
{
	cpu->PC++;
	cpu->S = cpu->X;

	return 2;
}

uint8_t sta_abs_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_x(cpu);
	sta(addr, cpu);

	return 5;
}

uint8_t ldy_imm(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = imm(cpu);
	ldy(addr, cpu);

	return 2;
}

uint8_t lda_ind_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = ind_x(cpu);
	lda(addr, cpu);

	return 6;
}

uint8_t ldx_imm(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = imm(cpu);
	ldx(addr, cpu);

	return 2;
}

uint8_t ldy_zero_pg(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu);
	ldy(addr, cpu);

	return 3;
}

uint8_t lda_zero_pg(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu);
	lda(addr, cpu);

	return 3;
}

uint8_t ldx_zero_pg(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu);
	ldx(addr, cpu);

	return 3;
}

uint8_t tay(struct cpu *cpu)
{
	cpu->PC++;
	cpu->Y = cpu->A;

	CPU_set_zero_flag_for_value(cpu, cpu->A);
	CPU_set_negative_flag_for_value(cpu, cpu->A);

	return 2;
}

uint8_t lda_imm(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = imm(cpu);
	lda(addr, cpu);

	return 2;
}

uint8_t tax(struct cpu *cpu)
{
	cpu->PC++;
	cpu->X = cpu->A;

	CPU_set_zero_flag_for_value(cpu, cpu->A);
	CPU_set_negative_flag_for_value(cpu, cpu->A);

	return 2;
}

uint8_t ldy_abs(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu);
	ldy(addr, cpu);

	return 4;
}

uint8_t lda_abs(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu);
	lda(addr, cpu);

	return 4;
}

uint8_t ldx_abs(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu);
	ldx(addr, cpu);

	return 4;
}

uint8_t bcs_r(struct cpu *cpu)
{
	cpu->PC++;
	/* Offset must be treated as a signed number */
	int8_t offset = (int8_t)CPU_pop8_mem(cpu);

	if(CPU_carry_flag_is_set(cpu) == 1) {
		cpu->PC += offset;
		return 3;
	}
	return 2;
}

uint8_t lda_ind_y(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = ind_y(cpu);
	lda(addr, cpu);

	return 5;
}

uint8_t ldy_zero_pg_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg_x(cpu);
	ldy(addr, cpu);

	return 4;
}

uint8_t lda_zero_pg_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr =zero_pg_x(cpu);
	lda(addr, cpu);

	return 4;
}

uint8_t ldx_zero_pg_y(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg_y(cpu);
	ldx(addr, cpu);

	return 4;
}

uint8_t clv(struct cpu *cpu)
{
	cpu->PC++;
	CPU_clear_overflow_flag(cpu);

	return 2;
}

uint8_t lda_abs_y(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_y(cpu);
	lda(addr, cpu);

	return 4;
}

uint8_t tsx(struct cpu *cpu)
{
	cpu->PC++;
	cpu->X = cpu->S;

	return 2;
}

uint8_t ldy_abs_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_x(cpu);
	ldy(addr, cpu);

	return 4;
}

uint8_t lda_abs_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_x(cpu);
	lda(addr, cpu);

	return 4;
}

uint8_t ldx_abs_y(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_y(cpu);
	ldx(addr, cpu);

	return 4;
}

uint8_t cpy_imm(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = imm(cpu);
	cpy(addr, cpu);

	return 2;
}

uint8_t cmp_ind_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = ind_x(cpu);
	cmp(addr, cpu);

	return 6;
}

uint8_t cpy_zero_pg(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu);
	cpy(addr, cpu);

	return 3;
}

uint8_t cmp_zero_pg(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu);
	cmp(addr, cpu);

	return 3;
}

uint8_t dec_zero_pg(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu);
	dec(addr, cpu);

	return 5;
}

uint8_t iny(struct cpu *cpu)
{
	cpu->PC++;
	cpu->Y++;
	CPU_set_zero_flag_for_value(cpu, cpu->Y);
	CPU_set_negative_flag_for_value(cpu, cpu->Y);

	return 2;
}

uint8_t cmp_imm(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = imm(cpu);
	cmp(addr, cpu);

	return 2;
}

uint8_t dex(struct cpu *cpu)
{
	cpu->PC++;
	cpu->X--;
	CPU_set_zero_flag_for_value(cpu, cpu->X);
	CPU_set_negative_flag_for_value(cpu, cpu->X);

	return 2;
}

uint8_t cpy_abs(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu);
	cpy(addr, cpu);

	return 4;
}

uint8_t cmp_abs(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu);
	cmp(addr, cpu);

	return 4;
}

uint8_t dec_abs(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu);
	dec(addr, cpu);

	return 6;
}

uint8_t bne_r(struct cpu *cpu)
{
	cpu->PC++;
	/* Offset must be treated as a signed number */
	int8_t offset = (int8_t)CPU_pop8_mem(cpu);

	if(CPU_zero_flag_is_set(cpu) == 0) {
		cpu->PC += offset;
		return 3;
	}
	return 2;
}

uint8_t cmp_ind_y(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = ind_y(cpu);
	cmp(addr, cpu);

	return 5;
}

uint8_t cmp_zero_pg_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg_x(cpu);
	cmp(addr, cpu);

	return 4;
}

uint8_t dec_zero_pg_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg_x(cpu);
	dec(addr, cpu);

	return 6;
}

uint8_t cld(struct cpu *cpu)
{
	cpu->PC++;
	CPU_clear_decimal_flag(cpu);

	return 2;
}

uint8_t cmp_abs_y(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_y(cpu);
	cmp(addr, cpu);

	return 4;
}

uint8_t cmp_abs_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_x(cpu);
	cmp(addr, cpu);

	return 4;
}

uint8_t dec_abs_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_x(cpu);
	dec(addr, cpu);

	return 7;
}

uint8_t cpx_imm(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = imm(cpu);
	cpx(addr, cpu);

	return 2;
}

uint8_t sbc_ind_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = ind_x(cpu);
	sbc(addr, cpu);

	return 6;
}

uint8_t cpx_zero_pg(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu);
	cpx(addr, cpu);

	return 3;
}

uint8_t sbc_zero_pg(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu);
	sbc(addr, cpu);

	return 3;
}

uint8_t inc_zero_pg(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu);
	inc(addr, cpu);

	return 5;
}

uint8_t inx(struct cpu *cpu)
{
	cpu->PC++;
	cpu->X++;
	CPU_set_zero_flag_for_value(cpu, cpu->X);
	CPU_set_negative_flag_for_value(cpu, cpu->X);

	return 2;
}

uint8_t sbc_imm(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = imm(cpu);
	sbc(addr, cpu);

	return 2;
}

uint8_t nop(struct cpu *cpu)
{
	cpu->PC++;
	return 2;
}

uint8_t cpx_abs(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu);
	cpx(addr, cpu);

	return 4;
}

uint8_t sbc_abs(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu);
	sbc(addr, cpu);

	return 4;
}

uint8_t inc_abs(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu);
	inc(addr, cpu);

	return 6;
}

uint8_t beq_r(struct cpu *cpu)
{
	cpu->PC++;
	/* Offset must be treated as a signed number */
	int8_t offset = (int8_t)CPU_pop8_mem(cpu);

	if(CPU_zero_flag_is_set(cpu) == 1) {
		cpu->PC += offset;
		return 3;
	}
	return 2;
}

uint8_t sbc_ind_y(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = ind_y(cpu);
	sbc(addr, cpu);

	return 5;
}

uint8_t sbc_zero_pg_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg_x(cpu);
	sbc(addr, cpu);

	return 4;
}

uint8_t inc_zero_pg_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = zero_pg_x(cpu);
	inc(addr, cpu);

	return 6;
}

uint8_t sed(struct cpu *cpu)
{
	cpu->PC++;
	CPU_set_decimal_flag(cpu);

	return 2;
}

uint8_t sbc_abs_y(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_y(cpu);
	sbc(addr, cpu);

	return 4;
}

uint8_t sbc_abs_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_x(cpu);
	sbc(addr, cpu);

	return 4;
}

uint8_t inc_abs_x(struct cpu *cpu)
{
	cpu->PC++;
	uint16_t addr = abs_x(cpu);
	inc(addr, cpu);

	return 7;
}
