/*
 * =============================================================================
 *
 *       Filename:  test_controller.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  13-03-30 01:03:49 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alan Kydd (), akydd@ualberta.net
 *
 * =============================================================================
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "controller.c"

#define mu_assert(message, test) do { if (!(test)) return message; } while (0)
#define mu_run_test(test) do { char *message = test(); tests_run++; \
	if (message) return message; } while (0)

int tests_run = 0;

struct controller *controller;

static char *test_controller_init()
{
	controller = CONTROLLER_init();

	mu_assert("strobe is not high", controller->strobe_state == strobe_high);
	mu_assert("Read position is not for button A", controller->read_position == A);
	mu_assert("Keys should not be pressed on init", controller->key_states == 0);

	CONTROLLER_delete(&controller);
	return 0;
}

static char *test_read_with_strobe_high()
{
	controller = CONTROLLER_init();
	controller->strobe_state = strobe_high;
	controller->key_states = 0x80;

	// Repeated reads should always give the state of button A
	mu_assert("Read position is not for button A", CONTROLLER_read(controller) == 0x41);
	mu_assert("Read position is not for button A", CONTROLLER_read(controller) == 0x41);

	controller->key_states = 0x00;

	mu_assert("Read position is not for button A", CONTROLLER_read(controller) == 0x40);
	mu_assert("Read position is not for button A", CONTROLLER_read(controller) == 0x40);

	CONTROLLER_delete(&controller);
	return 0;
}

static char *test_full_cycles()
{
	controller = CONTROLLER_init();
	CONTROLLER_write(controller, 1);
	CONTROLLER_write(controller, 0);
	// Pressed: A, SELECT, UP, LEFT
	controller->key_states = 0xAA;

	// Repeated reads should cycle through all buttons
	mu_assert("Incorrect for A", CONTROLLER_read(controller) == 0x41);
	mu_assert("Incorrect for B", CONTROLLER_read(controller) == 0x40);
	mu_assert("Incorrect for SELECT", CONTROLLER_read(controller) == 0x41);
	mu_assert("Incorrect for START", CONTROLLER_read(controller) == 0x40);
	mu_assert("Incorrect for UP", CONTROLLER_read(controller) == 0x41);
	mu_assert("Incorrect for DOWN", CONTROLLER_read(controller) == 0x40);
	mu_assert("Incorrect for LEFT", CONTROLLER_read(controller) == 0x41);
	mu_assert("Incorrect for RIGHT", CONTROLLER_read(controller) == 0x40);

	// Pressed: B, START, DOWN, RIGHT
	controller->key_states = 0x55;
	CONTROLLER_write(controller, 1);
	CONTROLLER_write(controller, 0);

	mu_assert("Incorrect for A", CONTROLLER_read(controller) == 0x40);
	mu_assert("Incorrect for B", CONTROLLER_read(controller) == 0x41);
	mu_assert("Incorrect for SELECT", CONTROLLER_read(controller) == 0x40);
	mu_assert("Incorrect for START", CONTROLLER_read(controller) == 0x41);
	mu_assert("Incorrect for UP", CONTROLLER_read(controller) == 0x40);
	mu_assert("Incorrect for DOWN", CONTROLLER_read(controller) == 0x41);
	mu_assert("Incorrect for LEFT", CONTROLLER_read(controller) == 0x40);
	mu_assert("Incorrect for RIGHT", CONTROLLER_read(controller) == 0x41);

	CONTROLLER_delete(&controller);
	return 0;
}

static char *test_read_position_cycles()
{
	controller = CONTROLLER_init();
	controller->strobe_state = strobe_low;
	controller->read_position = RIGHT;

	// Cycle should move from last button back to first and set strobe high
	(void)CONTROLLER_read(controller);
	mu_assert("Read position did not cycle", controller->read_position == A);
	mu_assert("Strobe not high", controller->strobe_state == strobe_high);

	CONTROLLER_delete(&controller);
	return 0;
}

static char *test_write_1_then_0_to_get_strobe_low()
{
	controller = CONTROLLER_init();

	// Cycle should move from last button back to first
	CONTROLLER_write(controller, 1);
	mu_assert("Controller pulled strobe low too soon", controller->strobe_state != strobe_low);
	CONTROLLER_write(controller, 0);
	mu_assert("Controller did not pull strobe low", controller->strobe_state == strobe_low);

	CONTROLLER_delete(&controller);
	return 0;
}

static char *all_tests()
{
	mu_run_test(test_controller_init);
	mu_run_test(test_full_cycles);
	mu_run_test(test_read_with_strobe_high);
	mu_run_test(test_read_position_cycles);
	mu_run_test(test_write_1_then_0_to_get_strobe_low);
	return 0;
}

int main()
{
	char *result = all_tests();
	if (result != 0) {
		(void) printf("%s\n", result);
	} else {
		(void) printf("All tests passed!\n");
	}
	(void) printf("Tests run: %d\n", tests_run);

	return result != 0;
}
