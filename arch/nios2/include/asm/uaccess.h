/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2009, Wind River Systems Inc
 * Implemented by fredrik.markstrom@gmail.com and ivarholmqvist@gmail.com
 */
#ifndef _ASM_NIOS2_UACCESS_H
#define _ASM_NIOS2_UACCESS_H

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/thread_info.h>

/* FIXME: Add comment
 */
extern int fixup_exception(struct pt_regs *regs, unsigned long address);
struct exception_table_entry
{
	unsigned long insn;
	unsigned long nextinsn;
};


/* Segment stuff
 */
#define USER_DS   (mm_segment_t){ 0x80000000UL }
#define KERNEL_DS (mm_segment_t){ 0 }
extern int segment_eq(mm_segment_t a, mm_segment_t b);
extern mm_segment_t get_ds(void);
extern void set_fs(mm_segment_t seg);
extern mm_segment_t get_fs(void);


/* Misc user access functions (implemented in uaccess.c)
 * FIXME: copy_from_user, __copy_from_user_inatomic, copy_to_user and 
 * __copy_to_user_inatomic can and should be inlined.
 */
extern long copy_from_user(void* to, const void __user *from, int n);
extern long __copy_from_user(void* to, const void __user *from, int n);
extern long __copy_from_user_inatomic(void* to, const void __user *from, int n);

extern long copy_to_user(void __user *to, const void *from, long n);
extern long __copy_to_user(void __user *to, const void *from, long n);
extern long __copy_to_user_inatomic(void __user *to, const void *from, long n);

#define VERIFY_READ    0
#define VERIFY_WRITE   1

/* These functions are implemented in uaccess.c
 */
extern long strncpy_from_user(char *__to, const char __user *__from, 
                              long __len);
extern long strnlen_user(const char __user *s, long n);

__kernel_size_t __clear_user(void __user *addr, __kernel_size_t size);
size_t clear_user(void __user *addr, __kernel_size_t size);

/* Optimized macros
 */
#define access_ok(type,from,len)                                        \
   (((signed long)(((long)get_fs().seg) &                               \
                   ((long)(from) | (((long)from) + (len)) | len))) == 0)

#define __get_user_asm(val, insn, addr, err)                         \
   {                                                                 \
                                                                     \
      __asm__ __volatile__(                                          \
        "       movi    %0, %3                                  \n"  \
        "1:   " insn " %1, 0(%2)                                \n"  \
        "       movi     %0, 0                                  \n"  \
        "2:                                                     \n"  \
        "       .section __ex_table,\"a\"                       \n"  \
        "       .word 1b, 2b                                    \n"  \
        "       .previous                                       "    \
        : "=&r" (err), "=r" (val)                                    \
        : "r" (addr), "i" (-EFAULT));                                \
   }

#define __get_user_unknown(val, size, ptr, err) do {           \
      err = 0;                                                 \
      if(copy_from_user(&(val), ptr, size)) {                  \
         err = -EFAULT;                                        \
      }                                                        \
   } while(0)

#define __get_user_common(val, size, ptr, err)                     \
   do {                                                            \
      switch (size) {                                              \
         case 1: __get_user_asm(val, "ldbu", ptr, err); break;     \
         case 2: __get_user_asm(val, "ldhu", ptr, err); break;     \
         case 4: __get_user_asm(val, "ldw", ptr, err); break;      \
         default: __get_user_unknown(val, size, ptr, err); break;  \
      }                                                            \
   } while (0)


#define __get_user(x,ptr)                                               \
   ({                                                                   \
      long __gu_err = -EFAULT;                                          \
      const __typeof__(*(ptr)) __user * __gu_ptr = (ptr);               \
      unsigned long __gu_val; /* FIXME: should be __typeof__ */         \
      __get_user_common(__gu_val, sizeof(*(ptr)), __gu_ptr, __gu_err);  \
      (x) = (__typeof__(x))__gu_val;                                    \
      __gu_err;                                                         \
   })

#define get_user(x,ptr)                                                 \
   ({                                                                   \
      long __gu_err = -EFAULT;                                          \
      const __typeof__(*(ptr)) __user * __gu_ptr = (ptr);               \
      unsigned long __gu_val = 0;                                       \
      if (likely(access_ok(VERIFY_READ,  __gu_ptr, sizeof(*__gu_ptr)))) { \
         __get_user_common(__gu_val, sizeof(*__gu_ptr), __gu_ptr, __gu_err); \
      }                                                                 \
      (x) = (__typeof__(x))__gu_val;                                    \
      __gu_err;                                                         \
   })

#define __put_user_asm(val, insn, ptr,err)                           \
   {                                                                 \
      __asm__ __volatile__(                                          \
        "       movi    %0, %3                                  \n"  \
        "1:   " insn " %1, 0(%2)                                \n"  \
        "       movi     %0, 0                                  \n"  \
        "2:                                                     \n"  \
        "       .section __ex_table,\"a\"                       \n"  \
        "       .word 1b, 2b                                    \n"  \
        "       .previous                                       \n"  \
        : "=&r" (err)                                                \
        : "r" (val), "r" (ptr), "i" (-EFAULT));                      \
   }

#define __put_user_unknown(val, size, ptr, err) do {           \
      err = 0;                                                 \
      if(copy_to_user(ptr, &(val), size)) {                    \
         err = -EFAULT;                                        \
      }                                                        \
   } while(0)

#define __put_user_common(val, size, ptr, err)                    \
   do {                                                           \
      switch (size) {                                             \
         case 1: __put_user_asm(val, "stb", ptr, err); break;     \
         case 2: __put_user_asm(val, "sth", ptr, err); break;     \
         case 4: __put_user_asm(val, "stw", ptr, err); break;     \
         default: __put_user_unknown(val, size, ptr, err); break; \
      }                                                           \
   } while(0)

#define __put_user(x, ptr)                                              \
   ({                                                                   \
      long __pu_err = -EFAULT;                                          \
      __typeof__(*(ptr)) __user * __pu_ptr = (ptr);                     \
      __typeof__(*(ptr)) __pu_val = (__typeof(*ptr))(x);                \
      if(likely(access_ok(VERIFY_WRITE, __pu_ptr, sizeof(*__pu_ptr))))  \
         __put_user_common(__pu_val, sizeof(*__pu_ptr), __pu_ptr, __pu_err); \
      __pu_err;                                                         \
   })

#define put_user(x, ptr)                                                \
   ({                                                                   \
      long __pu_err = -EFAULT;                                          \
      __typeof__(*(ptr)) __user * __pu_ptr = (ptr);                     \
      __typeof__(*(ptr)) __pu_val = (__typeof(*ptr))(x);                \
      if(likely(access_ok(VERIFY_WRITE, __pu_ptr, sizeof(*__pu_ptr))))  \
         __put_user_common(__pu_val, sizeof(*__pu_ptr), __pu_ptr, __pu_err); \
      __pu_err;                                                         \
   })

#endif /* _ASM_NIOS2_UACCESS_H */
