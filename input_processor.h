/*
 * =====================================================================================
 *
 *       Filename:  input_processor.h
 *
 *    Description:  API for handling game input
 *
 *        Version:  1.0
 *        Created:  14-03-10 09:36:11 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alan Kydd
 *
 * =====================================================================================
 */

#ifndef INPUT_PROCESSOR_H
#define INPUT_PROCESSOR_H

#include <inttypes.h>

#include "controller.h"

struct input_processor;

extern struct input_processor *INPUT_init(const uint8_t **);

extern void INPUT_delete(struct input_processor **);

extern void INPUT_process(struct input_processor *, struct controller *, int *, const uint8_t **);

#endif
