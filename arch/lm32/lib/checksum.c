/*--------------------------------------------------------------------
 *
 * Derived from various works, Alpha, ix86, M68K, Sparc, ...et al
 *
 * Copyright (C) 2004   Microtronix Datacom Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *
 * Jan/20/2004		dgt	    NiosII
 *
 ---------------------------------------------------------------------*/

#include <net/checksum.h>
#include <asm/checksum.h>

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */



extern inline unsigned long do_csum(const unsigned char * buff, int len)
{
 int odd, count;
 unsigned long result = 0;

    barrier();
 if (len <= 0)
 	goto out;
 odd = 1 & (unsigned long) buff;
 if (odd) {
////result = *buff;                     // dgt: Big    endian
 	result = *buff << 8;                // dgt: Little endian

 	len--;
 	buff++;
 }
 count = len >> 1;		/* nr of 16-bit words.. */
 if (count) {
 	if (2 & (unsigned long) buff) {
 		result += *(unsigned short *) buff;
 		count--;
 		len -= 2;
 		buff += 2;
 	}
 	count >>= 1;		/* nr of 32-bit words.. */
 	if (count) {
 	        unsigned long carry = 0;
 		do {
 			unsigned long w = *(unsigned long *) buff;
 			count--;
 			buff += 4;
 			result += carry;
 			result += w;
 			carry = (w > result);
 		} while (count);
 		result += carry;
 		result = (result & 0xffff) + (result >> 16);
 	}
 	if (len & 2) {
 		result += *(unsigned short *) buff;
 		buff += 2;
 	}
 }
 if (len & 1)
 	result += *buff;  /* This is little machine, byte is right */
 result = from32to16(result);
 if (odd)
 	result = ((result >> 8) & 0xff) | ((result & 0xff) << 8);
out:
	return result;
    barrier();
  }



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

__wsum csum_partial(const void *buff, int len, __wsum sum)
{
#if 0
	__asm__ __volatile__ ...//;dgt2;tmp;not (yet) available...
                         ...//;dgt2;tmp;NiosI didn't offer either...
#else
	unsigned int result = do_csum(buff, len);

	/* add in old sum, and carry.. */
	result += sum;
	if (sum > result)
		result += 1;
	return result;
#endif
}


/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


/*
 * the same as csum_partial, but copies from fs:src while it
 * checksums
 *
 * here even more important to align src and dst on a 32-bit (or even
 * better 64-bit) boundary
 */

__wsum csum_partial_copy(const void *src, void *dst, int len, __wsum sum)
{
	memcpy(dst, src, len);
	return csum_partial(dst, len, sum);

}
