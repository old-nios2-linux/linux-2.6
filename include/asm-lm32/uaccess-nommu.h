/*
 *  linux/include/asm-lm32/uaccess-nommu.h
 *
 *  Lattice's Mico32 port by Andrea della Porta (sfaragnaus@gmail.com)
 *  based upon the ARM port from:
 *  Copyright (C) 2003 Hyok S. Choi, Samsung Electronics Co.,Ltd.

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef _ASMLM32_UACCESS_NOMMU_H
#define _ASMLM32_UACCESS_NOMMU_H

/*
 * Note that this is actually 0x1,0000,0000
 */
#define KERNEL_DS			0x00000000
/* uClinux has only one addr space. */
#define USER_DS				KERNEL_DS

#define get_ds()			(KERNEL_DS)
#define get_fs()			(USER_DS)

static inline void set_fs (mm_segment_t fs)
{ /* nothing to do here for uClinux */
}

/* segment always equal. */
#define segment_eq(a,b)			1

/*
 * assuming __range_ok & __addr_ok always succeed.
 */
#define __addr_ok(addr) 		1
#define __range_ok(addr,size) 		0

#define access_ok(type,addr,size)	1

/*
 * These are the main single-value transfer routines.  They automatically
 * use the right size if we just have the right pointer type.
 */

#define get_user(x, p)						\
	({							\
		int __e = 0;					\
		unsigned long __gu_val;				\
		switch (sizeof(*(p))) {				\
		case 1:						\
			__get_user_asm_byte(__gu_val, p, __e);	\
			break;					\
		case 2:						\
			__get_user_asm_half(__gu_val, p, __e);	\
			break;					\
		case 4:						\
			__get_user_asm_word(__gu_val, p, __e);	\
			break;					\
		default: __e = __get_user_bad(); break;		\
		}						\
		(x) = (typeof(*(p)))__gu_val;			\
		__e;						\
	})

#define __get_user(x, ptr)		get_user(x, ptr)
#define __get_user_error(x,ptr,err)	get_user(x, ptr)

#define __get_user_asm_byte(x,addr,err)				\
	__asm__ __volatile__(					\
	"mv	r1,%1\n\t"					\
	"lbu	%0,(r1+0)\n"					\
	: "=&r" (x)						\
	: "r" (addr)						\
	: "r1")

/*#ifndef __ARMEB__
#define __get_user_asm_half(x,__gu_addr,err)			\
({								\
	unsigned long __b1, __b2;				\
	__get_user_asm_byte(__b1, __gu_addr, err);		\
	__get_user_asm_byte(__b2, __gu_addr + 1, err);		\
	(x) = __b1 | (__b2 << 8);				\
})
#else*/		
//Mico is definitely Big-endian,so we do not bother with mixed/little endianness
//we could also have used 'lhu' asm instruction to optimize away, but this way we can assure no undefined-behaviour misaligned access, good ARM boys!
#define __get_user_asm_half(x,__gu_addr,err)			\
({								\
	unsigned long __b1, __b2;				\
	__get_user_asm_byte(__b1, __gu_addr, err);		\
	__get_user_asm_byte(__b2, __gu_addr + 1, err);		\
	(x) = (__b1 << 8) | __b2;				\
})
//#endif

//we have just speak 'bout avoiding misaligned address... ok, here we are again on good old asm instructions!
#define __get_user_asm_word(x,addr,err)				\
	__asm__ __volatile__(					\
	"mv	r1,%1\n\t"					\
	"lw	%0,(r1+0)\n"					\
	: "=&r" (x)						\
	: "r" (addr)						\
	: "r1")

extern int __get_user_bad(void);


#define put_user(x, p)						\
	({							\
		int __e = 0;					\
		typeof(*(p)) __pu_val = (x);			\
		typeof(*(p)) __user *__p = (p);			\
		switch (sizeof(*(p))) {				\
		case 1:						\
			__put_user_asm_byte(__pu_val, __p, __e);\
			break;					\
		case 2:						\
			__put_user_asm_half(__pu_val, __p, __e);\
			break;					\
		case 4:						\
			__put_user_asm_word(__pu_val, __p, __e);\
			break;					\
		case 8:						\
			__put_user_asm_dword(__pu_val, __p, __e);\
			break;					\
		default: __e = __put_user_bad(); break;		\
		}						\
		__e;						\
	})

#define __put_user(x, ptr)		put_user(x, ptr)
#define __put_user_error(x,ptr,err)	put_user(x, ptr)

#define __put_user_asm_byte(x,__pu_addr,err) 			\
	__asm__ __volatile__(					\
	"mv	r1,%1\n\t"					\
	"sb	(r1+0),%0\n"					\
	: /* none */						\
	: "r" (x), "r" (__pu_addr)				\
	: "r1")


/*#ifndef __ARMEB__
#define __put_user_asm_half(x,__pu_addr,err)			\
({								\
	unsigned long __temp = (unsigned long)(x);		\
	__put_user_asm_byte(__temp, __pu_addr, err);		\
	__put_user_asm_byte(__temp >> 8, __pu_addr + 1, err);	\
})
#else*/
#define __put_user_asm_half(x,__pu_addr,err)			\
({								\
	unsigned long __temp = (unsigned long)(x);		\
	__put_user_asm_byte(__temp >> 8, __pu_addr, err);	\
	__put_user_asm_byte(__temp, __pu_addr + 1, err);	\
})
//#endif

#define __put_user_asm_word(x,__pu_addr,err)			\
	__asm__ __volatile__(					\
	"mv	r1,%1\n\t"					\
	"sw	(r1+0),%0\n"					\
	: /* none */						\
	: "r" (x), "r" (__pu_addr)				\
	: "r1")

/*#ifndef __ARMEB__
#define	__reg_oper0	"%R1"
#define	__reg_oper1	"%Q1"
#else
#define	__reg_oper0	"%Q1"
#define	__reg_oper1	"%R1"
#endif*/

#define __put_user_asm_dword(x,__pu_addr,err)			\
({								\
 	unsigned long long __temp = (unsigned long long)(x);	\
	__put_user_asm_word(__temp >> 32,__pu_addr,err);		\
	__put_user_asm_word(__temp,__pu_addr,err);		\
})

extern int __put_user_bad(void);

#define copy_from_user(to, from, n)		(memcpy(to, from, n), 0)
#define copy_to_user(to, from, n)		(memcpy(to, from, n), 0)
#define clear_user(to, n)			(memset(to, 0, n), 0)

#define __copy_from_user(to, from, n)		copy_from_user(to, from, n)
#define __copy_to_user(to, from, n)		copy_to_user(to, from, n)
#define __clear_user(ptr, n)			clear_user(ptr, n)

#define __copy_to_user_inatomic			__copy_to_user
#define __copy_from_user_inatomic		__copy_from_user

/* these are just for symbol compatibility */
static inline unsigned long __arch_copy_from_user (void *to, void *from, unsigned long n)
{
	return copy_from_user(to, from, n);
}

static inline unsigned long __arch_copy_to_user (void *to, void *from, unsigned long n)
{
	return copy_to_user(to, from, n);
}

static inline unsigned long __arch_clear_user(void *to, unsigned long n)
{
	return clear_user(to, n);
}

#endif /* _ASMLM32_UACCESS-NOMMU_H */
