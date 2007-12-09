/*
 *  linux/include/asm-lm32/arch-lm32/time.h
 *
 *  Lattice's Mico32 port from Andrea della Porta (sfaragnaus@gmail.com)
 *  ibased upon the lpc22xx ARM port by Philips Semiconductors
 */

#ifndef __ASM_ARCH_TIME_H__
#define __ASM_ARCH_TIME_H__

#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/hardware.h>
#include <asm/arch/timex.h>

#define LM32_wishbone_clk 	CLOCK_TICK_RATE 	//a.k.a. CONFIG_LM32_CLK
#define CLOCKS_PER_USEC		(LM32_wishbone_clk/1000000)

/* set timer to generate interrupt every 10ms */
#define LM32_TIMER_PERIOD_VALUE	(LM32_wishbone_clk/100)

//status register
#define LM32_TIMER_TO		(1 << 0)
#define LM32_TIMER_RUN		(1 << 1)

//control register
#define LM32_TIMER_ITO		(1 << 0)
#define LM32_TIMER_CONT		(1 << 1)
#define LM32_TIMER_START	(1 << 2)
#define LM32_TIMER_STOP		(1 << 3)

typedef struct 
{
	volatile unsigned int status;
	volatile unsigned int control;
	volatile unsigned int period;
	volatile unsigned int snapshot;
} /*__attribute__ ((packed))*/ mico_timer_t;	//it seems that packed attribute transforms unsigned int into char

#endif /*__ASM_ARCH_TIME_H__*/
