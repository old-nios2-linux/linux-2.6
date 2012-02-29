/*
 * Copyright (C) 2011 Tobias Klauser <tklauser@distanz.ch>
 * Copyright (C) 2004 Microtronix Datacom Ltd.
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 */

#include <linux/syscalls.h>
#include <linux/file.h>
#include <linux/export.h>

#include <asm/cacheflush.h>
#include <asm/traps.h>

asmlinkage long sys_mmap(unsigned long addr, unsigned long len,
			 unsigned long prot, unsigned long flags,
			 unsigned long fd, unsigned long offset)
{
	if (offset & ~PAGE_MASK)
		return -EINVAL;

	return sys_mmap_pgoff(addr, len, prot, flags, fd, offset >> PAGE_SHIFT);
}

/* sys_cacheflush -- flush the processor cache. */
asmlinkage int sys_cacheflush(unsigned long addr, int scope, int cache,
			      unsigned long len)
{
#ifndef CONFIG_MMU
	flush_cache_all();
#else
	struct vm_area_struct *vma;

	if (len == 0)
		return 0;

	/* Check for overflow */
	if (addr + len < addr)
		return -EFAULT;

	/*
	 * Verify that the specified address region actually belongs
	 * to this process.
	 */
	vma = find_vma(current->mm, addr);
	if (vma == NULL || addr < vma->vm_start || addr + len > vma->vm_end)
		return -EFAULT;

	/* Ignore the scope and cache arguments. */
	flush_cache_range(vma, addr, addr + len);
#endif /* CONFIG_MMU */

	return 0;
}

asmlinkage int sys_getpagesize(void)
{
	return PAGE_SIZE;
}

/*
 * Do a system call from kernel instead of calling sys_execve so we
 * end up with proper pt_regs.
 */
#ifdef CONFIG_MMU
int kernel_execve(const char *filename,
		  const char *const argv[],
		  const char *const envp[])
{
	register long __res __asm__ ("r2");
	register long __sc  __asm__ ("r2") = __NR_execve;
	register long __a   __asm__ ("r4") = (long) filename;
	register long __b   __asm__ ("r5") = (long) argv;
	register long __c   __asm__ ("r6") = (long) envp;
	__asm__ __volatile__ ("trap" : "=r" (__res)
			: "0" (__sc), "r" (__a), "r" (__b), "r" (__c)
			: "memory");

	return __res;
}
#else
int kernel_execve(const char *filename,
		  const char *const argv[],
		  const char *const envp[])
{
	register long __res __asm__ ("r2") = TRAP_ID_SYSCALL;
	register long __sc  __asm__ ("r3") = __NR_execve;
	register long __a   __asm__ ("r4") = (long) filename;
	register long __b   __asm__ ("r5") = (long) argv;
	register long __c   __asm__ ("r6") = (long) envp;
	__asm__ __volatile__ ("trap" : "=r" (__res)
			: "0" (__res), "r" (__sc), "r" (__a), "r" (__b), "r" (__c)
			: "memory");

	return __res;
}

#if defined(CONFIG_FB) || defined(CONFIG_FB_MODULE)
#include <linux/fb.h>
unsigned long get_fb_unmapped_area(struct file *filp, unsigned long orig_addr, unsigned long len, unsigned long pgoff, unsigned long flags)
{

	struct fb_info *info = filp->private_data;
	return info->screen_base;
}
EXPORT_SYMBOL(get_fb_unmapped_area);
#endif /* CONFIG_FB */
#endif /* CONFIG_MMU */

/* Uncomment if you uncomment the syscall printk tracing in entry.S */
#if 0
void print_syscall(int sc)
{
	printk("Syscall %d\n", sc);
}


void print_syscall_ret(int rv, int err)
{
	printk("  RET %d %d\n", err, rv);
}
#endif
