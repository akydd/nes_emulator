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
	/* holds the vram address */
	uint16_t vram_addr;
	uint16_t temp_vram_addr;

	uint8_t fine_x_scroll;

	/* Toggle for writes to PPUSCROLL and PPUADDR */
	uint8_t toggle;

	/* Internal latches for attribute table and name table addresses */
	uint8_t at_latch;
	uint8_t nt_latch;

	/*  Internal latches for background tile addresses */
	uint8_t bg_tile_low;
	uint8_t bg_tile_high;

	/* tracks current line and dot */
	unsigned int line;
	unsigned int dot;
};

struct ppu *PPU_init(struct memory *mem)
{
	struct ppu *ppu = malloc(sizeof(struct ppu));

	/* Initialize values as of power-on */
	MEM_write(mem, PPUCTRL_ADDR, 0x00);
	MEM_write(mem, PPUMASK_ADDR, 0x00);
	MEM_write(mem, PPUSTATUS_ADDR, 0xa0);

	ppu->toggle = 0;

	/* PPU starts at cycle 0 */
	ppu->line = 0;
	ppu->dot = 0;

	return ppu;
}

void PPU_delete(struct ppu **ppu)
{
	free(*ppu);
}

inline uint8_t read_status(struct ppu *ppu, struct memory *mem)
{
	// clear latch used by PPUADDR
	// TODO
	// Get the original value
	uint8_t val = MEM_read(mem, MEM_PPU_STATUS_REG_ADDR);
	// Unset the vblank start flag and write back to mem
	uint8_t new_val = (val & ~(1<<7));
	MEM_write(mem, MEM_PPU_STATUS_REG_ADDR, new_val);

	// return original value
	return val;
}

inline void write_toggle(struct ppu *ppu)
{
	if (ppu->toggle == 0) {
		ppu->toggle = 1;
	} else {
		ppu->toggle = 0;
	}
}

inline void write_scroll(struct ppu *ppu, struct memory *mem, uint8_t val)
{
	uint8_t low_3 = val & 7;
	uint8_t hi_5 = val & 504;

	if (ppu->toggle == 0) {
		ppu->fine_x_scroll = low_3;
		ppu->temp_vram_addr = hi_5 >> 3;
	} else {
		ppu->temp_vram_addr |= ((uint16_t)low_3 << 12) & ((uint16_t)hi_5 << 2);
	}

	write_toggle(ppu);
	
	MEM_write(mem, MEM_PPU_SCROLL_REG_ADDR, val);
}

inline void write_address(struct ppu *ppu, struct memory *mem, uint8_t val)
{
	if (ppu->toggle == 0) {
		ppu->temp_vram_addr = (uint16_t)val << 8;
	} else {
		ppu->temp_vram_addr |= val;
		ppu->vram_addr = ppu->temp_vram_addr;
	}

	write_toggle(ppu);

	MEM_write(mem, MEM_PPU_ADDR_REG_ADDR, val);
}

inline void write_OAM_addr(struct memory *mem, const uint8_t val)
{
	// write to the register
	MEM_write(mem, MEM_PPU_OAMADDR_REG_ADDR, val);
}

inline void write_OAM_data(struct memory *mem, const uint8_t val)
{
	// write to the register
	MEM_write(mem, MEM_PPU_OAMDATA_REG_ADDR, val);
	// increment OAM address register, too
	uint8_t incremented_val = MEM_read(mem, MEM_PPU_OAMADDR_REG_ADDR) + 1;
	MEM_write(mem, MEM_PPU_OAMADDR_REG_ADDR, incremented_val);
}

inline void write_data(struct memory *mem, const uint8_t val)
{
	// write to the register
	MEM_write(mem, MEM_PPU_DATA_REG_ADDR, val);
	// increment address register based on control register VRAM address
	// increment bit value (0 = add 1, 1 = add 32)
	uint8_t addr = MEM_read(mem, MEM_PPU_ADDR_REG_ADDR);
	if ((MEM_read(mem, MEM_PPU_STATUS_REG_ADDR) & 1<<2) == 0) {
		MEM_write(mem, MEM_PPU_ADDR_REG_ADDR, addr + 1);
	} else {
		MEM_write(mem, MEM_PPU_ADDR_REG_ADDR, addr + 32);
	}
}

inline uint8_t vblank_is_enabled(struct memory *mem)
{
	uint8_t ctrl = MEM_read(mem, PPUCTRL_ADDR);
	return ctrl & (1<<7);
}

inline void set_vblank_flag(struct memory *mem)
{
	uint8_t status = MEM_read(mem, PPUSTATUS_ADDR);
	status |= (1<<7);
	MEM_write(mem, PPUSTATUS_ADDR, status);
	(void)printf("VBLANK set\n");
}

inline void clear_vblank_flag(struct memory *mem)
{
	uint8_t status = MEM_read(mem, PPUSTATUS_ADDR);
	status &= ~(1<<7);
	MEM_write(mem, PPUSTATUS_ADDR, status);
	(void)printf("VBLANK cleared\n");
}

inline void clear_sprite_overflow_flag(struct memory *mem)
{
	uint8_t status = MEM_read(mem, PPUSTATUS_ADDR);
	status &= ~(1<<5);
	MEM_write(mem, PPUSTATUS_ADDR, status);
}

inline void clear_sprite_0_hit_flag(struct memory *mem)
{
	uint8_t status = MEM_read(mem, PPUSTATUS_ADDR);
	status &= ~(1<<6);
	MEM_write(mem, PPUSTATUS_ADDR, status);
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

inline void render(struct ppu *ppu, struct memory *mem, struct ppu_memory *ppu_mem)
{
	/* VBLANK, sprite overflow, and sprite 0 hit flags are cleared at the 2nd cycle of scanline 261 */
	if (ppu->line == 261 && ppu->dot == 1) {
		clear_vblank_flag(mem);
		clear_sprite_overflow_flag(mem);
		clear_sprite_0_hit_flag(mem);
	}

	if((ppu->dot >= 275) && (ppu->dot <= 320)) {
		write_OAM_addr(mem, 0);
	}

	if ((ppu->dot < 257) && (ppu->dot > 0) && (ppu->dot >= 321)) {
		uint8_t fetch_cycle = ppu->dot % 8;

		switch(fetch_cycle) {
			case 0:
				// increment horizontal
				break;
			case 1:
				// fetch nametable address
				break;
			case 3:
				// fetch attribute table address
				break;
			case 5:
				// fetch low background tile byte
				break;
			case 7:
				// fetch high background tile byte
				break;
		}
	}
}

uint8_t PPU_step(struct ppu *ppu, struct memory *mem, struct ppu_memory *ppu_mem)
{
	// (void)printf("PPU line: %d, dot: %d\n", ppu->line, ppu->dot);

	/* VBLANK flag is set at the 2nd cycle of scanline 241.  This is the
	 * start of the VBLANKing interval. */
	if (ppu->dot == 1 && ppu->line == 241) {
		set_vblank_flag(mem);

		/* This return indicates an NMI to the CPU */
		if (vblank_is_enabled(mem) != 0) {
			(void)printf("Executing VBLANK\n");
			return 0;
		}
	}

	if (ppu->line < 240 || ppu->line == 261) {
		render(ppu, mem, ppu_mem);	
	}

	increment_cycle(ppu);
	return 1;
}
