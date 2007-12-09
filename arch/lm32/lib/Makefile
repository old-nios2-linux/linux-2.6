#
# linux/arch/lm32/lib/Makefile
#
# Copyright (C) 2007 Andrea della Porta (sfaragnaus@gmail.com)
# Copyright (C) 1995-2000 Russell King
#

#striped-off with respect to ARM port: csumpartial.o
lib-y		:= string.o memcpy.o memchr.o strchr.o strrchr.o checksum.o delay.o backtrace.o strncpy_from_user.o strnlen_user.o 
		   #csum_partial_copy.o			\	
		   #changebit.o csumipv6.o      \
		   #csumpartialcopy.o csumpartialcopyuser.o clearbit.o \
		   #findbit.o                \
		   #memzero.o setbit.o              \
		   #testchangebit.o testclearbit.o testsetbit.o        \
		   #ashldi3.o ashrdi3.o lshrdi3.o muldi3.o             \
		   #ucmpdi2.o lib1funcs.o div64.o sha1.o               \
		   #io-readsb.o io-writesb.o io-readsl.o io-writesl.o  \
		   #---------------------------------------------------
		   #memmove.o memset.o 


lib-$(CONFIG_MMU) += copy_page.o getuser.o putuser.o clear_user.o

# the code in uaccess.S is not preemption safe and
# probably faster on ARMv3 only
ifeq ($(CONFIG_PREEMPT),y)
  lib-$(CONFIG_MMU) += copy_from_user.o copy_to_user.o
else
ifneq ($(CONFIG_CPU_32v3),y)
  lib-$(CONFIG_MMU) += copy_from_user.o copy_to_user.o
else
  lib-$(CONFIG_MMU) += uaccess.o
endif
endif

ifeq ($(CONFIG_CPU_32v3),y)
  #lib-y	+= io-readsw-armv3.o io-writesw-armv3.o
else
  #lib-y	+= io-readsw-armv4.o io-writesw-armv4.o
endif

lib-$(CONFIG_ARCH_RPC)		+= ecard.o io-acorn.o floppydma.o
lib-$(CONFIG_ARCH_CLPS7500)	+= io-acorn.o
lib-$(CONFIG_ARCH_L7200)	+= io-acorn.o
lib-$(CONFIG_ARCH_SHARK)	+= io-shark.o

$(obj)/csumpartialcopy.o:	$(obj)/csumpartialcopygeneric.S
$(obj)/csumpartialcopyuser.o:	$(obj)/csumpartialcopygeneric.S
