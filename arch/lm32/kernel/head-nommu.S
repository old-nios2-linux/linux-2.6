/*
 *  linux/arch/lm32/kernel/head-nommu.S
 *
 *  Copyright (C) 1994-2002 Russell King
 *  Copyright (C) 2003-2006 Hyok S. Choi
 *  Copyright (C) 2007 	    Andrea della Porta
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  Common kernel startup code (non-paged MM)
 *
 */
#include <linux/linkage.h>
#include <linux/init.h>

#include <asm/assembler.h>
#include <asm/mach-types.h>
#include <asm/ptrace.h>
#include <asm/asm-offsets.h>
#include <asm/thread_info.h>
#include <asm/system.h>
#include <asm/arch/hardware.h>

/*
 * Kernel startup entry point.
 * ---------------------------
 *
 * This is normally called from the decompressor code.  The requirements
 * are: MMU = off, D-cache = off, I-cache = dont care, r0 = 0,
 * r1 = machine nr.
 *
 * See linux/arch/lm32/tools/mach-types for the complete list of machine
 * numbers for r1.
 *
 */
	.section ".text.head", "ax"
	.type	stext, %function
ENTRY(stext)

        xor r0, r0, r0                  /* lm32 compiler expect r0 to be zero */
                                        /* at startup, interrupt are disabled and cache, if present, are invalidated. So we do not have too much to do here...... */
        /*wcsr IE, r0*/                 /* make sure interrupt are disabled */

        rcsr    r9, CFG                 /* take the configuration register */
	mvi	r1, MACH_TYPE_LM32 & 0xFFFF
	mvhi	r1, MACH_TYPE_LM32 >> 16	/* add LM32 to 'arch/lm32/tools/mach-types' */

	calli	__lookup_processor_type		/* r9=cpuid , returns: r5=procinfo */
	mv	r10, r5				/* does ARM port do anything with this register later on? */
	bne	r5, r0, 1f			/* invalid processor (r5=0)? */
	bi	__error_p			/* yes, error 'p' */
1:	calli	__lookup_machine_type		/* r1=machtype , returns: r5=machinfo */
	mv	r8, r5				/* does ARM port do anything with this register later on? */
	bne	r5, r0, 2f			/* invalid machine (r5=0)? */
	bi	__error_a			/* yes, error 'a' */
2:
	/* by now, on Mico32, we do not take care about MMU initialization */
	/*ldr	r13, __switch_data		@ address to jump to after
						@ the initialization is done
	adr	lr, __after_proc_init		@ return (PIC) address
	add	pc, r10, #PROCINFO_INITFUNC*/
	
	mva	r13, __switch_data
	lw	r13, (r13+0)			/* __mmap_switched */
	b	r13


//  Set the Control Register and Read the process ID.
/* 
	.type	__after_proc_init, %function
__after_proc_init:
#ifdef CONFIG_CPU_CP15
	mrc	p15, 0, r0, c1, c0, 0		@ read control reg
#ifdef CONFIG_ALIGNMENT_TRAP
	orr	r0, r0, #CR_A
#else
	bic	r0, r0, #CR_A
#endif
#ifdef CONFIG_CPU_DCACHE_DISABLE
	bic	r0, r0, #CR_C
#endif
#ifdef CONFIG_CPU_BPREDICT_DISABLE
	bic	r0, r0, #CR_Z
#endif
#ifdef CONFIG_CPU_ICACHE_DISABLE
	bic	r0, r0, #CR_I
#endif
	mcr	p15, 0, r0, c1, c0, 0		@ write control reg
#endif // CONFIG_CPU_HAS_NO_CP15 

	mov	pc, r13				@ clear the BSS and jump
						@ to start_kernel
*/
/*	.ltorg */  /* lm32-as does not support this?????? */

#include "head-common.S"
