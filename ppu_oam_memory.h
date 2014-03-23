/*
 * =====================================================================================
 *
 *       Filename:  ppu_oam_memory.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  14-03-23 12:02:09 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef PPU_OAM_MEMORY_H
#define PPU_OAM_MEMORY_H

#include <stdint.h>

/*
 * Sprite info is layed out in 4 bytes:
 *
 * Byte 0: Y position of the top of the sprite
 * Byte 1: Tile index number
 * Byte 2: Attribute
 * 76543210
 * ||||||||
 * ||||||++- Palette (4 to 7) of sprite
 * |||+++--- Unimplemented
 * ||+------ Priority (0: in front of background; 1: behind background)
 * |+------- Flip sprite horizontally
 * +-------- Flip sprite vertically
 *
 * Byte 3: X position of the left side of the sprite
 */
struct ppu_oam_memory;

#endif
