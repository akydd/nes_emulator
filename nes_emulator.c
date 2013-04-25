/*
 * ============================================================================
 *
 *       Filename:  nes_emulator.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  12-10-27 01:03:51 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alan Kydd (), akydd@ualberta.net
 *   Organization:  
 *
 * ============================================================================
 */
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "cpu.h"
#include "instructions.h"
#include "memory.h"

/* func declarations */

int main(void)
{
	/*
	 * Load file into memory
	 */

	/*
	 * Initialize CPU with cpu->PC and cpu->S pointing to memory.
	 */
	
	/*
	 * Initialize PPU with registers set to 0x2000 to 0x2007 in main mem.
	 */

	/* Execution:
	 * for(;;)
	 * {
	 *	int op_code = &(cpu->PC);
	 *	int cpu_cycles = cycles[op_code];
	 *
	 *	op[op_code]();
	 *
	 *	int ppu_cycles = 3 * cpu_cycles;
	 *	while(ppu_cycles > 0)
	 *	{
	 *		ppu_execute();
	 *	}
	 * }
	 */
	return 0;
}
