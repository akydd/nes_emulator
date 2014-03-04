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

struct controller {
	int state; // 0 - just strobed, 1 - ready to strobe, 2 - reading

	// 1 bit per button, in this order (left to right):
	// A, B, Select, Start, Up, Down, Left, Right
	uint8_t key_state;
	uint8_t read_number;
};

struct controller *CONTROLLER_init()
{
	struct controller *controller = malloc(sizeof(struct controller));
	controller->read_number = 1<<7;
	controller->state = 0;

	return controller;
}

void CONTROLLER_delete(struct controller **controller)
{
	free(*controller);
	*controller = NULL;
}

void strobe(struct controller *controller)
{
	controller->read_number = 1<<7;
	controller->state = 0;
}

uint8_t CONTROLLER_read(struct controller *controller)
{
	uint8_t result = 0;

	if (controller->state == 0) {
		controller->state = 2;
	}

	if (controller->state == 2) {
		result = (controller->state && controller->read_number);
		controller->read_number>>1;
	}

	return (result != 0);
}

void CONTROLLER_write(struct controller *controller, const uint8_t val)
{
	if (val == 1) {
		controller->state = 1;
		return;
	}

	if (val == 0 && controller->state == 1) {
		strobe(controller);
		return;
	}

	return;
}
