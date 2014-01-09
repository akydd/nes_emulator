/*
 * =============================================================================
 *
 *       Filename:  cpu.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  13-03-28 08:39:54 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alan Kydd (), akydd@ualberta.net
 *
 * =============================================================================
 */

#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include "memory.h"

/*
 * Initialize the cpu with starting values.
 * Memory must be initialized before passing into this function.
 */
extern struct cpu *CPU_init(struct memory *);

/*
 * Delete cpu
 */
extern void CPU_delete(struct cpu **);

/*
 * Perform the next instruction.
 */
extern int CPU_step(struct cpu *, struct memory *);

/*
 * Interrupt handler
 */
extern void CPU_handle_nmi(struct cpu *, struct memory *);

#endif
