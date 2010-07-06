/*
 * Copyright (C) 2010 Tobias Klauser <tklauser@distanz.ch>
 * Copyright (C) 2004 Microtronix Datacom Ltd.
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License. See the file "COPYING" in the main directory of this archive
 * for more details.
 */

#ifndef _ASM_NIOS_CHECKSUM_H
#define _ASM_NIOS_CHECKSUM_H

/*
 * computes the checksum of the TCP/UDP pseudo-header
 * returns a 16-bit checksum, already complemented
 */
#define csum_tcpudp_nofold __csum_tcpudp_nofold
static inline __wsum
__csum_tcpudp_nofold(__be32 saddr, __be32 daddr, unsigned short len,
		unsigned short proto, __wsum sum)
{
	__asm__ __volatile__(
		"add	%0, %1, %0\n"
		"cmpltu	r8, %0, %1\n"
		"add	%0, %0, r8\n"	/* add carry */
		"add	%0, %2, %0\n"
		"cmpltu	r8, %0, %2\n"
		"add	%0, %0, r8\n"	/* add carry */
		"add	%0, %3, %0\n"
		"cmpltu	r8, %0, %3\n"
		"add	%0, %0, r8\n"	/* add carry */
		: "=r" (sum), "=r" (saddr)
		: "r" (daddr), "r" ((ntohs(len)<<16) + (proto*256)),
		  "0" (sum),
		  "1" (saddr)
		: "r8");

	return sum;
}

#include <asm-generic/checksum.h>

#endif /* _ASM_NIOS_CHECKSUM_H */
