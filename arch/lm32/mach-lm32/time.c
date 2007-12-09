/*
 *  linux/arch/arm/mach-lm32/time.c
 *
 *  Lattice's Mico32 support by Andrea della Porta (sfaragnaus@gmail.com)
 *  based upon lpc22xx ARM port from Philips Semiconductors
 *
 */

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/time.h>
#include <linux/clocksource.h>
#include <linux/clockchips.h>
//#include <asm/system.h>
#include <asm/leds.h>
#include <asm/mach-types.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/hardware.h>
#include <asm/mach/time.h>
#include <asm/arch/time.h>

extern void	lm32_unmask_irq(unsigned int);

unsigned long lm32_gettimeoffset (void)
{
    mico_timer_t *timer = (mico_timer_t*)TIMER_BASE;

    return (LM32_TIMER_PERIOD_VALUE - timer->snapshot)/CLOCKS_PER_USEC;
}

static irqreturn_t
lm32_timer_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
    mico_timer_t *timer = (mico_timer_t*)dev_id;

    if (!(timer->status & LM32_TIMER_TO)) return IRQ_NONE;
/*    do_timer(regs);
    do_profile(regs);
*/ timer_tick(/*regs*/);  /* modified 20050608 for new version */

    timer->status = 0;	/* reset interrupt */
    return IRQ_HANDLED;
}

static struct irqaction lm32_timer_irq = {
        .name           = "LM32 Timer Tick",
        .flags          = IRQF_DISABLED | IRQF_TIMER,
        .handler        = lm32_timer_interrupt
};

/*
 * Set up timer interrupt, and return the current time in seconds.
 */

void __init  lm32_time_init (void)
{
	mico_timer_t *timer = (mico_timer_t*)TIMER_BASE;
	/*
	 * disable and clear timer 
	 */
	if(timer->status & LM32_TIMER_RUN)
		timer->control = LM32_TIMER_STOP;
	timer->status = 0;
	/* initialize the timer period */
	timer->period = LM32_TIMER_PERIOD_VALUE;
	/* generate interrupt when timer reaches 0, continuous */
	timer->control |= LM32_TIMER_ITO | LM32_TIMER_CONT;

	/*
	 * @todo do those really need to be function pointers ?
	 */
/*	gettimeoffset     = lm32_gettimeoffset; */
	lm32_timer_irq.handler = lm32_timer_interrupt;
	lm32_timer_irq.dev_id = TIMER_BASE;	//interrupt handler private data

	/* set up the interrupt vector for timer  */
	setup_irq(LM32_INTERRUPT_TIMER0, &lm32_timer_irq);
	
	/* enable the timer IRQ */
	lm32_unmask_irq(LM32_INTERRUPT_TIMER0);

	/* let timer run... */
	timer->control &= ~LM32_TIMER_STOP;
	timer->control |= LM32_TIMER_START;
}

struct sys_timer lm32_timer = {
	.init		= lm32_time_init,
	.offset		= lm32_gettimeoffset,
};
