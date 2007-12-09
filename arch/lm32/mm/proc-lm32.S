/*
 *  linux/arch/lm32/mm/proc-lm32.S
 *  
 *  brought to you by Andrea della Porta <sfaragnaus@gmail.com>
 *  based on Philips LPC22xx ARM version
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#include <linux/linkage.h>
#include <linux/init.h>
#include <asm/assembler.h>
#include <asm/pgtable.h>
//#include <asm/procinfo.h>
#include <asm/ptrace.h>
#include <asm/errno.h>
#include <asm/hardware.h>

/* //already defined in cache-lm32.S
ENTRY(lm32_flush_user_cache_all)
ENTRY(lm32_flush_kern_cache_all)
ENTRY(lm32_flush_user_cache_range)
ENTRY(lm32_coherent_kern_range)
ENTRY(lm32_coherent_user_range)
ENTRY(lm32_flush_kern_dcache_page)
ENTRY(lm32_dma_inv_range)
ENTRY(lm32_dma_flush_range)
ENTRY(lm32_dma_clean_range)
	ret
*/
ENTRY(cpu_lm32_dcache_clean_area)
	ret  


/*
 * Function: lm32_data_abort ()
 *
 * Params  : r2 = address of aborted instruction
 *	   : sp = pointer to registers
 *
 * Purpose : obtain information about current aborted instruction
 *
 * Returns : r0 = address of abort
 *	   : r1 = FSR
 */

ENTRY(cpu_lm32_data_abort)
/*@FIXME
	ldr     r8, [r2]                        @ read arm instruction
	tst     r8, #1 << 20                    @ L = 1 -> write?
	and	r7, r8, #15 << 24
	orr	r1, r7, #1
	orreq   r1, r1, #1 << 8                 @ yes.
	add	pc, pc, r7, lsr #22		@ Now branch to the relevant processing routine
	nop
*/
	ret		//we really should do better abort handling here

/* 0 */	bi	.data_unknown
/* 1 */	ret				/* swp */
/* 2 */	bi	.data_unknown
/* 3 */	bi	.data_unknown
/* 4 */	bi	.data_arm_lateldrpostconst	/* ldr	rd, [rn], #m */
/* 5 */	bi	.data_arm_lateldrpreconst	/* ldr	rd, [rn, #m] */
/* 6 */	bi	.data_arm_lateldrpostreg	/* ldr	rd, [rn], rm */
/* 7 */	bi	.data_arm_lateldrprereg		/* ldr	rd, [rn, rm] */
/* 8 */	bi	.data_arm_ldmstm		/* ldm*a	rn, <rlist> */
/* 9 */	bi	.data_arm_ldmstm		/* ldm*b	rn, <rlist> */
/* a */	bi	.data_unknown
/* b */	bi	.data_unknown
/* c */	ret				/* ldc	rd, [rn], #m	@ Same as ldr	rd, [rn], #m */
/* d */	ret				/* ldc	rd, [rn, #m] */
/* e */	bi	.data_unknown
/* f */
.data_unknown:	/* Part of jumptable */
	/*mov	r0, r2
	mov	r1, r8
	mov	r2, sp
	bl	baddataabort*/
	bi	ret_from_exception

ENTRY(cpu_arm6_data_abort)
/*FIXME
	ldr	r8, [r2]			@ read arm instruction
	tst	r8, #1 << 20			@ L = 1 -> write?
	orreq	r1, r1, #1 << 8			@ yes.
	and	r7, r8, #14 << 24
	teq	r7, #8 << 24			@ was it ldm/stm
	movne	pc, lr*/
	ret

.data_arm_ldmstm:
/*	tst	r8, #1 << 21			@ check writeback bit
	moveq	pc, lr				@ no writeback -> no fixup
	mov	r7, #0x11
	orr	r7, r7, #0x1100
	and	r6, r8, r7
	and	r2, r8, r7, lsl #1
	add	r6, r6, r2, lsr #1
	and	r2, r8, r7, lsl #2
	add	r6, r6, r2, lsr #2
	and	r2, r8, r7, lsl #3
	add	r6, r6, r2, lsr #3
	add	r6, r6, r6, lsr #8
	add	r6, r6, r6, lsr #4
	and	r6, r6, #15			@ r6 = no. of registers to transfer.
	and	r5, r8, #15 << 16		@ Extract 'n' from instruction
	ldr	r7, [sp, r5, lsr #14]		@ Get register 'Rn'
	tst	r8, #1 << 23			@ Check U bit
	subne	r7, r7, r6, lsl #2		@ Undo increment
	addeq	r7, r7, r6, lsl #2		@ Undo decrement
	str	r7, [sp, r5, lsr #14]		@ Put register 'Rn'
	mov	pc, lr*/
	ret

.data_arm_apply_r6_and_rn:
/*	and	r5, r8, #15 << 16		@ Extract 'n' from instruction
	ldr	r7, [sp, r5, lsr #14]		@ Get register 'Rn'
	tst	r8, #1 << 23			@ Check U bit
	subne	r7, r7, r6			@ Undo incrmenet
	addeq	r7, r7, r6			@ Undo decrement
	str	r7, [sp, r5, lsr #14]		@ Put register 'Rn'
	mov	pc, lr*/
	ret

.data_arm_lateldrpreconst:
/*	tst	r8, #1 << 21			@ check writeback bit
	moveq	pc, lr				@ no writeback -> no fixup
	*/
.data_arm_lateldrpostconst:
/*	movs	r2, r8, lsl #20			@ Get offset
	moveq	pc, lr				@ zero -> no fixup
	and	r5, r8, #15 << 16		@ Extract 'n' from instruction
	ldr	r7, [sp, r5, lsr #14]		@ Get register 'Rn'
	tst	r8, #1 << 23			@ Check U bit
	subne	r7, r7, r2, lsr #20		@ Undo increment
	addeq	r7, r7, r2, lsr #20		@ Undo decrement
	str	r7, [sp, r5, lsr #14]		@ Put register 'Rn'
	mov	pc, lr*/
	ret

.data_arm_lateldrprereg:
/*	tst	r8, #1 << 21			@ check writeback bit
	moveq	pc, lr				@ no writeback -> no fixup
	*/
.data_arm_lateldrpostreg:
/*	and	r7, r8, #15			@ Extract 'm' from instruction
	ldr	r6, [sp, r7, lsl #2]		@ Get register 'Rm'
	mov	r5, r8, lsr #7			@ get shift count
	ands	r5, r5, #31
	and	r7, r8, #0x70			@ get shift type
	orreq	r7, r7, #8			@ shift count = 0
	add	pc, pc, r7
	nop

	mov	r6, r6, lsl r5			@ 0: LSL #!0
	b	.data_arm_apply_r6_and_rn
	b	.data_arm_apply_r6_and_rn	@ 1: LSL #0
	nop
	b	.data_unknown			@ 2: MUL?
	nop
	b	.data_unknown			@ 3: MUL?
	nop
	mov	r6, r6, lsr r5			@ 4: LSR #!0
	b	.data_arm_apply_r6_and_rn
	mov	r6, r6, lsr #32			@ 5: LSR #32
	b	.data_arm_apply_r6_and_rn
	b	.data_unknown			@ 6: MUL?
	nop
	b	.data_unknown			@ 7: MUL?
	nop
	mov	r6, r6, asr r5			@ 8: ASR #!0
	b	.data_arm_apply_r6_and_rn
	mov	r6, r6, asr #32			@ 9: ASR #32
	b	.data_arm_apply_r6_and_rn
	b	.data_unknown			@ A: MUL?
	nop
	b	.data_unknown			@ B: MUL?
	nop
	mov	r6, r6, ror r5			@ C: ROR #!0
	b	.data_arm_apply_r6_and_rn
	mov	r6, r6, rrx			@ D: RRX
	b	.data_arm_apply_r6_and_rn
	b	.data_unknown			@ E: MUL?
	nop
	b	.data_unknown			@ F: MUL?
*/

/*
 * Notes   : This processor does not require these
 */
ENTRY(cpu_lm32_proc_init)
		ret

ENTRY(cpu_lm32_proc_fin)
		/*mov	r0, #PSR_F_BIT | PSR_I_BIT | SVC_MODE
		msr	cpsr_c, r0
		mov	pc, lr
		*/
		ret

ENTRY(cpu_lm32_do_idle)
		xor	r1, r1, r1
		addi    r1, r1, -EINVAL
		ret

/*
 * Params  : pgd_phys	Physical address of page table
 * Purpose : Perform a task switch, saving the old processes state, and restoring
 *	     the new.
 */
ENTRY(cpu_lm32_switch_mm)
		ret	

/*
 * Params  : r1 = address to jump to
 * Notes   : This sets up everything for a reset
 */
ENTRY(cpu_lm32_reset)
		b 	r1


		__INIT

		.type	__lm32_setup, %function
__lm32_setup:
		ret
		.size	__lm32_setup, . - __lm32_setup
	
		__INITDATA

		.type	lm32_cache_fns, %object
ENTRY(lm32_cache_fns)
		.long	lm32_flush_kern_cache_all
		.long	lm32_flush_user_cache_all
		.long	lm32_flush_user_cache_range
		.long	lm32_coherent_kern_range
		.long	lm32_coherent_user_range
		.long	lm32_flush_kern_dcache_page
		.long	lm32_dma_inv_range
		.long	lm32_dma_clean_range
		.long	lm32_dma_flush_range
		.size	lm32_cache_fns, . - lm32_cache_fns
	
/*
 * Purpose : Function pointers used to access above functions - all calls
 *	     come through these
 */
		.type	lm32_processor_functions, %object
ENTRY(lm32_processor_functions)
		.word	cpu_lm32_data_abort
		.word	cpu_lm32_proc_init
		.word	cpu_lm32_proc_fin
		.word	cpu_lm32_reset
		.word	cpu_lm32_do_idle
		.word	cpu_lm32_dcache_clean_area
		.word	cpu_lm32_switch_mm
		.size	lm32_processor_functions, . - lm32_processor_functions

		.section ".rodata"

		.type	cpu_arch_name, %object
cpu_arch_name:	.asciz	"lm32"
		.size	cpu_arch_name, . - cpu_arch_name

		.type	cpu_elf_name, %object
cpu_elf_name:	.asciz	"mico32"
		.size	cpu_elf_name, . - cpu_elf_name

		.type	cpu_lm32, %object
cpu_lm32_name:
		.asciz	"Lattice Mico32"
		.size	cpu_lm32_name, . - cpu_lm32_name

		.align

		.section ".proc.info.init",  "ax" /* "alloc", "execinstr" */

		.type	__lm32_proc_info, %object
__lm32_proc_info:
		.long	0xe0000000	//take this from real cpu
		.long	0xfc000000	//mask
		.long	0
		bi	__lm32_setup
		.long	cpu_arch_name
		.long	cpu_elf_name
		.long	0
		.long	cpu_lm32_name
		.long	lm32_processor_functions
		.long	0
		.long	0
		.long	lm32_cache_fns
		.size	__lm32_proc_info, . - __lm32_proc_info

