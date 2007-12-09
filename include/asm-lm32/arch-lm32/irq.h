/*
 *  linux/include/asm-lm32/arch-lm32/irq.h
 *
 *  Lattice's Mico32 port by Andrea della Porta (sfaragnaus@gmail.com)
 *  based unpon the lpc22xx ARM port from Philips Semiconductors
 */
#ifndef __LM32_irq_h
#define __LM32_irq_h

#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/mach/irq.h>
#include <asm/arch/irqs.h>

extern unsigned  int fixup_irq(int i);

extern void do_IRQ(int irq, struct pt_regs *regs);

extern void lm32_init_irq(void);

#define 	irq_init_irq 	lm32_init_irq

#endif 
