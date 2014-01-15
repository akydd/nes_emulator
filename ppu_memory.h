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
#include <stdio.h>

struct ppu_memory;
/*
 * Memory Map
 * ==========
 *
 * 16 kilobytes (0x4000 bytes) of memory.  It is organized as:
 *
 *  __________________
 * |                  | 0x3FFF   16 kb
 * | Mirror of        |
 * | 0x3F00 to 0x3F1F |
 * |__________________| 0x3F20
 * |                  | 0x3F1F
 * | Sprite Palette   |
 * |__________________| 0x3F10
 * |                  | 0x3F0F
 * | Background       |
 * | Palette          |
 * |__________________| 0x3F00
 * |                  | 0x3EFF
 * | Mirror of        |
 * | 0x2000 to 0x2EFF.|
 * |__________________| 0x3000
 * |                  | 0x2FFF   12 kb
 * | Attr. Table #3   |
 * |__________________| 0x2FC0
 * |                  | 0x2FBF
 * | Name Table #3    |
 * |__________________| 0x2C00
 * |                  | 0x2BFF
 * | Attr. Table #2   |
 * |__________________| 0x2BC0
 * |                  | 0x2BBF
 * | Name Table #2    |
 * |__________________| 0x2800
 * |                  | 0x27FF
 * | Attr. Table #1   |
 * |__________________| 0x27C0
 * |                  | 0x27BF
 * | Name Table #1    |
 * |__________________| 0x2400
 * |                  | 0x23FF
 * | Attr. Table #0   |
 * |__________________| 0x23C0
 * |                  | 0x23BF
 * | Name Table #0    |
 * |__________________| 0x2000
 * |                  | 0x1FFF   8 kb
 * | Pattern Table 1  |
 * | [upper CHR bank] |
 * |__________________| 0x1000	 
 * |                  | 0x0FFF
 * | Pattern Table 0  |
 * | [lower CHR bank] |
 * |__________________| 0x0000
 *
 *
 */

#define PPU_MEM_SIZE 0x4000


extern struct ppu_memory *PPU_MEM_init();

extern void PPU_MEM_delete(struct ppu_memory **);

extern uint8_t PPU_MEM_read(struct ppu_memory *, const uint16_t);

extern void PPU_MEM_write(struct ppu_memory *, const uint16_t, uint8_t);

extern void PPU_MEM_load_vrom(struct ppu_memory *, FILE *);

extern void PPU_MEM_set_mirroring(struct ppu_memory *, uint8_t);

#endif
