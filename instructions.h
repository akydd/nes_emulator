/*
 * =============================================================================
 *
 *       Filename:  instructions.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  13-03-28 09:23:00 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alan Kydd (), akydd@ualberta.net
 *   Organization:  
 *
 * =============================================================================
 */

#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <stdlib.h>
#include "cpu.h"

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

void brk(struct cpu *);
void ora_ind_x(struct cpu *);
void ora_zero_pg(struct cpu *);
void asl_zero_pg(struct cpu *);
void php(struct cpu *);
void ora_imm(struct cpu *);
void asl_acc(struct cpu *);
void ora_abs(struct cpu *);
void asl_abs(struct cpu *);
void bpl_r(struct cpu *);
void ora_ind_y(struct cpu *);
void ora_zero_pg_x(struct cpu *);
void asl_zero_pg_x(struct cpu *);
void clc(struct cpu *);
void ora_abs_y(struct cpu *);
void ora_abs_x(struct cpu *);
void asl_abs_x(struct cpu *);
void jsr_abs(struct cpu *);
void and_ind_x(struct cpu *);
void bit_zero_pg(struct cpu *);
void and_zero_pg(struct cpu *);
void rol_zero_pg(struct cpu *);
void plp(struct cpu *);
void and_imm(struct cpu *);
void rol_acc(struct cpu *);
void bit_abs(struct cpu *cpu);
void and_abs(struct cpu *cpu);
void rol_abs(struct cpu *cpu);
void bmi_r(struct cpu *cpu);
void and_ind_y(struct cpu *);
void and_zero_pg_x(struct cpu *);
void rol_zero_pg_x(struct cpu *);
void sec(struct cpu *);
void and_abs_y(struct cpu *);
void and_abs_x(struct cpu *);
void rol_abs_x(struct cpu *);
void rti(struct cpu *);
void eor_ind_x(struct cpu *);
void eor_zero_pg(struct cpu *);
void lsr_zero_pg(struct cpu *);
void pha(struct cpu *);
void eor_imm(struct cpu *);
void lsr_acc(struct cpu *);
void jmp_abs(struct cpu *);
void eor_abs(struct cpu *);
void lsr_abs(struct cpu *);
void bvc_r(struct cpu *);
void eor_ind_y(struct cpu *);
void eor_zero_pg_x(struct cpu *);
void lsr_zero_pg_x(struct cpu *);
void cli(struct cpu *);
void eor_abs_y(struct cpu *);
void eor_abs_x(struct cpu *);
void lsr_abs_x(struct cpu *);
void rts(struct cpu *);
void adc_ind_x(struct cpu *);
void adc_zero_pg(struct cpu *);
void ror_zero_pg(struct cpu *);
void pla(struct cpu *);
void adc_imm(struct cpu *);
void ror_acc(struct cpu *);
void jmp_ind(struct cpu *);
void adc_abs(struct cpu *);
void ror_abs(struct cpu *);
void bvs_r(struct cpu *);
void adc_ind_y(struct cpu *);
void adc_zero_pg_x(struct cpu *);
void ror_zero_pg_x(struct cpu *);
void sei(struct cpu *);
void adc_abs_y(struct cpu *);
void adc_abs_x(struct cpu *);
void ror_abs_x(struct cpu *);
void sta_ind_x(struct cpu *);
void sty_zero_pg(struct cpu *);
void sta_zero_pg(struct cpu *);
void stx_zero_pg(struct cpu *);
void dey(struct cpu *);
void txa(struct cpu *);
void sty_abs(struct cpu *);
void sta_abs(struct cpu *);
void stx_abs(struct cpu *);
void bcc_r(struct cpu *);
void sta_ind_y(struct cpu *);
void sty_zero_pg_x(struct cpu *);
void sta_zero_pg_x(struct cpu *);
void stx_zero_pg_x(struct cpu *);
void tya(struct cpu *);
void sta_abs_y(struct cpu *);
void txs(struct cpu *);
void sta_abs_x(struct cpu *);
void ldy_imm(struct cpu *);
void lda_ind_x(struct cpu *);
void ldx_imm(struct cpu *);
void ldy_zero_pg(struct cpu *);
void lda_zero_pg(struct cpu *);
void ldx_zero_pg(struct cpu *);
void tay(struct cpu *);
void lda_imm(struct cpu *);
void tax(struct cpu *);
void ldy_abs(struct cpu *);
void lda_abs(struct cpu *);
void ldx_abs(struct cpu *);
void bcs_r(struct cpu *);
void lda_ind_y(struct cpu *);
void ldy_zero_pg_x(struct cpu *);
void lda_zero_pg_x(struct cpu *);
void ldx_zero_pg_y(struct cpu *);
void clv(struct cpu *);
void lda_abs_y(struct cpu *);
void tsx(struct cpu *);
void ldy_abs_x(struct cpu *);
void lda_abs_x(struct cpu *);
void ldx_abs_y(struct cpu *);
void cpy_imm(struct cpu *);
void cmp_ind_x(struct cpu *);
void cpy_zero_pg(struct cpu *);
void cmp_zero_pg(struct cpu *);
void dec_zero_pg(struct cpu *);
void iny(struct cpu *);
void cmp_imm(struct cpu *);
void dex(struct cpu *);
void cpy_abs(struct cpu *);
void cmp_abs(struct cpu *);
void dec_abs(struct cpu *);
void bne_r(struct cpu *);
void cmp_ind_y(struct cpu *);
void cmp_zero_pg_x(struct cpu *);
void dec_zero_pg_x(struct cpu *);
void cld(struct cpu *);
void cmp_abs_y(struct cpu *);
void cmp_abs_x(struct cpu *);
void dec_abs_x(struct cpu *);
void cpx_imm(struct cpu *);
void sbc_ind_x(struct cpu *);
void cpx_zero_pg(struct cpu *);
void sbc_zero_pg(struct cpu *);
void inc_zero_pg(struct cpu *);
void inx(struct cpu *);
void sbc_imm(struct cpu *);
void nop(struct cpu *);
void cpx_abs(struct cpu *);
void sbc_abs(struct cpu *);
void inc_abs(struct cpu *);
void beq_r(struct cpu *);
void sbc_ind_y(struct cpu *);
void sbc_zero_pg_x(struct cpu *);
void inc_zero_pg_x(struct cpu *);
void sed(struct cpu *);
void sbc_abs_y(struct cpu *);
void sbc_abs_x(struct cpu *);
void inc_abs_x(struct cpu *);


/* codes 0x00 to 0xFF  */
static void (* const pf[]) (struct cpu *) = {
	&brk, &ora_ind_x, NULL, NULL, NULL, &ora_zero_pg, &asl_zero_pg, NULL, &php, &ora_imm, &asl_acc, NULL, NULL, &ora_abs, &asl_abs, NULL,
	&bpl_r, &ora_ind_y, NULL, NULL, NULL, &ora_zero_pg_x, &asl_zero_pg_x, NULL, &clc, &ora_abs_y, NULL, NULL, NULL, &ora_abs_x, &asl_abs_x, NULL,
	&jsr_abs, &and_ind_x, NULL, NULL, &bit_zero_pg, &and_zero_pg, &rol_zero_pg, NULL, &plp, &and_imm, &rol_acc, NULL, &bit_abs, &and_abs, &rol_abs, NULL,
	&bmi_r, &and_ind_y, NULL, NULL, NULL, &and_zero_pg_x, &rol_zero_pg_x, NULL, &sec, &and_abs_y, NULL, NULL, NULL, &and_abs_x, &rol_abs_x, NULL,
	&rti, &eor_ind_x, NULL, NULL, NULL, &eor_zero_pg, &lsr_zero_pg, NULL, &pha, &eor_imm, &lsr_acc, NULL, &jmp_abs, &eor_abs, &lsr_abs, NULL,
	&bvc_r, &eor_ind_y, NULL, NULL, NULL, &eor_zero_pg_x, &lsr_zero_pg_x, NULL, &cli, &eor_abs_y, NULL, NULL, NULL, &eor_abs_x, &lsr_abs_x, NULL,
	&rts, &adc_ind_x, NULL, NULL, NULL, &adc_zero_pg, &ror_zero_pg, NULL, &pla, &adc_imm, &ror_acc, NULL, &jmp_ind, &adc_abs, &ror_abs, NULL,
	&bvs_r, &adc_ind_y, NULL, NULL, NULL, &adc_zero_pg_x, &ror_zero_pg_x, NULL, &sei, &adc_abs_y, NULL, NULL, NULL, &adc_abs_x, &ror_abs_x, NULL,
	NULL, &sta_ind_x, NULL, NULL, &sty_zero_pg, &sta_zero_pg, &stx_zero_pg, NULL, &dey, NULL, &txa, NULL, &sty_abs, &sta_abs, &stx_abs, NULL,
	&bcc_r, &sta_ind_y, NULL, NULL, &sty_zero_pg_x, &sta_zero_pg_x, &stx_zero_pg_x, NULL, &tya, &sta_abs_y, &txs, NULL, NULL, &sta_abs_x, NULL, NULL,
	&ldy_imm, &lda_ind_x, &ldx_imm, NULL, &ldy_zero_pg, &lda_zero_pg, &ldx_zero_pg, NULL, &tay, &lda_imm, &tax, NULL, &ldy_abs, &lda_abs, &ldx_abs, NULL,
	&bcs_r, &lda_ind_y, NULL, NULL, &ldy_zero_pg_x, &lda_zero_pg_x, &ldx_zero_pg_y, NULL, &clv, &lda_abs_y, &tsx, NULL, &ldy_abs_x, &lda_abs_x, &ldx_abs_y, NULL,
	&cpy_imm, &cmp_ind_x, NULL, NULL, &cpy_zero_pg, &cmp_zero_pg, &dec_zero_pg, NULL, &iny, &cmp_imm, &dex, NULL, &cpy_abs, &cmp_abs, &dec_abs, NULL,
	&bne_r, &cmp_ind_y, NULL, NULL, NULL, &cmp_zero_pg_x, &dec_zero_pg_x, NULL, &cld, &cmp_abs_y, NULL, NULL, NULL, &cmp_abs_x, &dec_abs_x, NULL,
	&cpx_imm, &sbc_ind_x, NULL, NULL, &cpx_zero_pg, &sbc_zero_pg, &inc_zero_pg, NULL, &inx, &sbc_imm, &nop, NULL, &cpx_abs, &sbc_abs, &inc_abs, NULL,
	&beq_r, &sbc_ind_y, NULL, NULL, NULL, &sbc_zero_pg_x, &inc_zero_pg_x, NULL, &sed, &sbc_abs_y, NULL, NULL, NULL, &sbc_abs_x, &inc_abs_x, NULL
};

#endif
