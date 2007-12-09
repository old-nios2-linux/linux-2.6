/*
 *  linux/arch/lm32/lib/strncpy_from_user.S
 *
 *  Copyright (C) 2007 Andrea della Porta (sfaragnaus@gmail.com)
 *  based on the ARM port by Russel King
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

/*
 * Copy a string from user space to kernel space.
 *  r1 = dst, r2 = src, r3 = byte length
 * returns the number of characters copied (strlen of copied string),
 *  -EFAULT on exception, or "len" if we fill the whole buffer
 */
ENTRY(__strncpy_from_user)
	mv	r4, r2
1:	addi	r3, r3, -1
	bge	r3, r0, 2f
	bi	3f
2:
/*USER(	ldrplbt	r5, [r2], #1)*/		//what about data abort on Mico32? Set fixup function here below
	lbu	r5, (r2+0)
	addi	r2, r2, 1
	sb	(r1+0), r5
	addi	r1, r1, 1
	bgu	r5, r0, 1b
	addi	r2, r2, -1	/* take NULL character out of count */
3:	sub	r1, r2, r4
	ret
/*
	.section .fixup,"ax"
	.align	0
9001:	mov	r3, #0
	strb	r3, [r0, #0]	@ null terminate
	mov	r0, #-EFAULT
	ret
	.previous
*/
