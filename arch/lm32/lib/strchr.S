/*
 *  linux/arch/lm32/lib/strchr.S
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
ENTRY(strchr)
		andi	r2, r2, 0xff
1:		lb	r3, (r1+0)
		be	r3, r2, 3f
		be	r3, r0, 2f
		addi	r1, r1, 1
		bi	1b
2:
		mv	r1, r0
3:		
		ret
