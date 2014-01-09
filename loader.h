/*
 * =====================================================================================
 *
 *       Filename:  loader.h
 *
 *    Description:  Functions for loading data into memory
 *
 *        Version:  1.0
 *        Created:  13-08-09 10:54:35 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alan Kydd (), akydd@ualberta.net
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef LOADER_H
#define LOADER_H

#include "memory.h"
#include "ppu_memory.h"

/*
 * Load the specified file into CPU and PPU memory.  Returns 1 on success, 0 otherwise.
 */
extern int LOADER_load_file(struct memory *, struct ppu_memory *, char *filename);

#endif
