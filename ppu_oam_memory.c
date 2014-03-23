/*
 * =====================================================================================
 *
 *       Filename:  ppu_oam_memory.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  14-03-23 12:03:24 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdlib.h>

#include "ppu_oam_memory.h"

struct oam_data {
	uint8_t y_pos;
	uint8_t index;
	uint8_t attribute;
	uint8_t x_pos;
};

struct ppu_oam_memory {
	struct oam_data primary[64];
	struct oam_data secondary[8];
};
