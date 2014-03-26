/*
 * =============================================================================
 *
 *       Filename:  ppu.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  13-08-09 10:33:58 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alan Kydd (), akydd@ualberta.net
 *
 * =============================================================================
 */
#include <stdlib.h>

#include "ppu.h"

struct ppu {
	// Registers.
	uint8_t ctrl;
	uint8_t mask;
	uint8_t status;
	uint8_t oam_addr;
	uint8_t oam_data;
	uint8_t scroll;
	uint8_t addr;
	uint8_t data;

	// Odd frame
	int odd_frame;

	// tracks current line and dot.  There are 262 scanlines (0 to 261) and
	// 341 dots (0 to 340).
	unsigned int line;
	unsigned int dot;

	// background shift registers
	uint16_t high_bg;
	uint16_t low_bg;
	uint8_t high_bg_attribute;
	uint8_t low_bg_attribute;
};

struct ppu *PPU_init()
{
	struct ppu *ppu = malloc(sizeof(struct ppu));

	// Initialize values as of power-on
	ppu->ctrl = 0x00;
	ppu->mask = 0x00;
	ppu->status = 0xa0;

	// PPU starts at pre-render scanline 261, dot 0, even frame
	ppu->odd_frame = 0;
	ppu->line = 261;
	ppu->dot = 0;

	return ppu;
}

uint8_t PPU_read_register(struct ppu *ppu, uint16_t addr)
{
	uint8_t val;
	switch(addr) {
		case 0x2000:
			val = ppu->ctrl;
			break;
		case 0x2001:
			val = ppu->mask;
			break;
		case 0x2002:
			val = ppu->status;
			break;
		case 0x2003:
			val = ppu->oam_addr;
			break;
		case 0x2004:
			val = ppu->oam_data;
			break;
		case 0x2005:
			val = ppu->scroll;
			break;
		case 0x2006:
			val = ppu->addr;
			break;
		case 0x2007:
			val = ppu->data;
			break;
	}
	return val;
}

void PPU_write_register(struct ppu *ppu, uint16_t addr, uint8_t value)
{
	switch(addr) {
		case 0x2000:
			ppu->ctrl = value;
			break;
		case 0x2001:
			ppu->mask = value;
			break;
		case 0x2002:
			ppu->status = value;
			break;
		case 0x2003:
			ppu->oam_addr = value;
			break;
		case 0x2004:
			ppu->oam_data = value;
			break;
		case 0x2005:
			ppu->scroll = value;
			break;
		case 0x2006:
			ppu->addr = value;
			break;
		case 0x2007:
			ppu->data = value;
			break;
	}
}

void PPU_delete(struct ppu **ppu)
{
	free(*ppu);
}

inline uint8_t read_status(struct ppu *ppu)
{
	// clear latch used by PPUADDR
	// TODO
	// Get the original value
	uint8_t val = ppu->status;
	// Unset the vblank start flag and write back to mem
	uint8_t new_val = (val & ~(1<<7));
	ppu->status = new_val;

	// return original value
	return val;
}

inline void write_OAM_addr(struct ppu *ppu, const uint8_t val)
{
	// write to the register
	ppu->oam_addr = val;
}

inline void write_OAM_data(struct ppu *ppu, const uint8_t val)
{
	// write to the register
	ppu->oam_data = val;
	// increment OAM address register, too
	ppu->oam_addr++;
}

inline void write_data(struct ppu *ppu, const uint8_t val)
{
	// write to the register
	ppu->data = val;
	// increment address register based on control register VRAM address
	// increment bit value (0 = add 1, 1 = add 32)
	if ((ppu->status & 1<<2) == 0) {
		ppu->addr++;
	} else {
		ppu->addr += 32;
	}
}

inline uint8_t vblank_is_enabled(struct ppu *ppu)
{
	return ppu->ctrl & (1<<7);
}

inline void set_vblank_flag(struct ppu *ppu)
{
	ppu->status |= (1<<7);
#ifdef DEBUG_PPU
	(void)printf("VBLANK set\n");
#endif
}

inline void clear_vblank_flag(struct ppu *ppu)
{
	ppu->status &= ~(1<<7);
#ifdef DEBUG_PPU
	(void)printf("VBLANK cleared\n");
#endif
}

inline void clear_sprite_overflow_flag(struct ppu *ppu)
{
	ppu->status &= ~(1<<5);
}

inline void clear_sprite_0_hit_flag(struct ppu *ppu)
{
	ppu->status &= ~(1<<6);
}

inline void increment_cycle(struct ppu *ppu)
{
	if (ppu->dot == 340) { // end of line
		ppu->dot = 0;
		if (ppu->line == 261) { // end of frame
			ppu->line = 0;
		} else {
			ppu->line++;
		}
	} else {
		ppu->dot++;
	}
}

inline void render(struct ppu *ppu, struct ppu_memory *ppu_mem)
{
	// VBLANK, sprite overflow, and sprite 0 hit flags are cleared at the 2nd dot of scanline 261
	if (ppu->line == 261 && ppu->dot == 1) {
		clear_vblank_flag(ppu);
		clear_sprite_overflow_flag(ppu);
		clear_sprite_0_hit_flag(ppu);
	}

	if((ppu->dot >= 257) && (ppu->dot <= 320)) {
		write_OAM_addr(ppu, 0);
	}

	if ((ppu->dot < 257 && ppu->dot > 0) || (ppu->dot > 320)) {
		uint8_t fetch_cycle = ppu->dot % 8;

		switch(fetch_cycle) {
			case 0:
				// increment horizontal
				break;
			case 2:
				// fetch nametable address
				break;
			case 4:
				// fetch attribute table address
				break;
			case 6:
				// fetch low background tile byte
				break;
			case 8:
				// fetch high background tile byte
				break;
		}
	}
}

uint8_t PPU_step(struct ppu *ppu, struct ppu_memory *ppu_mem)
{
#ifdef DEBUG_PPU
	(void)printf("SL %d.%d\n", ppu->line, ppu->dot);
#endif
	/* VBLANK flag is set at the 2nd cycle of scanline 241.  This is the
	 * start of the VBLANKing interval. */
	if (ppu->dot == 1 && ppu->line == 241) {
		set_vblank_flag(ppu);

		/* This return indicates an NMI to the CPU */
		if (vblank_is_enabled(ppu) != 0) {
			increment_cycle(ppu);
#ifdef DEBUG_CPU
			(void)printf("Executing VBLANK\n");
#endif
			return 0;
		}
	}

	if (ppu->line < 240) {
		render(ppu, ppu_mem);	
	}

	increment_cycle(ppu);
	return 1;
}
