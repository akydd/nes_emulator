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

uint8_t brk(struct cpu *);
uint8_t ora_ind_x(struct cpu *);
uint8_t ora_zero_pg(struct cpu *);
uint8_t asl_zero_pg(struct cpu *);
uint8_t php(struct cpu *);
uint8_t ora_imm(struct cpu *);
uint8_t asl_acc(struct cpu *);
uint8_t ora_abs(struct cpu *);
uint8_t asl_abs(struct cpu *);
uint8_t bpl_r(struct cpu *);
uint8_t ora_ind_y(struct cpu *);
uint8_t ora_zero_pg_x(struct cpu *);
uint8_t asl_zero_pg_x(struct cpu *);
uint8_t clc(struct cpu *);
uint8_t ora_abs_y(struct cpu *);
uint8_t ora_abs_x(struct cpu *);
uint8_t asl_abs_x(struct cpu *);
uint8_t jsr_abs(struct cpu *);
uint8_t and_ind_x(struct cpu *);
uint8_t bit_zero_pg(struct cpu *);
uint8_t and_zero_pg(struct cpu *);
uint8_t rol_zero_pg(struct cpu *);
uint8_t plp(struct cpu *);
uint8_t and_imm(struct cpu *);
uint8_t rol_acc(struct cpu *);
uint8_t bit_abs(struct cpu *cpu);
uint8_t and_abs(struct cpu *cpu);
uint8_t rol_abs(struct cpu *cpu);
uint8_t bmi_r(struct cpu *cpu);
uint8_t and_ind_y(struct cpu *);
uint8_t and_zero_pg_x(struct cpu *);
uint8_t rol_zero_pg_x(struct cpu *);
uint8_t sec(struct cpu *);
uint8_t and_abs_y(struct cpu *);
uint8_t and_abs_x(struct cpu *);
uint8_t rol_abs_x(struct cpu *);
uint8_t rti(struct cpu *);
uint8_t eor_ind_x(struct cpu *);
uint8_t eor_zero_pg(struct cpu *);
uint8_t lsr_zero_pg(struct cpu *);
uint8_t pha(struct cpu *);
uint8_t eor_imm(struct cpu *);
uint8_t lsr_acc(struct cpu *);
uint8_t jmp_abs(struct cpu *);
uint8_t eor_abs(struct cpu *);
uint8_t lsr_abs(struct cpu *);
uint8_t bvc_r(struct cpu *);
uint8_t eor_ind_y(struct cpu *);
uint8_t eor_zero_pg_x(struct cpu *);
uint8_t lsr_zero_pg_x(struct cpu *);
uint8_t cli(struct cpu *);
uint8_t eor_abs_y(struct cpu *);
uint8_t eor_abs_x(struct cpu *);
uint8_t lsr_abs_x(struct cpu *);
uint8_t rts(struct cpu *);
uint8_t adc_ind_x(struct cpu *);
uint8_t adc_zero_pg(struct cpu *);
uint8_t ror_zero_pg(struct cpu *);
uint8_t pla(struct cpu *);
uint8_t adc_imm(struct cpu *);
uint8_t ror_acc(struct cpu *);
uint8_t jmp_ind(struct cpu *);
uint8_t adc_abs(struct cpu *);
uint8_t ror_abs(struct cpu *);
uint8_t bvs_r(struct cpu *);
uint8_t adc_ind_y(struct cpu *);
uint8_t adc_zero_pg_x(struct cpu *);
uint8_t ror_zero_pg_x(struct cpu *);
uint8_t sei(struct cpu *);
uint8_t adc_abs_y(struct cpu *);
uint8_t adc_abs_x(struct cpu *);
uint8_t ror_abs_x(struct cpu *);
uint8_t sta_ind_x(struct cpu *);
uint8_t sty_zero_pg(struct cpu *);
uint8_t sta_zero_pg(struct cpu *);
uint8_t stx_zero_pg(struct cpu *);
uint8_t dey(struct cpu *);
uint8_t txa(struct cpu *);
uint8_t sty_abs(struct cpu *);
uint8_t sta_abs(struct cpu *);
uint8_t stx_abs(struct cpu *);
uint8_t bcc_r(struct cpu *);
uint8_t sta_ind_y(struct cpu *);
uint8_t sty_zero_pg_x(struct cpu *);
uint8_t sta_zero_pg_x(struct cpu *);
uint8_t stx_zero_pg_y(struct cpu *);
uint8_t tya(struct cpu *);
uint8_t sta_abs_y(struct cpu *);
uint8_t txs(struct cpu *);
uint8_t sta_abs_x(struct cpu *);
uint8_t ldy_imm(struct cpu *);
uint8_t lda_ind_x(struct cpu *);
uint8_t ldx_imm(struct cpu *);
uint8_t ldy_zero_pg(struct cpu *);
uint8_t lda_zero_pg(struct cpu *);
uint8_t ldx_zero_pg(struct cpu *);
uint8_t tay(struct cpu *);
uint8_t lda_imm(struct cpu *);
uint8_t tax(struct cpu *);
uint8_t ldy_abs(struct cpu *);
uint8_t lda_abs(struct cpu *);
uint8_t ldx_abs(struct cpu *);
uint8_t bcs_r(struct cpu *);
uint8_t lda_ind_y(struct cpu *);
uint8_t ldy_zero_pg_x(struct cpu *);
uint8_t lda_zero_pg_x(struct cpu *);
uint8_t ldx_zero_pg_y(struct cpu *);
uint8_t clv(struct cpu *);
uint8_t lda_abs_y(struct cpu *);
uint8_t tsx(struct cpu *);
uint8_t ldy_abs_x(struct cpu *);
uint8_t lda_abs_x(struct cpu *);
uint8_t ldx_abs_y(struct cpu *);
uint8_t cpy_imm(struct cpu *);
uint8_t cmp_ind_x(struct cpu *);
uint8_t cpy_zero_pg(struct cpu *);
uint8_t cmp_zero_pg(struct cpu *);
uint8_t dec_zero_pg(struct cpu *);
uint8_t iny(struct cpu *);
uint8_t cmp_imm(struct cpu *);
uint8_t dex(struct cpu *);
uint8_t cpy_abs(struct cpu *);
uint8_t cmp_abs(struct cpu *);
uint8_t dec_abs(struct cpu *);
uint8_t bne_r(struct cpu *);
uint8_t cmp_ind_y(struct cpu *);
uint8_t cmp_zero_pg_x(struct cpu *);
uint8_t dec_zero_pg_x(struct cpu *);
uint8_t cld(struct cpu *);
uint8_t cmp_abs_y(struct cpu *);
uint8_t cmp_abs_x(struct cpu *);
uint8_t dec_abs_x(struct cpu *);
uint8_t cpx_imm(struct cpu *);
uint8_t sbc_ind_x(struct cpu *);
uint8_t cpx_zero_pg(struct cpu *);
uint8_t sbc_zero_pg(struct cpu *);
uint8_t inc_zero_pg(struct cpu *);
uint8_t inx(struct cpu *);
uint8_t sbc_imm(struct cpu *);
uint8_t nop(struct cpu *);
uint8_t cpx_abs(struct cpu *);
uint8_t sbc_abs(struct cpu *);
uint8_t inc_abs(struct cpu *);
uint8_t beq_r(struct cpu *);
uint8_t sbc_ind_y(struct cpu *);
uint8_t sbc_zero_pg_x(struct cpu *);
uint8_t inc_zero_pg_x(struct cpu *);
uint8_t sed(struct cpu *);
uint8_t sbc_abs_y(struct cpu *);
uint8_t sbc_abs_x(struct cpu *);
uint8_t inc_abs_x(struct cpu *);


/* codes 0x00 to 0xFF  */
static uint8_t (* const pf[]) (struct cpu *) = {
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

#endif
