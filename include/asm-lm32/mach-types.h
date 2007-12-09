/*
 * Unlike ARM32 this is NOT automatically generated. DONT delete it
 * Instead, consider FIXME-ing it so its auto-detected.
 */

#ifndef __ASM_LM32_MACH_TYPE_H
#define __ASM_LM32_MACH_TYPE_H

#ifndef __ASSEMBLY__
extern unsigned int __machine_arch_type;
#endif

#define MACH_TYPE_LM32  	       0xe0000000

#ifdef CONFIG_ARCH_LM32
# define machine_arch_type		MACH_TYPE_LM32
# define machine_is_lm32()	(machine_arch_type == MACH_TYPE_LM32)
#else
# define machine_is_lm32()	(0)
#endif


#ifndef machine_arch_type
#error Unknown machine type
#define machine_arch_type       __machine_arch_type
#endif

#endif
