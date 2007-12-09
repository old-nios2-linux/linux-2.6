/*
 *  linux/arch/lm32/kernel/head-common.S
 *
 *  Copyright (C) 1994-2002 Russell King
 *  Copyright (c) 2003 ARM Limited
 *  Copyright (c) 2007 Andrea della Porta
 *  All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */


#define ATAG_CORE 0x54410001
#define ATAG_CORE_SIZE ((2*4 + 3*4) >> 2)

	.type	__switch_data, %object
__switch_data:
	.long	__mmap_switched
	.long	__data_loc			 /* r4 */
	.long	__data_start			 /* r5 */
	.long	__bss_start			 /* r6 */
	.long	_end				 /* r7 */
	.long	processor_id			 /* r4 */
	.long	__machine_arch_type		 /* r5 */
	.long	__atags_pointer		 	 /* r6 */
	.long	cr_alignment			 /* r7 - not used on Mico32 */
	.long	init_thread_union + THREAD_START_SP  /* sp */

/*
 * The following fragment of code is executed with the MMU on in MMU mode,
 * and uses absolute addresses; this is not position independent.
 *
 *  r0  = cp#15 control register	//not used on Mico32
 *  r1  = machine ID
 *  r2  = atags pointer
 *  r9  = processor ID
 */
	.type	__mmap_switched, %function
__mmap_switched:
	mva	r3, __switch_data 
	addi	r3, r3, 4

	lw	r4, (r3+0)			/* __data_loc */
	lw	r5, (r3+4)			/* __data_start */
	lw	r6, (r3+8)			/* __bss_start */
	lw	r7, (r3+12)			/* _end */
	addi	r3, r3, 16			/* r3 -> 'processor_id' in __switch_data structure */
	be	r4, r5, 2f			/* copy data segment if needed */
1:	be	r5, r6, 2f
	lw	fp, (r4+0)
	addi	r4, r4, 4
	sw	(r5+0), fp
	addi	r5, r5, 4
	bi	1b

2:	mv	fp, r0				/* clear BSS (and zero fp) */
1:	be	r6, r7, 2f
	sw	(r6+0), fp
	addi	r6, r6, 4
	bi	1b

2:	lw 	r4, (r3+0)			/* processor_id */
	lw	r5, (r3+4)			/* __machine_arch_type */
	lw	r6, (r3+8)			/* __atags_pointer */
	lw	r7, (r3+12)			/* cr_alignment - not used on Mico32 */
	lw	sp, (r3+16)			/* init_thread_union + THREAD_START_SP */
	sw	(r4+0), r9			/* save processor ID */
	sw	(r5+0), r1			/* save machine type */
	sw	(r6+0), r2			/* save atags pointer */
	/*bic	r4, r0, #CR_A			@ Clear 'A' bit 
	stmia	r6, {r0, r4}			@ Save control register values
	*/

	bi	start_kernel

/*
 * Exception handling.  Something went wrong and we can't proceed.  We
 * ought to tell the user, but since we don't have any guarantee that
 * we're even running on the right architecture, we do virtually nothing.
 *
 * If CONFIG_DEBUG_LL is set we try to print out something about the error
 * and hope for the best (useful if bootloader fails to pass a proper
 * machine ID for example).
 */

	.type	__error_p, %function
__error_p:
#ifdef CONFIG_DEBUG_LL
	adr	r0, str_p1
	bl	printascii
	b	__error
str_p1:	.asciz	"\nError: unrecognized/unsupported processor variant.\n"
	.align
#endif

	.type	__error_a, %function
__error_a:
#ifdef CONFIG_DEBUG_LL
	mov	r4, r1				@ preserve machine ID
	adr	r0, str_a1
	bl	printascii
	mov	r0, r4
	bl	printhex8
	adr	r0, str_a2
	bl	printascii
	adr	r3, 3f
	ldmia	r3, {r4, r5, r6}		@ get machine desc list
	sub	r4, r3, r4			@ get offset between virt&phys
	add	r5, r5, r4			@ convert virt addresses to
	add	r6, r6, r4			@ physical address space
1:	ldr	r0, [r5, #MACHINFO_TYPE]	@ get machine type
	bl	printhex8
	mov	r0, #'\t'
	bl	printch
	ldr     r0, [r5, #MACHINFO_NAME]	@ get machine name
	add	r0, r0, r4
	bl	printascii
	mov	r0, #'\n'
	bl	printch
	add	r5, r5, #SIZEOF_MACHINE_DESC	@ next machine_desc
	cmp	r5, r6
	blo	1b
	adr	r0, str_a3
	bl	printascii
	b	__error
str_a1:	.asciz	"\nError: unrecognized/unsupported machine ID (r1 = 0x"
str_a2:	.asciz	").\n\nAvailable machine support:\n\nID (hex)\tNAME\n"
str_a3:	.asciz	"\nPlease check your kernel config and/or bootloader.\n"
	.align
#endif

	.type	__error, %function
__error:
1:	mv	r1, r1
	bi	1b


/*
 * Take the processor ID register (from CFG CSR), and look up in the linker-built
 * supported processor list.  Note that we can't use the absolute addresses
 * for the __proc_info lists since we aren't running with the MMU on
 * (and therefore, we are not in the correct address space).  We have to
 * calculate the offset.
 *
 *	r9 = cpuid
 * Returns:
 *	r3, r4, r6 corrupted
 *	r5 = proc_info_list pointer in physical address space
 *	r9 = cpuid (preserved)
 */
	.type	__lookup_processor_type, %function
__lookup_processor_type:
	mva	r3, 5f				
	lw	r3, (r3+0)			/* index of '__proc_info_begin' where all the descriptors are stored */
	lw	r6, (r3+4)			/* index of the end (__proc_info_end) of the descriptors list */
	mv	r5, r3
1:	lw	r3, (r5+0)			/* value */
	lw	r4, (r5+4)			/* mask */
	and	r4, r4, r9			/* mask out wanted bits */
	be	r3, r4, 2f			/* test if we've found the corret processor type */
	addi	r5, r5, PROC_INFO_SZ		/* sizeof(proc_info_list) */
	bge	r5, r6, 4f			/* we reach the end of the list */
	bi	1b				/* reiterate to the next check */
4:	mv	r5, r0				/* unknown processor */
2:	ret

/*
 * This provides a C-API version of the above function.
 */
ENTRY(lookup_processor_type)
	/*pushed	r3*/				/* they are caller saved, so you can avoid saving these */
	/*pushed	r4
	pushed	r5
	pushed	r6
	pushed	r9*/

	mv	r9, r1				/* ARM port wants argument in r9 for no (obvious) reason, we should really clean-up this */
	bi	__lookup_processor_type
	mv	r1, r5

	/*poped	r9
	poped	r6
	poped	r5
	poped	r4
	poped	r3*/

	ret

/*
 * Look in include/asm-arm/procinfo.h and arch/arm/kernel/arch.[ch] for
 * more information about the __proc_info and __arch_info structures.
 */
5:
	.long	__proc_info_begin
	.long	__proc_info_end
//3:	.long	.
3:	.long	__arch_info_begin
	.long	__arch_info_end

/*
 * Lookup machine architecture in the linker-build list of architectures.
 * Note that we can't use the absolute addresses for the __arch_info
 * lists since we aren't running with the MMU on (and therefore, we are
 * not in the correct address space).  We have to calculate the offset.
 *
 *  r1 = machine architecture number
 * Returns:
 *  r3, r4, r6 corrupted
 *  r5 = machine_desc pointer in physical address space
 */
	.type	__lookup_machine_type, %function
__lookup_machine_type:
	mva	r3, 3b				
	lw	r3, (r3+0)			/* index of '__arch_info_begin' where all the descriptors are stored */
	lw	r6, (r3+4)			/* index of the end (__arch_info_end) of the descriptors list */
	mv	r5, r3
1:	lw	r3, (r5+MACHINFO_TYPE)		/* machine type */
	be	r3, r1, 2f			/* test if we've found the corret processor type */
	addi	r5, r5, SIZEOF_MACHINE_DESC	/* sizeof(proc_info_list) */
	bge	r5, r6, 4f			/* we reach the end of the list */
	bi	1b				/* reiterate to the next check */
4:	mv	r5, r0				/* unknown processor */
2:	ret


/*
 * This provides a C-API version of the above function.
 */
ENTRY(lookup_machine_type)
	/*pushed	r3*/				/* they are caller saved, so you can avoid saving these */
	/*pushed	r4
	pushed	r5
	pushed	r6
	pushed	r1*/

	mv	r1, r1				/* ARM port wants argument in r1 for no (obvious) reason, we should really clean-up this */
	bi	__lookup_machine_type
	mv	r1, r5

	/*poped	r1
	poped	r6
	poped	r5
	poped	r4
	poped	r3*/

	ret


/* Determine validity of the r2 atags pointer.  The heuristic requires
 * that the pointer be aligned, in the first 16k of physical RAM and
 * that the ATAG_CORE marker is first and present.  Future revisions
 * of this function may be more lenient with the physical address and
 * may also be able to move the ATAGS block if necessary.
 *
 * r8  = machinfo
 *
 * Returns:
 *  r2 either valid atags pointer, or zero
 *  r5, r6 corrupted
 */

       .type   __vet_atags, "function"
__vet_atags:
       andi    r5, r2, 0x3                     /* aligned? */
       be      r5, r0, 1f		       /* not aligned, so exit setting r2 to null */

       lw      r5, (r2+0)                      /* is first tag ATAG_CORE? */
       addi    r5, r5, -ATAG_CORE_SIZE	       /* subtract ATAG_CORE_SIZE */	
       bne     r5, r0, 1f
       lw      r5, (r2+4)
       mva     r6, ATAG_CORE
       lw      r6, (r6+0)
       bne     r5, r6, 1f

       ret		                       /* atag pointer is ok */

1:     mv      r2, r0
       ret

