/*
 *  linux/arch/lm32/kernel/entry-common.S
 *
 *  Lattice's Mico32 support from Andrea della Porta (sfaragnaus@gmail.com)
 *  based upon the ARM's port from  Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <asm/unistd.h>
#include <asm/arch/entry-macro.S>

#include "entry-header.S"

	.align	16	//TODO: should be replaced by BYTES_PER_LINE
/*
 * This is the fast syscall return path.  We do as little as
 * possible here, and this includes saving r0 back into the SVC
 * stack.
 */
ret_fast_syscall:
	disable_irq				//disable interrupts

	get_thread_info r3
	lw	r10, (r3 + TI_FLAGS)		// check for syscall tracing
	mva	r11, _TIF_WORK_MASK		// any work pending?
	and	r10, r10, r11
	bne	r10, r0, fast_work_pending

        /* perform architecture specific actions before user return */
        arch_ret_to_user r1, ra    		//TODO: implement it for Mico32

	bi no_work_pending

/*
 * Ok, we need to do extra processing, enter the slow path.
 */
fast_work_pending:
	//str	r0, [sp, #S_R0+S_OFF]!		// returned r0
work_pending:
        lw      r10, (r3 + TI_FLAGS)           		// check for syscall tracing
        mva     r11, _TIF_SIGPENDING      // any work pending?
        and     r10, r10, r11
        be      r10, r0, no_work_pending

	mv	r1, sp				// 'pt_regs'
	lw      r2, (r3 + TI_FLAGS)
	mv	r3, r4	//why			// 'syscall'    <--- this is suspicious: is it why=tbl or the syscall number?
	calli	do_notify_resume

	bi	ret_slow_syscall		// Check work again

work_resched:
	calli	schedule

/*
 * "slow" syscall return path.  "why" tells us if this was a real syscall.
 */
ENTRY(ret_to_user)
ret_slow_syscall:
	disable_irq				// disable interrupts

	get_thread_info r3
        lw      r10, (r3 + TI_FLAGS)           // check for syscall tracing
        mva     r11, _TIF_WORK_MASK            // any work pending?
        and     r10, r10, r11
        bne     r10, r0, work_pending

no_work_pending:
        /* perform architecture specific actions before user return */
        arch_ret_to_user r1, ra    		//TODO: implement it for Mico32

	RESTORE_ALL
	eret

/*
 * This is how we return from a fork.
 */
ENTRY(ret_from_fork)
	calli	schedule_tail

	get_thread_info r3
        lw      r10, (r3 + TI_FLAGS)           // check for syscall tracing
        mva     r11, _TIF_SYSCALL_TRACE        // are we tracing syscalls?
        and     r10, r10, r11
	mvi	r4, 1				//why=tbl=1 ??????
        be      r10, r0, ret_slow_syscall

	
	mvi r1, 1	//why
	mv  r3, r2	//scno
	mv  r2, sp				//pt_regs
	calli syscall_trace

	bi	ret_slow_syscall


	.equ NR_syscalls,0
#define CALL(x) .equ NR_syscalls,NR_syscalls+1
#include "calls.S"
#undef CALL
#define CALL(x) .long x

/*=============================================================================
 * Syscall handler	
 *-----------------------------------------------------------------------------
 */

	.align	16	//TODO: should be replaced by BYTES_PER_LINE
ENTRY(vector_swi)	//scno -> r2,  tbl -> r4,  tsk -> r3     -    syscall parameters begin from r5 onwards 
	//registers already saved by _system_call_handler2 
	
	//zero_fp

	enable_irq

	get_thread_info r3
	mva	r4, sys_call_table		// load syscall table pointer
	lw	r10, (r3 + TI_FLAGS)		// check for syscall tracing

	//stmdb	sp!, {r4, r5}			// push fifth and sixth args
	mva	r11, _TIF_SYSCALL_TRACE	// are we tracing syscalls?
	and	r10, r10, r11
	be	r10, r11, __sys_trace

	mva 	r10, NR_syscalls
	bgeu	r2, r10, 3f	// check upper syscall limit
	mva	ra, ret_fast_syscall		// return address
	sli 	r12, r2, 2			// calculating the addres of the syscall entry to be called
	add	r12, r12, r4
	b	r12				// call sys_* routine

3:
	/*add	r1, sp, #S_OFF
2:	mov	why, #0				// no longer a real syscall
	cmp	scno, #(__LM32_NR_BASE - __NR_SYSCALL_BASE)
	eor	r0, scno, #__NR_SYSCALL_BASE	// put OS number back
	bcs	arm_syscall   */	
	bi	sys_ni_syscall			// not private func

	/*
	 * This is the really slow path.  We're going to be doing
	 * context switches, and waiting for our parent to respond.
	 */
__sys_trace:
	break
/*
	mov	r2, scno
	add	r1, sp, #S_OFF
	mov	r0, #0				// trace entry [IP = 0]
	bl	syscall_trace

	adr	lr, __sys_trace_return		// return address
	mov	scno, r0			// syscall number (possibly new)
	add	r1, sp, #S_R0 + S_OFF		// pointer to regs
	cmp	scno, #NR_syscalls		// check upper syscall limit
	ldmccia	r1, {r0 - r3}			// have to reload r0 - r3
	ldrcc	pc, [tbl, scno, lsl #2]		// call sys_* routine
	b	2b

__sys_trace_return:
	str	r0, [sp, #S_R0 + S_OFF]!	// save returned r0
	mov	r2, scno
	mov	r1, sp
	mov	r0, #1				// trace exit [IP = 1]
	bl	syscall_trace
	b	ret_slow_syscall
*/
	.align	16	//TODO: should be replaced by BYTES_PER_LINE
#ifdef CONFIG_ALIGNMENT_TRAP
	.type	__cr_alignment, "object"
__cr_alignment:
	.word	cr_alignment
#endif
	//.ltorg


/*
 * This is the syscall table declaration for native ABI syscalls.
 * With EABI a couple syscalls are obsolete and defined as sys_ni_syscall.
 */
#define ABI(native, compat)	  native
#define OBSOLETE(syscall) 	  sys_ni_syscall

	.type	sys_call_table, "object"
ENTRY(sys_call_table)
#include "calls.S"
#undef ABI
#undef OBSOLETE

/*============================================================================
 * Special system call wrappers
 */
// r2 = syscall number
// r8 = syscall table
		.type	sys_syscall, "function"
sys_syscall:
	        mva     r10, NR_syscalls
	        bgeu    r2, r10, 4f        //check upper syscall limit
		mv	r2, r5		   //shift registers	
		mv	r5, r6
		mv	r6, r7
		mv	r8, r9
		//add any other register here - ARM version pass r5-r6 on the stack
	        sli     r13, r2, 2                     // calculating the addres of the syscall entry to be called
	        add     r13, r13, r4
	        b       r13                             // call sys_* routine
	4:
		mva 	r13, sys_ni_syscall
		b	r13

sys_fork_wrapper:
		//break
		//add	r0, sp, #S_OFF
		mva r13, sys_fork
		b   r13

sys_vfork_wrapper:
		//break
		//add	r0, sp, #S_OFF
		mva r13, sys_vfork
		b   r13

sys_execve_wrapper:
		//break
		//add	r3, sp, #S_OFF
		mva r13, sys_execve
		b   r13

sys_clone_wrapper:
		//break
		//add	ip, sp, #S_OFF
		//str	ip, [sp, #4]
		mva r13, sys_clone
		b   r13

sys_sigsuspend_wrapper:
		//break
		//add	r3, sp, #S_OFF
		mva r13, sys_sigsuspend
		b   r13

sys_rt_sigsuspend_wrapper:
		//break
		//add	r2, sp, #S_OFF
		mva r13, sys_rt_sigsuspend
		b   r13

sys_sigreturn_wrapper:
		//break
		//add	r0, sp, #S_OFF
		mva r13, sys_sigreturn
		b   r13

sys_rt_sigreturn_wrapper:
		//break
		//add	r0, sp, #S_OFF
		mva r13, sys_rt_sigreturn
		b   r13

sys_sigaltstack_wrapper:
		//break
		//ldr	r2, [sp, #S_OFF + S_SP]
		mva r13, do_sigaltstack
		b   r13

sys_statfs64_wrapper:
		//break
		//teq	r1, #88
		//moveq	r1, #84
		mva r13, sys_statfs64
		b   r13

sys_fstatfs64_wrapper:
		//break;
		//teq	r1, #88
		//moveq	r1, #84
		mva r13, sys_fstatfs64
		b   r13

/*
 * Note: off_4k (r5) is always units of 4K.  If we can't do the requested
 * offset, we return EINVAL.
 */
sys_mmap2:
		//break
/*
#if PAGE_SHIFT > 12
		tst	r5, #PGOFF_MASK
		moveq	r5, r5, lsr #PAGE_SHIFT - 12
		streq	r5, [sp, #4]
		beq	do_mmap2
		mov	r0, #-EINVAL
		mov	pc, lr
#else
		//str	r5, [sp, #4]
		mva r13, do_mmap2
		b   r13
#endif
*/

#ifdef CONFIG_OABI_COMPAT

/*
 * These are syscalls with argument register differences
 */

sys_oabi_pread64:
		stmia	sp, {r3, r4}
		b	sys_pread64

sys_oabi_pwrite64:
		stmia	sp, {r3, r4}
		b	sys_pwrite64

sys_oabi_truncate64:
		mov	r3, r2
		mov	r2, r1
		b	sys_truncate64

sys_oabi_ftruncate64:
		mov	r3, r2
		mov	r2, r1
		b	sys_ftruncate64

sys_oabi_readahead:
		str	r3, [sp]
		mov	r3, r2
		mov	r2, r1
		b	sys_readahead

/*
 * Let's declare a second syscall table for old ABI binaries
 * using the compatibility syscall entries.
 */
#define ABI(native, compat) compat
#define OBSOLETE(syscall) syscall

	.type	sys_oabi_call_table, #object
ENTRY(sys_oabi_call_table)
#include "calls.S"
#undef ABI
#undef OBSOLETE

#endif

