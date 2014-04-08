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

	// Odd frame toggle
	int odd_frame;

	// for scrolling.  Here, loopy_v and loopy_t are decoded like so:
	// U yyy NN YYYYY XXXXX
	// | ||| || ||||| +++++-- coarse X scroll
	// | ||| || +++++-------- coarse Y scroll
	// | ||| ++-------------- nametable select
	// | +++----------------- fine Y scroll
	// +--------------------- unused
	uint16_t loopy_v; // VRAM address value
	uint16_t loopy_t; // scroll & addr address latch
	uint16_t loopy_x; // only 3 bits were really needed for this guy
	int write_toggle;

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

	ppu->write_toggle = 0;
	ppu->loopy_v = 0;
	ppu->loopy_t = 0;
	ppu->loopy_x = 0;

	return ppu;
}

inline void read_ctrl(struct ppu *ppu)
{
	ppu->write_toggle = 0;
}

inline void read_status(struct ppu *ppu)
{
	// clear latch used by PPUADDR
	ppu->loopy_t = 0;
	// Unset the vblank start flag and write back to mem
	ppu->status &= ~(1<<7);
}

inline void read_or_write_data(struct ppu *ppu)
{
	// increment loopy_v based on control register VRAM address
	// increment bit value (0 = add 1, 1 = add 32)
	if ((ppu->status & 1<<2) == 0) {
		ppu->loopy_v++;
	} else {
		ppu->loopy_v += 32;
	}
}

uint8_t PPU_read_register(struct ppu *ppu, uint16_t addr)
{
	uint8_t val;
	switch(addr) {
		case 0x2000:
			val = ppu->ctrl;
			read_ctrl(ppu);
			break;
		case 0x2001:
			val = ppu->mask;
			break;
		case 0x2002:
			val = ppu->status;
			read_status(ppu);
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
			read_or_write_data(ppu);
			break;
	}
	return val;
}

/* 
 * This was fun to figure out.  Copy num bits at source_pos from source
 * into dest at dest_pos.  Positions start at 0, the LSB.
 * */
inline uint16_t set_bits(uint16_t source, uint16_t dest, int num, int source_pos, int dest_pos)
{
	unsigned long mask = ((1UL<<(num))-1UL)<<(source_pos);

	if (dest_pos >= source_pos) {
		return (dest & (~(mask << dest_pos))) | ((source & mask) << dest_pos);
	}

	return (dest & (~(mask >> (source_pos - dest_pos)))) | ((source & mask) >> (source_pos - dest_pos));
}

inline void write_to_ctrl(struct ppu *ppu, uint8_t value)
{
	ppu->loopy_t = set_bits(value, ppu->loopy_t, 2, 0, 10);
}

inline void write_to_scroll(struct ppu *ppu, uint8_t value)
{
	if (ppu->write_toggle == 0) {
		ppu->loopy_x = value % 8;
		ppu->loopy_t = set_bits(value, ppu->loopy_t, 5, 3, 0);
	} else {
		ppu->loopy_t = set_bits(value, ppu->loopy_t, 3, 0, 12);
		ppu->loopy_t = set_bits(value, ppu->loopy_t, 5, 3, 5);
	}

	ppu->write_toggle ^= 1;
}

inline void write_to_addr(struct ppu *ppu, uint8_t value)
{
	if (ppu->write_toggle == 0) {
		ppu->loopy_t = set_bits(value, ppu->loopy_t, 6, 0, 8);
		// clear bit 15 of loopy_t, too
		ppu->loopy_t &= ~(1<<15);
	} else {
		ppu->loopy_t = set_bits(value, ppu->loopy_t, 8, 0, 0);
		ppu->loopy_v = ppu->loopy_t;
	}

	ppu->write_toggle ^= 1;
}

void PPU_write_register(struct ppu *ppu, uint16_t addr, uint8_t value)
{
	switch(addr) {
		case 0x2000:
			ppu->ctrl = value;
			write_to_ctrl(ppu, value);
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
			write_to_scroll(ppu, value);
			break;
		case 0x2006:
			// valid values are 0x0000 through 0x3FFF.  Mirror down
			// otherwise.
			value = value % 0x4000;
			ppu->addr = value;
			write_to_addr(ppu, value);
			break;
		case 0x2007:
			ppu->data = value;
			read_or_write_data(ppu);
			break;
	}
}

void PPU_delete(struct ppu **ppu)
{
	free(*ppu);
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

inline void process_sprites(struct ppu *ppu, struct ppu_memory *ppu_mem)
{
	if (ppu->line == 261 || ppu->line >= 240) {
		if((ppu->dot >= 257) && (ppu->dot <= 320)) {
			ppu->oam_addr = 0;
		}
	}
}

inline void process_background(struct ppu *ppu, struct ppu_memory *ppu_mem)
{
	if (ppu->line < 240 || ppu->line == 261) {
		// special case for line 261
		if (ppu->line == 261) {
			if (ppu->dot >= 280 && ppu->dot <= 304) {
				// vertical_v = vertical_t
				ppu->loopy_v = set_bits(ppu->loopy_t, ppu->loopy_v, 5, 5, 5);
				ppu->loopy_v = set_bits(ppu->loopy_t, ppu->loopy_v, 4, 11, 11);
			}
		}

		if (ppu->dot == 257) {
			// horizontal_v = horizontal_t
			ppu->loopy_v = set_bits(ppu->loopy_t, ppu->loopy_v, 5, 0, 0);
			ppu->loopy_v = set_bits(ppu->loopy_t, ppu->loopy_v, 1, 10, 10);
		}

		if (ppu->dot == 256) {
			// increment vertical_v
			// Code taken from
			// http://wiki.nesdev.com/w/index.php/The_skinny_on_NES_scrolling#Wrapping_around
			if ((ppu->loopy_v & 0x7000) != 0x7000) {			// if fine Y < 7
				ppu->loopy_v += 0x1000;					// increment fine Y
			} else {
				ppu->loopy_v &= ~0x7000;				// fine Y = 0
				int y = (ppu->loopy_v & 0x03E0) >> 5;			// let y = coarse Y
				if (y == 29) {
					y = 0;						// coarse Y = 0
					ppu->loopy_v ^= 0x0800;				// switch vertical nametable
				} else if (y == 31) {
					y = 0;						// coarse Y = 0, nametable not switched
				} else {
					y += 1;						// increment coarse Y
				}
				ppu->loopy_v = (ppu->loopy_v & ~0x03E0) | (y << 5);	// put coarse Y back into v
			}
		}

		if ((ppu->dot < 257 && ppu->dot > 0) || (ppu->dot > 320)) {
			uint8_t fetch_cycle = ppu->dot % 8;

			switch(fetch_cycle) {
				case 0:
					// increment horizontal_v
					if ((ppu->loopy_v & 0x001F) == 31) {	// if coarse X == 31
						ppu->loopy_v &= ~0x001F;	// coarse X = 0
						ppu->loopy_v ^= 0x0400;		// switch horizontal nametable
					} else {
						ppu->loopy_v += 1;		// increment coarse X
					}
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
}

inline void set_flags(struct ppu *ppu, struct ppu_memory *ppu_mem)
{
	// VBLANK, sprite overflow, and sprite 0 hit flags are cleared at the 2nd dot of scanline 261
	if (ppu->line == 261 && ppu->dot == 1) {
		clear_vblank_flag(ppu);
		clear_sprite_overflow_flag(ppu);
		clear_sprite_0_hit_flag(ppu);
	}

	/* VBLANK flag is set at the 2nd cycle of scanline 241.  This is the
	 * start of the VBLANKing interval. */
	if (ppu->line == 241 && ppu->dot == 1) {
		set_vblank_flag(ppu);
	}
}

uint8_t PPU_step(struct ppu *ppu, struct ppu_memory *ppu_mem)
{
#ifdef DEBUG_PPU
	(void)printf("SL %03d.%03d ", ppu->line, ppu->dot);
#endif
	int return_val = 1;
	set_flags(ppu, ppu_mem);	
	process_sprites(ppu, ppu_mem);
	process_background(ppu, ppu_mem);

	// check for NMI
	if (ppu->line == 241 && ppu->dot == 1) {
		if (vblank_is_enabled(ppu) != 0) {
#ifdef DEBUG_PPU
			(void)printf("\n*** Executing VBLANK ***\n");
#endif
			return_val = 0;
		}
	}


#ifdef DEBUG_PPU
	(void)printf("ctrl:%02x mask:%02x status:%02x oamaddr:%02x oamdata:%02x scroll:%02x addr:%02x data:%02x loopy_v:%04x loopy_t:%04x loopy_x:%04x\n", ppu->ctrl, ppu->mask, ppu->status, ppu->oam_addr, ppu->oam_data, ppu->scroll, ppu->addr, ppu->data, ppu->loopy_v, ppu->loopy_t, ppu->loopy_x);
#endif
	increment_cycle(ppu);
	return return_val;
}
