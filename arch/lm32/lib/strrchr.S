/*
 *  linux/arch/lm32/lib/strrchr.S
 *
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
ENTRY(strrchr)
		mvi	r3, 0
1:		lbu	r4, (r1+0)
		addi	r1, r1, 1
		be	r4, r0, 2f 
		bne	r4, r2, 1b
		addi	r3, r1, -1
		bi	1b
2:		mv	r1, r3
		ret

