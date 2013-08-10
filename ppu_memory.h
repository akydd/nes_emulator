/*
 * =====================================================================================
 *
 *       Filename:  ppu_memory.h
 *
 *    Description:  Interface for PPU Memory
 *
 *        Version:  1.0
 *        Created:  13-08-09 11:06:07 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alan Kydd (), akydd@ualberta.net
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef PPU_MEMORY_H
#define PPU_MEMORY_H

#include <stdint.h>

struct ppu_memory;
/*
 * Memory Map
 * ==========
 *
 * 16 kilobytes (0x4000 bytes) of memory.  It is organized as:
 *
 *  __________________
 * |                  | 0x3FFF
 * | Mirrors of       |
 * | 0x3F00 to 0X3F1F |
 * |__________________| 0x3F20
 * |                  | 0x3F1F
 * | Palette RAM      |
 * | indexes          |
 * | [not RGB values] |
 * |__________________| 0x3F00
 * |                  | 0x3EFF
 * | Mirrors of       |
 * | 0x2000 to ox2EFF |
 * |__________________| 0x3000
 * |                  | 0x2FFF
 * | Name Table #3    |
 * |__________________| 0x2C00
 * |                  | 0x2BFF
 * | Name Table #2    |
 * |__________________| 0x2800
 * |                  | 0x27FF
 * | Name Table #1    |
 * |__________________| 0x2400
 * |                  | 0x23FF
 * | Name Table #0    |
 * |__________________| 0x2000
 * |                  | 0x1FFF
 * | Pattern Table 1  |
 * | [upper CHR bank] |
 * |__________________| 0x1000	 
 * |                  | 0x0FFF
 * | Pattern Table 0  |
 * | [lower CHR bank] |
 * |__________________| 0x0000
 *
 *
 *
 *
 */

struct ppu_memory *PPU_MEM_init();

void PPU_MEM_delete(struct ppu_memory **);

uint8_t PPU_MEM_read(struct ppu_memory *, const uint16_t);

void PPU_MEM_write(struct ppu_memory *, const uint16_t, uint8_t);

#endif
