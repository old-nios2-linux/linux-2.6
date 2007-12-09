/*
 *  linux/arch/lm32/mm/cache-lm32.S
 *
 * Modified by Andrea della Porta <sfaragnaus@gmail.com>
 * based on the ARM port from Russel King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/linkage.h>
#include <linux/init.h>
#include <asm/hardware.h>
#include <asm/page.h>
#include "proc-macros.S"

/*
 *	flush_user_cache_all()
 *
 *	Invalidate all cache entries in a particular address
 *	space.
 *
 *	- mm	- mm_struct describing address space
 */
ENTRY(lm32_flush_user_cache_all)
	/* FALLTHROUGH */
/*
 *	flush_kern_cache_all()
 *
 *	Clean and invalidate the entire cache.
 */
ENTRY(lm32_flush_kern_cache_all)
	/* FALLTHROUGH */

/*
 *	flush_user_cache_range(start, end, flags)
 *
 *	Invalidate a range of cache entries in the specified
 *	address space.
 *
 *	- start - start address (may not be aligned)
 *	- end	- end address (exclusive, may not be aligned)
 *	- flags	- vma_area_struct flags describing address space
 */
ENTRY(lm32_flush_user_cache_range)
	wcsr ICC, r0		/* invalidate instruction cache */
	nop
	nop
	nop
	nop
	wcsr DCC, r0		/* invalidate data cache */
	nop
	ret

/*
 *	coherent_kern_range(start, end)
 *
 *	Ensure coherency between the Icache and the Dcache in the
 *	region described by start.  If you have non-snooping
 *	Harvard caches, you need to implement this function.
 *
 *	- start  - virtual start address
 *	- end	 - virtual end address
 */
ENTRY(lm32_coherent_kern_range)
	/* FALLTHROUGH */

/*
 *	coherent_user_range(start, end)
 *
 *	Ensure coherency between the Icache and the Dcache in the
 *	region described by start.  If you have non-snooping
 *	Harvard caches, you need to implement this function.
 *
 *	- start  - virtual start address
 *	- end	 - virtual end address
 */
ENTRY(lm32_coherent_user_range)
	wcsr ICC, r0		/* invalidate instruction cache */
	nop
	nop
	nop
	nop
	wcsr DCC, r0		/* invalidate data cache */
	nop
	ret

/*
 *	flush_kern_dcache_page(void *page)
 *
 *	Ensure no D cache aliasing occurs, either with itself or
 *	the I cache
 *
 *	- addr	- page aligned address
 */
ENTRY(lm32_flush_kern_dcache_page)
	wcsr ICC, r0		/* @invalidate instruction cache */
	nop
	nop
	nop
	nop
	ret

/*
 *	dma_inv_range(start, end)
 *
 *	Invalidate (discard) the specified virtual address range.
 *	May not write back any entries.  If 'start' or 'end'
 *	are not cache line aligned, those lines must be written
 *	back.
 *
 *	- start  - virtual start address
 *	- end	 - virtual end address
 */
ENTRY(lm32_dma_inv_range)
	wcsr DCC, r0		/* invalidate data cache */
	nop
	ret

/*
 *	dma_flush_range(start, end)
 *
 *	Clean and invalidate the specified virtual address range.
 *
 *	- start  - virtual start address
 *	- end	 - virtual end address
 */
ENTRY(lm32_dma_flush_range)
	/* FALLTHROUGH */

/*
 *	dma_clean_range(start, end)
 *
 *	Clean (write back) the specified virtual address range.
 *
 *	- start  - virtual start address
 *	- end	 - virtual end address
 */
ENTRY(lm32_dma_clean_range)
	ret

	__INITDATA
/*
	.type	lm32_cache_fns, %object
ENTRY(lm32_cache_fns)
	.long	lm32_flush_kern_cache_all
	.long	lm32_flush_user_cache_all
	.long	lm32_flush_user_cache_range
	.long	lm32_coherent_kern_range
	.long	lm32_coherent_user_range
	.long	lm32_flush_kern_dcache_page
	.long	lm32_dma_inv_range
	.long	lm32_dma_clean_range
	.long	lm32_dma_flush_range
	.size	lm32_cache_fns, . - lm32_cache_fns
*/
