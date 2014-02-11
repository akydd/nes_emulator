/*
 * =============================================================================
 *
 *       Filename:  cpu.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  13-03-28 09:01:46 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alan Kydd (), akydd@ualberta.net
 *   Organization:  
 *
 * =============================================================================
 */
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "cpu.h"
#include "memory.h"

/* 
 * Flags, from left to right:
 * Negative, oVerflow, Unused, Break, Decimal mode, Interrupt disable, Zero, Carry
 * NV_BDIZC.
 *
 * The Unused flag is literally unused, but should always be set to 1.
 */
#define N_FLAG 1<<7
#define V_FLAG 1<<6
#define U_FLAG 1<<5
#define B_FLAG 1<<4
#define D_FLAG 1<<3
#define I_FLAG 1<<2
#define Z_FLAG 1<<1
#define C_FLAG 1<<0

struct cpu {
	uint16_t PC;	/* program counter */
	uint16_t S;	/* stack pointer */
	uint8_t A;	/* accumulator */
	uint8_t X;	/* X index */
	uint8_t Y;	/* Y index */
	uint8_t P;	/* processor status flags */
};

/* Stack manipulation */
inline void CPU_push16_stack(struct cpu *cpu, struct memory *memory, const uint16_t val)
{
	/* push high byte, then low */
	MEM_write(memory, cpu->S, val>>8);
	cpu->S--;

	/* Wrap stack if needed */
	if (cpu->S < MEM_STACK_END) {
		cpu->S = MEM_STACK_START;
	}

	MEM_write(memory, cpu->S, val);
	cpu->S--;

	/* Wrap stack if needed */
	if (cpu->S < MEM_STACK_END) {
		cpu->S = MEM_STACK_START;
	}
}

inline void CPU_push8_stack(struct cpu *cpu, struct memory *memory, const uint8_t val)
{
	MEM_write(memory, cpu->S, val);
	cpu->S--;

	/* Wrap stack if needed */
	if (cpu->S < MEM_STACK_END) {
		cpu->S = MEM_STACK_START;
	}
}

inline uint8_t CPU_pop8_stack(struct cpu *cpu, struct memory *memory)
{
	cpu->S++;

	/* Wrap stack if needed */
	if (cpu->S > MEM_STACK_START) {
		cpu->S = MEM_STACK_END;
	}

	return MEM_read(memory, cpu->S);
}

inline uint16_t CPU_pop16_stack(struct cpu *cpu, struct memory *memory)
{
	cpu->S++;
	/* Wrap stack if needed */
	if (cpu->S > MEM_STACK_START) {
		cpu->S = MEM_STACK_END;
	}
	uint16_t low = MEM_read(memory, cpu->S);
	cpu->S++;
	/* Wrap stack if needed */
	if (cpu->S > MEM_STACK_START) {
		cpu->S = MEM_STACK_END;
	}
	uint16_t high = MEM_read(memory , cpu->S);
	return (high<<8) | low;
}


/* Memory manipulation */
inline uint16_t CPU_pop16_mem(struct cpu *cpu, struct memory *memory)
{
	uint16_t low = MEM_read(memory, cpu->PC);
	cpu->PC++;
	uint16_t high = MEM_read(memory, cpu->PC);
	cpu->PC++;
	return (high<<8) | low;
}

inline uint8_t CPU_pop8_mem(struct cpu *cpu, struct memory *memory)
{
	uint8_t val = MEM_read(memory, cpu->PC);
	cpu->PC++;
	return val;
}

/* CPU Instructions */

/* Opcodes are endoded in one of the following bit sequences:
 *
 * 1) aaabbbcc
 * 	cc determines the instruction set
 * 	aaa determines the instruction
 * 	bbb determines the addressing mode
 *
 * 2) dddd1000
 * 	16 single-byte opcodes, indexed by dddd
 *
 * 3) xxy10000
 * 	Conditional branch instructions.
 * 	xx indicates the flag
 * 	y is compared with the above flag
 *
 * 4) 1eee1010
 * 	Other 6 single-byte opcodes, indexed by 0 <= eee <= 5
 *
 * However, trying to implement this filter, especially when each
 * intruction can operate on operands of different sizes, was too much
 * for me!  So instead each instruction-mode pair is implemented
 * separately, and indexed by opcode in a function pointer table.
 *
 * Function suffix	Notation	Addressing mode
 * ----------------------------------------------------
 * ind_x		(d, X)		(Indirect, X)
 * zero_pg		d		Zero Page
 * (blank)		(blank)		Implied
 * imm			#		Immediate
 * acc 			A		Accumulator
 * abs			a		Absolute
 * r			r		Relative
 * ind_y		(d), Y		(Indirect), Y
 * zero_pg_x		d, X		Zero Page, X
 * zero_pg_y		d, Y		Zero Page, Y
 * abs_y		a, Y		Absolute, Y
 * abs_x		a, X		Absolute, X
 * ind			(a)		Indirect
 *
 */

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

inline uint16_t zero_pg(struct cpu *cpu, struct memory *memory)
{
	return (uint16_t)CPU_pop8_mem(cpu, memory);
}

inline uint16_t abs_(struct cpu *cpu, struct memory *memory)
{
	return CPU_pop16_mem(cpu, memory);
}

inline uint16_t ind_x(struct cpu *cpu, struct memory *memory)
{
	uint8_t addr_of_low = CPU_pop8_mem(cpu, memory) + cpu->X;
	uint16_t low = MEM_read(memory, addr_of_low);
	// Need cast to uint8_t for zero page wrap around
	uint16_t high = MEM_read(memory, (uint8_t)(addr_of_low + 1))<<8;
	return (high | low);
}

inline uint16_t ind_y(struct cpu *cpu, struct memory *memory)
{
	uint8_t addr_of_low = CPU_pop8_mem(cpu, memory);
	uint16_t low = MEM_read(memory, addr_of_low);
	// Need cast to uint8_t for zero page wrap around
	uint16_t high = MEM_read(memory, (uint8_t)(addr_of_low + 1))<<8;
	return (high|low) + cpu->Y;
}

inline uint16_t ind(struct cpu *cpu, struct memory *memory)
{
	uint16_t addr_of_low = CPU_pop16_mem(cpu, memory);
	uint16_t low = MEM_read(memory, addr_of_low);
	uint16_t high = MEM_read(memory, addr_of_low + 1)<<8;
	return (high | low);
}

inline uint16_t abs_y(struct cpu *cpu, struct memory *memory)
{
	return CPU_pop16_mem(cpu, memory) + cpu->Y;
}

inline uint16_t abs_x(struct cpu *cpu, struct memory *memory)
{
	return CPU_pop16_mem(cpu, memory) + cpu->X;
}

inline uint16_t zero_pg_x(struct cpu *cpu, struct memory *memory)
{
	uint16_t addr = (uint16_t)CPU_pop8_mem(cpu, memory);
	return addr + cpu->X;
}

inline uint16_t zero_pg_y(struct cpu *cpu, struct memory *memory)
{
	uint16_t addr = (uint16_t)CPU_pop8_mem(cpu, memory);
	return addr + cpu->Y;
}

/*
 * setting/clearing/testing status flags
 */
inline uint8_t status_flag_is_set(const struct cpu *cpu, const uint8_t flag)
{
	if ((cpu->P & flag) == flag)
	{
		return 1;
	}
	return 0;
}

inline void set_status_flag(struct cpu *cpu, const uint8_t flag)
{
	cpu->P |= flag;
}

inline void clear_status_flag(struct cpu *cpu, const uint8_t flag)
{
	cpu->P &= ~(flag);
}

/* Carry flag */
inline uint8_t CPU_carry_flag_is_set(const struct cpu *cpu)
{
	return status_flag_is_set(cpu, C_FLAG);
}

/*
 * Manipulate the carry flag based on an ADC operation.
 */
inline void CPU_set_carry_flag_on_add(struct cpu *cpu, const uint8_t a, const uint8_t b)
{
	/* if carry flag is zero, check if a + b > 0xff.
	 * Otherwise, check if a + b >= 0xff. */
	if (CPU_carry_flag_is_set(cpu) == 0) {
		if (a > 0xff - b) {
			set_status_flag(cpu, C_FLAG);
		} else {
			clear_status_flag(cpu, C_FLAG);
		}
	} else {
		if (a >= 0xff - b) {
			set_status_flag(cpu, C_FLAG);
		} else {
			clear_status_flag(cpu, C_FLAG);
		}
	}
}

/*
 * Manipulate the carry flag based on an SBC operation
 */
inline void CPU_set_carry_flag_on_sub(struct cpu *cpu, const uint8_t a, const uint8_t b)
{
	/* If carry flag is set:
	 * 	If a >= b, set the carry flag.
	 *	Otherwise clear the carry flag.
	 * If carry flag is not set:
	 * 	If a > b, set the carry flag.
	 * 	Otherwise clear the carry flag.
	 */
	if (CPU_carry_flag_is_set(cpu) == 1) {
		if (a < b) {
			clear_status_flag(cpu, C_FLAG);
		}
	} else {
		if (a > b) {
			set_status_flag(cpu, C_FLAG);
		}
	}
}

/* Overflow flag */
inline uint8_t CPU_overflow_flag_is_set(const struct cpu *cpu)
{
	return status_flag_is_set(cpu, V_FLAG);
}

/*
 * Manipulate the overflow flag for an ADC operation.
 * Formula found at
 * http://www.righto.com/2012/12/the-6502-overflow-flag-explained.html
 */
inline void CPU_set_overflow_flag_for_adc(struct cpu *cpu, const uint8_t a, const uint8_t b, const uint8_t result)
{
	if (((a^result) & (b^result) & 0x80) != 0)
	{
		set_status_flag(cpu, V_FLAG);
	} else {
		clear_status_flag(cpu, V_FLAG);
	}
}

/*
 * Manipulate the overflow flag for an SBC operation.
 * Formula found at
 * http://www.righto.com/2012/12/the-6502-overflow-flag-explained.html
 */
inline void CPU_set_overflow_flag_for_sbc(struct cpu *cpu, const uint8_t a, const uint8_t b, const uint8_t result)
{
	if (((a^result) & ((0xff-b)^result) & 0x80) != 0)
	{
		set_status_flag(cpu, V_FLAG);
	} else {
		clear_status_flag(cpu, V_FLAG);
	}
}

/* Zero flag */
inline uint8_t CPU_zero_flag_is_set(const struct cpu *cpu)
{
	return status_flag_is_set(cpu, Z_FLAG);
}

inline void CPU_set_zero_flag_for_value(struct cpu *cpu, const uint8_t a)
{
	if (a == 0) {
		set_status_flag(cpu, Z_FLAG);
	} else {
		clear_status_flag(cpu, Z_FLAG);
	}
}

/* Negative flag */
inline uint8_t CPU_negative_flag_is_set(const struct cpu *cpu)
{
	return status_flag_is_set(cpu, N_FLAG);
}

inline void CPU_set_negative_flag_for_value(struct cpu *cpu, const uint8_t a)
{
	if ((a & N_FLAG) == N_FLAG) {
		set_status_flag(cpu, N_FLAG);
	} else {
		clear_status_flag(cpu, N_FLAG);
	}
}

/* Break flag */
inline uint8_t CPU_break_flag_is_set(const struct cpu *cpu)
{
	return status_flag_is_set(cpu, B_FLAG);
}

/* Interrupt flag */
inline uint8_t CPU_interrupt_flag_is_set(const struct cpu *cpu)
{
	return status_flag_is_set(cpu, I_FLAG);
}

/* Common functions for executing instructions */
inline void ora(uint16_t addr, struct cpu *cpu, struct memory *memory)
{
	uint8_t val = MEM_read(memory, addr);
	cpu->A |= val;
	CPU_set_negative_flag_for_value(cpu, cpu->A);
	CPU_set_zero_flag_for_value(cpu, cpu->A);
}

inline void and(uint16_t addr, struct cpu *cpu, struct memory *memory)
{
	uint8_t val = MEM_read(memory, addr);
	cpu->A &= val;
	CPU_set_negative_flag_for_value(cpu, cpu->A);
	CPU_set_zero_flag_for_value(cpu, cpu->A);
}

inline void eor(uint16_t addr, struct cpu *cpu, struct memory *memory)
{
	uint8_t val = MEM_read(memory, addr);
	cpu->A ^= val;
	CPU_set_negative_flag_for_value(cpu, cpu->A);
	CPU_set_zero_flag_for_value(cpu, cpu->A);
}

inline void asl(uint16_t addr, struct cpu *cpu, struct memory *memory)
{
	uint8_t val = MEM_read(memory, addr);
	uint8_t new_val = val<<1;
	MEM_write(memory, addr, new_val);

	CPU_set_zero_flag_for_value(cpu, new_val);
	CPU_set_negative_flag_for_value(cpu, new_val);
	if((val & N_FLAG) == N_FLAG) {
		cpu->P |= C_FLAG;
	} else {
		cpu->P &= ~(C_FLAG);
	}
}

inline void bit(uint16_t addr, struct cpu *cpu, struct memory *memory)
{
	uint8_t val = MEM_read(memory, addr);
	uint8_t result = cpu->A & val;
	CPU_set_zero_flag_for_value(cpu, result);
	
	if(bit_is_set(val, 7)) {
		set_status_flag(cpu, N_FLAG);
	} else {
		clear_status_flag(cpu, N_FLAG);
	}

	if(bit_is_set(val, 6)) {
		set_status_flag(cpu, V_FLAG);
	} else {
		clear_status_flag(cpu, V_FLAG);
	}
}

inline void rol(uint16_t addr, struct cpu *cpu, struct memory *memory)
{
	uint8_t val = MEM_read(memory, addr);
	uint8_t result = val<<1;

	/* shift carry bit into low bit */
	if (CPU_carry_flag_is_set(cpu))
	{
		result |= 0x01;
	}

	/* shift bit 7 into carry flag */
	if (high_bit_is_set(val))
	{
		set_status_flag(cpu, C_FLAG);
	} else {
		clear_status_flag(cpu, C_FLAG);
	}

	MEM_write(memory, addr, result);
	CPU_set_negative_flag_for_value(cpu, result);
	CPU_set_zero_flag_for_value(cpu, result);
}

inline void ror(uint16_t addr, struct cpu *cpu, struct memory *memory)
{
	uint8_t val = MEM_read(memory, addr);
	uint8_t result = val>>1;

	/* shift carry bit into high bit */
	if (CPU_carry_flag_is_set(cpu))
	{
		result |= 0x80;
	}

	/* shift bit 0 into carry flag */
	if (low_bit_is_set(val))
	{
		set_status_flag(cpu, C_FLAG);
	} else {
		clear_status_flag(cpu, C_FLAG);
	}

	MEM_write(memory, addr, result);
	CPU_set_negative_flag_for_value(cpu, result);
	CPU_set_zero_flag_for_value(cpu, result);
}

inline void lsr(uint16_t addr, struct cpu *cpu, struct memory *memory)
{
	uint8_t val = MEM_read(memory, addr);
	uint8_t result = val>>1;

	/* shift old bit 0 into the carry flag */
	if(low_bit_is_set(val))
	{
		set_status_flag(cpu, C_FLAG);
	} else {
		clear_status_flag(cpu, C_FLAG);
	}

	MEM_write(memory, addr, result);
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
inline void adc(uint16_t addr, struct cpu *cpu, struct memory *memory)
{
	uint8_t a = cpu->A;
	uint8_t b = MEM_read(memory, addr);

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

inline void sbc(uint16_t addr, struct cpu *cpu, struct memory *memory)
{
	uint8_t a = cpu->A;
	uint8_t b = MEM_read(memory, addr);

	uint8_t diff = a - b;
	if(!CPU_carry_flag_is_set(cpu))
	{
		diff--;
	}
	cpu->A = diff;

	CPU_set_zero_flag_for_value(cpu, diff);
	CPU_set_negative_flag_for_value(cpu, diff);
	CPU_set_carry_flag_on_sub(cpu, a, b);
	CPU_set_overflow_flag_for_sbc(cpu, a, b, diff);
}

inline void sta(uint16_t addr, struct cpu *cpu, struct memory *memory)
{
	MEM_write(memory, addr, cpu->A);
}

inline void sty(uint16_t addr, struct cpu *cpu, struct memory *memory)
{
	MEM_write(memory, addr, cpu->Y);
}

inline void stx(uint16_t addr, struct cpu *cpu, struct memory *memory)
{
	MEM_write(memory, addr, cpu->X);
}

inline void ldy(uint16_t addr, struct cpu *cpu, struct memory *memory)
{
	cpu->Y = MEM_read(memory, addr);
	CPU_set_zero_flag_for_value(cpu, cpu->Y);
	CPU_set_negative_flag_for_value(cpu, cpu->Y);
}

inline void ldx(uint16_t addr, struct cpu *cpu, struct memory *memory)
{
	cpu->X = MEM_read(memory, addr);
	CPU_set_zero_flag_for_value(cpu, cpu->X);
	CPU_set_negative_flag_for_value(cpu, cpu->X);
}

inline void lda(uint16_t addr, struct cpu *cpu, struct memory *memory)
{
	cpu->A = MEM_read(memory, addr);
	CPU_set_zero_flag_for_value(cpu, cpu->A);
	CPU_set_negative_flag_for_value(cpu, cpu->A);
}

inline void compare(uint8_t a, uint8_t b, struct cpu *cpu)
{
	if(a >= b) {
		set_status_flag(cpu, C_FLAG);
	} else {
		clear_status_flag(cpu, C_FLAG);
	}

	CPU_set_zero_flag_for_value(cpu, a-b);
	CPU_set_negative_flag_for_value(cpu, a-b);
}

inline void cpy(uint16_t addr, struct cpu *cpu, struct memory *memory)
{
	uint8_t a = cpu->Y;
	uint8_t b = MEM_read(memory, addr);
	compare(a, b, cpu);
}

inline void cpx(uint16_t addr, struct cpu *cpu, struct memory *memory)
{
	uint8_t a = cpu->X;
	uint8_t b = MEM_read(memory, addr);
	compare(a, b, cpu);
}

inline void cmp(uint16_t addr, struct cpu *cpu, struct memory *memory)
{
	uint8_t a = cpu->A;
	uint8_t b = MEM_read(memory, addr);
	compare(a, b, cpu);
}

inline void dec(uint16_t addr, struct cpu *cpu, struct memory *memory)
{
	uint8_t val = MEM_read(memory, addr);
	uint8_t result = val - 1;
	MEM_write(memory, addr, result);
	CPU_set_zero_flag_for_value(cpu, result);
	CPU_set_negative_flag_for_value(cpu, result);
}

inline void inc(uint16_t addr, struct cpu *cpu, struct memory *memory)
{
	uint8_t val = MEM_read(memory, addr);
	uint8_t result = val + 1;
	MEM_write(memory, addr, result);
	CPU_set_zero_flag_for_value(cpu, result);
	CPU_set_negative_flag_for_value(cpu, result);
}

/* 
 * break: push PC onto the stack, push the status flags on the stack with
 * the break flag virtually set, then address
 * $FFFE/$FFFF is loaded into the PC. BRK is really a two-byte instruction.
 */
uint8_t brk(struct cpu *cpu, struct memory *memory)
{
	cpu->PC += 2;
	CPU_push16_stack(cpu, memory, cpu->PC);
	CPU_push8_stack(cpu, memory, cpu->P | B_FLAG | U_FLAG);

	uint16_t low = MEM_read(memory, MEM_BRK_VECTOR);
	uint16_t high = MEM_read(memory, MEM_BRK_VECTOR + 1)<<8;
	cpu->PC = (high | low);

	return 7;
}

/*
 * bitwise or of operand and the accumulator, stored in accumulator
 */
uint8_t ora_ind_x(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = ind_x(cpu, memory);
	ora(addr, cpu, memory);

	return 6;
}

uint8_t ora_zero_pg(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu, memory);
	ora(addr, cpu, memory);

	return 3;
}

/*
 * shift bits to the left, pushing in 0.
 */
uint8_t asl_zero_pg(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu, memory);
	asl(addr, cpu, memory);

	return 5;
}

/*
 * Push processor status flags onto the stack.
 */
uint8_t php(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	CPU_push8_stack(cpu, memory, cpu->P | B_FLAG | U_FLAG);

	return 3;
}

uint8_t ora_imm(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = imm(cpu);
	ora(addr, cpu, memory);

	return 2;
}

uint8_t asl_acc(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;

	uint8_t val = cpu->A;
	uint8_t new_val = val<<1;
	cpu->A = new_val;

	CPU_set_zero_flag_for_value(cpu, new_val);
	CPU_set_negative_flag_for_value(cpu, new_val);
	if((val & N_FLAG) == N_FLAG) {
		cpu->P |= C_FLAG;
	} else {
		cpu->P &= ~(C_FLAG);
	}

	return 2;
}

uint8_t ora_abs(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu, memory);
	ora(addr, cpu, memory);

	return 4;
}

uint8_t asl_abs(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu, memory);
	asl(addr, cpu, memory);

	return 6;
}

/*
 * If negative flag is clear, add relative displacement to cpu->PC
 */
uint8_t bpl_r(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	/* Offset must be handled as a signed number */
	int8_t offset = (int8_t)CPU_pop8_mem(cpu, memory);

	if(CPU_negative_flag_is_set(cpu) == 0) {
		cpu->PC += offset;
		return 3;
	}
	return 2;
}

uint8_t ora_ind_y(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = ind_y(cpu, memory);
	ora(addr, cpu, memory);

	return 5;
}

uint8_t ora_zero_pg_x(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = zero_pg_x(cpu, memory);
	ora(addr, cpu, memory);

	return 4;
}

uint8_t asl_zero_pg_x(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = zero_pg_x(cpu, memory);
	asl(addr, cpu, memory);

	return 6;
}

/*
 * Clear carry flag
 */
uint8_t clc(struct cpu *cpu, struct memory *memory)
{
	cpu->P &= ~C_FLAG;
	cpu->PC++;

	return 2;
}

uint8_t ora_abs_y(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = abs_y(cpu, memory);
	ora(addr, cpu, memory);

	return 4;
}

uint8_t ora_abs_x(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = abs_x(cpu, memory);
	ora(addr, cpu, memory);

	return 4;
}

uint8_t asl_abs_x(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = abs_x(cpu, memory);
	asl(addr, cpu, memory);

	return 7;
}

uint8_t jsr_abs(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t transfer_addr = abs_(cpu, memory);
	uint16_t next_op_addr = cpu->PC-1;
	CPU_push16_stack(cpu, memory, next_op_addr);
	cpu->PC = transfer_addr;

	return 6;
}

uint8_t and_ind_x(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = ind_x(cpu, memory);
	and(addr, cpu, memory);

	return 6;
}

uint8_t bit_zero_pg(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu, memory);
	bit(addr, cpu, memory);

	return 3;
}

uint8_t and_zero_pg(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu, memory);
	and(addr, cpu, memory);

	return 3;
}

uint8_t rol_zero_pg(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu, memory);
	rol(addr, cpu, memory);

	return 5;
}

uint8_t plp(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint8_t val = CPU_pop8_stack(cpu, memory);

	// cpu->P must always have the U_FLAG set, but existing value of B_FLAG is not modified.
	if (status_flag_is_set(cpu, B_FLAG)) {
		val |= B_FLAG;
	} else {
		val &= ~(B_FLAG);
	}
	cpu->P = val | U_FLAG;

	return 4;
}

uint8_t and_imm(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = imm(cpu);
	and(addr, cpu, memory);

	return 2;
}

uint8_t rol_acc(struct cpu *cpu, struct memory *memory)
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
		set_status_flag(cpu, C_FLAG);
	} else {
		clear_status_flag(cpu, C_FLAG);
	}

	cpu->A = result;

	CPU_set_negative_flag_for_value(cpu, result);
	CPU_set_zero_flag_for_value(cpu, result);

	return 2;
}

uint8_t bit_abs(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu, memory);
	bit(addr, cpu, memory);

	return 4;
}

uint8_t and_abs(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu, memory);
	and(addr, cpu, memory);

	return 4;
}

uint8_t rol_abs(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu, memory);
	rol(addr, cpu, memory);

	return 6;
}

/*
 * Branch when negative
 */
uint8_t bmi_r(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	/* Offset must ba handled as a signed number */
	int8_t offset = (int8_t)CPU_pop8_mem(cpu, memory);

	if(CPU_negative_flag_is_set(cpu) == 1) {
		cpu->PC += offset;
		return 3;
	}
	return 2;
}

uint8_t and_ind_y(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = ind_y(cpu, memory);
	and(addr, cpu, memory);

	return 5;
}

uint8_t and_zero_pg_x(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = zero_pg_x(cpu, memory);
	and(addr, cpu, memory);

	return 4;
}

uint8_t rol_zero_pg_x(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = zero_pg_x(cpu, memory);
	rol(addr, cpu, memory);

	return 6;
}

uint8_t sec(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	set_status_flag(cpu, C_FLAG);

	return 2;
}

uint8_t and_abs_y(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = abs_y(cpu, memory);
	and(addr, cpu, memory);

	return 4;
}

uint8_t and_abs_x(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = abs_x(cpu, memory);
	and(addr, cpu, memory);

	return 4;
}

uint8_t rol_abs_x(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = abs_x(cpu, memory);
	rol(addr, cpu, memory);

	return 7;
}

uint8_t rti(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	// Unused flag should always be set when loading status flags from
	// memory.  Break flag should stay at existing value.
	uint8_t val = CPU_pop8_stack(cpu, memory) | U_FLAG;
	if (status_flag_is_set(cpu, B_FLAG)) {
		val |= B_FLAG;
	} else {
		val &= ~(B_FLAG);
	}

	cpu->P = val;
	cpu->PC = CPU_pop16_stack(cpu, memory);

	return 6;
}

uint8_t eor_ind_x(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = ind_x(cpu, memory);
	eor(addr, cpu, memory);

	return 6;
}

uint8_t eor_zero_pg(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu, memory);
	eor(addr, cpu, memory);

	return 3;
}


uint8_t lsr_zero_pg(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu, memory);
	lsr(addr, cpu, memory);

	return 5;
}

uint8_t pha(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	CPU_push8_stack(cpu, memory, cpu->A);

	return 3;
}

uint8_t eor_imm(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = imm(cpu);
	eor(addr, cpu, memory);

	return 2;
}

uint8_t lsr_acc(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint8_t val = cpu->A;
	uint8_t result = val>>1;

	/* shift old bit 0 into the carry flag */
	if(low_bit_is_set(val))
	{
		set_status_flag(cpu, C_FLAG);
	} else {
		clear_status_flag(cpu, C_FLAG);
	}

	CPU_set_zero_flag_for_value(cpu, result);
	CPU_set_negative_flag_for_value(cpu, result);

	cpu->A = result;

	return 2;
}

uint8_t jmp_abs(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu, memory);
	jmp(addr, cpu);

	return 3;
}

uint8_t eor_abs(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu, memory);
	eor(addr, cpu, memory);

	return 4;
}

uint8_t lsr_abs(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu, memory);
	lsr(addr, cpu, memory);

	return 6;
}

/*
 * If overflow flag is clear, add relative displacement to cpu->PC
 */
uint8_t bvc_r(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	/* Offset must be handled as a signed number */
	int8_t offset = (int8_t)CPU_pop8_mem(cpu, memory);

	if(CPU_overflow_flag_is_set(cpu) == 0) {
		cpu->PC += offset;
		return 3;
	}
	return 2;
}

uint8_t eor_ind_y(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint8_t addr = ind_y(cpu, memory);
	eor(addr, cpu, memory);

	return 5;
}

uint8_t eor_zero_pg_x(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint8_t addr = zero_pg_x(cpu, memory);
	eor(addr, cpu, memory);

	return 4;
}

uint8_t lsr_zero_pg_x(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint8_t addr = zero_pg_x(cpu, memory);
	lsr(addr, cpu, memory);

	return 6;
}

uint8_t cli(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	clear_status_flag(cpu, I_FLAG);

	return 2;
}

uint8_t eor_abs_y(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = abs_y(cpu, memory);
	eor(addr, cpu, memory);

	return 4;
}

uint8_t eor_abs_x(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = abs_x(cpu, memory);
	eor(addr, cpu, memory);

	return 4;
}

uint8_t lsr_abs_x(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = abs_x(cpu, memory);
	lsr(addr, cpu, memory);

	return 7;
}

uint8_t rts(struct cpu *cpu, struct memory *memory)
{
	cpu->PC = CPU_pop16_stack(cpu, memory) + 1;

	return 6;
}

uint8_t adc_ind_x(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = ind_x(cpu, memory);
	adc(addr, cpu, memory);

	return 6;
}

uint8_t adc_zero_pg(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu, memory);
	adc(addr, cpu, memory);

	return 3;
}

uint8_t ror_zero_pg(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu, memory);
	ror(addr, cpu, memory);

	return 5;
}

uint8_t pla(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint8_t val = CPU_pop8_stack(cpu, memory);
	cpu->A = val;

	CPU_set_zero_flag_for_value(cpu, val);
	CPU_set_negative_flag_for_value(cpu, val);

	return 4;
}

uint8_t adc_imm(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = imm(cpu);
	adc(addr, cpu, memory);

	return 2;
}

uint8_t ror_acc(struct cpu *cpu, struct memory *memory)
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
		set_status_flag(cpu, C_FLAG);
	} else {
		clear_status_flag(cpu, C_FLAG);
	}

	cpu->A = result;
	CPU_set_negative_flag_for_value(cpu, result);
	CPU_set_zero_flag_for_value(cpu, result);

	return 2;
}

uint8_t jmp_ind(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = ind(cpu, memory);
	jmp(addr, cpu);

	return 5;
}

uint8_t adc_abs(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu, memory);
	adc(addr, cpu, memory);

	return 4;
}

uint8_t ror_abs(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu, memory);
	ror(addr, cpu, memory);

	return 6;
}

uint8_t bvs_r(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	/* Offset must be handles as a signed number */
	int8_t offset = (int8_t)CPU_pop8_mem(cpu, memory);

	if(CPU_overflow_flag_is_set(cpu) == 1) {
		cpu->PC += offset;
		return 3;
	}
	return 2;
}

uint8_t adc_ind_y(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = ind_y(cpu, memory);
	adc(addr, cpu, memory);

	return 2;
}

uint8_t adc_zero_pg_x(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = zero_pg_x(cpu, memory);
	adc(addr, cpu, memory);

	return 4;
}

uint8_t ror_zero_pg_x(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = zero_pg_x(cpu, memory);
	ror(addr, cpu, memory);

	return 6;
}

uint8_t sei(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	set_status_flag(cpu, I_FLAG);

	return 2;
}

uint8_t adc_abs_y(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = abs_y(cpu, memory);
	adc(addr, cpu, memory);

	return 4;
}

uint8_t adc_abs_x(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = abs_x(cpu, memory);
	adc(addr, cpu, memory);

	return 4;
}

uint8_t ror_abs_x(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = abs_x(cpu, memory);
	ror(addr, cpu, memory);

	return 7;
}

uint8_t sta_ind_x(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = ind_x(cpu, memory);
	sta(addr, cpu, memory);

	return 6;
}

uint8_t sty_zero_pg(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu, memory);
	sty(addr, cpu, memory);

	return 3;
}

uint8_t sta_zero_pg(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu, memory);
	sta(addr, cpu, memory);

	return 3;
}

uint8_t stx_zero_pg(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu, memory);
	stx(addr, cpu, memory);

	return 3;
}

uint8_t dey(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	cpu->Y--;

	CPU_set_zero_flag_for_value(cpu, cpu->Y);
	CPU_set_negative_flag_for_value(cpu, cpu->Y);

	return 2;
}

uint8_t txa(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	cpu->A = cpu->X;

	CPU_set_zero_flag_for_value(cpu, cpu->A);
	CPU_set_negative_flag_for_value(cpu, cpu->A);

	return 2;
}

uint8_t sty_abs(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu, memory);
	sty(addr, cpu, memory);

	return 4;
}

uint8_t sta_abs(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu, memory);
	sta(addr, cpu, memory);

	return 4;
}

uint8_t stx_abs(struct cpu * cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu, memory);
	stx(addr, cpu, memory);

	return 4;
}

uint8_t bcc_r(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	/* Offset must be treated as a signed number */
	int8_t offset = (int8_t)CPU_pop8_mem(cpu, memory);

	if(CPU_carry_flag_is_set(cpu) == 0) {
		cpu->PC += offset;
		return 3;
	}
	return 2;
}

uint8_t sta_ind_y(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = ind_y(cpu, memory);
	sta(addr, cpu, memory);

	return 6;
}

uint8_t sty_zero_pg_x(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = zero_pg_x(cpu, memory);
	sty(addr, cpu, memory);

	return 4;
}

uint8_t sta_zero_pg_x(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = zero_pg_x(cpu, memory);
	sta(addr, cpu, memory);

	return 4;
}

uint8_t stx_zero_pg_y(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = zero_pg_y(cpu, memory);
	stx(addr, cpu, memory);

	return 4;
}

uint8_t tya(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	cpu->A = cpu->Y;

	CPU_set_zero_flag_for_value(cpu, cpu->A);
	CPU_set_negative_flag_for_value(cpu, cpu->A);

	return 2;
}

uint8_t sta_abs_y(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = abs_y(cpu, memory);
	sta(addr, cpu, memory);

	return 5;
}

uint8_t txs(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	cpu->S = cpu->X + MEM_STACK_OFFSET;

	return 2;
}

uint8_t sta_abs_x(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = abs_x(cpu, memory);
	sta(addr, cpu, memory);

	return 5;
}

uint8_t ldy_imm(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = imm(cpu);
	ldy(addr, cpu, memory);

	return 2;
}

uint8_t lda_ind_x(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = ind_x(cpu, memory);
	lda(addr, cpu, memory);

	return 6;
}

uint8_t ldx_imm(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = imm(cpu);
	ldx(addr, cpu, memory);

	return 2;
}

uint8_t ldy_zero_pg(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu, memory);
	ldy(addr, cpu, memory);

	return 3;
}

uint8_t lda_zero_pg(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu, memory);
	lda(addr, cpu, memory);

	return 3;
}

uint8_t ldx_zero_pg(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu, memory);
	ldx(addr, cpu, memory);

	return 3;
}

uint8_t tay(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	cpu->Y = cpu->A;

	CPU_set_zero_flag_for_value(cpu, cpu->A);
	CPU_set_negative_flag_for_value(cpu, cpu->A);

	return 2;
}

uint8_t lda_imm(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = imm(cpu);
	lda(addr, cpu, memory);

	return 2;
}

uint8_t tax(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	cpu->X = cpu->A;

	CPU_set_zero_flag_for_value(cpu, cpu->A);
	CPU_set_negative_flag_for_value(cpu, cpu->A);

	return 2;
}

uint8_t ldy_abs(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu, memory);
	ldy(addr, cpu, memory);

	return 4;
}

uint8_t lda_abs(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu, memory);
	lda(addr, cpu, memory);

	return 4;
}

uint8_t ldx_abs(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu, memory);
	ldx(addr, cpu, memory);

	return 4;
}

uint8_t bcs_r(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	/* Offset must be treated as a signed number */
	int8_t offset = (int8_t)CPU_pop8_mem(cpu, memory);

	if(CPU_carry_flag_is_set(cpu) == 1) {
		cpu->PC += offset;
		return 3;
	}
	return 2;
}

uint8_t lda_ind_y(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = ind_y(cpu, memory);
	lda(addr, cpu, memory);

	return 5;
}

uint8_t ldy_zero_pg_x(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = zero_pg_x(cpu, memory);
	ldy(addr, cpu, memory);

	return 4;
}

uint8_t lda_zero_pg_x(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr =zero_pg_x(cpu, memory);
	lda(addr, cpu, memory);

	return 4;
}

uint8_t ldx_zero_pg_y(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = zero_pg_y(cpu, memory);
	ldx(addr, cpu, memory);

	return 4;
}

uint8_t clv(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	clear_status_flag(cpu, V_FLAG);

	return 2;
}

uint8_t lda_abs_y(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = abs_y(cpu, memory);
	lda(addr, cpu, memory);

	return 4;
}

uint8_t tsx(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	cpu->X = cpu->S - MEM_STACK_OFFSET;

	CPU_set_negative_flag_for_value(cpu, cpu->X);
	CPU_set_zero_flag_for_value(cpu, cpu->X);

	return 2;
}

uint8_t ldy_abs_x(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = abs_x(cpu, memory);
	ldy(addr, cpu, memory);

	return 4;
}

uint8_t lda_abs_x(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = abs_x(cpu, memory);
	lda(addr, cpu, memory);

	return 4;
}

uint8_t ldx_abs_y(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = abs_y(cpu, memory);
	ldx(addr, cpu, memory);

	return 4;
}

uint8_t cpy_imm(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = imm(cpu);
	cpy(addr, cpu, memory);

	return 2;
}

uint8_t cmp_ind_x(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = ind_x(cpu, memory);
	cmp(addr, cpu, memory);

	return 6;
}

uint8_t cpy_zero_pg(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu, memory);
	cpy(addr, cpu, memory);

	return 3;
}

uint8_t cmp_zero_pg(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu, memory);
	cmp(addr, cpu, memory);

	return 3;
}

uint8_t dec_zero_pg(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu, memory);
	dec(addr, cpu, memory);

	return 5;
}

uint8_t iny(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	cpu->Y++;
	CPU_set_zero_flag_for_value(cpu, cpu->Y);
	CPU_set_negative_flag_for_value(cpu, cpu->Y);

	return 2;
}

uint8_t cmp_imm(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = imm(cpu);
	cmp(addr, cpu, memory);

	return 2;
}

uint8_t dex(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	cpu->X--;
	CPU_set_zero_flag_for_value(cpu, cpu->X);
	CPU_set_negative_flag_for_value(cpu, cpu->X);

	return 2;
}

uint8_t cpy_abs(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu, memory);
	cpy(addr, cpu, memory);

	return 4;
}

uint8_t cmp_abs(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu, memory);
	cmp(addr, cpu, memory);

	return 4;
}

uint8_t dec_abs(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu, memory);
	dec(addr, cpu, memory);

	return 6;
}

uint8_t bne_r(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	/* Offset must be treated as a signed number */
	int8_t offset = (int8_t)CPU_pop8_mem(cpu, memory);

	if(CPU_zero_flag_is_set(cpu) == 0) {
		cpu->PC += offset;
		return 3;
	}
	return 2;
}

uint8_t cmp_ind_y(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = ind_y(cpu, memory);
	cmp(addr, cpu, memory);

	return 5;
}

uint8_t cmp_zero_pg_x(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = zero_pg_x(cpu, memory);
	cmp(addr, cpu, memory);

	return 4;
}

uint8_t dec_zero_pg_x(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = zero_pg_x(cpu, memory);
	dec(addr, cpu, memory);

	return 6;
}

uint8_t cld(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	clear_status_flag(cpu, D_FLAG);

	return 2;
}

uint8_t cmp_abs_y(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = abs_y(cpu, memory);
	cmp(addr, cpu, memory);

	return 4;
}

uint8_t cmp_abs_x(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = abs_x(cpu, memory);
	cmp(addr, cpu, memory);

	return 4;
}

uint8_t dec_abs_x(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = abs_x(cpu, memory);
	dec(addr, cpu, memory);

	return 7;
}

uint8_t cpx_imm(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = imm(cpu);
	cpx(addr, cpu, memory);

	return 2;
}

uint8_t sbc_ind_x(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = ind_x(cpu, memory);
	sbc(addr, cpu, memory);

	return 6;
}

uint8_t cpx_zero_pg(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu, memory);
	cpx(addr, cpu, memory);

	return 3;
}

uint8_t sbc_zero_pg(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu, memory);
	sbc(addr, cpu, memory);

	return 3;
}

uint8_t inc_zero_pg(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = zero_pg(cpu, memory);
	inc(addr, cpu, memory);

	return 5;
}

uint8_t inx(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	cpu->X++;
	CPU_set_zero_flag_for_value(cpu, cpu->X);
	CPU_set_negative_flag_for_value(cpu, cpu->X);

	return 2;
}

uint8_t sbc_imm(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = imm(cpu);
	sbc(addr, cpu, memory);

	return 2;
}

uint8_t nop(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	return 2;
}

uint8_t cpx_abs(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu, memory);
	cpx(addr, cpu, memory);

	return 4;
}

uint8_t sbc_abs(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu, memory);
	sbc(addr, cpu, memory);

	return 4;
}

uint8_t inc_abs(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = abs_(cpu, memory);
	inc(addr, cpu, memory);

	return 6;
}

uint8_t beq_r(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	/* Offset must be treated as a signed number */
	int8_t offset = (int8_t)CPU_pop8_mem(cpu, memory);

	if(CPU_zero_flag_is_set(cpu) == 1) {
		cpu->PC += offset;
		return 3;
	}
	return 2;
}

uint8_t sbc_ind_y(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = ind_y(cpu, memory);
	sbc(addr, cpu, memory);

	return 5;
}

uint8_t sbc_zero_pg_x(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = zero_pg_x(cpu, memory);
	sbc(addr, cpu, memory);

	return 4;
}

uint8_t inc_zero_pg_x(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = zero_pg_x(cpu, memory);
	inc(addr, cpu, memory);

	return 6;
}

uint8_t sed(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	set_status_flag(cpu, D_FLAG);

	return 2;
}

uint8_t sbc_abs_y(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = abs_y(cpu, memory);
	sbc(addr, cpu, memory);

	return 4;
}

uint8_t sbc_abs_x(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = abs_x(cpu, memory);
	sbc(addr, cpu, memory);

	return 4;
}

uint8_t inc_abs_x(struct cpu *cpu, struct memory *memory)
{
	cpu->PC++;
	uint16_t addr = abs_x(cpu, memory);
	inc(addr, cpu, memory);

	return 7;
}
/* Array of function pointers to opcode instructions */
/* codes 0x00 to 0xFF  */
static uint8_t (* const pf[]) (struct cpu *, struct memory *) = {
/* 0x00 */	&brk, &ora_ind_x, NULL, NULL, NULL, &ora_zero_pg, &asl_zero_pg, NULL, &php, &ora_imm, &asl_acc, NULL, NULL, &ora_abs, &asl_abs, NULL,
/* 0x10 */	&bpl_r, &ora_ind_y, NULL, NULL, NULL, &ora_zero_pg_x, &asl_zero_pg_x, NULL, &clc, &ora_abs_y, NULL, NULL, NULL, &ora_abs_x, &asl_abs_x, NULL,
/* 0x20 */	&jsr_abs, &and_ind_x, NULL, NULL, &bit_zero_pg, &and_zero_pg, &rol_zero_pg, NULL, &plp, &and_imm, &rol_acc, NULL, &bit_abs, &and_abs, &rol_abs, NULL,
/* 0x30 */	&bmi_r, &and_ind_y, NULL, NULL, NULL, &and_zero_pg_x, &rol_zero_pg_x, NULL, &sec, &and_abs_y, NULL, NULL, NULL, &and_abs_x, &rol_abs_x, NULL,
/* 0x40 */	&rti, &eor_ind_x, NULL, NULL, NULL, &eor_zero_pg, &lsr_zero_pg, NULL, &pha, &eor_imm, &lsr_acc, NULL, &jmp_abs, &eor_abs, &lsr_abs, NULL,
/* 0x50 */	&bvc_r, &eor_ind_y, NULL, NULL, NULL, &eor_zero_pg_x, &lsr_zero_pg_x, NULL, &cli, &eor_abs_y, NULL, NULL, NULL, &eor_abs_x, &lsr_abs_x, NULL,
/* 0x60 */	&rts, &adc_ind_x, NULL, NULL, NULL, &adc_zero_pg, &ror_zero_pg, NULL, &pla, &adc_imm, &ror_acc, NULL, &jmp_ind, &adc_abs, &ror_abs, NULL,
/* 0x70 */	&bvs_r, &adc_ind_y, NULL, NULL, NULL, &adc_zero_pg_x, &ror_zero_pg_x, NULL, &sei, &adc_abs_y, NULL, NULL, NULL, &adc_abs_x, &ror_abs_x, NULL,
/* 0x80 */	NULL, &sta_ind_x, NULL, NULL, &sty_zero_pg, &sta_zero_pg, &stx_zero_pg, NULL, &dey, NULL, &txa, NULL, &sty_abs, &sta_abs, &stx_abs, NULL,
/* 0x90 */	&bcc_r, &sta_ind_y, NULL, NULL, &sty_zero_pg_x, &sta_zero_pg_x, &stx_zero_pg_y, NULL, &tya, &sta_abs_y, &txs, NULL, NULL, &sta_abs_x, NULL, NULL,
/* 0xA0 */	&ldy_imm, &lda_ind_x, &ldx_imm, NULL, &ldy_zero_pg, &lda_zero_pg, &ldx_zero_pg, NULL, &tay, &lda_imm, &tax, NULL, &ldy_abs, &lda_abs, &ldx_abs, NULL,
/* 0xB0 */	&bcs_r, &lda_ind_y, NULL, NULL, &ldy_zero_pg_x, &lda_zero_pg_x, &ldx_zero_pg_y, NULL, &clv, &lda_abs_y, &tsx, NULL, &ldy_abs_x, &lda_abs_x, &ldx_abs_y, NULL,
/* 0xC0 */	&cpy_imm, &cmp_ind_x, NULL, NULL, &cpy_zero_pg, &cmp_zero_pg, &dec_zero_pg, NULL, &iny, &cmp_imm, &dex, NULL, &cpy_abs, &cmp_abs, &dec_abs, NULL,
/* 0xD0 */	&bne_r, &cmp_ind_y, NULL, NULL, NULL, &cmp_zero_pg_x, &dec_zero_pg_x, NULL, &cld, &cmp_abs_y, NULL, NULL, NULL, &cmp_abs_x, &dec_abs_x, NULL,
/* 0xE0 */	&cpx_imm, &sbc_ind_x, NULL, NULL, &cpx_zero_pg, &sbc_zero_pg, &inc_zero_pg, NULL, &inx, &sbc_imm, &nop, NULL, &cpx_abs, &sbc_abs, &inc_abs, NULL,
/* 0xF0 */	&beq_r, &sbc_ind_y, NULL, NULL, NULL, &sbc_zero_pg_x, &inc_zero_pg_x, NULL, &sed, &sbc_abs_y, NULL, NULL, NULL, &sbc_abs_x, &inc_abs_x, NULL
};

/* Public functions */

struct cpu *CPU_init(struct memory *memory)
{
	struct cpu *cpu = malloc(sizeof(struct cpu));

	/* initialize PC to 2-byte address at the reset vector */
	uint16_t low = MEM_read(memory, MEM_RESET_VECTOR);
	uint16_t high = MEM_read(memory, MEM_RESET_VECTOR + 1);
	uint16_t addr = (high<<8) | low;
	cpu->PC = addr;
	(void)printf("Initializing PC to %#x\n", addr);

	/* Stack grows down from 0x1FF, but is decremented to 0x1FD on power up. */
	cpu->S = MEM_STACK_START - 2;
	cpu->A = 0;
	cpu->X = 0;
	cpu->Y = 0;
	// break flag is not set on init
	cpu->P = 0x24;

	return cpu;
}

struct cpu *CPU_init_to_address(struct memory *memory, uint16_t addr)
{
	struct cpu *cpu = CPU_init(memory);
	cpu->PC = addr;
	(void)printf("Reinitializing PC to %#x\n", addr);

	return cpu;
}

void CPU_delete(struct cpu **cpu)
{
	free(*cpu);
	*cpu = NULL;
}

int CPU_step(struct cpu *cpu, struct memory *memory)
{
	/* Get opcode at PC */
	uint8_t opcode = MEM_read(memory, cpu->PC);
#if (defined DEBUG)
	(void)printf("%x  %02x A:%02x X:%02x Y:%02x P:%02x SP:%03x\n", cpu->PC, opcode, cpu->A, cpu->X, cpu->Y, cpu->P, cpu->S);
#endif
	return pf[opcode](cpu, memory);
}

/*
 * NMI handler does 3 things:
 * 1. Push CPU status reg onto the stack, with the blank flag cleared
 * 2. Push CPU return addr onto the stack
 * 3. Set the PC to the NMI handler's address at 0xFFFA - 0XFFFB
 */
void CPU_handle_nmi(struct cpu *cpu, struct memory *memory)
{
	/* 1 */
	CPU_push8_stack(cpu, memory, cpu->P & ~B_FLAG);
	/* 2 */
	CPU_push16_stack(cpu, memory, cpu->PC);
	/* 3 */
	uint16_t low = MEM_read(memory, MEM_NMI_VECTOR);
	uint16_t high = MEM_read(memory, MEM_NMI_VECTOR + 1);
	uint16_t addr = (high<<8) | low;
	cpu->PC = addr;
	(void)printf("NMI set PC to %#x\n", addr);
}
