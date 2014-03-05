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

uint8_t process_input(const uint8_t *state)
{
	return state[SDL_SCANCODE_Z]<<7 | state[SDL_SCANCODE_X]<<6 | state[SDL_SCANCODE_Q]<<5 | state[SDL_SCANCODE_W]<<4 | state[SDL_SCANCODE_UP]<<3 | state[SDL_SCANCODE_DOWN]<<2 | state[SDL_SCANCODE_LEFT]<<1 | state[SDL_SCANCODE_RIGHT]<<0;
}

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
	struct ppu *ppu = PPU_init(mem);
	struct controller *gamepad = CONTROLLER_init();
	MEM_attach_controller(mem, gamepad);

	// Setup SDL
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

	/* Execution: */
	uint16_t cpu_cycles = 0;
	SDL_Event input_event;
	uint16_t i;
	uint8_t ppu_result = 0;
	for(;;) {
		while(SDL_PollEvent(&input_event)) {
			const uint8_t *state = SDL_GetKeyboardState(NULL);
			CONTROLLER_set_keys(gamepad, process_input(state));
		}

		cpu_cycles = CPU_step(cpu, mem);

		// PPU steps 3 times for each CPU step
		for(i = 0; i <= 3 * cpu_cycles; i++) {
			ppu_result = PPU_step(ppu, mem, ppu_mem);

			if(ppu_result == 0) {
				CPU_handle_nmi(cpu, mem);
			}
		}
#ifdef BLARGG 
		MEM_print_test_status(mem);
#endif
	}

	/*
	 * Shutdown
	 */
	CPU_delete(&cpu);
	PPU_delete(&ppu);
	CONTROLLER_delete(&gamepad);
	PPU_MEM_delete(&ppu_mem);
	MEM_delete(&mem);

	SDL_Quit();

	return 0;
}
