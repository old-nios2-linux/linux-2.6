/*
 *  linux/include/asm-lm32/byteorder.h
 *
 * Endian-ness.  In little endian mode, the data bus is connected such
 * that byte accesses appear as:
 *  0 = d0...d7, 1 = d8...d15, 2 = d16...d23, 3 = d24...d31
 * and word accesses (data or instruction) appear as:
 *  d0...d31
 *
 * When in big endian mode, byte accesses appear as:
 *  0 = d24...d31, 1 = d16...d23, 2 = d8...d15, 3 = d0...d7
 * and word accesses (data or instruction) appear as:
 *  d0...d31
 * 
 * modified to support Lattice's Mico32 by Andrea della Porta <sfaragnaus@gmail.com>
 */
#ifndef __ASM_LM32_BYTEORDER_H
#define __ASM_LM32_BYTEORDER_H

#include <linux/compiler.h>
#include <asm/types.h>

static inline __attribute_const__ __u32 ___arch__swab32(__u32 x)
{
	__u32 t;

	if (__builtin_constant_p(x)) {
		t = x ^ ((x << 16) | (x >> 16)); /* eor r1,r0,r0,ror #16 */
	} else {
		/*
		 * The compiler needs a bit of a hint here to always do the
		 * right thing and not screw it up to different degrees
		 * depending on the gcc version.
		 */
		asm ("srui r2, %1, 16\n\t"
		     "sli  r3, %1, 16\n\t"		
		     "or   %0, r2, r0\n\t"
		     "or   %0, r3, r0\n\t"
		     "xor   %0, %0, %1\n\t" 
		     : "=r" (t) : "r" (x)
		    );
	}
	x = (x << 24) | (x >> 8);		/* mov r0,r0,ror #8      */
	t &= ~0x00FF0000;			/* bic r1,r1,#0x00FF0000 */
	x ^= (t >> 8);				/* eor r0,r0,r1,lsr #8   */

	return x;
}

#define __arch__swab32(x) ___arch__swab32(x)

#if !defined(__STRICT_ANSI__) || defined(__KERNEL__)
#  define __BYTEORDER_HAS_U64__
#  define __SWAB_64_THRU_32__
#endif

#include <linux/byteorder/big_endian.h>

#endif

