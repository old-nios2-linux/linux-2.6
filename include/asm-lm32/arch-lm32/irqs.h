/*
 *  linux/include/asm-lm32/arch-lm32/irqs.h
 *
 *  Lattice's Mico32 support by Andrea della Porta (sfaragnaus@gmail.com)
 *  basedupon the lpc22xx ARM port by Philips Semiconductors
 *
 *  IRQ number definition
 *  All IRQ numbers of the MICO32 CPUs.
 *
 */

#ifndef __LM32_irqs_h
#define __LM32_irqs_h                        1

#define NR_IRQS	32
	
#define LM32_INTERRUPT_WDINT	 0	/* Watchdog int. 0 */
#define LM32_INTERRUPT_RSV0	 1	/* Reserved int. 1 */
#define LM32_INTERRUPT_DBGRX	 2	/* Embedded ICE DbgCommRx receive */
#define LM32_INTERRUPT_DBGTX	 3	/* Embedded ICE DbgCommRx Transmit*/
#define LM32_INTERRUPT_TIMER0 4	/* Timer 0 */
#define LM32_INTERRUPT_TIMER1 5	/* Timer 1 */
#define LM32_INTERRUPT_UART0	 6	/* UART 0 */
#define LM32_INTERRUPT_UART1  7	/* UART 1 */
#define LM32_INTERRUPT_PWM0 	 8	/* PWM */
#define LM32_INTERRUPT_I2C 	 9	/* I2C  */
#define LM32_INTERRUPT_SPI0 	10	/* SPI0 */
#define LM32_INTERRUPT_SPI1 	11	/* SPI1 */
#define LM32_INTERRUPT_PLL 	12	/* PLL */
#define LM32_INTERRUPT_RTC 	13	/* RTC */
#define LM32_INTERRUPT_EINT0	14	/* Externel Interrupt 0 */
#define LM32_INTERRUPT_EINT1	15	/* Externel Interrupt 1 */
#define LM32_INTERRUPT_EINT2	16	/* Externel Interrupt 2 */
#define LM32_INTERRUPT_EINT3	17	/* Externel Interrupt 3 */
#define LM32_INTERRUPT_ADC 	18	/* AD Converter */
#define LM32_INTERRUPT_CANERR 19	/* CAN LUTerr interrupt */
#define LM32_INTERRUPT_CAN1TX 20	/* CAN1 Tx interrupt */
#define LM32_INTERRUPT_CAN1RX 21	/* CAN1 Rx interrupt */
#define LM32_INTERRUPT_CAN2TX 22	/* CAN2 Tx interrupt */
#define LM32_INTERRUPT_CAN2RX 23	/* CAN2 Rx interrupt */
#define LM32_INTERRUPT_CAN3TX 24	/* CAN1 Tx interrupt */
#define LM32_INTERRUPT_CAN3RX 25	/* CAN1 Rx interrupt */
#define LM32_INTERRUPT_CAN4TX 26	/* CAN2 Tx interrupt */
#define LM32_INTERRUPT_CAN4RX 27	/* CAN2 Rx interrupt */

#endif /* End of __irqs_h */
