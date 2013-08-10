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

struct ppu {
	struct memory *mem; /*  shared memory */
	struct ppu_memory *ppu_mem;
};

/*
 * Create a new ppu struct
 */
struct ppu *PPU_init(struct memory *, struct ppu_memory *);

void PPU_delete(struct ppu **);

void PPU_step();

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
