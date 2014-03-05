/*
 * =====================================================================================
 *
 *       Filename:  controller.h
 *
 *    Description:  Public interface to a NES Gamepad
 *
 *        Version:  1.0
 *        Created:  14-03-03 09:51:04 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alan Kydd
 *
 * =====================================================================================
 */

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <stdint.h>

struct controller;

extern struct controller *CONTROLLER_init();

extern void CONTROLLER_delete(struct controller **);

extern void CONTROLLER_write(struct controller *, const uint8_t);

extern uint8_t CONTROLLER_read(struct controller *);

extern void CONTROLLER_set_keys(struct controller *, const uint8_t);

#endif
