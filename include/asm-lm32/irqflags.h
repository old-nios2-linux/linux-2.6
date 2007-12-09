#ifndef __ASM_LM32_IRQFLAGS_H
#define __ASM_LM32_IRQFLAGS_H

#ifdef __KERNEL__

#include <asm/ptrace.h>

/*
 * CPU interrupt mask handling.
 */

/*
 * Save the current interrupt enable state & disable IRQs
 */
#define raw_local_irq_save(x)					\
	({							\
		unsigned long temp;				\
		(void) (&temp == &x);				\
	__asm__ __volatile__(					\
	"rcsr	%0, IE		# raw_local_irq_save\n\t"		\
	"xnori  r2, r0, 1\n\t"					\
	"and	%1, %0, r2\n\t"					\
	"wcsr	IE, %1"						\
	: "=r" (x), "=r" (temp)					\
	:							\
	: "memory", "r2");				\
	})
	
/*
 * Enable IRQs
 */
#define raw_local_irq_enable()					\
	({							\
		unsigned long temp;				\
	__asm__ __volatile__(					\
	"rcsr	%0, IE		# raw_local_irq_save\n\t"		\
	"ori	%0, %0, 1\n\t"					\
	"wcsr	IE, %0"						\
	: "=r" (temp)						\
	:							\
	: "memory");					\
	})

/*
 * Disable IRQs
 */
#define raw_local_irq_disable()					\
	({							\
		unsigned long temp;				\
	__asm__ __volatile__(					\
	"rcsr	%0, IE		# raw_local_irq_save\n\t"		\
	"xnori  r2, r0, 1\n\t"					\
	"and	%0, %0, r2\n\t"					\
	"wcsr	IE, %0"						\
	: "=r" (temp)						\
	:							\
	: "memory", "r2");				\
	})


/*
 * Save the current interrupt enable state.
 */
#define raw_local_save_flags(x)					\
	({							\
	__asm__ __volatile__(					\
	"rcsr	%0, IE		# raw_local_irq_save"		\
	: "=r" (x) : : "memory");				\
	})

/*
 * restore saved IRQ & FIQ state
 */
#define raw_local_irq_restore(x)					\
	__asm__ __volatile__(					\
	"wcsr	IE, %0	     	# raw_local_irq_restore"	\
	:							\
	: "r" (x)						\
	: "memory")

/*#define irqs_disabled()			\
({					\
	unsigned long flags;		\
	raw_local_save_flags(flags);	\
	(int)(flags & PSR_I_BIT);	\
})*/
#define raw_irqs_disabled_flags(flags)  \
	({                                      \
	         (int)((flags) & PSR_I_BIT);     \
	 })

#endif
#endif
