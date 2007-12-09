/*
 *  linux/arch/arm/mach-lm32/head.S
 *
 *  uClinux kernel startup code for Lattice's MICO32 (lm32)
 *
 *  based upon Philips (LPC22xx) ARM startup code
 *  written by Andrea della Porta <sfaragnaus@gmail.com>
 */

#include <linux/config.h>
#include <linux/linkage.h>
#include <linux/init.h>


#include <asm/procinfo.h>		/* probably useful only for Mico32 processor */
#include <asm/ptrace.h>			/* we will implement ptrace support asap */
#include <asm/mach/arch.h>		/* again, Mico32-centric declarations */
#include <asm/arch/hardware.h>   	/* this can be useful for Mico32 specific stuff */
#include <asm/assembler.h>   		/* this can be useful for Mico32 specific stuff */


/*
 * Kernel startup entry point.
 */
	__INIT
	.type	stext, %function	/* this is the main entry point, see lm32/kernel/vmlinux.lds.S */
ENTRY(stext)

	xor r0, r0, r0			/* lm32 compiler expect r0 to be zero */
					/* at startup, interrupt are disabled and cache, if present, are invalidated. So we do not have too much to do here...... */
	/*wcsr IE, r0*/			/* make sure interrupt are disabled */
	/*rcsr r1, CFG*/		/* take the configuration register */
	

	mva	r1, LC0			/* if this does not compile, try the following: 	lw 	r1, (LC0) */
	lw	r5, (r1+0)		/* sets several pointers. They will be probably come in handy from start_kernel onwards. This is start of BSS segment */
	lw	r8, (r1+4)		/* end of BSS segment */
	lw	r6, (r1+8)		/* processor_id */
	lw	sp, (r1+12)		/* init_thread_union a.k.a. stack? */

	/* we should really move data sections from non-volatile memory to RAM here, unless the bootloader has already copied us into volatile memory and we're executing from there */

    1:  					
	sw	(r5+0), r0		/* looping to clear BSS segment */
	addi	r5, r5, 4
	bg	r8, r5, 1b

	rcsr 	r2, CFG			/* take the configuration register */
	/*srui 	r2, r2, 26*/		/* and get the processor revision, by now it will be masked from __lookup_proc_* (see __lm32_proc_info) */
	sw	(r6+0), r2		/* store it in 'processor_id' variable for future use */

	/*mva     r2, LM32_MACH_TYPE*/	/* hard-coded in mach-types.h */
	/*sw      (r9+0), r2*/

        mv 	fp, r0			/* don't be too sure you will use the frame pointer, at least at this early stage in the kernel! */
  	bi	start_kernel		
        
LC0:
  	.long	__bss_start		/* see vmlinux.lds.S */
        .long	_end
        .long	processor_id
        .long	init_thread_union+8192	/* what if the stack is empy-descending? In this case you probably want to subtract 4 */

	.align	8
	
/*LM32_MACH_TYPE:
	.long	MACH_TYPE_LM32*/


