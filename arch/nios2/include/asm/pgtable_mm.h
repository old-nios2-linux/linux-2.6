/*
 * Copyright (C) 2011 Tobias Klauser <tklauser@distanz.ch>
 * Copyright (C) 2009 Wind River Systems Inc
 *
 * Based on asm/pgtable-32.h from mips which is:
 *
 * Copyright (C) 1994, 95, 96, 97, 98, 99, 2000, 2003 Ralf Baechle
 * Copyright (C) 1999, 2000, 2001 Silicon Graphics, Inc.
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 */

#ifndef _ASM_NIOS2_PGTABLE_H
#define _ASM_NIOS2_PGTABLE_H

#include <asm/io.h>
#include <asm/page.h>
#include <asm/bug.h>
#include <asm/cacheflush.h>
#include <asm/tlbflush.h>

#include <asm/pgtable-bits.h>
#include <asm-generic/pgtable-nopmd.h>

#define FIRST_USER_ADDRESS	0

#define VMALLOC_START		CONFIG_KERNEL_MMU_REGION_BASE
#define VMALLOC_END		(CONFIG_KERNEL_REGION_BASE - 1)

struct mm_struct;

/* Helper macro */
#define MKP(x,w,r) __pgprot(_PAGE_PRESENT | _PAGE_CACHED |	\
			    ((x) ? _PAGE_EXEC : 0) |		\
			    ((r) ? _PAGE_READ : 0) |		\
			    ((w) ? _PAGE_WRITE : 0))
/*
 * These are the macros that generic kernel code needs
 * (to populate protection_map[])
 */

/* Remove W bit on private pages for COW support */
#define __P000	MKP(0,0,0)
#define __P001	MKP(0,0,1)
#define __P010	MKP(0,0,0)	/* COW */
#define __P011	MKP(0,0,1)	/* COW */
#define __P100	MKP(1,0,0)
#define __P101	MKP(1,0,1)
#define __P110	MKP(1,0,0)	/* COW */
#define __P111	MKP(1,0,1)	/* COW */

/* Shared pages can have exact HW mapping */
#define __S000	MKP(0,0,0)
#define __S001	MKP(0,0,1)
#define __S010	MKP(0,1,0)
#define __S011	MKP(0,1,1)
#define __S100	MKP(1,0,0)
#define __S101	MKP(1,0,1)
#define __S110	MKP(1,1,0)
#define __S111	MKP(1,1,1)

/* Used all over the kernel */
#define PAGE_KERNEL __pgprot(_PAGE_PRESENT | _PAGE_CACHED | _PAGE_READ | \
			     _PAGE_WRITE | _PAGE_EXEC | _PAGE_GLOBAL)

#define PAGE_COPY MKP(0,0,1)

#define PGD_ORDER	0
#define PTE_ORDER	0

#define PTRS_PER_PGD	((PAGE_SIZE << PGD_ORDER) / sizeof(pgd_t))
#define PTRS_PER_PTE	((PAGE_SIZE << PTE_ORDER) / sizeof(pte_t))

#define USER_PTRS_PER_PGD	(CONFIG_KERNEL_MMU_REGION_BASE / PGDIR_SIZE)

#define PGDIR_SHIFT	22
#define PGDIR_SIZE	(1UL << PGDIR_SHIFT)
#define PGDIR_MASK	(~(PGDIR_SIZE-1))

#define PTE_FILE_MAX_BITS	28

/*
 * ZERO_PAGE is a global shared page that is always zero: used
 * for zero-mapped memory areas etc..
 */
extern unsigned long empty_zero_page[PAGE_SIZE / sizeof(unsigned long)];
#define ZERO_PAGE(vaddr)	(virt_to_page(empty_zero_page))

extern pgd_t swapper_pg_dir[PTRS_PER_PGD];
extern pte_t invalid_pte_table[PAGE_SIZE/sizeof(pte_t)];

/*
 * (pmds are folded into puds so this doesn't get actually called,
 * but the define is needed for a generic inline function.)
 */
static inline void set_pmd(pmd_t *pmdptr, pmd_t pmdval)
{
	pmdptr->pud.pgd.pgd = pmdval.pud.pgd.pgd;
}

#define pgd_index(address)	(((address) >> PGDIR_SHIFT) \
				 & (PTRS_PER_PGD - 1))

/* to find an entry in a page-table-directory */
#define pgd_offset(mm, addr)	((mm)->pgd + pgd_index(addr))

static inline int pte_write(pte_t pte)		{ return pte_val(pte) & _PAGE_WRITE; }
static inline int pte_dirty(pte_t pte)		{ return pte_val(pte) & _PAGE_DIRTY; }
static inline int pte_young(pte_t pte)		{ return pte_val(pte) & _PAGE_ACCESSED; }
static inline int pte_file(pte_t pte)		{ return pte_val(pte) & _PAGE_FILE; }
static inline int pte_special(pte_t pte)	{ return 0; }

#define pgprot_noncached pgprot_noncached

static inline pgprot_t pgprot_noncached(pgprot_t _prot)
{
	unsigned long prot = pgprot_val(_prot);

	prot &= ~_PAGE_CACHED;

	return __pgprot(prot);
}

#include <linux/swap.h>
swp_entry_t   __pte_to_swp_entry(pte_t pte);
pte_t         __swp_entry_to_pte(swp_entry_t swp);
unsigned long __swp_type(swp_entry_t);
pgoff_t       __swp_offset(swp_entry_t);
swp_entry_t   __swp_entry(unsigned long, pgoff_t offset);

/*
 * FIXME: Today unmapped pages are mapped to the low physical addresses
 * and not 0 (to avoid to trigger the false alias detection in the iss)
 * Also check pte_clear.
 */
static inline int pte_none(pte_t pte)
{
#if 0
	return !(pte_val(pte) & ~_PAGE_GLOBAL);
#else
	return !(pte_val(pte) & ~(_PAGE_GLOBAL|0xf));
#endif
}

static inline int pte_present(pte_t pte)	{ return pte_val(pte) & _PAGE_PRESENT; }

/*
 * The following only work if pte_present() is true.
 * Undefined behaviour if not..
 */
static inline pte_t pte_wrprotect(pte_t pte)
{
	pte_val(pte) &= ~_PAGE_WRITE;
	return pte;
}

static inline pte_t pte_mkclean(pte_t pte)
{
	pte_val(pte) &= ~_PAGE_DIRTY;
	return pte;
}

static inline pte_t pte_mkold(pte_t pte)
{
	pte_val(pte) &= ~_PAGE_ACCESSED;
	return pte;
}

static inline pte_t pte_mkwrite(pte_t pte)
{
	pte_val(pte) |= _PAGE_WRITE;
	return pte;
}

static inline pte_t pte_mkdirty(pte_t pte)
{
	pte_val(pte) |= _PAGE_DIRTY;
	return pte;
}

static inline pte_t pte_mkspecial(pte_t pte)	{ return pte; }

static inline pte_t pte_mkyoung(pte_t pte)
{
	pte_val(pte) |= _PAGE_ACCESSED;
	return pte;
}

static inline pte_t pte_modify(pte_t pte, pgprot_t newprot)
{
	const unsigned long mask = _PAGE_READ | _PAGE_WRITE | _PAGE_EXEC;
	pte_val(pte) = (pte_val(pte) & ~mask) | (pgprot_val(newprot) & mask);
	return pte;
}

static inline int pmd_present(pmd_t pmd)
{
	return (pmd_val(pmd) != (unsigned long) invalid_pte_table)
			&& (pmd_val(pmd) != 0UL);
}

static inline void pmd_clear(pmd_t *pmdp)
{
	pmd_val(*pmdp) = (unsigned long) invalid_pte_table;
}

/*
 * Store a linux PTE into the linux page table.
 */
static inline void set_pte(pte_t *ptep, pte_t pteval)
{
	*ptep = pteval;
}

static inline unsigned long pte_pfn(pte_t pte)
{
	return pte_val(pte) & 0xfffff;
}

#define pte_page(pte)		(pfn_to_page(pte_pfn(pte)))

static inline void set_pte_at(struct mm_struct *mm, unsigned long addr,
			      pte_t *ptep, pte_t pteval)
{
	unsigned long paddr = page_to_virt(pte_page(pteval));
	flush_dcache_range(paddr, paddr + PAGE_SIZE);
	set_pte(ptep, pteval);
}

static inline int pmd_none(pmd_t pmd)
{
	return (pmd_val(pmd) == (unsigned long) invalid_pte_table) || (pmd_val(pmd) == 0UL);
}

#define pmd_bad(pmd)	(pmd_val(pmd) & ~PAGE_MASK)

static inline void pte_clear(struct mm_struct *mm, unsigned long addr, pte_t *ptep)
{
	pte_t null;
	/* FIXME: check FIXME at pte_none */
#if 0
	pte_val(null) = 0;
#else
	pte_val(null) = (addr >> PAGE_SHIFT) & 0xf;
#endif
	set_pte_at(mm, addr, ptep, null);
	flush_tlb_one(addr);
}

/*
 * Conversion functions: convert a page and protection to a page entry,
 * and a page entry and page directory to the page they refer to.
 */
#define mk_pte(page, prot)	(pfn_pte(page_to_pfn(page), prot))

#define pte_unmap(pte)	do { } while (0)

static inline pte_t pgoff_to_pte(pgoff_t off)	{ BUG(); /* FIXME */ }

/*
 * Conversion functions: convert a page and protection to a page entry,
 * and a page entry and page directory to the page they refer to.
 */
#define pmd_phys(pmd)		virt_to_phys((void *)pmd_val(pmd))
#define pmd_page(pmd)		(pfn_to_page(pmd_phys(pmd) >> PAGE_SHIFT))
#define pmd_page_vaddr(pmd)	pmd_val(pmd)

#define pte_offset_map(dir, addr)			\
	((pte_t *) page_address(pmd_page(*dir)) +	\
	 (((addr) >> PAGE_SHIFT) & (PTRS_PER_PTE - 1)))

/* to find an entry in a kernel page-table-directory */
#define pgd_offset_k(addr)	pgd_offset(&init_mm, addr)

/* Get the address to the PTE for a vaddr in specfic directory */
#define pte_offset_kernel(dir, addr)			\
	((pte_t *) pmd_page_vaddr(*(dir)) +		\
	 (((addr) >> PAGE_SHIFT) & (PTRS_PER_PTE - 1)))

#define pte_ERROR(e) \
	printk(KERN_ERR "%s:%d: bad pte %08lx.\n", \
		__FILE__, __LINE__, pte_val(e))
#define pgd_ERROR(e) \
	printk(KERN_ERR "%s:%d: bad pgd %08lx.\n", \
		__FILE__, __LINE__, pgd_val(e))

static inline pte_t pfn_pte(unsigned long pfn, pgprot_t prot)
{
	pte_t pte;
	pte_val(pte) = pfn | pgprot_val(prot);
	return pte;
}

static inline pgoff_t pte_to_pgoff(pte_t pte)	{ BUG(); /* FIXME */ }

#define kern_addr_valid(addr)	BUG()

#define io_remap_pfn_range(vma, vaddr, pfn, size, prot)	\
	remap_pfn_range(vma, vaddr, pfn, size, prot)

#include <asm-generic/pgtable.h>

#define pgtable_cache_init()		do { } while (0)

extern void __init paging_init(void);
extern void __init mmu_init(void);

extern void update_mmu_cache(struct vm_area_struct *vma,
			     unsigned long address, pte_t *pte);

#endif /* _ASM_NIOS2_PGTABLE_H */
