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
#include "ppu_memory.h"
#include "ppu.h"
#include "loader.h"


int main(int argc, char **argv)
{
	/* Check for input file */
	if (argc !=2) {
		(void)printf("You must enter a filename!\n");
		return 1;
	}

	/* initialize memory and load data */
	struct memory *mem = MEM_init();
	struct ppu_memory *ppu_mem = PPU_MEM_init();
	if(LOADER_load_file(mem, ppu_mem, *++argv) == 0) {
		(void)printf("Could not load file '%s'.  Exiting main program.\n", *argv);
		MEM_delete(&mem);
		PPU_MEM_delete(&ppu_mem);
		return 1;
	}

	/* Initialize the CPU and PPU */
	struct cpu *cpu = CPU_init(mem);
	struct ppu *ppu = PPU_init(mem, ppu_mem);


	/* Execution: */
	uint8_t cpu_cycles = 0;
	uint8_t ppu_cycles;
	uint8_t elapsed_ppu_cycles = 0;
	for(;;) {
		cpu_cycles = CPU_step(cpu);

		/* PPU steps 3 times for each CPU step */
		for(ppu_cycles = 0; ppu_cycles < 4; ppu_cycles++) {
			elapsed_ppu_cycles += PPU_step(ppu, elapsed_ppu_cycles);
		}
	}

	/*
	 * Shutdown
	 */
	CPU_delete(&cpu);
	PPU_delete(&ppu);
	PPU_MEM_delete(&ppu_mem);
	MEM_delete(&mem);
	return 0;
}
