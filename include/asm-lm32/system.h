#ifndef __ASM_LM32_SYSTEM_H
#define __ASM_LM32_SYSTEM_H

#ifdef __KERNEL__

#include <asm/memory.h>

#define CPU_ARCH_LM32		0

//Mico 32 special control register
#define IE_IE	(1 << 0)	/* Interrupt enable			*/
#define IE_EIE	(1 << 1)	/* Saved IE state on normal exception	*/
#define IE_BIE	(1 << 2)	/* Saved IE state on debug exception	*/

#define CFG_M	(1 << 0)	/* Multiplier			*/
#define CFG_D  	(1 << 1)	/* Divider 			*/
#define CFG_S	(1 << 2)	/* Barrel shifter 		*/
#define CFG_U	(1 << 3)	/* Reserved			*/
#define CFG_X	(1 << 4)	/* Sign extend 			*/
#define CFG_CC	(1 << 5)	/* Cycle counter		*/
#define CFG_IC	(1 << 6)	/* I cache			*/
#define CFG_DC	(1 << 7)	/* D cache			*/
#define CFG_G	(1 << 8)	/* Software Debug		*/
#define CFG_H	(1 << 9)	/* Hardware Debug		*/
#define CFG_R	(1 << 10)	/* ROM-based debug support	*/
#define CFG_J	(1 << 11)	/* JTAG				*/
#define CFG_INT(x)	((x >> 12) & 0x3F)	/* How many ext interrupts	*/
#define CFG_BP(x)	((x >> 18) & 0xF)	/* How many breakpoints		*/
#define CFG_WP(x)	((x >> 22) & 0x7)	/* How many watchpoints		*/
#define CFG_REV(x)	((x >> 26) & 0x3F)	/* Revision			*/


#define read_cpuid(/*reg*/)						\
	({								\
		unsigned int __val;					\
		asm("rcsr %0, CFG\n\t"					\
		    : "=r" (__val)					\
		    :);							\
		__val;							\
	})

/*
 * This is used to ensure the compiler did actually allocate the register we
 * asked it for some inline assembly sequences.  Apparently we can't trust
 * the compiler from one version to another so a bit of paranoia won't hurt.
 * This string is meant to be concatenated with the inline asm string and
 * will cause compilation to stop on mismatch.
 * (for details, see gcc PR 15089)
 */
#define __asmeq(x, y)  ".ifnc " x "," y " ; .err ; .endif\n\t"

#ifndef __ASSEMBLY__

#include <linux/linkage.h>
#include <linux/irqflags.h>

#define __exception	__attribute__((section(".exception.text")))

struct thread_info;
struct task_struct;

/* information about the system we're running on */
extern unsigned int system_rev;
extern unsigned int system_serial_low;
extern unsigned int system_serial_high;
extern unsigned int mem_fclk_21285;

struct pt_regs;

void die(const char *msg, struct pt_regs *regs, int err)
		__attribute__((noreturn));

struct siginfo;
void lm32_notify_die(const char *str, struct pt_regs *regs, struct siginfo *info,
		unsigned long err, unsigned long trap);

void hook_fault_code(int nr, int (*fn)(unsigned long, unsigned int,
				       struct pt_regs *),
		     int sig, const char *name);

#define xchg(ptr,x) \
	((__typeof__(*(ptr)))__xchg((unsigned long)(x),(ptr),sizeof(*(ptr))))

extern asmlinkage void __backtrace(void);
extern asmlinkage void c_backtrace(unsigned long fp, int pmode);

struct mm_struct;
extern void show_pte(struct mm_struct *mm, unsigned long addr);
extern void __show_regs(struct pt_regs *);

extern int cpu_architecture(void);
extern void cpu_init(void);


void lm32_machine_restart(char mode);
extern void (*lm32_pm_restart)(char str);

/*
#define set_cr(x)					\
	__asm__ __volatile__(				\
	"mcr	p15, 0, %0, c1, c0, 0	@ set CR"	\
	: : "r" (x) : "cc")

#define get_cr()					\
	({						\
	unsigned int __val;				\
	__asm__ __volatile__(				\
	"mrc	p15, 0, %0, c1, c0, 0	@ get CR"	\
	: "=r" (__val) : : "cc");			\
	__val;						\
	})

extern unsigned long cr_no_alignment;	// defined in entry-armv.S 
extern unsigned long cr_alignment;	// defined in entry-armv.S 

#define UDBG_UNDEFINED	(1 << 0)
#define UDBG_SYSCALL	(1 << 1)
#define UDBG_BADABORT	(1 << 2)
#define UDBG_SEGV	(1 << 3)
#define UDBG_BUS	(1 << 4)

#define vectors_high()	(cr_alignment & CR_V)
*/

extern unsigned int user_debug;

/*#define mb() __asm__ __volatile__ ("" : : : "memory")
#define rmb() mb()
#define wmb() mb()
#define read_barrier_depends() do { } while(0)
#define set_mb(var, value)  do { var = value; mb(); } while (0)
#define set_wmb(var, value) do { var = value; wmb(); } while (0)*/
#define isb() __asm__ __volatile__ ("" : : : "memory")
#define dsb() __asm__ __volatile__ ("mcr p15, 0, %0, c7, c10, 4" \
                                  : : "r" (0) : "memory")
#define dmb() __asm__ __volatile__ ("" : : : "memory")
#ifndef CONFIG_SMP
#define mb()   do { if (arch_is_coherent()) dmb(); else barrier(); } while (0)
#define rmb()  do { if (arch_is_coherent()) dmb(); else barrier(); } while (0)
#define wmb()  do { if (arch_is_coherent()) dmb(); else barrier(); } while (0)
#define smp_mb()       barrier()
#define smp_rmb()      barrier()
#define smp_wmb()      barrier()
#else
#define mb()           dmb()
#define rmb()          dmb()
#define wmb()          dmb()
#define smp_mb()       dmb()
#define smp_rmb()      dmb()
#define smp_wmb()      dmb()
#endif
#define read_barrier_depends()         do { } while(0)
#define smp_read_barrier_depends()     do { } while(0)

#define set_mb(var, value)     do { var = value; smp_mb(); } while (0)
#define nop() __asm__ __volatile__("nop\t@ nop\n\t");

/*
 * switch_mm() may do a full cache flush over the context switch,
 * so enable interrupts over the context switch to avoid high
 * latency.
 */
#define __ARCH_WANT_INTERRUPTS_ON_CTXSW

/*
 * switch_to(prev, next) should switch from task `prev' to `next'
 * `prev' will never be the same as `next'.  schedule() itself
 * contains the memory barrier to tell GCC not to cache `current'.
 */
extern struct task_struct *__switch_to(struct task_struct *, struct thread_info *, struct thread_info *);

#define switch_to(prev,next,last)					\
do {									\
	last = __switch_to(prev,task_thread_info(prev), task_thread_info(next));	\
} while (0)

#if 0
/*
 * On SMP systems, when the scheduler does migration-cost autodetection,
 * it needs a way to flush as much of the CPU's caches as possible.
 *
 * TODO: fill this in!
 */
static inline void sched_cacheflush(void)
{
}


/*
 * Save the current interrupt enable state & disable IRQs
 */
#define local_irq_save(x)					\
	({							\
		unsigned long temp;				\
		(void) (&temp == &x);				\
	__asm__ __volatile__(					\
	"rcsr	%0, IE		# local_irq_save\n\t"		\
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
#define local_irq_enable()					\
	({							\
		unsigned long temp;				\
	__asm__ __volatile__(					\
	"rcsr	%0, IE		# local_irq_save\n\t"		\
	"ori	%0, %0, 1\n\t"					\
	"wcsr	IE, %0"						\
	: "=r" (temp)						\
	:							\
	: "memory");					\
	})

/*
 * Disable IRQs
 */
#define local_irq_disable()					\
	({							\
		unsigned long temp;				\
	__asm__ __volatile__(					\
	"rcsr	%0, IE		# local_irq_save\n\t"		\
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
#define local_save_flags(x)					\
	({							\
	__asm__ __volatile__(					\
	"rcsr	%0, IE		# local_irq_save"		\
	: "=r" (x) : : "memory");				\
	})

/*
 * restore saved IRQ & FIQ state
 */
#define local_irq_restore(x)					\
	__asm__ __volatile__(					\
	"wcsr	IE, %0	     	# local_irq_restore"	\
	:							\
	: "r" (x)						\
	: "memory")

#define irqs_disabled()			\
({					\
	unsigned long flags;		\
	local_save_flags(flags);	\
	(int)(flags & PSR_I_BIT);	\
})

#ifdef CONFIG_SMP

#define smp_mb()		mb()
#define smp_rmb()		rmb()
#define smp_wmb()		wmb()
#define smp_read_barrier_depends()		read_barrier_depends()

#else

#define smp_mb()		barrier()
#define smp_rmb()		barrier()
#define smp_wmb()		barrier()
#define smp_read_barrier_depends()		do { } while(0)

#endif /* CONFIG_SMP */

#endif //0

static inline unsigned long __xchg(unsigned long x, volatile void *ptr, int size)
{
	extern void __bad_xchg(volatile void *, int);
	unsigned long ret;
	unsigned long flags;

	switch (size) {
#ifdef CONFIG_SMP
#error SMP is not supported on this platform
#endif
	case 1:
		raw_local_irq_save(flags);
		ret = *(volatile unsigned char *)ptr;
		*(volatile unsigned char *)ptr = x;
		raw_local_irq_restore(flags);
		break;

	case 4:
		raw_local_irq_save(flags);
		ret = *(volatile unsigned long *)ptr;
		*(volatile unsigned long *)ptr = x;
		raw_local_irq_restore(flags);
		break;
	default:
		__bad_xchg(ptr, size), ret = 0;
		break;
	}

	return ret;
}

extern void disable_hlt(void);
extern void enable_hlt(void);

#endif /* __ASSEMBLY__ */

#define arch_align_stack(x) (x)

#endif /* __KERNEL__ */

#endif
