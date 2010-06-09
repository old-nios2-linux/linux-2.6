/*
 *  linux/arch/nios2/mm/init.c
 *
 *  Copyright (C) 2009 Wind River Systems Inc trough
 *  Implemented by fredrik.markstrom@gmail.com and ivarholmqvist@gmail.com
 *
 *  Based on:
 * 
 *  linux/arch/nios2nommu/mm/init.c
 *
 *  Copyright (C) 1998  D. Jeff Dionne <jeff@lineo.ca>,
 *                      Kenneth Albanowski <kjahds@kjahds.com>,
 *  Copyright (C) 2000  Lineo, Inc.  (www.lineo.com) 
 * Copyright (C) 2004   Microtronix Datacom Ltd
 *
 *  Based on:
 *
 *  linux/arch/m68k/mm/init.c
 *
 *  Copyright (C) 1995  Hamish Macdonald
 *
 *  JAN/1999 -- hacked to support ColdFire (gerg@snapgear.com)
 *  DEC/2000 -- linux 2.4 support <davidm@snapgear.com>
 * Jan/20/2004		dgt	    NiosII
 *
 */

#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/ptrace.h>
#include <linux/mman.h>
#include <linux/mm.h>
#include <linux/swap.h>
#include <linux/init.h>
#include <linux/highmem.h>
#include <linux/pagemap.h>
#include <linux/bootmem.h>
#include <linux/slab.h>

#include <asm/setup.h>
#include <asm/segment.h>
#include <asm/page.h>
#include <asm/pgtable.h>
#include <asm/system.h>
#include <asm/percpu.h>
#include <asm/tlb.h>
#include <asm/mmu_context.h>

//;dgt2;#include <asm/machdep.h>
//;dgt2;#include <asm/shglcore.h>

#define DEBUG

extern void free_initmem(void);

/*
 * BAD_PAGE is the page that is used for page faults when linux
 * is out-of-memory. Older versions of linux just did a
 * do_exit(), but using this instead means there is less risk
 * for a process dying in kernel mode, possibly leaving a inode
 * unused etc..
 *
 * BAD_PAGETABLE is the accompanying page-table: it is initialized
 * to point to BAD_PAGE entries.
 *
 * ZERO_PAGE is a special page that is used for zero-initialized
 * data and COW.
 */
static unsigned long empty_bad_page_table;

static unsigned long empty_bad_page;

unsigned long empty_zero_page;

unsigned long empty_zero_page, zero_page_mask;

extern unsigned long rom_length;

DEFINE_PER_CPU(struct mmu_gather, mmu_gathers);

extern unsigned long memory_start;
extern unsigned long memory_end;

/*
 * paging_init() continues the virtual memory environment setup which
 * was begun by the code in arch/head.S.
 * The parameters are pointers to where to stick the starting and ending
 * addresses of available kernel virtual memory.
 */
void __init paging_init(void)
{
	/*
	 * Make sure start_mem is page aligned, otherwise bootmem and
	 * page_alloc get different views of the world.
	 */
	unsigned long start_mem = PHYS_OFFSET;
	unsigned long end_mem   = memory_end;

   pagetable_init();
   pgd_current[0] = (unsigned long)swapper_pg_dir;

	/*
	 * Initialize the bad page table and bad page to point
	 * to a couple of allocated pages.
	 */
	empty_bad_page_table = (unsigned long)alloc_bootmem_pages(PAGE_SIZE);
	empty_bad_page = (unsigned long)alloc_bootmem_pages(PAGE_SIZE);
	empty_zero_page = (unsigned long)alloc_bootmem_pages(DCACHE_SIZE);
	memset((void *)empty_zero_page, 0, DCACHE_SIZE);
   flush_dcache_range(empty_zero_page, empty_zero_page + DCACHE_SIZE);

	/*
	 * Set up SFC/DFC registers (user data space).
	 */
#if 0
	set_fs (USER_DS);
#endif

	{
		unsigned long zones_size[MAX_NR_ZONES] = {0, };

		zones_size[ZONE_DMA] = ((end_mem - start_mem) >> PAGE_SHIFT);
		zones_size[ZONE_NORMAL] = 0;
#ifdef CONFIG_HIGHMEM
		zones_size[ZONE_HIGHMEM] = 0;
#endif
		free_area_init(zones_size);
	}
}

void __init mem_init(void)
{
	int codek = 0, datak = 0, initk = 0;
	unsigned long tmp;
	extern char _etext, _stext, __init_begin, __init_end, _end;
	unsigned long start_mem = memory_start; /* DAVIDM - these must start at end of kernel */
	unsigned long end_mem   = memory_end; /* DAVIDM - this must not include kernel stack at top */

#ifdef DEBUG
	printk(KERN_DEBUG "Mem_init: start=%lx, end=%lx\n", start_mem, end_mem);
#endif

	end_mem &= PAGE_MASK;
	high_memory = __va(end_mem);

	start_mem = PAGE_ALIGN(start_mem);
	max_mapnr = ((unsigned long)end_mem) >> PAGE_SHIFT;
	num_physpages = max_mapnr;
	printk("We have %ld pages of RAM\n", num_physpages);

	/* this will put all memory onto the freelists */
	totalram_pages = free_all_bootmem();

	codek = (&_etext - &_stext) >> 10;
	datak = (&_end - &_etext) >> 10;
	initk = (&__init_begin - &__init_end) >> 10;

	tmp = nr_free_pages() << PAGE_SHIFT;
	printk(KERN_INFO "Memory available: %luk/%luk RAM, %luk/%luk ROM (%dk kernel code, %dk data)\n",
	       tmp >> 10,
	       (&_end - &_stext) >> 10,
	       (rom_length > 0) ? ((rom_length >> 10) - codek) : 0,
	       rom_length >> 10,
	       codek,
	       datak
	       );
}



#ifdef CONFIG_BLK_DEV_INITRD
void __init free_initrd_mem(unsigned long start, unsigned long end)
{
	int pages = 0;
	for (; start < end; start += PAGE_SIZE) {
		ClearPageReserved(virt_to_page(start));
		init_page_count(virt_to_page(start));
		free_page(start);
		totalram_pages++;
		pages++;
	}
	printk (KERN_NOTICE "Freeing initrd memory: %dk freed\n", pages);
}
#endif

void free_initmem(void)
{
#ifdef CONFIG_RAMKERNEL
	unsigned long addr;
	extern char __init_begin, __init_end;
	/*
	 * The following code should be cool even if these sections
	 * are not page aligned.
	 */
	addr = PAGE_ALIGN((unsigned long)(&__init_begin));
	/* next to check that the page we free is not a partial page */
	for (; addr + PAGE_SIZE < (unsigned long)(&__init_end); addr +=PAGE_SIZE) {
		ClearPageReserved(virt_to_page(addr));
		init_page_count(virt_to_page(addr));
		free_page(addr);
		totalram_pages++;
	}
	printk(KERN_NOTICE "Freeing unused kernel memory: %ldk freed (0x%x - 0x%x)\n",
			(addr - PAGE_ALIGN((long) &__init_begin)) >> 10,
			(int)(PAGE_ALIGN((unsigned long)(&__init_begin))),
			(int)(addr - PAGE_SIZE));
#endif
}

void __init fixrange_init(unsigned long start, unsigned long end,
	pgd_t *pgd_base)
{
#if defined(CONFIG_HIGHMEM)
	pgd_t *pgd;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *pte;
	int i, j, k;
	unsigned long vaddr;

	vaddr = start;
	i = __pgd_offset(vaddr);
	j = __pud_offset(vaddr);
	k = __pmd_offset(vaddr);
	pgd = pgd_base + i;

	for ( ; (i < PTRS_PER_PGD) && (vaddr != end); pgd++, i++) {
		pud = (pud_t *)pgd;
		for ( ; (j < PTRS_PER_PUD) && (vaddr != end); pud++, j++) {
			pmd = (pmd_t *)pud;
			for (; (k < PTRS_PER_PMD) && (vaddr != end); pmd++, k++) {
				if (pmd_none(*pmd)) {
					pte = (pte_t *) alloc_bootmem_low_pages(PAGE_SIZE);
					set_pmd(pmd, __pmd((unsigned long)pte));
					if (pte != pte_offset_kernel(pmd, 0))
						BUG();
				}
				vaddr += PMD_SIZE;
			}
			k = 0;
		}
		j = 0;
	}
#endif
}


#define __page_aligned(order) __attribute__((__aligned__(PAGE_SIZE<<order)))
unsigned long pgd_current[NR_CPUS];
pgd_t swapper_pg_dir[PTRS_PER_PGD] __page_aligned(PGD_ORDER);
pte_t invalid_pte_table[PTRS_PER_PTE] __page_aligned(PTE_ORDER);
