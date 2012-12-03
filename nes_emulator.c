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
/* processing instruction sets */
void process_code(uint8_t);
void process_00_code(uint8_t);
void process_01_code(uint8_t);
void process_10_code(uint8_t);
/* instructions */
void bit(uint8_t);
void jmp(uint8_t);
void jmp_abs(uint8_t);
void sty(uint8_t);
void ldy(uint8_t);
void cpy(uint8_t);
void cpx(uint8_t);
void nul(uint8_t);
void ora(uint8_t);
void and(uint8_t);
void eor(uint8_t);
void adc(uint8_t);
void sta(uint8_t);
void lda(uint8_t);
void cmp(uint8_t);
void sbc(uint8_t);
void asl(uint8_t);
void rol(uint8_t);
void lsr(uint8_t);
void ror(uint8_t);
void stx(uint8_t);
void ldx(uint8_t);
void dec(uint8_t);
void inc(uint8_t);
/* 16 single byte opcodes */
void php();
void clc();
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
/* 6 single-byte opcodes */
void txa();
void txs();
void tax();
void tsx();
void dex();
void nop();
/* branch instructions */
void bpl();
void bmi();
void bvc();
void bvs();
void bcc();
void bcs();
void bne();
void beq();
/* addressing modes */
uint16_t get_relative(void);
uint16_t get_accumulator(void);
uint16_t get_zero_page_indirect_index_y(void);
uint16_t get_zero(void);
uint16_t get_immediate(void);
uint16_t get_absolute(void);
uint16_t get_zero_page_indexed_indirect(void);
uint16_t get_zero_x(void);
uint16_t get_absolute_x(void);
uint16_t get_absolute_y(void);

#define MEM_SIZE 2 * 1024

/* 2 kilobytes of memory */
uint8_t memory[MEM_SIZE];	/* zero page: [0..255] */
				/* stack: [256..511] */

/* registers, all unsigned */
uint16_t PC = 0;	/* program counter */
uint16_t S = 256;	/* stack pointer */
uint8_t A = 0;		/* accumulator */
uint8_t X = 0;		/* X index */
uint8_t Y = 0;		/* Y index */
uint8_t P = 0;		/* processor status flags */

/* flags:
 * carry, zero, interrupt disable, decimal mode,
 * break, overflow, negative */
/* NV_BDIZC */
#define C_FLAG 1<<0
#define Z_FLAG 1<<1
#define I_FLAG 1<<2
#define D_FLAG 1<<3
#define B_FLAG 1<<4
#define V_FLAG 1<<6
#define N_FLAG 1<<7


int main(void)
{
	return 0;
}

/* function ptr array for memory access modes when cc = 01 */
static uint16_t (* const pf_01[]) (void) = {
	&get_zero_page_indexed_indirect,
	&get_zero,
	&get_immediate,
	&get_absolute,
	&get_zero_page_indirect_index_y,
	&get_zero_x,
	&get_absolute_y,
	&get_absolute_x
};

/* function ptr array for memory access modes when cc = 00 or cc = 10 */
static uint16_t (* const pf[]) (void) = {
	&get_immediate,
	&get_zero,
	&get_accumulator,
	&get_absolute,
	NULL,
	&get_zero_x,
	NULL,
	&get_absolute_x
};

void process_code(uint8_t code)
{
	/* Opcodes are endoded in one of the following bit sequences:
	 *
	 * aaabbbcc
	 * 	cc determines the instruction set
	 * 	aaa determines the instruction
	 * 	bbb determines the addressing mode
	 *
	 * dddd1000
	 * 	16 single-byte opcodes, indexed by dddd
	 *
	 * xxy10000
	 * 	These are conditional branch instructions
	 * 	xx indicates the flag
	 * 	y is compared with the above flag
	 *
	 * 1eee1010
	 * 	Another 6 single-byte opcodes, index by 0 <= eee <= 5
	 */

	/* 16 single-byte opcodes */
	static void (* const x8[]) (void) = {
		&php, &clc, &plp, &sec, &pha, &cli, &pla, &sei,
		&dey, &tya, &tay, &clv, &iny, &cld, &inx, &sed
	};

	/* 6 single-byte opcodes */
	static void (* const xa[]) (void) = {
		&txa, &txs, &tax, &tsx, &dex, &nop
	};

	/* branch instructions */
	static void (* const branch[]) (void) = {
		&bpl, &bmi, &bvc, &bvs,
		&bcc, &bcs, &bne, &beq
	};

	static void (* const pf[]) (uint8_t) = {
		&process_00_code, &process_01_code, &process_10_code
	};

	/* 
	 * Check for opcode format in the following order:
	 * 6 single-byte opcodes
	 * 16 single-byte opcodes
	 * Branch instructions
	 * All others
	 */
	if(((code & 0x0a) == 0x0a) && ((code>>4) <= 0x0d)
			&& ((code>>4) >= 0x08)) {
		/*
		 * The highest 4 bits take values [8..13].  We map them
		 * to [0..5].
		 */
		int index = (code>>4) - 8;
		xa[index]();
	} else if((code & 0x08) == 0x08) {
		/*
		 * The highest 4 bits map directly to [0..15].
		 */
		int index = code>>4;
		x8[index]();
	} else if((code & 0x10) == 0x10) {
		/*
		 * The highest 4 bits of code are odd numbers in [0..16].
		 * We map them to [0..7].
		 */
		int index = ((code>>4) - 1)/2;
		branch[index]();
	} else {
		/* 
		 * determine the instruction set.
		 * Cases for cc = 00, 01, or 10
		 */
		uint8_t cc = code & 0x03;
		if(cc < sizeof(pf) / sizeof(*pf)) {
			pf[cc](code);
		}
	}
}

/* 
 * Instructions in the 00 instruction set
 */
void process_00_code(uint8_t code)
{
	static void (* const i_00[]) (uint8_t) = {
		&nul, &bit, &jmp, &jmp_abs, &sty, &ldy, &cpy, &cpx
	};

	/* determine the instruction and addressing mode. */
	uint8_t aaa = (code>>5) & 0x07;
	uint8_t bbb = (code>>2) & 0x07;

	if(aaa < sizeof(i_00) / sizeof(*i_00)) {
		i_00[aaa](bbb);
	}
}

/*
 * Instructions in the 01 instruction set
 */
void process_01_code(uint8_t code)
{
	static void (* const i_01[]) (uint8_t) = {
		&ora, &and, &eor, &adc, &sta, &lda, &cmp, &sbc
	};

	/* determine the instruction and addressing mode. */
	uint8_t aaa = (code>>5) & 0x07;
	uint8_t bbb = (code>>2) & 0x07;

	if(aaa < sizeof(i_01) / sizeof(*i_01)) {
		i_01[aaa](bbb);
	}
}

/*
 * Instructions in the 10 instruction set
 */
void process_10_code(uint8_t code)
{
	static void (* const i_10[]) (uint8_t) = {
		&asl, &rol, &lsr, &ror, &stx, &ldx, &dec, &inc
	};

	/* determine the instruction and addressing mode. */
	uint8_t aaa = (code>>5) & 0x07;
	uint8_t bbb = (code>>2) & 0x07;

	if(aaa < sizeof(i_10) / sizeof(*i_10)) {
		i_10[aaa](bbb);
	}
}
/* 
 * common memory access functions for addressing modes.
 * This mode determines which value gets used in the computation, so these
 * functions can be reused for all instructions.
 */

/*
 * Relative mode converts the byte at the counter to a signed 8 bit int.
 */
uint16_t get_relative()
{
	int8_t offset = memory[PC];
	/* Had to do some trickery here, as I don't increment the PC
	 * properly. In the real CPU, the PC would already have been
	 * incremented past the instruction that uses relative addressing.
	 * That's why I add 1 to the return value. */
	int16_t addr = (int16_t)PC + offset;
	return (uint16_t)addr + 1;
}

/*
 * Accumulator mode return the value in the accumulator
 */
uint16_t get_accumulator()
{
	return (uint16_t)A;
}

/*
 * Immediate mode returns the operand after the instr
 */
uint16_t get_immediate()
{
	return PC;
}

/*
 * Absolute mode returns the 16-bit address at the PC
 */
uint16_t get_absolute()
{
	uint16_t addr = memory[PC];
	addr = addr << 8;
	PC++;
	addr |= memory[PC];

	return addr;
}

/*
 * Zero Page mode returns the address given by the operand, mod 0xFF.
 */
uint16_t get_zero()
{
	uint16_t addr = (uint16_t)(memory[PC] % 0xFF);
	return addr;
}

/*
 * Zero Page index with X mode returns the address given by the sum
 * of the operand and X, mod 0xFF.
 */
uint16_t get_zero_x()
{
	uint16_t addr = (uint16_t)((memory[PC] + X) % 0xFF);
	return addr;
}

/*
 * Zero Page index with Y mode returns the address given by the sum
 * of the operand and Y, mod 0xFF
 */
uint16_t get_zero_y()
{
	uint16_t addr = (uint16_t)((memory[PC] + Y) % 0xFF);
	return addr;
}

/*
 * Absolute index with X mode returns the address given by the
 * sum of X and the operand.
 */
uint16_t get_absolute_x()
{
	uint16_t addr = (uint16_t)memory[PC] + X;
	return addr;
}

/*
 * Absolute index with Y mode returns the address given by the
 * sum of Y and the operand.
 */
uint16_t get_absolute_y()
{
	uint16_t addr = (uint16_t)memory[PC] + Y;
	return addr;
}

/*
 * Zero Page indexed indirect mode returns the 2-byte address given
 * by the values of the memory at addresses (operand + X) and (operand + X + 1).
 */
uint16_t get_zero_page_indexed_indirect()
{
	uint16_t addr = (uint16_t)memory[PC + X];
	addr = addr << 8;
	addr |= memory[PC + X + 1];
	return addr;
}

/*
 * Zero Page indirect indexed with Y mode returns the 2-byte
 * address given by the ((value at the address of the operand) + Y) and ((value
 * at the address of the operand) + Y) + 1.
 */
uint16_t get_zero_page_indirect_index_y()
{
	uint16_t addr = (uint16_t)memory[PC] + Y;
	addr = addr << 8;
	addr |= memory[PC] + Y + 1;
	return addr;
}

uint8_t read_addr_mode_01(uint8_t mode)
{
	if(mode < sizeof(pf_01) / sizeof(*pf_01)) {
		return memory[pf_01[mode]()];
	}
	(void)printf("No such addressing mode for instruction set 01!\n");
	exit(EXIT_FAILURE);
}

uint8_t read_addr_mode_00(uint8_t mode)
{
	if(mode < sizeof(pf) / sizeof(*pf)) {
		return memory[pf[mode]()];
	}
	(void)printf("No such addressing mode for instruction set 00!\n");
	exit(EXIT_FAILURE);
}

void write_addr_mode_01(uint8_t mode, uint8_t byte)
{
	if(mode < sizeof(pf_01) / sizeof(*pf_01)) {
		memory[pf_01[mode]()] = byte;
	} else {
		(void)printf("No such addressing mode!\n");
		exit(EXIT_FAILURE);
	}
}

void write_addr_mode_00(uint8_t mode, uint8_t byte)
{
	if(mode < sizeof(pf) / sizeof(*pf)) {
		memory[pf[mode]()] = byte;
	} else {
		(void)printf("No such addressing mode!\n");
		exit(EXIT_FAILURE);
	}
}

void nul(uint8_t mode)
{
	(void)printf("No such code!\n");
}

/*
 * bitwise or of operand and the accumulator, stored in accumulator
 */
void ora(uint8_t mode)
{
	PC++;

	uint8_t val = read_addr_mode_01(mode);
	A |= val;

	set_negative_flag(A);
	set_zero_flag(A);

	PC++;
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
 * shift bits to the left, pushing in 0.
 */
void asl(uint8_t mode)
{
	PC++;

	uint8_t old_val = read_addr_mode_01(mode);
	uint8_t new_val = old_val<<1;
	write_addr_mode_01(mode, new_val);

	set_zero_flag(new_val);
	set_negative_flag(new_val);

	if((old_val & N_FLAG) == N_FLAG) {
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
 * Rotate bits right, pushing in carry flag value and poping into carry flag.
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
 * Push processor status flags onto the stack
 */
void php()
{
	memory[S] = P;
	S++;
	PC++;
}

/*
 * Clear carry flag
 */
void clc()
{
	P &= ~C_FLAG;
	PC++;
}

/*
 * Pull processor status from stack
 */
void plp()
{
	P = memory[S];
	S--;
	PC++;
}

/*
 * Set carry flag
 */
void sec()
{
	P |= C_FLAG;
	PC++;
}

/*
 * Push accumulator onto stack
 */
void pha()
{
	memory[S] = A;
	S++;
	PC++;
}

/*
 * Clear the interrupt disable flag
 */
void cli()
{
	P &= ~I_FLAG;
	PC++;
}

/*
 * Pull accumulator status from stack
 */
void pla()
{
	A = memory[S];
	S--;
	PC++;
}

/*
 * Set Interrupt disable flag
 */
void sei()
{
	P |= I_FLAG;
	PC++;
}

/*
 * Decrement Y
 */
void dey()
{
	Y--;
	set_negative_flag(Y);
	set_zero_flag(Y);
	PC++;
}

/*
 * Transfer Y to A
 */
void tya()
{
	A = Y;
	set_negative_flag(A);
	set_zero_flag(A);
	PC++;
}

/*
 * Transfer A to Y
 */
void tay()
{
	Y = A;
	set_negative_flag(Y);
	set_zero_flag(Y);
	PC++;
}

/*
 * Clear the overflow flag
 */
void clv()
{
	P &= ~V_FLAG;
	PC++;
}

/*
 * Increment Y
 */
void iny()
{
	Y++;
	set_negative_flag(Y);
	set_zero_flag(Y);
	PC++;
}

/*
 * Clear the decimal flag
 */
void cld()
{
	P &= ~D_FLAG;
	PC++;
}

/*
 * Increment X
 */
void inx()
{
	X++;
	set_negative_flag(X);
	set_zero_flag(X);
	PC++;
}

/*
 * Set Decimal mode flag
 */
void sed()
{
	P |= D_FLAG;
	PC++;
}

/*
 * If negative flag is clear, add relative displacement to PC
 */
void bpl()
{
	PC++;
	if((P & N_FLAG) != N_FLAG) {
		PC = get_relative();
	} else {
		PC++;
	}
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
