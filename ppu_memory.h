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
 * TODO
 *
 */

struct ppu_memory *PPU_MEM_init();

void PPU_MEM_delete(struct ppu_memory **);

uint8_t PPU_MEM_read(struct ppu_memory *, const uint16_t);

void PPU_MEM_write(struct ppu_memory *, const uint16_t, uint8_t);

#endif
