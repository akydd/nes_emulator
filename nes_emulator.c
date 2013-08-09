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


int main(int argc, char **argv)
{
	/* Check for input file */
	if (argc !=2) {
		(void)printf("You must enter a filename!\n");
		return 1;
	}

	/* initialize memory and load data */
	struct memory *mem = MEM_init();
	if(MEM_load_file(mem, *++argv) == 0) {
		(void)printf("Could not load file '%s'.  Exiting main program.\n", *argv);
		MEM_delete(&mem);
		return 1;
	}

	/* Initialize the CPU */
	struct cpu *cpu = CPU_init(mem);

	/*
	 * Initialize PPU with registers set to 0x2000 to 0x2007 in main mem.
	 */

	/* Execution: */
	int cpu_cycles = 0;
	for(;;) {
		cpu_cycles = CPU_step(cpu);

		/*
		   int ppu_cycles = 3 * cpu_cycles;
		   while(ppu_cycles > 0)
		   {
		   ppu_execute();
		 *
		 }
		 */
	}

	/*
	 * Shutdown
	 */
	CPU_delete(&cpu);
	return 0;
}
