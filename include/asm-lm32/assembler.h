/*
 *  linux/include/asm-lm32/assembler.h
 *
 *  Lattice's Mico32 port by Andrea della Porta (sfaragnaus@gmail.com) 
 *  base upon ARM's port from Russel King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  This file contains arm architecture specific defines
 *  for the different processors.
 *
 *  Do not include any C declarations in this file - it is included by
 *  assembler source.
 */
#ifndef __ASSEMBLY__
#error "Only include this from assembly code"
#endif

#include <asm/ptrace.h>
#include <asm/stack.h>


/*
 * Enable and disable interrupts
 */
	.macro	disable_irq
		pushfd r1
	        rcsr r1, IE
		andi r1, r1, 0xE
		wcsr IE, r1   
		popfd r1
	.endm

	.macro	enable_irq
		pushfd r1
	        rcsr r1, IE
		ori  r1, r1, 1
		wcsr IE, r1   
		popfd r1
	.endm

/*
 * Save the current IRQ state and disable IRQs.  
 */
	.macro	save_and_disable_irqs, oldcpsr
	        rcsr \oldcpsr, IE
		disable_irq
	.endm

/*
 * Restore interrupt state previously stored in a register. 
 */
	.macro	restore_irqs, oldcpsr
		wcsr IE, \oldcpsr  
	.endm

	.macro  mva, rd, value
	        orhi     \rd, r0, hi(\value)
	        ori      \rd, \rd, lo(\value)
	.endm

/*
 * These two are used to save LR/restore PC over a user-based access.
 * The old 26-bit architecture requires that we do.  On 32-bit
 * architecture, we can safely ignore this requirement.
 */
	.macro	save_lr
	.endm

	.macro	restore_pc
	/*mov	pc, lr*/
	.endm


#define USER(x...)				\
9999:	x;					\
	.section __ex_table,"a";		\
	.align	3;				\
	.long	9999b,9001f;			\
	.previous

