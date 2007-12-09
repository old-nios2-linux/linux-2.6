/*
 *  linux/arch/lm32/kernel/signal.c
 *
 *  Lattice's Mico32 soft core support by Andrea della Porta <sfaragnaus@gmail.com>
 *  based upon the ARM's port from Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/errno.h>
#include <linux/signal.h>
#include <linux/personality.h>
#include <linux/freezer.h>

#include <asm/elf.h>
#include <asm/cacheflush.h>
#include <asm/ucontext.h>
#include <asm/uaccess.h>
#include <asm/unistd.h>

#include "ptrace.h"
#include "signal.h"

#define _BLOCKABLE (~(sigmask(SIGKILL) | sigmask(SIGSTOP)))

/*
 * For ARM syscalls, we encode the syscall number into the instruction.
 */
//#define SWI_SYS_SIGRETURN	(0xef000000|(__NR_sigreturn))
//#define SWI_SYS_RT_SIGRETURN	(0xef000000|(__NR_rt_sigreturn))

/*
 * For Mico32 syscalls, the syscall 'scall' instruction does not contain the syscall number 
 */
#define SCALL			(0xac000007)			//WARNING!!!!!! scall instruction does disable interrupt on Mico32

/*
 * With EABI, the syscall number has to be loaded into r7.
 */
//#define MOV_R7_NR_SIGRETURN	(0xe3a07000 | (__NR_sigreturn - __NR_SYSCALL_BASE))
//#define MOV_R7_NR_RT_SIGRETURN	(0xe3a07000 | (__NR_rt_sigreturn - __NR_SYSCALL_BASE))

/*
 * For Mico32, the syscall number has to be loaded into r2.
 */
#define MV_R2_NR_SIGRETURN	(0x3802 | (__NR_sigreturn - __NR_SYSCALL_BASE))
#define MV_R2_NR_RT_SIGRETURN	(0x3802 | (__NR_rt_sigreturn - __NR_SYSCALL_BASE))

/*
 * For Thumb syscalls, we pass the syscall number via r7.  We therefore
 * need two 16-bit instructions.
 */
//#define SWI_THUMB_SIGRETURN	(0xdf00 << 16 | 0x2700 | (__NR_sigreturn - __NR_SYSCALL_BASE))
//#define SWI_THUMB_RT_SIGRETURN	(0xdf00 << 16 | 0x2700 | (__NR_rt_sigreturn - __NR_SYSCALL_BASE))

const unsigned long sigreturn_codes[5] = {
	MV_R2_NR_SIGRETURN,    SCALL, 
	MV_R2_NR_RT_SIGRETURN, SCALL,
};

static int do_signal(sigset_t *oldset, struct pt_regs * regs, int syscall);

/*
 * atomically swap in the new signal mask, and wait for a signal.
 */
asmlinkage int sys_sigsuspend(int restart, unsigned long oldmask, old_sigset_t mask, struct pt_regs *regs)
{
	sigset_t saveset;

	mask &= _BLOCKABLE;
	spin_lock_irq(&current->sighand->siglock);
	saveset = current->blocked;
	siginitset(&current->blocked, mask);
	recalc_sigpending();
	spin_unlock_irq(&current->sighand->siglock);
	regs->LM32_r1 = -EINTR;

	while (1) {
		current->state = TASK_INTERRUPTIBLE;
		schedule();
		if (do_signal(&saveset, regs, 0))
			return regs->LM32_r1;
	}
}

asmlinkage int
sys_rt_sigsuspend(sigset_t __user *unewset, size_t sigsetsize, struct pt_regs *regs)
{
	sigset_t saveset, newset;

	/* XXX: Don't preclude handling different sized sigset_t's. */
	if (sigsetsize != sizeof(sigset_t))
		return -EINVAL;

	if (copy_from_user(&newset, unewset, sizeof(newset)))
		return -EFAULT;
	sigdelsetmask(&newset, ~_BLOCKABLE);

	spin_lock_irq(&current->sighand->siglock);
	saveset = current->blocked;
	current->blocked = newset;
	recalc_sigpending();
	spin_unlock_irq(&current->sighand->siglock);
	regs->LM32_r1 = -EINTR;

	while (1) {
		current->state = TASK_INTERRUPTIBLE;
		schedule();
		if (do_signal(&saveset, regs, 0))
			return regs->LM32_r1;
	}
}

asmlinkage int 
sys_sigaction(int sig, const struct old_sigaction __user *act,
	      struct old_sigaction __user *oact)
{
	struct k_sigaction new_ka, old_ka;
	int ret;

	if (act) {
		old_sigset_t mask;
		if (!access_ok(VERIFY_READ, act, sizeof(*act)) ||
		    __get_user(new_ka.sa.sa_handler, &act->sa_handler) ||
		    __get_user(new_ka.sa.sa_restorer, &act->sa_restorer))
			return -EFAULT;
		__get_user(new_ka.sa.sa_flags, &act->sa_flags);
		__get_user(mask, &act->sa_mask);
		siginitset(&new_ka.sa.sa_mask, mask);
	}

	ret = do_sigaction(sig, act ? &new_ka : NULL, oact ? &old_ka : NULL);

	if (!ret && oact) {
		if (!access_ok(VERIFY_WRITE, oact, sizeof(*oact)) ||
		    __put_user(old_ka.sa.sa_handler, &oact->sa_handler) ||
		    __put_user(old_ka.sa.sa_restorer, &oact->sa_restorer))
			return -EFAULT;
		__put_user(old_ka.sa.sa_flags, &oact->sa_flags);
		__put_user(old_ka.sa.sa_mask.sig[0], &oact->sa_mask);
	}

	return ret;
}

#ifdef CONFIG_CRUNCH
static int preserve_crunch_context(struct crunch_sigframe *frame)
{
	char kbuf[sizeof(*frame) + 8];
	struct crunch_sigframe *kframe;

	/* the crunch context must be 64 bit aligned */
	kframe = (struct crunch_sigframe *)((unsigned long)(kbuf + 8) & ~7);
	kframe->magic = CRUNCH_MAGIC;
	kframe->size = CRUNCH_STORAGE_SIZE;
	crunch_task_copy(current_thread_info(), &kframe->storage);
	return __copy_to_user(frame, kframe, sizeof(*frame));
}

static int restore_crunch_context(struct crunch_sigframe *frame)
{
	char kbuf[sizeof(*frame) + 8];
	struct crunch_sigframe *kframe;

	/* the crunch context must be 64 bit aligned */
	kframe = (struct crunch_sigframe *)((unsigned long)(kbuf + 8) & ~7);
	if (__copy_from_user(kframe, frame, sizeof(*frame)))
		return -1;
	if (kframe->magic != CRUNCH_MAGIC ||
	    kframe->size != CRUNCH_STORAGE_SIZE)
		return -1;
	crunch_task_restore(current_thread_info(), &kframe->storage);
	return 0;
}
#endif

#ifdef CONFIG_IWMMXT

static int preserve_iwmmxt_context(struct iwmmxt_sigframe *frame)
{
	char kbuf[sizeof(*frame) + 8];
	struct iwmmxt_sigframe *kframe;

	/* the iWMMXt context must be 64 bit aligned */
	kframe = (struct iwmmxt_sigframe *)((unsigned long)(kbuf + 8) & ~7);
	kframe->magic = IWMMXT_MAGIC;
	kframe->size = IWMMXT_STORAGE_SIZE;
	iwmmxt_task_copy(current_thread_info(), &kframe->storage);
	return __copy_to_user(frame, kframe, sizeof(*frame));
}

static int restore_iwmmxt_context(struct iwmmxt_sigframe *frame)
{
	char kbuf[sizeof(*frame) + 8];
	struct iwmmxt_sigframe *kframe;

	/* the iWMMXt context must be 64 bit aligned */
	kframe = (struct iwmmxt_sigframe *)((unsigned long)(kbuf + 8) & ~7);
	if (__copy_from_user(kframe, frame, sizeof(*frame)))
		return -1;
	if (kframe->magic != IWMMXT_MAGIC ||
	    kframe->size != IWMMXT_STORAGE_SIZE)
		return -1;
	iwmmxt_task_restore(current_thread_info(), &kframe->storage);
	return 0;
}

#endif

/*
 * Do a signal return; undo the signal stack.  These are aligned to 64-bit.
 */
struct sigframe {
	struct ucontext uc;
	unsigned long retcode[2];
};

struct rt_sigframe {
	struct siginfo info;
	struct sigframe sig;
};

static int restore_sigframe(struct pt_regs *regs, struct sigframe __user *sf)
{
	struct aux_sigframe __user *aux;
	sigset_t set;
	int err;

	err = __copy_from_user(&set, &sf->uc.uc_sigmask, sizeof(set));
	if (err == 0) {
	          sigdelsetmask(&set, ~_BLOCKABLE);
	          spin_lock_irq(&current->sighand->siglock);
	          current->blocked = set;
	          recalc_sigpending();
	          spin_unlock_irq(&current->sighand->siglock);
	}

	__get_user_error(regs->LM32_r0, &sf->uc.uc_mcontext.lm32_r0, err);
	__get_user_error(regs->LM32_r1, &sf->uc.uc_mcontext.lm32_r1, err);
	__get_user_error(regs->LM32_r2, &sf->uc.uc_mcontext.lm32_r2, err);
	__get_user_error(regs->LM32_r3, &sf->uc.uc_mcontext.lm32_r3, err);
	__get_user_error(regs->LM32_r4, &sf->uc.uc_mcontext.lm32_r4, err);
	__get_user_error(regs->LM32_r5, &sf->uc.uc_mcontext.lm32_r5, err);
	__get_user_error(regs->LM32_r6, &sf->uc.uc_mcontext.lm32_r6, err);
	__get_user_error(regs->LM32_r7, &sf->uc.uc_mcontext.lm32_r7, err);
	__get_user_error(regs->LM32_r8, &sf->uc.uc_mcontext.lm32_r8, err);
	__get_user_error(regs->LM32_r9, &sf->uc.uc_mcontext.lm32_r9, err);
	__get_user_error(regs->LM32_r10, &sf->uc.uc_mcontext.lm32_r10, err);
	__get_user_error(regs->LM32_r11, &sf->uc.uc_mcontext.lm32_r11, err);
	__get_user_error(regs->LM32_r12, &sf->uc.uc_mcontext.lm32_r12, err);
	__get_user_error(regs->LM32_r13, &sf->uc.uc_mcontext.lm32_r13, err);
	__get_user_error(regs->LM32_r14, &sf->uc.uc_mcontext.lm32_r14, err);
	__get_user_error(regs->LM32_r15, &sf->uc.uc_mcontext.lm32_r15, err);
	__get_user_error(regs->LM32_r16, &sf->uc.uc_mcontext.lm32_r16, err);
	__get_user_error(regs->LM32_r17, &sf->uc.uc_mcontext.lm32_r17, err);
	__get_user_error(regs->LM32_r18, &sf->uc.uc_mcontext.lm32_r18, err);
	__get_user_error(regs->LM32_r19, &sf->uc.uc_mcontext.lm32_r19, err);
	__get_user_error(regs->LM32_r20, &sf->uc.uc_mcontext.lm32_r20, err);
	__get_user_error(regs->LM32_r21, &sf->uc.uc_mcontext.lm32_r21, err);
	__get_user_error(regs->LM32_r22, &sf->uc.uc_mcontext.lm32_r22, err);
	__get_user_error(regs->LM32_r23, &sf->uc.uc_mcontext.lm32_r23, err);
	__get_user_error(regs->LM32_r24, &sf->uc.uc_mcontext.lm32_r24, err);
	__get_user_error(regs->LM32_r25, &sf->uc.uc_mcontext.lm32_r25, err);
	__get_user_error(regs->LM32_gp, &sf->uc.uc_mcontext.lm32_gp, err);
	__get_user_error(regs->LM32_fp, &sf->uc.uc_mcontext.lm32_fp, err);
	__get_user_error(regs->LM32_sp, &sf->uc.uc_mcontext.lm32_sp, err);
	__get_user_error(regs->LM32_ra, &sf->uc.uc_mcontext.lm32_ra, err);
	__get_user_error(regs->LM32_ea, &sf->uc.uc_mcontext.lm32_ea, err);
	__get_user_error(regs->LM32_ba, &sf->uc.uc_mcontext.lm32_ba, err);
	__get_user_error(regs->LM32_ie, &sf->uc.uc_mcontext.lm32_ie, err);
	__get_user_error(regs->LM32_psr, &sf->uc.uc_mcontext.lm32_psr, err);
	//__get_user_error(regs->LM32_ORIG_r1, &sf->uc.uc_mcontext.lm32_ORIG_r1, err);		//ARM port does not restore it

	err |= !valid_user_regs(regs);

	aux = (struct aux_sigframe __user *) sf->uc.uc_regspace;
#ifdef CONFIG_CRUNCH
        if (err == 0)
               err |= restore_crunch_context(&aux->crunch);
#endif
#ifdef CONFIG_IWMMXT
        if (err == 0 && test_thread_flag(TIF_USING_IWMMXT))
               err |= restore_iwmmxt_context(&aux->iwmmxt);
#endif
#ifdef CONFIG_VFP
//	if (err == 0)
//	err |= vfp_restore_state(&sf->aux.vfp);
#endif

	return err;
}

asmlinkage int sys_sigreturn(struct pt_regs *regs)
{
	struct sigframe __user *frame;

	/* Always make any pending restarted system calls return -EINTR */
	current_thread_info()->restart_block.fn = do_no_restart_syscall;

	/*
	 * Since we stacked the signal on a 64-bit boundary,
	 * then 'sp' should be word aligned here.  If it's
	 * not, then the user is trying to mess with us.
	 */
	if (regs->LM32_sp & 7)
		goto badframe;

	frame = (struct sigframe __user *)regs->LM32_sp;

	if (!access_ok(VERIFY_READ, frame, sizeof (*frame)))
		goto badframe;

	if (restore_sigframe(regs, frame))
		goto badframe;

	single_step_trap(current);

	return regs->LM32_r1;

badframe:
	force_sig(SIGSEGV, current);
	return 0;
}

asmlinkage int sys_rt_sigreturn(struct pt_regs *regs)
{
	struct rt_sigframe __user *frame;

	/* Always make any pending restarted system calls return -EINTR */
	current_thread_info()->restart_block.fn = do_no_restart_syscall;

	/*
	 * Since we stacked the signal on a 64-bit boundary,
	 * then 'sp' should be word aligned here.  If it's
	 * not, then the user is trying to mess with us.
	 */
	if (regs->LM32_sp & 7)
		goto badframe;

	frame = (struct rt_sigframe __user *)regs->LM32_sp;

	if (!access_ok(VERIFY_READ, frame, sizeof (*frame)))
		goto badframe;
	if (restore_sigframe(regs, &frame->sig))
		goto badframe;

	if (do_sigaltstack(&frame->sig.uc.uc_stack, NULL, regs->LM32_sp) == -EFAULT)
		goto badframe;

	single_step_trap(current);

	return regs->LM32_r1;

badframe:
	force_sig(SIGSEGV, current);
	return 0;
}

static int
setup_sigframe(struct sigframe __user *sf, struct pt_regs *regs, sigset_t *set)
{
	struct aux_sigframe __user *aux;
	int err = 0;

	__put_user_error(regs->LM32_r0, &sf->uc.uc_mcontext.lm32_r0, err);
	__put_user_error(regs->LM32_r1, &sf->uc.uc_mcontext.lm32_r1, err);
	__put_user_error(regs->LM32_r2, &sf->uc.uc_mcontext.lm32_r2, err);
	__put_user_error(regs->LM32_r3, &sf->uc.uc_mcontext.lm32_r3, err);
	__put_user_error(regs->LM32_r4, &sf->uc.uc_mcontext.lm32_r4, err);
	__put_user_error(regs->LM32_r5, &sf->uc.uc_mcontext.lm32_r5, err);
	__put_user_error(regs->LM32_r6, &sf->uc.uc_mcontext.lm32_r6, err);
	__put_user_error(regs->LM32_r7, &sf->uc.uc_mcontext.lm32_r7, err);
	__put_user_error(regs->LM32_r8, &sf->uc.uc_mcontext.lm32_r8, err);
	__put_user_error(regs->LM32_r9, &sf->uc.uc_mcontext.lm32_r9, err);
	__put_user_error(regs->LM32_r10, &sf->uc.uc_mcontext.lm32_r10, err);
	__put_user_error(regs->LM32_r11, &sf->uc.uc_mcontext.lm32_r11, err);
	__put_user_error(regs->LM32_r12, &sf->uc.uc_mcontext.lm32_r12, err);
	__put_user_error(regs->LM32_r13, &sf->uc.uc_mcontext.lm32_r13, err);
	__put_user_error(regs->LM32_r14, &sf->uc.uc_mcontext.lm32_r14, err);
	__put_user_error(regs->LM32_r15, &sf->uc.uc_mcontext.lm32_r15, err);
	__put_user_error(regs->LM32_r16, &sf->uc.uc_mcontext.lm32_r16, err);
	__put_user_error(regs->LM32_r17, &sf->uc.uc_mcontext.lm32_r17, err);
	__put_user_error(regs->LM32_r18, &sf->uc.uc_mcontext.lm32_r18, err);
	__put_user_error(regs->LM32_r19, &sf->uc.uc_mcontext.lm32_r19, err);
	__put_user_error(regs->LM32_r20, &sf->uc.uc_mcontext.lm32_r20, err);
	__put_user_error(regs->LM32_r21, &sf->uc.uc_mcontext.lm32_r21, err);
	__put_user_error(regs->LM32_r22, &sf->uc.uc_mcontext.lm32_r22, err);
	__put_user_error(regs->LM32_r23, &sf->uc.uc_mcontext.lm32_r23, err);
	__put_user_error(regs->LM32_r24, &sf->uc.uc_mcontext.lm32_r24, err);
	__put_user_error(regs->LM32_r25, &sf->uc.uc_mcontext.lm32_r25, err);
	__put_user_error(regs->LM32_gp, &sf->uc.uc_mcontext.lm32_gp, err);
	__put_user_error(regs->LM32_fp, &sf->uc.uc_mcontext.lm32_fp, err);
	__put_user_error(regs->LM32_sp, &sf->uc.uc_mcontext.lm32_sp, err);
	__put_user_error(regs->LM32_ra, &sf->uc.uc_mcontext.lm32_ra, err);
	__put_user_error(regs->LM32_ea, &sf->uc.uc_mcontext.lm32_ea, err);
	__put_user_error(regs->LM32_ba, &sf->uc.uc_mcontext.lm32_ba, err);
	__put_user_error(regs->LM32_ie, &sf->uc.uc_mcontext.lm32_ie, err);
	__put_user_error(regs->LM32_psr, &sf->uc.uc_mcontext.lm32_ie, err);
	//__put_user_error(regs->LM32_ORIG_r1, &sf->uc.uc_mcontext.lm32_ORIG_r1, err);	//ARM port does not set it

	__put_user_error(current->thread.trap_no, &sf->uc.uc_mcontext.trap_no, err);
	__put_user_error(current->thread.error_code, &sf->uc.uc_mcontext.error_code, err);
	__put_user_error(current->thread.address, &sf->uc.uc_mcontext.fault_address, err);
	__put_user_error(set->sig[0], &sf->uc.uc_mcontext.oldmask, err);

        err |= __copy_to_user(&sf->uc.uc_sigmask, set, sizeof(*set));

        aux = (struct aux_sigframe __user *) sf->uc.uc_regspace;
#ifdef CONFIG_CRUNCH
        if (err == 0)
               err |= preserve_crunch_context(&aux->crunch);
#endif
#ifdef CONFIG_IWMMXT
	if (err == 0 && test_thread_flag(TIF_USING_IWMMXT))
		err |= preserve_iwmmxt_context(&aux->iwmmxt);
#endif
#ifdef CONFIG_VFP
//	if (err == 0)
//		err |= vfp_save_state(&sf->aux.vfp);
#endif
	__put_user_error(0, &aux->end_magic, err);

	return err;
}

static inline void __user *
get_sigframe(struct k_sigaction *ka, struct pt_regs *regs, int framesize)
{
	unsigned long sp = regs->LM32_sp;
	void __user *frame;

	/*
	 * This is the X/Open sanctioned signal stack switching.
	 */
	if ((ka->sa.sa_flags & SA_ONSTACK) && !sas_ss_flags(sp))
		sp = current->sas_ss_sp + current->sas_ss_size;

	/*
	 * ATPCS B01 mandates 8-byte alignment
	 */
	frame = (void __user *)((sp - framesize) & ~7);

	/*
	 * Check that we can actually write to the signal frame.
	 */
	if (!access_ok(VERIFY_WRITE, frame, framesize))
		frame = NULL;

	return frame;
}

static int
setup_return(struct pt_regs *regs, struct k_sigaction *ka,
	     unsigned long __user *rc, void __user *frame, int usig)
{
	unsigned long handler = (unsigned long)ka->sa.sa_handler;
	unsigned long retcode;
	int thumb = 0;
	//unsigned long cpsr = regs->ARM_cpsr & ~PSR_f;

	/*
	 * Maybe we need to deliver a 32-bit signal to a 26-bit task.
	 */
	/*if (ka->sa.sa_flags & SA_THIRTYTWO)
		cpsr = (cpsr & ~MODE_MASK) | USR_MODE;*/

#ifdef CONFIG_ARM_THUMB
	if (elf_hwcap & HWCAP_THUMB) {
		/*
		 * The LSB of the handler determines if we're going to
		 * be using THUMB or ARM mode for this signal handler.
		 */
		thumb = handler & 1;

		if (thumb)
			cpsr |= PSR_T_BIT;
		else
			cpsr &= ~PSR_T_BIT;
	}
#endif

	if (ka->sa.sa_flags & SA_RESTORER) {
		retcode = (unsigned long)ka->sa.sa_restorer;
	} else {
		unsigned int idx = 0; //thumb << 1;

		if (ka->sa.sa_flags & SA_SIGINFO)
			idx += 2;

		if (__put_user(sigreturn_codes[idx],   rc) ||
		    __put_user(sigreturn_codes[idx+1], rc+1))
			return 1;

		/*if (cpsr & MODE32_BIT) {
			//
			// 32-bit code can use the new high-page
			// signal return code support.
			// 
			retcode = KERN_SIGRETURN_CODE + (idx << 2) + thumb;
		} else*/ {
			/*
			 * Ensure that the instruction cache sees
			 * the return code written onto the stack.
			 */
			flush_icache_range((unsigned long)rc,
					   (unsigned long)(rc + 2));

			retcode = ((unsigned long)rc); //+ thumb;
		}
	}

	regs->LM32_r1 = usig;
	regs->LM32_sp = (unsigned long)frame;
	regs->LM32_ra = retcode;
	regs->LM32_ea = handler;
	//regs->ARM_cpsr = cpsr;

	return 0;
}

static int
setup_frame(int usig, struct k_sigaction *ka, sigset_t *set, struct pt_regs *regs)
{
	struct sigframe __user *frame = get_sigframe(ka, regs, sizeof(*frame));
	int err = 0;

	if (!frame)
		return 1;

	/*
	 * Set uc.uc_flags to a value which sc.trap_no would never have.
	 */
	__put_user_error(0x5ac3c35a, &frame->uc.uc_flags, err);

	err |= setup_sigframe(frame, regs, set);
	if (err == 0)
		err = setup_return(regs, ka, frame->retcode, frame, usig);

	return err;
}

static int
setup_rt_frame(int usig, struct k_sigaction *ka, siginfo_t *info,
	       sigset_t *set, struct pt_regs *regs)
{
	struct rt_sigframe __user *frame = get_sigframe(ka, regs, sizeof(*frame));
	stack_t stack;
	int err = 0;

	if (!frame)
		return 1;

	err |= copy_siginfo_to_user(&frame->info, info);

        __put_user_error(0, &frame->sig.uc.uc_flags, err);
        __put_user_error(NULL, &frame->sig.uc.uc_link, err);

	memset(&stack, 0, sizeof(stack));
	stack.ss_sp = (void __user *)current->sas_ss_sp;
	stack.ss_flags = sas_ss_flags(regs->LM32_sp);
	stack.ss_size = current->sas_ss_size;
        err |= __copy_to_user(&frame->sig.uc.uc_stack, &stack, sizeof(stack));

        err |= setup_sigframe(&frame->sig, regs, set);
	if (err == 0)
		err = setup_return(regs, ka, frame->sig.retcode, frame, usig);

	if (err == 0) {
		/*
		 * For realtime signals we must also set the second and third
		 * arguments for the signal handler.
		 *   -- Peter Maydell <pmaydell@chiark.greenend.org.uk> 2000-12-06
		 */
		regs->LM32_r2 = (unsigned long)&frame->info;
		regs->LM32_r3 = (unsigned long)&frame->sig.uc;
	}

	return err;
}

static inline void restart_syscall(struct pt_regs *regs)
{
	regs->LM32_r2 = regs->LM32_ORIG_r1;	//oddly enough, we've chosen to use 'r2' to pass the syscall number through SCALL instruction
	regs->LM32_ea -= 4;			//on Mico32, SCALL is only one isntruction back, since we've just put its argument into 'r2'
}

/*
 * OK, we're invoking a handler
 */	
static void
handle_signal(unsigned long sig, struct k_sigaction *ka,
	      siginfo_t *info, sigset_t *oldset,
	      struct pt_regs * regs, int syscall)
{
	struct thread_info *thread = current_thread_info();
	struct task_struct *tsk = current;
	int usig = sig;
	int ret;

	/*
	 * If we were from a system call, check for system call restarting...
	 */
	if (syscall) {
		switch (regs->LM32_r1) {
		case -ERESTART_RESTARTBLOCK:
		case -ERESTARTNOHAND:
			regs->LM32_r1 = -EINTR;
			break;
		case -ERESTARTSYS:
			if (!(ka->sa.sa_flags & SA_RESTART)) {
				regs->LM32_r1 = -EINTR;
				break;
			}
			/* fallthrough */
		case -ERESTARTNOINTR:
			restart_syscall(regs);
		}
	}

	/*
	 * translate the signal
	 */
	if (usig < 32 && thread->exec_domain && thread->exec_domain->signal_invmap)
		usig = thread->exec_domain->signal_invmap[usig];

	/*
	 * Set up the stack frame
	 */
	if (ka->sa.sa_flags & SA_SIGINFO)
		ret = setup_rt_frame(usig, ka, info, oldset, regs);
	else
		ret = setup_frame(usig, ka, oldset, regs);

	/*
	 * Check that the resulting registers are actually sane.
	 */
	ret |= !valid_user_regs(regs);

	if (ret != 0) {
		force_sigsegv(sig, tsk);
		return;
	}

	/*
	 * Block the signal if we were successful.
	 */
	spin_lock_irq(&tsk->sighand->siglock);
	sigorsets(&tsk->blocked, &tsk->blocked,
		  &ka->sa.sa_mask);
	if (!(ka->sa.sa_flags & SA_NODEFER))
		sigaddset(&tsk->blocked, sig);
	recalc_sigpending();
	spin_unlock_irq(&tsk->sighand->siglock);
}

/*
 * Note that 'init' is a special process: it doesn't get signals it doesn't
 * want to handle. Thus you cannot kill init even with a SIGKILL even by
 * mistake.
 *
 * Note that we go through the signals twice: once to check the signals that
 * the kernel can handle, and then we build all the user-level signal handling
 * stack-frames in one go after that.
 */
static int do_signal(sigset_t *oldset, struct pt_regs *regs, int syscall)
{
	struct k_sigaction ka;
	siginfo_t info;
	int signr;

	/*
	 * We want the common case to go fast, which
	 * is why we may in certain cases get here from
	 * kernel mode. Just return without doing anything
	 * if so.
	 */
	if (!user_mode(regs))
		return 0;

	if (try_to_freeze())
		goto no_signal;

	single_step_clear(current);

	signr = get_signal_to_deliver(&info, &ka, regs, NULL);
	if (signr > 0) {
		handle_signal(signr, &ka, &info, oldset, regs, syscall);
		single_step_set(current);
		return 1;
	}

 no_signal:
	/*
	 * No signal to deliver to the process - restart the syscall.
	 */
	if (syscall) {
		if (regs->LM32_r1 == -ERESTART_RESTARTBLOCK) {
			//if (thumb_mode(regs)) {
				regs->LM32_r2 = __NR_restart_syscall - __NR_SYSCALL_BASE;
				regs->LM32_ea -= 4;
			/*} else {
				u32 __user *usp;

				regs->ARM_sp -= 12;
				usp = (u32 __user *)regs->ARM_sp;

				put_user(regs->ARM_pc, &usp[0]);
				// swi __NR_restart_syscall 
				put_user(0xef000000 | __NR_restart_syscall, &usp[1]);
				// ldr	pc, [sp], #12 
				put_user(0xe49df00c, &usp[2]);

				flush_icache_range((unsigned long)usp,
						   (unsigned long)(usp + 3));

				regs->ARM_pc = regs->ARM_sp + 4;
			}*/
		}
		if (regs->LM32_r1 == -ERESTARTNOHAND ||
		    regs->LM32_r1 == -ERESTARTSYS ||
		    regs->LM32_r1 == -ERESTARTNOINTR) {
			restart_syscall(regs);
		}
	}
	single_step_set(current);
	return 0;
}

asmlinkage void
do_notify_resume(struct pt_regs *regs, unsigned int thread_flags, int syscall)
{
	if (thread_flags & _TIF_SIGPENDING)
		do_signal(&current->blocked, regs, syscall);
}
