/*
 *  linux/include/asm-arm/xor.h
 *
 *  Copyright (C) 2001 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <asm-generic/xor.h>

#define __XOR(a1, a2) a1 ^= a2

#define GET_BLOCK_2(dst) \
	__asm__("mv	r1, %0\n\t" 	\
		"lw	%1, (r1+0)\n\t" \
		"lw	%2, (r1+4)" 	\
		: "=r" (dst), "=r" (a1), "=r" (a2) \
		: "0" (dst) : "r1" )

#define GET_BLOCK_4(dst) \
	__asm__("mv     r1, %0\n\t"     \
		"lw     %1, (r1+0)\n\t" \
		"lw     %2, (r1+4)\n\t" \
		"lw     %3, (r1+8)\n\t" \
		"lw     %4, (r1+12)"	\
		: "=r" (dst), "=r" (a1), "=r" (a2), "=r" (a3), "=r" (a4) \
		: "0" (dst) : "r1" )

#define XOR_BLOCK_2(src) \
	__asm__("mv     r1, %0\n\t"     \
		"lw     %1, (r1+0)\n\t" \
		"lw     %2, (r1+4)\n\t" \
		"addi	%0, %0, 8" \
		: "=r" (src), "=r" (b1), "=r" (b2) \
		: "0" (src) : "r1" ); \
	__XOR(a1, b1); __XOR(a2, b2);

#define XOR_BLOCK_4(src) \
	__asm__("mv     r1, %0\n\t"     \
		"lw     %1, (r1+0)\n\t" \
		"lw     %2, (r1+4)\n\t" \
		"lw     %3, (r1+8)\n\t" \
		"lw     %4, (r1+12)\n\t" \
		"addi   %0, %0, 16" \
		: "=r" (src), "=r" (b1), "=r" (b2), "=r" (b3), "=r" (b4) \
		: "0" (src) : "r1" ); \
	__XOR(a1, b1); __XOR(a2, b2); __XOR(a3, b3); __XOR(a4, b4)

#define PUT_BLOCK_2(dst) \
	__asm__ __volatile__(			\
			"mv     r1, %0\n\t"     \
			"sw     (r1+0), %2\n\t" \
			"sw     (r1+4), %3\n\t" \
			"addi	%0, %0, 8" \
			: "=r" (dst) \
			: "0" (dst), "r" (a1), "r" (a2) : "r1" )

#define PUT_BLOCK_4(dst) \
	__asm__ __volatile__(			\
                        "mv     r1, %0\n\t"     \
			"sw     (r1+0), %2\n\t" \
			"sw     (r1+4), %3\n\t" \
			"sw     (r1+8), %4\n\t" \
			"sw     (r1+12), %5\n\t" \
			"addi   %0, %0, 16" \
			: "=r" (dst) \
			: "0" (dst), "r" (a1), "r" (a2), "r" (a3), "r" (a4) : "r1")

static void
xor_arm4regs_2(unsigned long bytes, unsigned long *p1, unsigned long *p2)
{
	unsigned int lines = bytes / sizeof(unsigned long) / 4;
	register unsigned int a1 __asm__("r10");		//are these regs automatically saved on stack by the 'register' keyword? Remeber they should be callee saved regs
	register unsigned int a2 __asm__("r11");
	register unsigned int a3 __asm__("r12");
	register unsigned int a4 __asm__("r13");
	register unsigned int b1 __asm__("r14");
	register unsigned int b2 __asm__("r15");
	register unsigned int b3 __asm__("r16");
	register unsigned int b4 __asm__("r17");

	do {
		GET_BLOCK_4(p1);
		XOR_BLOCK_4(p2);
		PUT_BLOCK_4(p1);
	} while (--lines);
}

static void
xor_arm4regs_3(unsigned long bytes, unsigned long *p1, unsigned long *p2,
		unsigned long *p3)
{
	unsigned int lines = bytes / sizeof(unsigned long) / 4;
	register unsigned int a1 __asm__("r10");
	register unsigned int a2 __asm__("r11");
	register unsigned int a3 __asm__("r12");
	register unsigned int a4 __asm__("r13");
	register unsigned int b1 __asm__("r14");
	register unsigned int b2 __asm__("r15");
	register unsigned int b3 __asm__("r16");
	register unsigned int b4 __asm__("r17");

	do {
		GET_BLOCK_4(p1);
		XOR_BLOCK_4(p2);
		XOR_BLOCK_4(p3);
		PUT_BLOCK_4(p1);
	} while (--lines);
}

static void
xor_arm4regs_4(unsigned long bytes, unsigned long *p1, unsigned long *p2,
		unsigned long *p3, unsigned long *p4)
{
	unsigned int lines = bytes / sizeof(unsigned long) / 2;
	register unsigned int a1 __asm__("r10");
	register unsigned int a2 __asm__("r11");
	register unsigned int b1 __asm__("r12");
	register unsigned int b2 __asm__("r13");

	do {
		GET_BLOCK_2(p1);
		XOR_BLOCK_2(p2);
		XOR_BLOCK_2(p3);
		XOR_BLOCK_2(p4);
		PUT_BLOCK_2(p1);
	} while (--lines);
}

static void
xor_arm4regs_5(unsigned long bytes, unsigned long *p1, unsigned long *p2,
		unsigned long *p3, unsigned long *p4, unsigned long *p5)
{
	unsigned int lines = bytes / sizeof(unsigned long) / 2;
	register unsigned int a1 __asm__("r10");
	register unsigned int a2 __asm__("r11");
	register unsigned int b1 __asm__("r12");
	register unsigned int b2 __asm__("r13");

	do {
		GET_BLOCK_2(p1);
		XOR_BLOCK_2(p2);
		XOR_BLOCK_2(p3);
		XOR_BLOCK_2(p4);
		XOR_BLOCK_2(p5);
		PUT_BLOCK_2(p1);
	} while (--lines);
}

static struct xor_block_template xor_block_arm4regs = {
	.name	= "arm4regs",
	.do_2	= xor_arm4regs_2,
	.do_3	= xor_arm4regs_3,
	.do_4	= xor_arm4regs_4,
	.do_5	= xor_arm4regs_5,
};

#undef XOR_TRY_TEMPLATES
#define XOR_TRY_TEMPLATES			\
	do {					\
		xor_speed(&xor_block_arm4regs);	\
		xor_speed(&xor_block_8regs);	\
		xor_speed(&xor_block_32regs);	\
	} while (0)
