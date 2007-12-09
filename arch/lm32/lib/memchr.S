/*
 *  linux/arch/lm32/lib/memchr.S
 *
 *  Copyright (C) 1995-2000 Russell King
 *  Copyright (C) 2007 Andrea della Porta (sfaragnaus@gmail.com) 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  ASM optimised string functions
 */
#include <linux/linkage.h>
#include <asm/assembler.h>

	.text
	.align	32
ENTRY(memchr)
1:	addi	r3, r3, -1
	bg	r0, r3, 2f
	lb	r4, (r1+0)
	addi	r1, r1, 1
	bne	r4, r2, 1b
	addi	r1,  r1, -1
	bi	3f
2:	mv	r1, r0
3:	ret
