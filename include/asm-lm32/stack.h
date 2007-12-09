/*
 *  linux/include/asm-lm32/stack.h
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

        .macro  pushed, rd         /*assuming stack is empty descending*/
            sw      (sp+0), \rd
            addi    sp, sp, -4
        .endm

        .macro  poped, rd          /*assuming stack is empty descending*/
            addi    sp, sp, 4
            lw      \rd, (sp+0)
        .endm

        .macro  pushfd, rd         /*assuming stack is full descending*/
            addi    sp, sp, -4
            sw      (sp+0), \rd
        .endm

        .macro  popfd, rd          /*assuming stack is full descending*/
            lw      \rd, (sp+0)
            addi    sp, sp, 4
        .endm

        .macro  enter
	    pushfd	r1
	    pushfd	r2
	    pushfd	r3
	    pushfd	r4
	    pushfd	r5
	    pushfd	r6
	    pushfd	r7
	    pushfd	r8
	    pushfd	r9
	    pushfd	r10
	    pushfd	ra
	.endm

	.macro  leave
	    popfd	ra
	    popfd	r10
	    popfd	r9
	    popfd	r8
	    popfd	r7
	    popfd	r6
	    popfd	r5
	    popfd	r4
	    popfd	r3
	    popfd	r2
	    popfd	r1
	.endm

