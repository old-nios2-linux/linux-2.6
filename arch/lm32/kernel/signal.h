/*
 *  linux/arch/lm32/kernel/signal.h
 *
 *  Lattice's Mico32 port by Andrea della Porta (sfaragnaus@gmail.com)
 *  based on ARM port from Russell King.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#define KERN_SIGRETURN_CODE	(CONFIG_VECTORS_BASE + 0x00000500)

extern const unsigned long sigreturn_codes[5];
