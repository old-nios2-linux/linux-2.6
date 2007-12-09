/*
 *  linux/arch/arm/mach-lm32/irq.c
 *
 *  Lattice's Mico32 support by Andrea della Porta (sfaragnaus@gmail.com)
 *  based upon lpc22xx ARM port from Philips Semiconductors
 *
 */
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/sysdev.h>

#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/setup.h>
#include <asm/mach-types.h>

#include <asm/mach/arch.h>
#include <asm/mach/irq.h>
#include <asm/mach/map.h>

void __inline__ lm32_mask_irq(unsigned int irq)
{
	asm volatile ("rcsr r1, IM\n\t"
		      "and  r1, r1, %0\n\t"
		      "wcsr IM, r1" : : "r"(1<<irq) : "r1");
}

void __inline__ lm32_unmask_irq(unsigned int irq)
{
	asm volatile ("rcsr r1, IM\n\t"
		      "or   r1, r1, %0\n\t"
		      "wcsr IM, r1" : : "r"(1<<irq) : "r1");
}

/* Clear pending bit */
void __inline__ lm32_clear_pb(unsigned int irq)
{
	asm volatile ("wcsr IM, %0\n\t" : : "r"(1<<irq) );
}

void __inline__ lm32_mask_ack_irq(unsigned int irq)
{

	lm32_clear_pb(irq);
	lm32_mask_irq(irq);
}


/* YOU CAN CHANGE THIS ROUTINE FOR SPEED UP */
__inline__ unsigned int fixup_irq (int irq )
{
	lm32_clear_pb(irq);
	return(irq);
}

static struct irq_chip lm32_chip = {
	.name	= "MPU",
	.ack	= lm32_mask_ack_irq,
	.mask	= lm32_mask_irq,
	.unmask = lm32_unmask_irq,
};

#ifdef CONFIG_PM
static unsigned long ic_irq_enable;

static int irq_suspend(struct sys_device *dev, u32 state)
{
	return 0;
}

static int irq_resume(struct sys_device *dev)
{
	/* disable all irq sources */
	return 0;
}
#else
#define irq_suspend NULL
#define irq_resume NULL
#endif

static struct sysdev_class irq_class = {
	set_kset_name("irq"),
	.suspend	= irq_suspend,
	.resume		= irq_resume,
};

static struct sys_device irq_device = {
	.id	= 0,
	.cls	= &irq_class,
};

static int __init irq_init_sysfs(void)
{
	int ret = sysdev_class_register(&irq_class);
	if (ret == 0)
		ret = sysdev_register(&irq_device);
	return ret;
}

device_initcall(irq_init_sysfs);

void __init lm32_init_irq(void)
{
        int irq;

	for (irq = 0; irq < NR_IRQS; irq++) {
		set_irq_chip(irq, &lm32_chip);
		set_irq_handler(irq, handle_level_irq);
		/*if(irq >= LPC22xx_INTERRUPT_EINT0 && irq <= LPC22xx_INTERRUPT_EINT3)
			set_irq_flags(irq, IRQF_VALID | IRQF_PROBE | IRQF_NOAUTOEN);
		else
			set_irq_flags(irq, IRQF_VALID | IRQF_PROBE);*/
	}

	/* mask and disable all further interrupts */
	asm volatile ("wcsr IM, %0" : : "r" ((int)-1) );

	/* set all to IRQ mode, not FIQ */
	//VICIntSelect = 0x00000000;
	
	/* Clear Intrerrupt pending register	*/
	asm volatile ("wcsr IP, %0" : : "r" ((int)-1) );

	/* Set external interrupts*/
	/* These should be different with your board */
	/* ext2: for ethernet controller rtl8019as */
	//EXTMODE = 0x04;		// set ext2 edge sensitive
	//EXTPOLAR = 0x04;	// set ext2 rising edge effective
	//EXTINT = 0x04;		// clear flags
}

