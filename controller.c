/*
 * =====================================================================================
 *
 *       Filename:  controller.c
 *
 *    Description:  Implementation of a NES controller
 *
 *        Version:  1.0
 *        Created:  14-03-03 10:05:48 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alan Kydd
 *
 * =====================================================================================
 */
#include <stdlib.h>

#include "controller.h"

#define A 1<<7
#define B 1<<6
#define SELECT 1<<5
#define START 1<<4
#define UP 1<<3
#define DOWN 1<<2
#define LEFT 1<<1
#define RIGHT 1<<0

enum state
{
	strobe_low,
	strobe_staged,
	strobe_high
};

struct controller {
	enum state strobe_state;

	// 1 bit per button, in this order (left to right):
	// A, B, Select, Start, Up, Down, Left, Right
	uint8_t key_states;
	uint8_t read_position;
};

struct controller *CONTROLLER_init()
{
	struct controller *controller = malloc(sizeof(struct controller));
	controller->read_position = A;
	controller->strobe_state = strobe_high;

	return controller;
}

void CONTROLLER_delete(struct controller **controller)
{
	free(*controller);
	*controller = NULL;
}

void strobe(struct controller *controller)
{
	controller->read_position = A;
	controller->strobe_state = strobe_low;
}

void shift_read_position(struct controller *controller)
{
	(controller->read_position)>>1;

	// shift read position back to the beginning if we've run through all
	// the buttons.
	if(controller->read_position == 0) {
		controller->read_position = A;
	}
}

uint8_t CONTROLLER_read(struct controller *controller)
{
	uint8_t result = 0;

	if (controller->strobe_state != strobe_low) {
		result = controller->key_states & A; 
	}

	if (controller->strobe_state == strobe_low) {
		result = (controller->key_states & controller->read_position);
		shift_read_position(controller);	
	}

	return ((result != 0) | 0x40);
}

void CONTROLLER_set_keys(struct controller *controller, const uint8_t state)
{
	controller->key_states = state;
}

void CONTROLLER_write(struct controller *controller, const uint8_t val)
{
	if (val == 1) {
		controller->strobe_state = strobe_staged;
		return;
	}

	if (val == 0 && controller->strobe_state == strobe_staged) {
		strobe(controller);
		return;
	}

	return;
}
