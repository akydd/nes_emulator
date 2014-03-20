/*
 * =====================================================================================
 *
 *       Filename:  input_processor.c
 *
 *    Description:  SDL implementation of the NES's input handler
 *
 *        Version:  1.0
 *        Created:  14-03-10 09:45:09 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alan Kydd
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <SDL2/SDL.h>

#include "input_processor.h"

struct input_processor {
	SDL_Event event;
};

struct input_processor *INPUT_init(const uint8_t **keypresses)
{
	// SDL_Init(SDL_INIT_VIDEO);
	*keypresses = SDL_GetKeyboardState(NULL);

	return malloc(sizeof(struct input_processor));
}

void INPUT_delete(struct input_processor **processor)
{
	free(*processor);
	// SDL_Quit();
}

uint8_t process_input(const uint8_t *state)
{
	return state[SDL_SCANCODE_Z]<<7 | state[SDL_SCANCODE_X]<<6 | state[SDL_SCANCODE_Q]<<5 | state[SDL_SCANCODE_W]<<4 | state[SDL_SCANCODE_UP]<<3 | state[SDL_SCANCODE_DOWN]<<2 | state[SDL_SCANCODE_LEFT]<<1 | state[SDL_SCANCODE_RIGHT]<<0;
}

void INPUT_process(struct input_processor *processor, struct controller *controller, int *nes_state, const uint8_t **keys)
{
	// Handle keyboard input and quit event
	if (SDL_PollEvent(&processor->event) != 0) {
		switch (processor->event.type) {
			case SDL_QUIT:
				*nes_state = 0;
				break;
			case SDL_KEYDOWN:
				switch (processor->event.key.keysym.sym) {
					case SDLK_ESCAPE:
					case SDLK_q:
						*nes_state = 0;
						break;
					case SDLK_r:
						*nes_state = 2;
						break;
				}
				CONTROLLER_set_keys(controller, process_input(*keys));
				break;
			case SDL_KEYUP:
				CONTROLLER_set_keys(controller, process_input(*keys));
				break;
		}
	}
}
