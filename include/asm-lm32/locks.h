/*
 *  linux/include/asm-arm/locks.h
 *
 *  Lattice's Mico32 support by Andrea della Porta (sfaragnaus@gmail.com)
 *  based upon the ARM port from Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  Interrupt safe locking assembler. 
 */
#ifndef __LM32_PROC_LOCKS_H
#define __LM32_PROC_LOCKS_H


#define __down_op(ptr,fail)			\
	({					\
	__asm__ __volatile__(			\
	"/* down_op */\n"				\
"	rcsr r3, IE\n"				\
"	addi    sp, sp, -4\n"			\
"	sw      (sp+0), r1\n"			\
"	rcsr r1, IE\n"				\
"	andi r1, r1, 0xE\n"			\
"	wcsr IE, r1\n"				\
"	lw      r1, (sp+0)\n"			\
"	addi    sp, sp, 4\n"			\
"	mv	r2, %0\n"			\
"	lw	r1, (r2+0)\n"			\
"	add	r1, r1, %1\n"			\
"	sw	(r2+0), r1\n"			\
"	wcsr IE, r3\n"				\
"	bge	r1, r0, 1f\n"	\
"	calli	" #fail	"\n"			\
"	1:"			\
	:					\
	: "r" (ptr), "r" (-1)			\
	: "r1", "r2", "r3");			\
	smp_mb();				\
	})

#define __down_op_ret(ptr,fail)			\
	({					\
		unsigned int ret;		\
	__asm__ __volatile__(			\
	"/* down_op_ret */\n"			\
"	rcsr r3, IE\n"				\
"	addi    sp, sp, -4\n"			\
"	sw      (sp+0), r1\n"			\
"	rcsr r1, IE\n"				\
"	andi r1, r1, 0xE\n"			\
"	wcsr IE, r1\n"				\
"	lw      r1, (sp+0)\n"			\
"	addi    sp, sp, 4\n"			\
"       mv      r2, %1\n"                       \
"       lw      r1, (r2+0)\n"                   \
"	add	r1, r1, %2\n"			\
"	sw	(r2+0), r1\n"			\
"	wcsr IE, r3\n"				\
"	bge	r1, r0, 1f\n"	\
"	calli	" #fail "\n"			\
"	bi	2f\n"		\
"	1:\n"			\
"	mv	r2, r0\n"			\
"	2:\n"			\
"	mv	%0, r2"				\
	: "=&r" (ret)				\
	: "r" (ptr), "r" (-1)			\
	: "r1", "r2", "r3");			\
	smp_mb();				\
	ret;					\
	})

#define __up_op(ptr,wake)			\
	({					\
	smp_mb();				\
	__asm__ __volatile__(			\
	"/* up_op */\n"				\
"	rcsr r3, IE\n"				\
"	addi    sp, sp, -4\n"			\
"	sw      (sp+0), r1\n"			\
"	rcsr r1, IE\n"				\
"	andi r1, r1, 0xE\n"			\
"	wcsr IE, r1\n"				\
"	lw      r1, (sp+0)\n"			\
"	addi    sp, sp, 4\n"			\
"	mv	r2, %0\n"			\
"	lw	r1, (r2+0)\n"			\
"	add	r1, r1, %1\n"			\
"	sw	(r2+0), r1\n"			\
"	wcsr IE, r3\n"				\
"	bg	r1, r0, 1f\n"	\
"	calli	" #wake	"\n"			\
"	1:"			\
	:					\
	: "r" (ptr), "r" (1)			\
	: "r1", "r2", "r3");			\
	})

/*
 * The value 0x01000000 supports up to 128 processors and
 * lots of processes.  BIAS must be chosen such that sub'ing
 * BIAS once per CPU will result in the long remaining
 * negative.
 */
#define RW_LOCK_BIAS      0x01000000
#define RW_LOCK_BIAS_STR "0x01000000"

#define __down_op_write(ptr,fail)		\
	({					\
	__asm__ __volatile__(			\
	"/* down_op_write */\n"			\
"	rcsr r3, IE\n"				\
"	addi    sp, sp, -4\n"			\
"	sw      (sp+0), r1\n"			\
"	rcsr r1, IE\n"				\
"	andi r1, r1, 0xE\n"			\
"	wcsr IE, r1\n"				\
"	lw      r1, (sp+0)\n"			\
"	addi    sp, sp, 4\n"			\
"	mv	r2, %0\n"			\
"	lw	r1, (r2+0)\n"			\
"	add	r1, r1, %1\n"			\
"	sw	(r2+0), r1\n"			\
"	wcsr IE, r3\n"				\
"	be	r1, r0, 1f\n"	\
"	calli	" #fail "\n"			\
"	1:"			\
	:					\
	: "r" (ptr), "r" -(RW_LOCK_BIAS)	\
	: "r1", "r2", "r3");			\
	smp_mb();				\
	})

#if 0
#define __up_op_write(ptr,wake)			\
	({					\
	__asm__ __volatile__(			\
	"/* up_op_write */\n"			\
"	rcsr r3, IE\n"				\
"	addi    sp, sp, -4\n"			\
"	sw      (sp+0), r1\n"			\
"	rcsr r1, IE\n"				\
"	andi r1, r1, 0xE\n"			\
"	wcsr IE, r1\n"				\
"	lw      r1, (sp+0)\n"			\
"	addi    sp, sp, 4\n"			\
"	mv	r2, %0\n"			\
"	lw	r1, (r2+0)\n"			\
"	add	r1, r1, %1\n"			\
"	sw	(r2+0), r1\n"			\
"	wcsr IE, r3\n"				\
"	if(!cc)->1f (it was movcc)	r1, r0, 1f\n"		\
"	calli	" #wake	"\n"			\
"	1:"			\
	:					\
	: "r" (ptr), "r" (RW_LOCK_BIAS)		\
	: "r1", "r2", "r3");			\
	smp_mb();				\
	})
#endif //0

#define __down_op_read(ptr,fail)		\
	__down_op(ptr, fail)

#define __up_op_read(ptr,wake)			\
	({					\
	smp_mb();				\
	__asm__ __volatile__(			\
	"/* up_op_read */\n"			\
"	rcsr r3, IE\n"				\
"	addi    sp, sp, -4\n"			\
"	sw      (sp+0), r1\n"			\
"	rcsr r1, IE\n"				\
"	andi r1, r1, 0xE\n"			\
"	wcsr IE, r1\n"				\
"	lw      r1, (sp+0)\n"			\
"	addi    sp, sp, 4\n"			\
"	mv	r2, %0\n"			\
"	lw	r1, (r2+0)\n"			\
"	add	r1, r1, %1\n"			\
"	sw	(r2+0), r1\n"			\
"	wcsr IE, r3\n"				\
"	be	r1, r0, 1f\n"	\
"	calli	" #wake "\n"			\
"	1:"			\
	:					\
	: "r" (ptr), "r" (1)			\
	: "r1", "r2", "r3");			\
	})

#endif

