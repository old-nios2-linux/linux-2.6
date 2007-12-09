/*
 *  linux/include/asm-lm32/ptrace.h
 *
 * Andrea della Porta <sfaragnaus@gmail.com>
 *
 * based on the LM32 version
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __ASM_LM32_PTRACE_H
#define __ASM_LM32_PTRACE_H

#include <asm/hwcap.h>

#define PTRACE_GETREGS		12
#define PTRACE_SETREGS		13
#define PTRACE_GETFPREGS	14
#define PTRACE_SETFPREGS	15
/* PTRACE_ATTACH is 16 */
/* PTRACE_DETACH is 17 */
#define PTRACE_GETWMMXREGS	18
#define PTRACE_SETWMMXREGS	19
/* 20 is unused */
#define PTRACE_OLDSETOPTIONS	21
#define PTRACE_GET_THREAD_AREA	22
#define PTRACE_SET_SYSCALL	23

/*
 * PSR bits
 */
#define SYSTEM_MODE	0x0000001f
#define MODE32_BIT	0x00000010
#define MODE_MASK	0x0000001f
#define PSR_T_BIT	0x00000020
#define PSR_F_BIT	0x00000040
#define PSR_I_BIT	0x00000080
#define PSR_A_BIT	0x00000100
#define PSR_J_BIT	0x01000000
#define PSR_Q_BIT	0x08000000
#define PSR_V_BIT	0x10000000
#define PSR_C_BIT	0x20000000
#define PSR_Z_BIT	0x40000000
#define PSR_N_BIT	0x80000000
#define PCMASK		3			//On Mico32, PC must be 4 byte aligned
#define INT_ENABLE	1
#define EIE_INT_ENABLE	2
#define BIE_INT_ENABLE	4

/*
 * Groups of PSR bits
 */
#define PSR_f		0xff000000	/* Flags		*/
#define PSR_s		0x00ff0000	/* Status		*/
#define PSR_x		0x0000ff00	/* Extension		*/
#define PSR_c		0x000000ff	/* Control		*/

#ifndef __ASSEMBLY__

/*
 * This struct defines the way the registers are stored on the
 * stack during a system call.  Note that sizeof(struct pt_regs)
 * has to be a multiple of 8.
 */
struct pt_regs {
	long uregs[35];	//36, if you enable Interrupt Mask related registers
};

//There's no register involved in retaining condition flag and the like. Yup, context switching gonna be very fun!!!!!!
//#define LM32_im		uregs[35]
#define LM32_ORIG_r1	uregs[34]
#define LM32_psr	uregs[33]
#define LM32_ie		uregs[32]
#define LM32_ba		uregs[31]
#define LM32_ea		uregs[30]
#define LM32_ra		uregs[29]
#define LM32_sp		uregs[28]
#define LM32_fp		uregs[27]
#define LM32_gp		uregs[26]
#define LM32_r25	uregs[25]
#define LM32_r24	uregs[24]
#define LM32_r23	uregs[23]
#define LM32_r22	uregs[22]
#define LM32_r21	uregs[21]
#define LM32_r20	uregs[20]
#define LM32_r19	uregs[19]
#define LM32_r18	uregs[18]
#define LM32_r17	uregs[17]
#define LM32_r16	uregs[16]
#define LM32_r15	uregs[15]
#define LM32_r14	uregs[14]
#define LM32_r13	uregs[13]
#define LM32_r12	uregs[12]
#define LM32_r11	uregs[11]
#define LM32_r10	uregs[10]
#define LM32_r9		uregs[9]
#define LM32_r8		uregs[8]
#define LM32_r7		uregs[7]
#define LM32_r6		uregs[6]
#define LM32_r5		uregs[5]
#define LM32_r4		uregs[4]
#define LM32_r3		uregs[3]
#define LM32_r2		uregs[2]
#define LM32_r1		uregs[1]
#define LM32_r0		uregs[0]

#ifdef __KERNEL__

/*
#ifdef CONFIG_LM32_THUMB
#define thumb_mode(regs) \
	(((regs)->LM32_cpsr & PSR_T_BIT))
#else
#define thumb_mode(regs) (0)
#endif

#define isa_mode(regs) \
	((((regs)->ARM_cpsr & PSR_J_BIT) >> 23) | \
	 (((regs)->ARM_cpsr & PSR_T_BIT) >> 5))*/

#define processor_mode(regs) \
	((regs)->LM32_cpsr & MODE_MASK)

#define fast_interrupts_enabled(regs) \
	(!((regs)->LM32_cpsr & PSR_F_BIT))

#define condition_codes(regs) \
	((regs)->LM32_cpsr & (PSR_V_BIT|PSR_C_BIT|PSR_Z_BIT|PSR_N_BIT))


#define interrupts_enabled(regs) \
	 (((regs)->LM32_ie) & INT_ENABLE)	

#define eie_interrupts_enabled(regs) \
	 (((regs)->LM32_ie) & EIE_INT_ENABLE)	

#define bie_interrupts_enabled(regs) \
	 (((regs)->LM32_ie) & BIE_INT_ENABLE)	

/* Are the current registers suitable for user mode?
 * (used to maintain security in signal handlers)
 */
static inline int valid_user_regs(struct pt_regs *regs)
{
	//if (user_mode(regs) &&
	//    (regs->LM32_cpsr & (PSR_F_BIT|PSR_I_BIT)) == 0)
	if(interrupts_enabled(regs))
		return 1;

	
	 // Force CPSR to something logical...
	 
	//regs->LM32_cpsr &= PSR_f | PSR_s | PSR_x | PSR_T_BIT | MODE32_BIT;

	return 0;
}

#endif	/* __KERNEL__ */

#define pc_pointer(v) \
	((v) & ~PCMASK)

#define instruction_pointer(regs) \
	 (pc_pointer(\
		     ({long val; __asm__ __volatile__ ("calli 1f\n\t"\
			     		      "1: mv %0, ra\n\t" : "=r"(val) : : "ra");\
		       val;\
		      })\
		    )\
	  )

#ifdef CONFIG_SMP
extern unsigned long profile_pc(struct pt_regs *regs);
#else
#define profile_pc(regs) instruction_pointer(regs)
#endif

#define user_stack(regs) ((regs)->LM32_sp)

#ifdef __KERNEL__
//#define predicate(x)		((x) & 0xf0000000)
//#define PREDICATE_ALWAYS	0xe0000000

#define PSR_KERNEL_MODE		1
#define PSR_XXXXXXXXXXX		2

#define user_mode(regs)	\
	(((regs)->LM32_psr & PSR_KERNEL_MODE) == 0)

#endif

#endif /* __ASSEMBLY__ */

#endif

