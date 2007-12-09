#ifndef _ASM_LM32_IRQ_H
#define _ASM_LM32_IRQ_H

/*
 * Nothing to see here yet
 */
#ifndef _ARCH_LM32_HW_IRQ_H
#define _ARCH_LM32_HW_IRQ_H

#include <asm/arch/irqs.h>
//#include <asm/arch/irq.h>

#if defined(CONFIG_NO_IDLE_HZ)
# include <asm/dyntick.h>
# define handle_dynamic_tick(action)                                   \
       if (!(action->flags & IRQF_TIMER) && system_timer->dyn_tick) {  \
               write_seqlock(&xtime_lock);                             \
               if (system_timer->dyn_tick->state & DYN_TICK_ENABLED)   \
                       system_timer->dyn_tick->handler(irq, NULL);     \
               write_sequnlock(&xtime_lock);                           \
       }
#endif

#endif

#ifndef irq_canonicalize
#define irq_canonicalize(i)	(i)
#endif

#ifndef NR_IRQS
#define NR_IRQS	32
#endif

/*
 * Use this value to indicate lack of interrupt
 * capability
 */
#ifndef NO_IRQ
#define NO_IRQ	((unsigned int)(-1))
#endif


/*
 * Migration helpers
 */
#define __IRQT_FALEDGE	IRQ_TYPE_EDGE_FALLING
#define __IRQT_RISEDGE	IRQ_TYPE_EDGE_RISING
#define __IRQT_LOWLVL	IRQ_TYPE_LEVEL_LOW
#define __IRQT_HIGHLVL	IRQ_TYPE_LEVEL_HIGH

#define IRQT_NOEDGE	(0)
#define IRQT_RISING	(__IRQT_RISEDGE)
#define IRQT_FALLING	(__IRQT_FALEDGE)
#define IRQT_BOTHEDGE	(__IRQT_RISEDGE|__IRQT_FALEDGE)
#define IRQT_LOW	(__IRQT_LOWLVL)
#define IRQT_HIGH	(__IRQT_HIGHLVL)
#define IRQT_PROBE	IRQ_TYPE_PROBE

#ifndef __ASSEMBLY__
struct irqaction;
extern void migrate_irqs(void);
#endif

#endif

