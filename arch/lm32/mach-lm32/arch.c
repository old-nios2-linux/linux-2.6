/*
 *  linux/arch/lm32/mach-lm32/arch.c
 *
 *
 *  Architecture specific fixups.  This is where any
 *  parameters in the params struct are fixed up, or
 *  any additional architecture specific information
 *  is pulled from the params struct.
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

#include <linux/tty.h>
#include <asm/elf.h>
#include <linux/root_dev.h>
#include <linux/initrd.h>


extern void __init lm32_init_irq(void);

extern struct sys_timer lm32_timer;

MACHINE_START(LM32, "Mico32, Lattice's soft core.")
	//MAINTAINER -> " Andrea della Porta <sfaragnaus@gmail.com>"
	.init_irq = lm32_init_irq,
	.timer	= &lm32_timer,
MACHINE_END
