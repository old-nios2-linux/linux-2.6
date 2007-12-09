/*
 *  linux/arch/lm32/lib/delay.S
 *
 *  Copyright (C) 2007 Andrea della Porta (sfaragnaus@gmail.com)
 *  based upon ARM port from Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/linkage.h>
#include <asm/assembler.h>
#include <asm/param.h>
		.text

.LC0:		.word	loops_per_jiffy
.LC1:		.word	(2199023*HZ)>>11

/*
 * r1  <= 2000
 * lpj <= 0x01ffffff (max. 3355 bogomips)
 * HZ  <= 1000
 */

ENTRY(__udelay)
		mva	r2, .LC1
		mul	r1, r2, r1
ENTRY(__const_udelay)				/* 0 <= r1 <= 0x7fffff06 */
		mva	r2, .LC0
		lw	r2, (r2+0)		/* max = 0x01ffffff */
		srui	r1, r1, 14		/* max = 0x0001ffff */
		srui	r2, r2, 10		/* max = 0x00007fff */
		mul	r1, r2, r1		/* max = 2^32-1 */
		srui	r1, r1, 6
		bgu	r1, r0, __delay
		ret

/*
 * loops = r1 * HZ * loops_per_jiffy / 1000000
 *
 * Oh, if only we had a cycle counter...
 */

/* Delay routine */
ENTRY(__delay)
		addi	r1, r1, -1
#if 0
		RETINSTR(movls,pc,lr)
		subs	r0, r0, #1
		RETINSTR(movls,pc,lr)
		subs	r0, r0, #1
		RETINSTR(movls,pc,lr)
		subs	r0, r0, #1
		RETINSTR(movls,pc,lr)
		subs	r0, r0, #1
		RETINSTR(movls,pc,lr)
		subs	r0, r0, #1
		RETINSTR(movls,pc,lr)
		subs	r0, r0, #1
		RETINSTR(movls,pc,lr)
		subs	r0, r0, #1
#endif
		/*bhi	__delay	*/	/* check for wrap-around? */
		bgu	r1, r0, __delay
		ret

