#include <linux/init.h>
#include <linux/linkage.h>

#include <asm/assembler.h>
#include <asm/asm-offsets.h>
#include <asm/errno.h>
#include <asm/thread_info.h>

// Bad Abort numbers
// -----------------

#define BAD_PREFETCH	0
#define BAD_DATA	1
#define BAD_ADDREXCPTN	2
#define BAD_IRQ		3
#define BAD_UNDEFINSTR	4


// Most of the stack format comes from struct pt_regs, but with
// the addition of 8 bytes for storing syscall args 5 and 6.
// This _must_ remain a multiple of 8 for EABI.

#define S_OFF		8	//for Mico32, we have plenty of registers...... hehe ;)

/* 
 * The SWI code relies on the fact that R0 is at the bottom of the stack
 * (due to slow/fast restore user regs).
 */
#if S_R0 != 0
#error "Please fix"
#endif

.macro SAVE_ALL	//TODO: add also EA, BA, IE, (IM) register
        /* Save all caller/callee saved registers (plus all others) onto the stack */
        sw      (sp+S_R0), r0
        sw      (sp+S_R1), r1
        sw      (sp+S_R2), r2
        sw      (sp+S_R3), r3
        sw      (sp+S_R4), r4
        sw      (sp+S_R5), r5
        sw      (sp+S_R6), r6
        sw      (sp+S_R7), r7
        sw      (sp+S_R8), r8
        sw      (sp+S_R9), r9
        sw      (sp+S_R10), r10
        sw      (sp+S_R11), r11
        sw      (sp+S_R12), r12
        sw      (sp+S_R13), r13
        sw      (sp+S_R14), r14
        sw      (sp+S_R15), r15
        sw      (sp+S_R16), r16
        sw      (sp+S_R17), r17
        sw      (sp+S_R18), r18
        sw      (sp+S_R19), r19
        sw      (sp+S_R20), r20
        sw      (sp+S_R21), r21
        sw      (sp+S_R22), r22
        sw      (sp+S_R23), r23
        sw      (sp+S_R24), r24
        sw      (sp+S_R25), r25
        sw      (sp+S_GP), gp
        sw      (sp+S_FP), fp
        sw      (sp+S_SP), sp
        sw      (sp+S_RA), ra
        sw      (sp+S_EA), ea
        sw      (sp+S_BA), ba
        //sw      (sp+S_ORIG_R1), r1
	rcsr	r1, IE
        sw      (sp+S_IE), r1
	mva 	r1, _psr			
	lw      r1, (r1+0)	
	sw	(sp+S_PSR), r1
.endm

/* Restore all registers */
.macro RESTORE_ALL
        lw      r1, (sp+S_IE)
 	wcsr	IE, r1
	mva 	r1, _psr			
	lw      r1, (sp+S_PSR)	
	sw	(r1+0), r1
        lw      r0, (sp+S_R0)
        lw      r1, (sp+S_R1)
        lw      r2, (sp+S_R2)
        lw      r3, (sp+S_R3)
        lw      r4, (sp+S_R4)
        lw      r5, (sp+S_R5)
        lw      r6, (sp+S_R6)
        lw      r7, (sp+S_R7)
        lw      r8, (sp+S_R8)
        lw      r9, (sp+S_R9)
        lw      r10, (sp+S_R10)
        lw      r11, (sp+S_R11)
        lw      r12, (sp+S_R12)
        lw      r13, (sp+S_R13)
        lw      r14, (sp+S_R14)
        lw      r15, (sp+S_R15)
        lw      r16, (sp+S_R16)
        lw      r17, (sp+S_R17)
        lw      r18, (sp+S_R18)
        lw      r19, (sp+S_R19)
        lw      r20, (sp+S_R20)
        lw      r21, (sp+S_R21)
        lw      r22, (sp+S_R22)
        lw      r23, (sp+S_R23)
        lw      r24, (sp+S_R24)
        lw      r25, (sp+S_R25)
        lw      gp, (sp+S_GP)
        lw      fp, (sp+S_FP)
        lw      sp, (sp+S_SP)
        lw      ra, (sp+S_RA)
        lw      ea, (sp+S_EA)
        lw      ba, (sp+S_BA)
.endm


	.macro	zero_fp
#ifdef CONFIG_FRAME_POINTER
	mv	fp, r0
#endif
	.endm

	.macro	get_thread_info, rd
	sri	\rd, sp, 13
	sli	\rd, \rd, 13
	.endm

	.macro	alignment_trap, rtemp
#ifdef CONFIG_ALIGNMENT_TRAP
	//ldr	\rtemp, .LCcralign
	//ldr	\rtemp, [\rtemp]
	//mcr	p15, 0, \rtemp, c1, c0
#endif
	.endm


/*
 * These are the registers used in the syscall handler, and allow us to
 * have in theory up to 7 arguments to a function - r0 to r6.
 *
 * r7 is reserved for the system call number for thumb mode.
 *
 * Note that tbl == why is intentional.
 *
 * We must set at least "tsk" and "why" when calling ret_with_reschedule.
 */

//unfortunately it seems that gas' port from Lattice does not implement register aliasing...... don't blame me!
//scno	.req	r7		// syscall number
//tbl	.req	r8		// syscall table pointer
//why	.req	r8		// Linux syscall (!= 0)
//tsk	.req	r9		// current thread_info
