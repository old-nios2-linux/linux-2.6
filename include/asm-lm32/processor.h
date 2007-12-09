/*
 *  linux/include/asm-lm32/processor.h
 *
 *  Lattice's Mico32 port by Andrea della Porta (sfaragnaus@gmail.com)
 *  based upon the ARM port from Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __ASM_LM32_PROCESSOR_H
#define __ASM_LM32_PROCESSOR_H

/*
 * Default implementation of macro that returns current
 * instruction pointer ("program counter").
 */
#define current_text_addr() ({ __label__ _l; _l: &&_l;})

#ifdef __KERNEL__

#include <asm/ptrace.h>
#include <asm/types.h>

union debug_insn {
	u32	arm;
	u16	thumb;
};

struct debug_entry {
	u32			address;
	union debug_insn	insn;
};

struct debug_info {
	int			nsaved;
	struct debug_entry	bp[2];
};

struct thread_struct {
							/* fault info	  */
	unsigned long		address;
	unsigned long		trap_no;
	unsigned long		error_code;
							/* debugging	  */
	struct debug_info	debug;
};

#define INIT_THREAD  {	}

#ifdef CONFIG_MMU
#define nommu_start_thread(regs) do { } while (0)
#else
#define nommu_start_thread(regs) regs->LM32_r10 = current->mm->start_data
#endif

#define start_thread(regs,pc,sp)					\
({									\
	unsigned long *stack = (unsigned long *)sp;			\
	set_fs(USER_DS);						\
	memzero(regs->uregs, sizeof(regs->uregs));			\
	regs->LM32_ea = pc & ~3;	/* pc */			\
	regs->LM32_sp = sp;		/* sp */			\
	regs->LM32_r3 = stack[2];	/* r3 (envp) */			\
	regs->LM32_r2 = stack[1];	/* r2 (argv) */			\
	regs->LM32_r1 = stack[0];	/* r1 (argc) */			\
	nommu_start_thread(regs);					\
})

/* Forward declaration, a strange C thing */
struct task_struct;

/* Free all resources held by a thread. */
extern void release_thread(struct task_struct *);

/* Prepare to copy thread state - unlazy all lazy status */
#define prepare_to_copy(tsk)	do { } while (0)

unsigned long get_wchan(struct task_struct *p);

#define cpu_relax()			barrier()

/*
 * Create a new kernel thread
 */
extern int kernel_thread(int (*fn)(void *), void *arg, unsigned long flags);

#define task_pt_regs(p) \
	((struct pt_regs *)(THREAD_START_SP + task_stack_page(p)) - 1)

#define KSTK_EIP(tsk)	task_pt_regs(tsk)->LM32_ea
#define KSTK_ESP(tsk)	task_pt_regs(tsk)->LM32_sp

#endif

#endif /* __ASM_LM32_PROCESSOR_H */
