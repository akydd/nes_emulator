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
 * Backgroud Palettes
 * ==================
 * There are 4 background palettes of 4 colors, 1 byte per color, for a total of
 * 16 bytes.
 *
 *
 * Colors
 * ======
 * Colors take values 0x00 to 0x3F, for a total of 64 colors.  56 of them are
 * unique.
 *
 * The NES PPU does not use RGB, it uses NTSC to generate the colors.
 * http://wiki.nesdev.com/w/index.php/NTSC_video
 *
 * For list of the approx RGB values for these colors see
 * http://wiki.nesdev.com/w/index.php/PPU_palettes#RGB_palette_values
 *
 *
 * Name Tables
 * ===========
 * Each name table contains 960 bytes, which split up the screen into 30
 * rows, top to bottom, each with 32 columns, left to right.  Usually
 * the top and bottom most rows are not visible.  Each cell is 8x8 pixels.
 *
 *
 * Attribute Tables
 * ================
 * Each attribute table contains 64 bytes, or 512 bits, which are used to color
 * in the above 960 tiles, via compression.
 *
 * The data stored is the upper two bits of the background tiles colors, indexed
 * by 4 color palettes.  Each color palette contains 4 colors.  So
 * each attribute table contains 256 background color assignments.  Rounding
 * up, this means that 4 tiles must be handled by each color assignment, or
 * that 16 tiles must be handled by each byte in the attribute table.  This is
 * accomplished by grouping the 960 cells in the name tables into larger cells
 * of 4x4 name table cells, subdivided into 4 quadrants of 2x2 name table cells.
 * Each of these larger cells is 32x32 pixels.
 *
 * The assignment of each bytes to a 32x32 pixel cell is AABBCCDD to
 * DD CC
 * BB AA
 *
 *
 * Pattern Tables
 * ==============
 * Each pattern table is 4096 bytes.  An 8x8 pixel tile uses 16 bytes, 2 bytes
 * per 8X1 pixel row, for a total of 256 tiles.  For each 8x1 pixel row, the
 * first byte holds the low color bit, and the second byte holds the high color bit.
 * 
 *
 * Putting these together
 * ======================
 * Each 32x32 tile can contain at most 16 colors.
 *
 *
 * Each scanline, the PPU looks into the name tables 34 times and draws 8x1 pixels
 * for each table for the first 33 of those fetches.
 */

#define PPU_MEM_SIZE 0x4000


extern struct ppu_memory *PPU_MEM_init();

extern void PPU_MEM_delete(struct ppu_memory **);

extern uint8_t PPU_MEM_read(struct ppu_memory *, const uint16_t);

extern void PPU_MEM_write(struct ppu_memory *, const uint16_t, uint8_t);

extern void PPU_MEM_load_vrom(struct ppu_memory *, FILE *);

/*
 * 0 = horizontal mirroring, 1 = vertical mirroring
 */ 
extern void PPU_MEM_set_mirroring(struct ppu_memory *, const uint8_t);

#endif
