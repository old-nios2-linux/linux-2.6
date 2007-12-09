/*
 *  linux/include/asm-lm32/checksum.h
 *
 * IP checksum routines
 *
 * Copyright (C) Original authors of ../asm-i386/checksum.h
 * Copyright (C) 1996-1999 Russell King
 * Copyright (C) 2007 Andrea della Porta (sfaragnaus@gmail.com)
 */
#ifndef __ASM_LM32_CHECKSUM_H
#define __ASM_LM32_CHECKSUM_H

#include <linux/in6.h>

/*
 * computes the checksum of a memory block at buff, length len,
 * and adds in "sum" (32-bit)
 *
 * returns a 32-bit number suitable for feeding into itself
 * or csum_tcpudp_magic
 *
 * this function must be called with even lengths, except
 * for the last fragment, which may be odd
 *
 * it's best to have buff aligned on a 32-bit boundary
 */
__wsum csum_partial(const void *buff, int len, __wsum sum);

/*
 * the same as csum_partial, but copies from src while it
 * checksums, and handles user-space pointer exceptions correctly, when needed.
 *
 * here even more important to align src and dst on a 32-bit (or even
 * better 64-bit) boundary
 */

__wsum
csum_partial_copy_nocheck(const void *src, void *dst, int len, __wsum sum);

__wsum
csum_partial_copy_from_user(const void __user *src, void *dst, int len, __wsum sum, int *err_ptr);

/*
 *     Fold a partial checksum without adding pseudo headers
 */
#if 0
static inline __sum16 csum_fold(__wsum sum)
{
       __asm__(
       /*"add    %0, %1, %1, ror #16     @ csum_fold"*/
       "sli	r4, %1, 16	/* csum_fold */\n\t"
       "srui	r3, %1, 16	\n\t"
       "or	%0, r4, r3	\n\t"
       : "=r" (sum)
       : "r" (sum)
       : "r3", "r4");
       return (__force __sum16)(~(__force u32)sum >> 16);
}
#endif

/*
 *	This is a version of ip_compute_csum() optimized for IP headers,
 *	which always checksum on 4 octet boundaries. On Mico32 we do not have carry flag so we must add 16 bits at a time.
 */
static inline __sum16
ip_fast_csum(const void *iph, unsigned int ihl)
{
        unsigned int tmp1;
        __wsum sum;

	__asm__ __volatile__(
	"\n\
	mv	r1, %1			/* ip_fast_csum */	\n\
	add	r5, r1, %2					\n\
1:	lw	r2, (r1+0)		/* next iph word */	\n\
	srui	r3, r2, 16		/* iph high 16 bits */	\n\
	andi	r4, r2, 0xFFFF		/* iph low 16 bits */	\n\
	add	%0, %0, r3		/* perform add */	\n\
	add	%0, %0, r4		/* perform add */	\n\
	addi	r1, r1, 1					\n\
	bg	r5, r1, 1b					\n\
2:	srui	r2, %0, 16					\n\
	be	r2, r0, 3f					\n\
	add	%0, %0, r2					\n\
	bi	2b						\n\
3:	not	%0, %0"
	: "=r" (sum)	//, "=r" (iph), "=r" (ihl)
	: "r" (iph), "r" (ihl)
	: "memory", "r1", "r2", "r3", "r4", "r5");
	return sum;
}

static inline unsigned short from32to16(unsigned long x)
{
        /* add up 16-bit and 16-bit for 16+c bit */
        x = (x & 0xffff) + (x >> 16);
        /* add up carry.. */
        x = (x & 0xffff) + (x >> 16);
        return x;
}

/*
 * 	Fold a partial checksum without adding pseudo headers
 */
static inline unsigned int
csum_fold(unsigned int sum)
{
	__asm__(
	"\n\
	mv	r2, %1			/* csum_fold */		\n\
        srui    r3, r2, 16              /* iph high 16 bits */  \n\
        andi    r4, r2, 0xFFFF          /* iph low 16 bits */   \n\
	add	%0, %0, r3					\n\
	add	%0, %0, r4					\n\
2:	srui	r2, %0, 16					\n\
	be	r2, r0, 3f					\n\
	add	%0, %0, r2					\n\
	bi	2b						\n\
3:	not	%0, %0"
	: "=r" (sum)
	: "r" (sum)
	: "r2", "r3", "r4" );
	return sum;
}

static inline __wsum
csum_tcpudp_nofold(__be32 saddr, __be32 daddr, unsigned short len,
		  unsigned short proto, __wsum sum)   
{
	/*__asm__(
	"adds	%0, %1, %2		@csum_tcpudp_nofold 	\n\
	adcs	%0, %0, %3					\n\
	adcs	%0, %0, %4					\n\
	adcs	%0, %0, %5					\n\
	adc	%0, %0, #0"
	: "=&r"(sum)
	: "r" (sum), "r" (daddr), "r" (saddr), "r" (ntohs(len)), "Ir" (ntohs(proto))
	: "cc");
	return sum;*/

#define CHECK_CARRY(s)	 	s += (s >= ((unsigned long long)1<<32))?1:0
	unsigned long long s = sum + saddr;
	CHECK_CARRY(s);
	s += daddr;
	CHECK_CARRY(s);
	s += saddr;
	CHECK_CARRY(s);
	s += ntohs(len);
	CHECK_CARRY(s);
	s += ntohs(proto);
	CHECK_CARRY(s);

	return ((unsigned int)s);
}	
/*
 * computes the checksum of the TCP/UDP pseudo-header
 * returns a 16-bit checksum, already complemented
 */
static inline __sum16
csum_tcpudp_magic(__be32 saddr, __be32 daddr, unsigned short len,
                 unsigned short proto, __wsum sum)
{
	__asm__(
	"\n\
	mv	r2, %1			/* csum_tcpudp_magic */	\n\
        srui    r3, r2, 16              /* iph high 16 bits */  \n\
        andi    r4, r2, 0xFFFF          /* iph low 16 bits */   \n\
	add	%0, %0, r3					\n\
	add	%0, %0, r4					\n\
	mv	r2, %2			/* daddr */		\n\
        srui    r3, r2, 16              /* iph high 16 bits */  \n\
        andi    r4, r2, 0xFFFF          /* iph low 16 bits */   \n\
	add	%0, %0, r3					\n\
	add	%0, %0, r4					\n\
	mv	r2, %3			/* saddr */		\n\
        srui    r3, r2, 16              /* iph high 16 bits */  \n\
        andi    r4, r2, 0xFFFF          /* iph low 16 bits */   \n\
	add	%0, %0, r3					\n\
	add	%0, %0, r4					\n\
	add	%0, %0, %4		/* len */		\n\
	add	%0, %0, %5		/* proto */		\n\
2:	srui	r2, %0, 16					\n\
	be	r2, r0, 3f					\n\
	add	%0, %0, r2					\n\
	bi	2b						\n\
3:	not	%0, %0"
	: "=&r"(sum)
	: "r" (sum), "r" (daddr), "r" (saddr), "r" (ntohs(len)), "Ir" (ntohs(proto))
	: "r2", "r3", "r4");
	return sum;
}


/*
 * this routine is used for miscellaneous IP-like checksums, mainly
 * in icmp.c
 */
static inline __sum16
ip_compute_csum(const void *buff, int len)
{
	return csum_fold(csum_partial(buff, len, 0));
}

#define _HAVE_ARCH_IPV6_CSUM
extern __wsum
__csum_ipv6_magic(const struct in6_addr *saddr, const struct in6_addr *daddr, __be32 len,
               __be32 proto, __wsum sum);

static inline __sum16
csum_ipv6_magic(const struct in6_addr *saddr, const struct in6_addr *daddr, __u32 len,
               unsigned short proto, __wsum sum)
{
	return csum_fold(__csum_ipv6_magic(saddr, daddr, htonl(len),
					   htonl(proto), sum));
}
#endif
