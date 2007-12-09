/*
 *  linux/arch/lm32/kernel/entry-lm32.S
 *
 *  Lattice's Mico32 support by Andrea della Porta (sfaragnaus//gmail.com)
 *  based upon Russel King's arm port and others (see arm directory)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  Low-level vector interface routines
 *
 */

#include <asm/memory.h>
#include <asm/glue.h>
//#include <asm/vfpmacros.h>
#include <asm/arch/entry-macro.S>
#include <asm/thread_notify.h>

#include "entry-header.S"

.data

	.global  _psr
_psr:
	.long	0

	.global  _current_thread
_current_thread:
	.long	0
		
.text

/*
 * Interrupt handling. 
 */
	.macro	irq_handler
		rcsr	r11, IP		//get interrupt pending register relying on the calling convention, that is: r11, r12, r13 should be automatically preserved by the calle function
	      	mv	r13, r0	 	//zero-out r13
	      1:
		andi	r12, r11, 1	//test if the current irq is pending (irq number stored in r13)
		bgu	r12, r0, 2f	//if pending then we must process it
		srui	r11, r11, 1	//shift to test the next irq
		addi	r13, r13, 1	//increment the irq number
		mvi	r12, 32
		bgeu	r13, r12, 3f	//if we run out of interrupt (on Mico there is 32 interrupts), we're done
		bi	1b		//retry for any new pending interrupt. Can we starve the processor if multiple interrupts arrive?
	      2:
		mv	r1, r13		//r1 = irq (number)
		mv	r2, sp		//r2 = pt_regs
		mva	ra, 1b		//all interrupt will be checked for pending state, so get back to the check
		bi	asm_do_IRQ	//do the hard work
	      3:
	.endm

	.align	16  //TODO: should be replaced by BYTES_PER_LINE from config
__dabt_handler:
	//usr_entry

	//
	// Call the processor-specific abort handler:
	//
	//  r2 - aborted context pc
	//  r3 - aborted context cpsr
	//
	// The abort handler must return the aborted address in r0, and
	// the fault status register in r1.
	//

	//
	// IRQs on, then call the main handler
	//

	//we should check if aborting instruction can be retriggered and notify_die the task instead of entering debug mode here below
	break			
	enable_irq
	
	//b	do_DataAbort

	.align	16  //TODO: should be replaced by BYTES_PER_LINE from config
__irq_usr:
	//usr_entry

/*#ifdef CONFIG_TRACE_IRQFLAGS
       calli    trace_hardirqs_off
#endif*/
	//get_thread_info tsk	//SEE entry-header.S
#ifdef CONFIG_PREEMPT
	ldr	r8, [tsk, #TI_PREEMPT]		// get preempt count
	add	r7, r8, #1			// increment it
	str	r7, [tsk, #TI_PREEMPT]
#endif

	irq_handler
#ifdef CONFIG_PREEMPT
	ldr	r0, [tsk, #TI_PREEMPT]
	str	r8, [tsk, #TI_PREEMPT]
	teq	r0, r7
	strne	r0, [r0, -r0]
#endif
#ifdef CONFIG_TRACE_IRQFLAGS
	//tst     r0, #PSR_I_BIT	//if we came from supervisor mode
        calli   trace_hardirqs_on
#endif

	//mov	why, #0
	//b	ret_to_user

	.align	16  //TODO: should be replaced by BYTES_PER_LINE from config
__pabt_usr:
	//usr_entry

	break
	enable_irq				// Enable interrupts
	//mov	r0, r2				// address (pc)
	//mov	r1, sp				// regs
	//call	do_PrefetchAbort		// call abort handler
	/* fall through */
/*
 * This is the return code to user mode for abort handlers
 */
ENTRY(ret_from_exception)
	//get_thread_info tsk
	//mov	why, #0
	//b	ret_to_user

/*
 * Register switch for Mico32 soft-core
 * r1 = previous task_struct, r2 = previous thread_info, r3 = next thread_info
 * previous and next are guaranteed not to be the same.
 */
ENTRY(__switch_to)
	addi	r2, r2, TI_CPU_SAVE	//offset of 'cpu_context_save' struct inside thread_info	
	sw      (r2+0), r10		//this numbering scheme differs from the one used on pt_regs (see S_*), so we won't use it here. TODO: replace with defines
	sw      (r2+4), r11
	sw      (r2+8), r12
	sw      (r2+12), r13
	sw      (r2+16), r14
	sw      (r2+20), r15
	sw      (r2+24), r16
	sw      (r2+28), r17
	sw      (r2+32), r18
	sw      (r2+36), r19
	sw      (r2+40), r20
	sw      (r2+44), r21
	sw      (r2+48), r22
	sw      (r2+52), r23
	sw      (r2+56), r24
	sw      (r2+60), r25
	sw      (r2+64), gp
	sw      (r2+68), fp
	sw      (r2+72), sp		//this is kernel stack
	sw      (r2+76), ra
	sw      (r2+80), ea
	sw      (r2+84), ba
	rcsr    r1, IE
	sw      (r2+88), r1
        mva     r1, _psr		//is it really necessary to store also the status register? It's saved by SAVE_ALL macro on entering kernel mode... 
        lw      r1, (r1+0)
	sw      (r2+92), r1
	//what about interrupt registers?

#ifdef CONFIG_VFP
	// Always disable VFP so we can lazily save/restore the old
	// state. This occurs in the context of the previous thread.
	VFPFMRX	r4, FPEXC
	bic	r4, r4, FPEXC_ENABLE
	VFPFMXR	FPEXC, r4
#endif

	//We need to switch between kernel stack here - this is not longer needed since we've saved the stack pointer (kernel) into the cpu context structure 
	// ------- CUT OFF THESE INSTRUCTION ONCE VERIFIED ----------------
	sw	(r2+TI_KSP-TI_CPU_SAVE), sp		//save the old kernel stack
	lw	sp, (r3+TI_KSP)				//and restore the kernel stack from the process that is going to be executed
	//what about psr?				//Nios save also psr, we store it in pt_regs out of here. Is it ok?
	mva	r4, _current_thread			
	get_thread_info	r1
	sw	(r4+0), r1				 //_current_thread now holds the thread_info address, so it's at the very bottom of the kernel stack
	// ------- CUT OFF END --------------------------------------------
	
	addi	r3, r3, TI_CPU_SAVE		
	lw      r10, (r3+0)
	lw      r11, (r3+4)
	lw      r12, (r3+8)
	lw      r13,(r3+12)
	lw      r14, (r3+16)
	lw      r15, (r3+20)
	lw      r16, (r3+24)
	lw      r17, (r3+28)
	lw      r18, (r3+32)
	lw      r19,(r3+36)
	lw      r20,(r3+40)
	lw      r21, (r3+44)
	lw      r22, (r3+48)
	lw      r23, (r3+52)
	lw      r24, (r3+56)
	lw      r25, (r3+60)
	lw      gp, (r3+64)
	lw      fp, (r3+68)
	lw      sp, (r3+72)	//the kernel stack, not the userspace one. Userspce will resume later.
	lw      ra, (r3+76)
	lw      ea, (r3+80)
	lw      ba, (r3+84)
        lw      r1, (r3+88)
	wcsr    IE, r1
	mva     r1, _psr
	lw      r1, (r3+92)
	sw      (r1+0), r1
	
	ret

	__INIT


/*=============================================================================
 * Address exception handler
 *-----------------------------------------------------------------------------
 * These aren't too critical.
 * (they're not supposed to happen, and won't happen in 32-bit data mode).
 */

vector_addrexcptn:
	bi	vector_addrexcptn

/*
 * We group all the following data together to optimise
 * for CPUs with separate I & D caches.
 */
	.align	16  //TODO: should be replaced by BYTES_PER_LINE from config

.LCvswi:
	.word	vector_swi

	.globl	__stubs_end
__stubs_end:

	//.equ	stubs_offset, __vectors_start + 0x200 - __stubs_start


	.globl	__vectors_start
__vectors_start:
	_reset_handler:
		mva r1, SYS_ERROR0
		scall	
		nop
		nop
		nop
		nop
		nop
		nop
	_breakpoint_handler:
		bi	_breakpoint_handler
		nop
		nop
		nop
		nop
		nop
		nop
		nop
	_instruction_bus_error_handler:
		bi	_instruction_bus_error_handler
		nop
		nop
		nop
		nop
		nop
		nop
		nop
	_watchpoint_handler:
		bi	_watchpoint_handler
		nop
		nop
		nop
		nop
		nop
		nop
		nop
	_data_bus_error_handler:
		bi      _data_bus_error_handler
		nop
		nop
		nop
		nop
		nop
		nop
		nop
	_divide_by_zero_handler:
		bi      _divide_by_zero_handler
		nop
		nop
		nop
		nop
		nop
		nop
		nop
	_interrupt_handler:
		bi _interrupt_handler2
		nop
		nop
		nop
		nop
		nop
		nop
		nop
	_system_call_handler:		//scno -> r2
		bi _system_call_handler2
		nop
		nop
		nop
		nop
		nop
		nop
		nop


	.globl	__vectors_end
__vectors_end:


_system_call_handler2:
	mva 	r0, _psr			//non conventional use of r0 register. It should be zero.
	lw      r0, (r0+0)			//get the program status register
	andi	r0, r0, PSR_KERNEL_MODE_ASM 	//are we in kernel mode already?
	sw	(sp+0xfffc), r1			//store r1 temporarily on the caller's stack (this should be avoided, but int are disabled and is just for a little time). 0xfffc=-4
	xor	r1, r1, r1
	cmpei	r0, r0, PSR_KERNEL_MODE_ASM 
	bgu	r0, r1, 1f			//jump to kernel mode 
	lw	r1, (sp+0xfffc)			//restore r1 from the stack, see also the label '1' below
	mva	r0, _current_thread		//take the _current_thread
	addi	r0, r0, PAGE_SZ
	addi	r0, r0, -S_FRAME_SIZE		//point to the top of the stack minus the space to contain the hardware context of the process (pt_regs)
	sw	(r0+S_SP), sp			//save the stack pointer in pt_regs
	mv	sp, r0
	bi	2f
      1:	
	lw	r1, (sp+0xfffc)			//restore r1 from the stack, in case the jump is taken
	mv	r0, sp
      	addi	sp, sp, -S_FRAME_SIZE		//we are already on the kernel stack
	sw	(sp+S_SP), r0			//save the stack pointer in pt_regs
      2:
      	xor 	r0, r0, r0			//r0 finally in place
	SAVE_ALL
	add	r1, r0, r2			
	sw      (sp+S_ORIG_R1), r2		//store scno into ORIG_r1 to denote we're from interrupt or exception, note that 'r2' should be preserved throughout all these path
	mva 	r1, _psr			//update the program status register
	lw      r2, (r1+0)			//to be in kernel mode
	ori	r2, r2, PSR_KERNEL_MODE_ASM
	sw	(r1+0), r2

	bi	vector_swi 

	lw	r2, (sp+S_PSR)			//take the program status register of the process just being saved from pt_regs, since by now we are in kernel mode
	andi	r2, r2, PSR_KERNEL_MODE_ASM 	//are we in kernel mode?
	cmpei	r2, r0, PSR_KERNEL_MODE_ASM 
	bgu	r2, r0,	ret_to_kernel
	mv	r4, r0				//?????? why=0 //(!=0 -> syscall)
	bi	ret_to_user


_interrupt_handler2:
	mva 	r0, _psr			//non conventional use of r0 register. It should be zero.
	lw      r0, (r0+0)			//get the program status register
	andi	r0, r0, PSR_KERNEL_MODE_ASM 	//are we in kernel mode already?
	sw	(sp+0xfffc), r1			//store r1 temporarily on the caller's stack (this should be avoided, but int are disabled and is just for a little time). 0xfffc=-4
	xor	r1, r1, r1
	cmpei	r0, r0, PSR_KERNEL_MODE_ASM 
	bgu	r0, r1, 1f			//jump to kernel mode 
	lw	r1, (sp+0xfffc)			//restore r1 from the stack, see also the label '1' below
	mva	r0, _current_thread		//take the _current_thread
	addi	r0, r0, PAGE_SZ
	addi	r0, r0, -S_FRAME_SIZE		//point to the top of the stack minus the space to contain the hardware context of the process (pt_regs)
	sw	(r0+S_SP), sp			//save the stack pointer in pt_regs
	mv	sp, r0
	bi	2f
      1:	
	lw	r1, (sp+0xfffc)			//restore r1 from the stack, in case the jump is taken
	mv	r0, sp
      	addi	sp, sp, -S_FRAME_SIZE		//we are already on the kernel stack
	sw	(sp+S_SP), r0			//save the stack pointer in pt_regs
      2:
      	xor 	r0, r0, r0			//r0 finally in place
	SAVE_ALL
	addi	r1, r0, -1
	sw      (sp+S_ORIG_R1), r1		//store -1 into ORIG_r1 to denote we're from interrupt or exception
	mva 	r1, _psr			//update the program status register
	lw      r2, (r1+0)			//to be in kernel mode
	ori	r2, r2, PSR_KERNEL_MODE_ASM
	sw	(r1+0), r2

	irq_handler 

	lw	r2, (sp+S_PSR)			//take the program status register of the process just being saved from pt_regs, since by now we are in kernel mode
	andi	r2, r2, PSR_KERNEL_MODE_ASM 	//are we in kernel mode?
	cmpei	r2, r0, PSR_KERNEL_MODE_ASM 
	bgu	r2, r0,	ret_to_kernel
	mv	r4, r0				//?????? why=0 //(!=0 -> syscall)
	bi	ret_to_user


ret_to_kernel:
	RESTORE_ALL
	eret



	.data

	.globl	cr_alignment
	.globl	cr_no_alignment
cr_alignment:
	.space	4
cr_no_alignment:
	.space	4
