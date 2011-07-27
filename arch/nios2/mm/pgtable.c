/*
 * Copyright (C) 2009 Wind River Systems Inc
 *   Implemented by fredrik.markstrom@gmail.com and ivarholmqvist@gmail.com
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 */

#include <linux/mm.h>

#include <asm/pgtable.h>
#include <asm/cpuinfo.h>

/* pteaddr: 
 *   ptbase | vpn* | zero
 *   31-22  | 21-2 | 1-0
 *   
 *   *vpn is preserved on double fault
 *   
 * tlbacc:
 *   IG   |*flags| pfn
 *   31-25|24-20 | 19-0
 *   
 *   *crwxg
 *   
 * tlbmisc:
 *   resv  |way   |rd | we|pid |dbl|bad|perm|d
 *   31-24 |23-20 |19 | 20|17-4|3  |2  |1   |0
 *  
 */

/*
 * Initialize a new pgd / pmd table with invalid pointers.
 */
static void pgd_init(unsigned long page)
{
	unsigned long *p = (unsigned long *) page;
	int i;

	for (i = 0; i < USER_PTRS_PER_PGD; i += 8) {
		p[i + 0] = (unsigned long) invalid_pte_table;
		p[i + 1] = (unsigned long) invalid_pte_table;
		p[i + 2] = (unsigned long) invalid_pte_table;
		p[i + 3] = (unsigned long) invalid_pte_table;
		p[i + 4] = (unsigned long) invalid_pte_table;
		p[i + 5] = (unsigned long) invalid_pte_table;
		p[i + 6] = (unsigned long) invalid_pte_table;
		p[i + 7] = (unsigned long) invalid_pte_table;
	}
}

pgd_t *pgd_alloc(struct mm_struct *mm)
{
	pgd_t *ret, *init;
	ret = (pgd_t *) __get_free_pages(GFP_KERNEL, PGD_ORDER);
	if (ret) {
		init = pgd_offset(&init_mm, 0UL);
		pgd_init((unsigned long) ret);
		memcpy(ret + USER_PTRS_PER_PGD, init + USER_PTRS_PER_PGD,
		       (PTRS_PER_PGD - USER_PTRS_PER_PGD) * sizeof(pgd_t));
	}

	return ret;
}

void __init pagetable_init(void)
{
	/* Initialize the entire pgd.  */
	pgd_init((unsigned long)swapper_pg_dir);
	pgd_init((unsigned long)swapper_pg_dir
		 + sizeof(pgd_t) * USER_PTRS_PER_PGD);
}

/* FIXME: Swap not implemented */
swp_entry_t   __pte_to_swp_entry(pte_t pte){BUG();}
pte_t         __swp_entry_to_pte(swp_entry_t swp){BUG();}
unsigned long __swp_type(swp_entry_t swp){BUG();}
pgoff_t       __swp_offset(swp_entry_t swp){BUG();}
swp_entry_t   __swp_entry(unsigned long type, pgoff_t offset){BUG();}
