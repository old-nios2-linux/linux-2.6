#ifndef _ASMARM_SIGCONTEXT_H
#define _ASMARM_SIGCONTEXT_H

/*
 * Signal context structure - contains all info to do with the state
 * before the signal handler was invoked.  Note: only add new entries
 * to the end of the structure.
 */
struct sigcontext {
	unsigned long trap_no;
	unsigned long error_code;
	unsigned long oldmask;
	unsigned long lm32_r0;
	unsigned long lm32_r1;
	unsigned long lm32_r2;
	unsigned long lm32_r3;
	unsigned long lm32_r4;
	unsigned long lm32_r5;
	unsigned long lm32_r6;
	unsigned long lm32_r7;
	unsigned long lm32_r8;
	unsigned long lm32_r9;
	unsigned long lm32_r10;
	unsigned long lm32_r11;
	unsigned long lm32_r12;
	unsigned long lm32_r13;
	unsigned long lm32_r14;
	unsigned long lm32_r15;
	unsigned long lm32_r16;
	unsigned long lm32_r17;
	unsigned long lm32_r18;
	unsigned long lm32_r19;
	unsigned long lm32_r20;
	unsigned long lm32_r21;
	unsigned long lm32_r22;
	unsigned long lm32_r23;
	unsigned long lm32_r24;
	unsigned long lm32_r25;
	unsigned long lm32_gp;
	unsigned long lm32_fp;
	unsigned long lm32_sp;
	unsigned long lm32_ra;
	unsigned long lm32_ea;
	unsigned long lm32_ba;
	unsigned long lm32_ie;
	unsigned long lm32_psr;
	//unsigned long lm32_ORIG_r1;
	unsigned long fault_address;
};


#endif
