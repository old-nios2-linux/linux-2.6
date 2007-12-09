/*
 *  linux/arch/lm32/lib/strnlen_user.S
 *
 *  Copyright (C) 2007 Andrea della Porta
 *  based on the ARM port from Russel King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/linkage.h>
#include <asm/assembler.h>
#include <asm/errno.h>

	.text
	.align	32

/* Prototype: unsigned long __strnlen_user(const char *str, long n)
 * Purpose  : get length of a string in user memory
 * Params   : str - address of string in user memory
 * Returns  : length of string *including terminator*
 *	      or zero on exception, or n + 1 if too long
 */
ENTRY(__strnlen_user)
	mv	r4, r1
1:
/*USER(	ldrbt	r3, [r1], #1)*/
	lbu 	r3, (r1+0)	//add fixup awareness on Mico32
	addi	r1, r1, 1
	be	r3, r0, 2f
	addi	r2, r2, -1
	bg	r2, r0, 1b
	addi	r1, r1, 1
2:	sub	r1, r1, r4
	ret
/*
	.section .fixup,"ax"
	.align	0
9001:	mov	r0, #0
	restore_pc
	.previous
*/
