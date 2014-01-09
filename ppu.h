/*
 * ===========================================================================
 *
 *       Filename:  ppu.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  13-08-03 11:13:13 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alan Kydd (), akydd@ualberta.net
 *   Organization:  
 *
 * ===========================================================================
 */

#ifndef PPU_H
#define PPU_H

#include <stdint.h>

#include "ppu_memory.h"
#include "memory.h"

/*
 * PPU registers are mapped to CPU memory:
 * Info below taken from 
 * http://wiki.nesdev.com/w/index.php/PPU_registers
 *
 * PPUCTRL - 0x2000
 * ================
 * 7654 3210
 * |||| ||||
 * |||| ||++- Base nametable address
 * |||| ||    (0 = $2000; 1 = $2400; 2 = $2800; 3 = $2C00)
 * |||| |+--- VRAM address increment per CPU read/write of PPUDATA
 * |||| |     (0: add 1, going across; 1: add 32, going down)
 * |||| +---- Sprite pattern table address for 8x8 sprites
 * ||||       (0: $0000; 1: $1000; ignored in 8x16 mode)
 * |||+------ Background pattern table address (0: $0000; 1: $1000)
 * ||+------- Sprite size (0: 8x8; 1: 8x16)
 * |+-------- PPU master/slave select
 * |          (0: read backdrop from EXT pins; 1: output color on EXT pins)
 * +--------- Generate an NMI at the start of the
 *            vertical blanking interval (0: off; 1: on)
 *
 * PPUMASK - 0x2001
 * ================
 * 76543210
 * ||||||||
 * |||||||+- Grayscale (0: normal color; 1: produce a monochrome display)
 * ||||||+-- 1: Show background in leftmost 8 pixels of screen; 0: Hide
 * |||||+--- 1: Show sprites in leftmost 8 pixels of screen; 0: Hide
 * ||||+---- 1: Show background
 * |||+----- 1: Show sprites
 * ||+------ Intensify reds (and darken other colors)
 * |+------- Intensify greens (and darken other colors)
 * +-------- Intensify blues (and darken other colors)
 *
 * PPUSTATUS - 0x2002
 * ==================
 * 7654 3210
 * |||| ||||
 * |||+-++++- Least significant bits previously written into a PPU register
 * |||        (due to register not being updated for this address)
 * ||+------- Sprite overflow. The intent was for this flag to be set
 * ||         whenever more than eight sprites appear on a scanline, but a
 * ||         hardware bug causes the actual behavior to be more complicated
 * ||         and generate false positives as well as false negatives; see
 * ||         PPU sprite evaluation. This flag is set during sprite
 * ||         evaluation and cleared at dot 1 (the second dot) of the
 * ||         pre-render line.
 * |+-------- Sprite 0 Hit.  Set when a nonzero pixel of sprite 0 overlaps
 * |          a nonzero background pixel; cleared at dot 1 of the pre-render
 * |          line.  Used for raster timing.
 * +--------- Vertical blank has started (0: not in VBLANK; 1: in VBLANK).
 *            Set at dot 1 of line 241 (the line *after* the post-render
 *            line); cleared after reading $2002 and at dot 1 of the
 *            pre-render line.
 *
 * OAMADDR   - 0x2003
 * ==================
 * Destination address in sprite RAM for use with OAMDATA.
 *
 * OAMDATA   - 0x2004
 * ==================
 * Read/write data from/to selected address in OAMADDR.  OAMADDR is incremented
 * after each write to this register.
 *
 * PPUSCROLL - 0x2005
 * PPUADDR   - 0x2006
 * PPUDATA   - 0x2007
 *
 */
#define PPUCTRL_ADDR 0x2000
#define PPUMASK_ADDR 0x2001
#define PPUSTATUS_ADDR 0x2002
#define OAMADDR_ADDR 0x2003
#define OAMDATA_ADDR 0x2004
#define PPUSCROLL_ADDR 0x2005
#define PPUADDR_ADDR 0x2006
#define PPUDATA_ADDR 0x2007

struct ppu {
	/* 2 registers for generating NMIs */
	unsigned int NMI_occured;
	unsigned int NMI_output;
	/* Other registers are stored in shared memory (?) */
};

/*
 * Create a new ppu struct.
 * Memory must be instantiated before passing into this function.
 */
extern struct ppu *PPU_init(struct memory *);

/*
 * Destroy the given ppu
 */
extern void PPU_delete(struct ppu **);

extern uint8_t PPU_step(struct ppu *, struct memory *, int);

/* 
 * Memory instructions.
 * Needed to create these because writes to PPU memory can affect PPU status registers.
 */
extern void PPU_write(struct ppu *, struct ppu_memory *, const uint16_t, const uint8_t);

/* PPUCTRL - Control Register 1 manipulation */
void PPU_set_nametable_address(struct ppu *, uint8_t);
void PPU_set_vertical_write(struct ppu *);
void PPU_clear_vertical_write(struct ppu *);
void PPU_set_sprite_pattern_table_address(struct ppu *);
void PPU_clear_sprite_pattern_table_address(struct ppu *);
void PPU_set_screen_pattern_table_address(struct ppu *);
void PPU_clear_screen_pattern_table_address(struct ppu *);
void PPU_set_sprite_size(struct ppu *);
void PPU_clear_sprite_size(struct ppu *);
void PPU_set_VBlank_enable(struct ppu *);
void PPU_clear_VBlank_enable(struct ppu *);
uint8_t PPU_VBlank_is_enabled(struct ppu *, struct memory *);

/* PPUMASK - Control Register 2 manipulation */
void PPU_set_greyscale(struct ppu *);
void PPU_clear_greyscale(struct ppu *);
void PPU_set_image_mask(struct ppu *);
void PPU_clear_image_mask(struct ppu *);
void PPU_set_sprite_mask(struct ppu *);
void PPU_clear_sprite_mask(struct ppu *);
void PPU_set_screen_enable(struct ppu *);
void PPU_clear_screen_enable(struct ppu *);
void PPU_set_sprites_enable(struct ppu *);
void PPU_clear_sprites_enable(struct ppu *);
void PPU_set_background_colors(struct ppu *, uint8_t);

/* PPUSTATUS - Status register manipulation */
uint8_t PPU_VBlank_flag_is_set(struct ppu *);
uint8_t PPU_hit_flag_is_set(struct ppu *);
uint8_t PPU_sprite_overflow_is_set(struct ppu *);

/* OAMADDR - OAM address */
void PPU_set_sprite_memory_address(struct ppu *, uint8_t);

/* OAMDATA - OAM data */
void PPU_set_sprite_memory_data(struct ppu *, uint8_t);
uint8_t PPU_get_sprite_memory_data(struct ppu *);

/* PPUSCROLL */
void PPU_set_scroll(struct ppu *, uint8_t);

/* PPUADDR */
void PPU_set_memory_addres(struct ppu *, uint8_t);

/* PPUDATA */
void PPU_set_memory_data(struct ppu *, uint8_t);
uint8_t PPU_get_memory_data(struct ppu *);

#endif
