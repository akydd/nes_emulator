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
#include <inttypes.h>
#include <stdio.h>
#include <SDL2/SDL.h>

#include "cpu.h"
#include "memory.h"
#include "ppu_memory.h"
#include "ppu.h"
#include "loader.h"
#include "input_processor.h"

// TODO: move SDL window stuff to a separate render module?
const int SCREEN_WIDTH = 256;
const int SCREEN_HEIGHT = 240;

int main(int argc, char **argv)
{
	/* Check for input file */
	if (argc < 2 || argc > 3) {
		(void)printf("Wrong number of  arguments.  You must enter a filename, and optionally specify an address to start CPU execution.\n");
		return 1;
	}

	char *filename = NULL;
	uint16_t pc;
	int use_pc = 0;
	int j;
	for(j = 1; j < argc; j++) {
		switch(argv[j][0]) {
			case '-':
				switch(argv[j][1]) {
					case 's':
						if (sscanf(argv[j] + 2, "%"SCNx16, &pc) != 1) {
							(void)printf("Unable to parse execution address '%s'.  Using default instead.\n", argv[j] + 2);
						} else {
							(void)printf("Execution point set to %#x.\n", pc);
							use_pc = 1;
						}
						break;
					default:
						(void)printf("Unrecognized option '%s'", argv[j]);
				}
				break;
			default:
				filename = argv[j];
		}
	}

	if (filename == NULL) {
		(void)printf("You must enter a filename!\n");
		return 1;
	}

	/* initialize memory and load data */
	struct memory *mem = MEM_init();
	struct ppu_memory *ppu_mem = PPU_MEM_init();
	if(LOADER_load_file(mem, ppu_mem, filename) == 0) {
		(void)printf("Could not load file '%s'.  Exiting main program.\n", filename);
		MEM_delete(&mem);
		PPU_MEM_delete(&ppu_mem);
		return 1;
	}

	// Initialize the Controller, CPU and PPU.
	struct cpu *cpu;
	if (use_pc == 1) {
		cpu = CPU_init_to_address(mem, pc);
	} else {
		cpu = CPU_init(mem);
	}
	struct ppu *ppu = PPU_init();
	struct controller *gamepad = CONTROLLER_init();
	const uint8_t *keys;
	struct input_processor *input_processor = INPUT_init(&keys);
	MEM_attach_controller(mem, gamepad);
	MEM_attach_ppu(mem, ppu);

	// Setup SDL
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window *window = SDL_CreateWindow("nes_emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

	/* Execution: */
	uint16_t cpu_cycles = 0;
	uint16_t i;
	uint8_t ppu_result = 0;
	int nes_state = 1;
	while(nes_state != 0) {
		// Handle keyboard input and quit event
		INPUT_process(input_processor, gamepad, &nes_state, &keys);

		// Handle soft reset
		if(nes_state == 2) {
			nes_state = 1;
			CPU_reset(cpu, mem);
			// TODO: reset the PPU
		}

		// Execute the cpu step
		cpu_cycles = CPU_step(cpu, mem);

		// PPU steps 3 times for each CPU step
		for(i = 0; i < 3 * cpu_cycles; i++) {
			ppu_result = PPU_step(ppu, ppu_mem);

			if(ppu_result == 0) {
				CPU_handle_nmi(cpu, mem);
				break;
			}
		}
#ifdef BLARGG 
		MEM_print_test_status(mem);
#endif
	}

	/*
	 * Shutdown
	 */
	(void)printf("Starting shutdown\n");
	INPUT_delete(&input_processor);
	CPU_delete(&cpu);
	PPU_delete(&ppu);
	CONTROLLER_delete(&gamepad);
	PPU_MEM_delete(&ppu_mem);
	MEM_delete(&mem);

	SDL_DestroyWindow(window);
	SDL_Quit();
	(void)printf("Shutdown complete!\n");
	return 0;
}
