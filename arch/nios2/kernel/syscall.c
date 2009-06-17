/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1995, 1996, 1997, 2000, 2001, 05 by Ralf Baechle
 * Copyright (C) 1999, 2000 Silicon Graphics, Inc.
 * Copyright (C) 2001 MIPS Technologies, Inc.
 */
#include <linux/a.out.h>
#include <linux/capability.h>
#include <linux/errno.h>
#include <linux/linkage.h>
#include <linux/mm.h>
#include <linux/smp.h>
#include <linux/mman.h>
#include <linux/ptrace.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/syscalls.h>
#include <linux/file.h>
#include <linux/slab.h>
#include <linux/utsname.h>
#include <linux/unistd.h>
#include <linux/sem.h>
#include <linux/msg.h>
#include <linux/shm.h>
#include <linux/compiler.h>
#include <linux/module.h>

#include <asm/cachectl.h>
#include <asm/cacheflush.h>
#include <asm/ipc.h>
#include <asm/asm-offsets.h>
#include <asm/signal.h>
#include <asm/shmparam.h>
#include <asm/uaccess.h>

unsigned long shm_align_mask = SHMLBA - 1;	/* Sane caches */

EXPORT_SYMBOL(shm_align_mask);

#define COLOUR_ALIGN(addr,pgoff)				\
	((((addr) + shm_align_mask) & ~shm_align_mask) +	\
	 (((pgoff) << PAGE_SHIFT) & shm_align_mask))

unsigned long arch_get_unmapped_area(struct file *filp, unsigned long addr,
	unsigned long len, unsigned long pgoff, unsigned long flags)
{
	struct vm_area_struct * vmm;
	int do_color_align;
	unsigned long task_size;

	task_size = STACK_TOP;

	if (flags & MAP_FIXED) {
		/*
		 * We do not accept a shared mapping if it would violate
		 * cache aliasing constraints.
		 */
		if ((flags & MAP_SHARED) && (addr & shm_align_mask))
			return -EINVAL;
		return addr;
	}

	if (len > task_size)
		return -ENOMEM;
	do_color_align = 0;
	if (filp || (flags & MAP_SHARED))
		do_color_align = 1;
	if (addr) {
		if (do_color_align)
			addr = COLOUR_ALIGN(addr, pgoff);
		else
			addr = PAGE_ALIGN(addr);
		vmm = find_vma(current->mm, addr);
		if (task_size - len >= addr &&
		    (!vmm || addr + len <= vmm->vm_start))
			return addr;
	}
	addr = TASK_UNMAPPED_BASE;
	if (do_color_align)
		addr = COLOUR_ALIGN(addr, pgoff);
	else
		addr = PAGE_ALIGN(addr);

	for (vmm = find_vma(current->mm, addr); ; vmm = vmm->vm_next) {
		/* At this point:  (!vmm || addr < vmm->vm_end). */
		if (task_size - len < addr)
			return -ENOMEM;
		if (!vmm || addr + len <= vmm->vm_start)
			return addr;
		addr = vmm->vm_end;
		if (do_color_align)
			addr = COLOUR_ALIGN(addr, pgoff);
	}
}

/* common code for old and new mmaps */
static inline unsigned long
do_mmap2(unsigned long addr, unsigned long len, unsigned long prot,
        unsigned long flags, unsigned long fd, unsigned long pgoff)
{
	unsigned long error = -EBADF;
	struct file * file = NULL;

	flags &= ~(MAP_EXECUTABLE | MAP_DENYWRITE);
	if (!(flags & MAP_ANONYMOUS)) {
		file = fget(fd);
		if (!file)
			goto out;
	}

	down_write(&current->mm->mmap_sem);
	error = do_mmap_pgoff(file, addr, len, prot, flags, pgoff);
	up_write(&current->mm->mmap_sem);

	if (file)
		fput(file);
out:
	return error;
}

/*
 * Compacrapability ...
 */
asmlinkage int sys_uname(struct old_utsname __user * name)
{
	if (name && !copy_to_user(name, utsname(), sizeof (*name)))
		return 0;
	return -EFAULT;
}

/*
 * Compacrapability ...
 */
asmlinkage int sys_olduname(struct oldold_utsname __user * name)
{
	int error;

	if (!name)
		return -EFAULT;
	if (!access_ok(VERIFY_WRITE,name,sizeof(struct oldold_utsname)))
		return -EFAULT;

	error = __copy_to_user(&name->sysname, &utsname()->sysname,
			       __OLD_UTS_LEN);
	error -= __put_user(0, name->sysname + __OLD_UTS_LEN);
	error -= __copy_to_user(&name->nodename, &utsname()->nodename,
				__OLD_UTS_LEN);
	error -= __put_user(0, name->nodename + __OLD_UTS_LEN);
	error -= __copy_to_user(&name->release, &utsname()->release,
				__OLD_UTS_LEN);
	error -= __put_user(0, name->release + __OLD_UTS_LEN);
	error -= __copy_to_user(&name->version, &utsname()->version,
				__OLD_UTS_LEN);
	error -= __put_user(0, name->version + __OLD_UTS_LEN);
	error -= __copy_to_user(&name->machine, &utsname()->machine,
				__OLD_UTS_LEN);
	error = __put_user(0, name->machine + __OLD_UTS_LEN);
	error = error ? -EFAULT : 0;

	return error;
}

asmlinkage int sys_set_thread_area(unsigned long addr)
{
   BUG();
   return 0;
}

/*
 * No implemented yet ...
 */
asmlinkage int sys_cachectl(char *addr, int nbytes, int op)
{
	return -ENOSYS;
}

/*
 * If we ever come here the user sp is bad.  Zap the process right away.
 * Due to the bad stack signaling wouldn't work.
 */
asmlinkage void bad_stack(void)
{
	do_exit(SIGSEGV);
}
