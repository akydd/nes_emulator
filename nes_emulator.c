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
void set_overflow_flag(uint8_t);
void set_break_flag();
void set_interrupt_flag();
/* instructions */
void brk();
void ora_ind_x();
void ora_zero_pg();
void asl_zero_pg();
void php();
void ora_imm();
void asl_A();
void ora_a();
void asl_a();
void bpl_r();
void ora_ind_y();
void ora_zero_pg_x();
void asl_zero_pg_x();
void clc();
void ora_abs_y();
void ora_abs_x();
void asl_abs_x();
/* TODO: 16 single byte opcodes
void plp();
void sec();
void pha();
void cli();
void pla();
void sei();
void dey();
void tya();
void tay();
void clv();
void iny();
void cld();
void inx();
void sed();
TODO: 6 single-byte opcodes
void txa();
void txs();
void tax();
void tsx();
void dex();
void nop();
TODO: branch instructions
void bmi();
void bvc();
void bvs();
void bcc();
void bcs();
void bne();
void beq();
TODO: other instructions
void jsr_abs();
void rti();
void rts();
*/

#define MEM_SIZE 2 * 1024

/* 2 kilobytes of memory */
uint8_t memory[MEM_SIZE];	/* zero page: [0..255] */
				/* stack: [256..511] */

/* registers, all unsigned */
uint16_t PC = 0;	/* program counter */
uint16_t S = 511;	/* stack pointer grows down from 0x1FF, or 511 */
uint8_t A = 0;		/* accumulator */
uint8_t X = 0;		/* X index */
uint8_t Y = 0;		/* Y index */
uint8_t P = 0;		/* processor status flags */

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

int main(void)
{
	return 0;
}

void process_code(uint8_t code)
{
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
	 * A			A		Accumulator
	 * a			a		Absolute
	 * r			r		Relative
	 * ind_y		(d), Y		(Indirect), Y
	 * zero_pg_x		d, X		Zero Page, X
	 * abs_y		a, Y		Absolute, Y
	 * abs_x		a, X		Absolute, X
	 * ind			(a)		Indirect
	 *
	 */

	/* codes 0x00 to 0xFF  */
	static void (* const pf[]) (void) = {
		&brk, &ora_ind_x, NULL, NULL, NULL, &ora_zero_pg, &asl_zero_pg, NULL, &php, &ora_imm, &asl_A, NULL, NULL, &ora_a, &asl_a, NULL,
		&bpl_r, &ora_ind_y, NULL, NULL, NULL, &ora_zero_pg_x, &asl_zero_pg_x, NULL, &clc, &ora_abs_y, NULL, NULL, NULL, &ora_abs_x, &asl_abs_x, NULL,
		&jsr_a, &and_ind_x, NULL, NULL, &bit_zero_pg, &and_zero_pg, &rol_zero_pg, NULL, &plp, &and_imm, &rol_A, NULL, &bit_a, &and_a, &rol_a, NULL,
		&bmi_r, &and_ind_y, NULL, NULL, NULL, &and_zero_pg_x, &rol_zero_pg_x, NULL, &sec, &and_abs_y, NULL, NULL, NULL, &and_abs_x, &rol_abs_x, NULL,
		&rti, &eor_ind_x, NULL, NULL, NULL, &eor_zero_pg, &lsr_zero_pg, NULL, &pha, &eor_imm, &lsr_A, NULL, &jmp_a, &eor_a, &lsr_a, NULL,
		&bvc_r, &eor_ind_y, NULL, NULL, NULL, &eor_zero_pg_x, &lsr_zero_pg_x, NULL, &cli, &eor_abs_y, NULL, NULL, NULL, &eor_abs_x, &lsr_abs_x, NULL,
		&rts, &adc_ind_x, NULL, NULL, NULL, &adc_zero_pg, &ror_zero_pg, NULL, &pla, &adc_imm, &ror_A, NULL, &jmp_ind, &adc_a, &ror_a, NULL,
		&bvs_r, &adc_ind_y, NULL, NULL, NULL, &adc_zero_pg_x, &ror_zero_pg_x, NULL, &sei, &adc_abs_y, NULL, NULL, NULL, &adc_abs_x, &ror_abs_x, NULL,
		NULL, &sta_ind_x, NULL, NULL, &sty_zero_pg, &sta_zero_pg, &stx_zero_pg, NULL, &dey, NULL, &txa, NULL, &sty_a, &sta_a, &stx_a, NULL,
		&bcc_r, &sta_ind_y, NULL, NULL, &sty_zero_pg_x, &sta_zero_pg_x, &stx_zero_pg_x, NULL, &tya, &sta_abs_y, &txs, NULL, NULL, &sta_abs_x, NULL, NULL,
		&ldy_imm, &lda_ind_x, &ldx_imm, NULL, &ldy_zero_pg, &lda_zero_pg, &ldx_zero_pg, NULL, &tay, &lda_imm, &tax, NULL, &ldy_a, &lda_a, &ldx_a, NULL,
		&bcs_r, &lda_ind_y, NULL, NULL, &ldy_zero_pg_x, &lda_zero_pg_x, &ldx_zero_pg_y, NULL, &clv, &lda_abs_y, &tsx, NULL, &ldy_abs_x, &lda_abs_x, &ldx_abs_y, NULL,
		&cpy_imm, &cmp_ind_x, NULL, NULL, &cpy_zero_pg, &cmp_zero_pg, &dec_zero_pg, NULL, &iny, &cmp_imm, &dex, NULL, &cpy_a, &cmp_a, &dec_a, NULL,
		&bne_r, &cmp_ind_y, NULL, NULL, NULL, &cmp_zero_pg_x, &dec_zero_pg_x, NULL, &cld, &cmp_abs_y, NULL, NULL, NULL< &cmp_abs_x, &dec_abs_x, NULL,
	       	&cpx_imm, sbc_ind_x, NULL, NULL, &cpx_zero_pg, &sbc_zero_pg, &inc_zero_pg, NULL, &inx, &sbc_imm, &nop, NULL, &cpx_a, &sbc_a, &inc_a, NULL,
		&beq_r, &sbc_ind_y, NULL, NULL, NULL, &sbc_zero_pg_x, &inc_zero_pg_x, NULL, &sed, &sbc_abs_y, NULL, NULL, NULL, &sbc_abs_x, &inc_abs_x, NULL
	};

	pf[code];
}

/* 
 * break: Set the break flag, push high byte of PC onto the stack, push the low
 * byte of PC onto the stack, push the status flags on the stack, then address
 * $FFFE/$FFFF is loaded into the PC. BRK is really a two-byte instruction.
 */
void brk()
{
	PC += 2;
	set_break_flag();
	uint8_t PC_low_byte = PC;
	memory[S] = PC>>8;
	S--;
	memory[S] = PC_low_byte;
	S--;
	memory[S] = P;
	S--;
}

/*
 * bitwise or of operand and the accumulator, stored in accumulator
 */
void ora_ind_x()
{
	PC++;
	uint16_t low = memory[PC + X];
	uint16_t high = memory[PC + X + 1]<<8;
	PC++;
	uint16_t addr = high | low;

	uint8_t val = memory[addr];
	A |= val;

	set_negative_flag(A);
	set_zero_flag(A);
}

void ora_zero_pg()
{
	PC++;
	uint16_t addr = memory[PC];
	PC++;

	uint8_t val = memory[addr];
	A |= val;

	set_negative_flag(A);
	set_zero_flag(A);
}

/*
 * shift bits to the left, pushing in 0.
 */
void asl_zero_pg()
{
	PC++;
	uint16_t addr = memory[PC];
	PC++;

	uint8_t val = memory[addr];
	uint8_t new_val = val<<1;
	memory[addr] = new_val;

	set_zero_flag(new_val);
	set_negative_flag(new_val);
	if((val & N_FLAG) == N_FLAG) {
		P |= C_FLAG;
	}
}

/*
 * Push processor status flags onto the stack
 */
void php()
{
	PC++;
	memory[S] = P;
	S--;
}

void ora_imm()
{
	PC++;
	uint8_t val = memory[PC];
	PC++;

	A |= val;
	set_negative_flag(A);
	set_zero_flag(A);
}

void asl_A()
{
	PC++;

	uint8_t val = A;
	uint8_t new_val = val<<1;
	A = new_val;

	set_zero_flag(new_val);
	set_negative_flag(new_val);
	if((val & N_FLAG) == N_FLAG) {
		P |= C_FLAG;
	}
}

void ora_a()
{
	PC++;
	uint16_t low = memory[PC];
	PC++;
	uint16_t high = memory[PC];
	uint16_t addr = (high<<8) | low;
	uint8_t val = memory[addr];
	PC++;

	A |= val;
	set_negative_flag(A);
	set_zero_flag(A);
}

void asl_a()
{
	PC++;
	uint16_t low = memory[PC];
	PC++;
	uint16_t high = memory[PC];
	uint16_t addr = (high<<8) | low;
	uint8_t val = memory[addr];
	PC++;

	uint8_t new_val = val<<1;
	memory[addr] = new_val;

	set_zero_flag(new_val);
	set_negative_flag(new_val);
	if((val & N_FLAG) == N_FLAG) {
		P |= C_FLAG;
	}
}

/*
 * If negative flag is clear, add relative displacement to PC
 */
void bpl_r()
{
	PC++;
	int offset = memory[PC];
	PC++;

	if((P & N_FLAG) != N_FLAG) {
		PC += offset;
	}
}

void ora_ind_y()
{
	PC++;
	uint16_t low = memory[PC];
	PC++;
	uint16_t addr = ((memory[low + 1]<<8) | memory[low]) + Y;

	uint8_t val = memory[addr];
	A |= val;

	set_negative_flag(A);
	set_zero_flag(A);
}

void ora_zero_pg_x()
{
	PC++;
	uint16_t addr = memory[PC] + X;
	PC++;
	uint8_t val = memory[addr];
	A |= val;

	set_negative_flag(A);
	set_zero_flag(A);
}

void asl_zero_pg_x()
{
	PC++;
	uint16_t addr = memory[PC] + X;
	PC++;
	uint8_t val = memory[addr];

	uint8_t new_val = val<<1;
	memory[addr] = new_val;

	set_zero_flag(new_val);
	set_negative_flag(new_val);
	if((val & N_FLAG) == N_FLAG) {
		P |= C_FLAG;
	}
}

/*
 * Clear carry flag
 */
void clc()
{
	P &= ~C_FLAG;
	PC++;
}

void ora_abs_y()
{
	PC++;
	uint16_t low = memory[PC];
	PC++;
	uint16_t high = memory[PC]<<8;
	PC++;
	uint16_t addr = (high | low) + Y;

	uint8_t val = memory[addr];
	A |= val;

	set_negative_flag(A);
	set_zero_flag(A);
}

void ora_abs_x()
{
	PC++;
	uint16_t low = memory[PC];
	PC++;
	uint16_t high = memory[PC]<<8;
	PC++;
	uint16_t addr = (high | low) + X;

	uint8_t val = memory[addr];
	A |= val;

	set_negative_flag(A);
	set_zero_flag(A);
}

void asl_abs_x()
{
	PC++;
	uint16_t addr = memory[PC] + X;
	PC++;
	uint8_t val = memory[addr];

	uint8_t new_val = val<<1;
	memory[addr] = new_val;

	set_zero_flag(new_val);
	set_negative_flag(new_val);
	if((val & N_FLAG) == N_FLAG) {
		P |= C_FLAG;
	}
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
	if ((a & N_FLAG) == N_FLAG) {
		P |= N_FLAG;
	}
}

void set_overflow_flag(uint8_t a)
{
	if ((a & V_FLAG) == V_FLAG) {
		P |= V_FLAG;
	}
}

void set_break_flag()
{
	P |= B_FLAG;
}

void set_interrupt_flag()
{
	P |= I_FLAG;
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
