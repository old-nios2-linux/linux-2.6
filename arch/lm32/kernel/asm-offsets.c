/*
 * Copyright (C) 1995-2003 Russell King
 *               2001-2002 Keith Owens
 *               2007	   Andrea della Porta
 *     
 * Generate definitions needed by assembly language modules.
 * This code generates raw asm output which is post-processed to extract
 * and format the required data.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/sched.h>
#include <linux/mm.h>
#include <asm/mach/arch.h>
#include <asm/thread_info.h>
#include <asm/memory.h>
#include <asm/procinfo.h>

/*
 * GCC 3.0, 3.1: general bad code generation.
 * GCC 3.2.0: incorrect function argument offset calculation.
 * GCC 3.2.x: miscompiles NEW_AUX_ENT in fs/binfmt_elf.c
 *            (http://gcc.gnu.org/PR8896) and incorrect structure
 *	      initialisation in fs/jffs2/erase.c
 */
#if (__GNUC__ == 3 && __GNUC_MINOR__ < 3)
#error Your compiler is too buggy; it is known to miscompile kernels.
#error    Known good compilers: 3.3
#endif

/* Use marker if you need to separate the values later */

#define DEFINE(sym, val) \
        asm volatile("\n->" #sym " %0 " #val : : "i" (val))

#define BLANK() asm volatile("\n->" : : )

int main(void)
{
  DEFINE(TSK_ACTIVE_MM,		offsetof(struct task_struct, active_mm));
  BLANK();
  DEFINE(TI_FLAGS,		offsetof(struct thread_info, flags));
  DEFINE(TI_PREEMPT,		offsetof(struct thread_info, preempt_count));
  DEFINE(TI_ADDR_LIMIT,		offsetof(struct thread_info, addr_limit));
  DEFINE(TI_TASK,		offsetof(struct thread_info, task));
  DEFINE(TI_EXEC_DOMAIN,	offsetof(struct thread_info, exec_domain));
  DEFINE(TI_CPU,		offsetof(struct thread_info, cpu));
  DEFINE(TI_CPU_DOMAIN,		offsetof(struct thread_info, cpu_domain));
  DEFINE(TI_CPU_SAVE,		offsetof(struct thread_info, cpu_context));
  DEFINE(TI_KSP,		offsetof(struct thread_info, ksp));
  DEFINE(TI_USED_CP,		offsetof(struct thread_info, used_cp));
  DEFINE(TI_TP_VALUE,		offsetof(struct thread_info, tp_value));
  DEFINE(TI_FPSTATE,		offsetof(struct thread_info, fpstate));
  DEFINE(TI_VFPSTATE,		offsetof(struct thread_info, vfpstate));
#ifdef CONFIG_IWMMXT
  DEFINE(TI_IWMMXT_STATE,	offsetof(struct thread_info, fpstate.iwmmxt));
#endif
#ifdef CONFIG_CRUNCH
  DEFINE(TI_CRUNCH_STATE,	offsetof(struct thread_info, crunchstate));
#endif
  BLANK();
  DEFINE(S_R0,			offsetof(struct pt_regs, LM32_r0));
  DEFINE(S_R1,			offsetof(struct pt_regs, LM32_r1));
  DEFINE(S_R2,			offsetof(struct pt_regs, LM32_r2));
  DEFINE(S_R3,			offsetof(struct pt_regs, LM32_r3));
  DEFINE(S_R4,			offsetof(struct pt_regs, LM32_r4));
  DEFINE(S_R5,			offsetof(struct pt_regs, LM32_r5));
  DEFINE(S_R6,			offsetof(struct pt_regs, LM32_r6));
  DEFINE(S_R7,			offsetof(struct pt_regs, LM32_r7));
  DEFINE(S_R8,			offsetof(struct pt_regs, LM32_r8));
  DEFINE(S_R9,			offsetof(struct pt_regs, LM32_r9));
  DEFINE(S_R10,			offsetof(struct pt_regs, LM32_r10));
  DEFINE(S_R11,			offsetof(struct pt_regs, LM32_r11));
  DEFINE(S_R12,			offsetof(struct pt_regs, LM32_r12));
  DEFINE(S_R13,			offsetof(struct pt_regs, LM32_r13));
  DEFINE(S_R14,			offsetof(struct pt_regs, LM32_r14));
  DEFINE(S_R15,			offsetof(struct pt_regs, LM32_r15));
  DEFINE(S_R16,			offsetof(struct pt_regs, LM32_r16));
  DEFINE(S_R17,			offsetof(struct pt_regs, LM32_r17));
  DEFINE(S_R18,			offsetof(struct pt_regs, LM32_r18));
  DEFINE(S_R19,			offsetof(struct pt_regs, LM32_r19));
  DEFINE(S_R20,			offsetof(struct pt_regs, LM32_r20));
  DEFINE(S_R21,			offsetof(struct pt_regs, LM32_r21));
  DEFINE(S_R22,			offsetof(struct pt_regs, LM32_r22));
  DEFINE(S_R23,			offsetof(struct pt_regs, LM32_r23));
  DEFINE(S_R24,			offsetof(struct pt_regs, LM32_r24));
  DEFINE(S_R25,			offsetof(struct pt_regs, LM32_r25));
  DEFINE(S_GP,			offsetof(struct pt_regs, LM32_gp));
  DEFINE(S_FP,			offsetof(struct pt_regs, LM32_fp));
  DEFINE(S_SP,			offsetof(struct pt_regs, LM32_sp));
  DEFINE(S_RA,			offsetof(struct pt_regs, LM32_ra));
  DEFINE(S_EA,			offsetof(struct pt_regs, LM32_ea));
  DEFINE(S_BA,			offsetof(struct pt_regs, LM32_ba));
  DEFINE(S_IE,			offsetof(struct pt_regs, LM32_ie));
  //DEFINE(S_IM,		offsetof(struct pt_regs, LM32_im));
  DEFINE(S_PSR,			offsetof(struct pt_regs, LM32_psr));
  DEFINE(S_ORIG_R1,		offsetof(struct pt_regs, LM32_ORIG_r1));
  DEFINE(S_FRAME_SIZE,		sizeof(struct pt_regs));
  BLANK();
  //DEFINE(MM_CONTEXT_ID,		offsetof(struct mm_struct, context.id));
  BLANK();
  DEFINE(VMA_VM_MM,		offsetof(struct vm_area_struct, vm_mm));
  DEFINE(VMA_VM_FLAGS,		offsetof(struct vm_area_struct, vm_flags));
  BLANK();
  DEFINE(VM_EXEC,	       	VM_EXEC);
  BLANK();
  DEFINE(PAGE_SZ,	       	PAGE_SIZE);
  BLANK();
  DEFINE(SYS_ERROR0,		0x9f0000);
  BLANK();
  DEFINE(SIZEOF_MACHINE_DESC,	sizeof(struct machine_desc));
  DEFINE(MACHINFO_TYPE,		offsetof(struct machine_desc, nr));
  DEFINE(MACHINFO_NAME,		offsetof(struct machine_desc, name));
  DEFINE(MACHINFO_PHYSIO,	offsetof(struct machine_desc, phys_io));
  DEFINE(MACHINFO_PGOFFIO,	offsetof(struct machine_desc, io_pg_offst));
  BLANK();
  DEFINE(PROC_INFO_SZ,		sizeof(struct proc_info_list));
  DEFINE(PROCINFO_INITFUNC,	offsetof(struct proc_info_list, __cpu_flush));
  DEFINE(PROCINFO_MM_MMUFLAGS,        offsetof(struct proc_info_list, __cpu_mm_mmu_flags));
  DEFINE(PROCINFO_IO_MMUFLAGS,        offsetof(struct proc_info_list, __cpu_io_mmu_flags));
  BLANK();
  DEFINE(PSR_KERNEL_MODE_ASM,	PSR_KERNEL_MODE);
  
  return 0; 
}
