/*
 *  linux/arch/arm/lib/copy_to_user.S
 *
 *  Author:	Nicolas Pitre
 *  Created:	Sep 29, 2005
 *  Copyright:	MontaVista Software, Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#include <linux/linkage.h>
#include <asm/assembler.h>

/*
 * Prototype:
 *
 *	size_t __copy_to_user(void *to, const void *from, size_t n)
 *
 * Purpose:
 *
 *	copy a block to user memory from kernel memory
 *
 * Params:
 *
 *	to = user memory
 *	from = kernel memory
 *	n = number of bytes to copy
 *
 * Return value:
 *
 *	Number of bytes NOT copied.
 */

	.macro ldr1w ptr reg abort
	ldr \reg, [\ptr], #4
	.endm

	.macro ldr4w ptr reg1 reg2 reg3 reg4 abort
	ldmia \ptr!, {\reg1, \reg2, \reg3, \reg4}
	.endm

	.macro ldr8w ptr reg1 reg2 reg3 reg4 reg5 reg6 reg7 reg8 abort
	ldmia \ptr!, {\reg1, \reg2, \reg3, \reg4, \reg5, \reg6, \reg7, \reg8}
	.endm

	.macro ldr1b ptr reg cond=al abort
	ldr\cond\()b \reg, [\ptr], #1
	.endm

	.macro str1w ptr reg abort
100:	strt \reg, [\ptr], #4
	.section __ex_table, "a"
	.long 100b, \abort
	.previous
	.endm

	.macro str8w ptr reg1 reg2 reg3 reg4 reg5 reg6 reg7 reg8 abort
	str1w \ptr, \reg1, \abort
	str1w \ptr, \reg2, \abort
	str1w \ptr, \reg3, \abort
	str1w \ptr, \reg4, \abort
	str1w \ptr, \reg5, \abort
	str1w \ptr, \reg6, \abort
	str1w \ptr, \reg7, \abort
	str1w \ptr, \reg8, \abort
	.endm

	.macro str1b ptr reg cond=al abort
100:	str\cond\()bt \reg, [\ptr], #1
	.section __ex_table, "a"
	.long 100b, \abort
	.previous
	.endm

	.macro enter reg1 reg2
	mov	r3, #0
	stmdb	sp!, {r0, r2, r3, \reg1, \reg2}
	.endm

	.macro exit reg1 reg2
	add	sp, sp, #8
	ldmfd	sp!, {r0, \reg1, \reg2}
	.endm

	.text

ENTRY(__copy_to_user)

//#include "copy_template.S"

	.section .fixup,"ax"
	.align 0

	pushfd	r4
       2:
	bg	r3, r0, 1f
	b	3f
       1:
	addi	r3, r3, -1
	lb	r4, (r2+0)
	sb	(r1+0), r4
	addi	r2, r2, 1
	addi	r1, r1, 1
       	b 2b
       3:
        mvi	r1, 0
	popfd	r4
       	ret
	
/*	copy_abort_preamble
	ldmfd	sp!, {r1, r2, r3}
	sub	r0, r0, r1
	rsb	r0, r0, r2
	copy_abort_end
*/
	.previous


