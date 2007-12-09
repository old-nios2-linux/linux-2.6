#
# arch/lm32/Makefile
#
# This file is included by the global makefile so that you can add your own
# architecture-specific flags and dependencies.
#
# This file is subject to the terms and conditions of the GNU General Public
# License.  See the file "COPYING" in the main directory of this archive
# for more details.
#
# Copyright (C) 1995-2001 by Russell King

#-p on ARM architecture does not use standard path. Do we need it in LDFLAGS_vmlinux on Mico32?
LDFLAGS_vmlinux	:= --no-undefined -X
CPPFLAGS_vmlinux.lds = -DTEXT_OFFSET=$(TEXT_OFFSET)
OBJCOPYFLAGS   :=-O binary -R .note -R .note.gnu.build-id -R .comment -S
GZFLAGS		:=-9
#KBUILD_CFLAGS +=-pipe
# Explicitly specifiy 32-bit ARM ISA since toolchain default can be -mthumb:
#KBUILD_CFLAGS  +=$(call cc-option,-marm,)
CFLAGS		+=-DCONFIG_LM32_CLK=1000000
CFLAGS		+=-DLM32_TIMER_BASE=$(CONFIG_LM32_TIMER_BASE)
CFLAGS		+=-DBYTES_PER_LINE=$(CONFIG_BYTES_PER_LINE)
CFLAGS		+=-Wa,-Iinclude,-Iinclude2

# Do not use arch/arm/defconfig - it's always outdated.
# Select a platform tht is kept up-to-date
KBUILD_DEFCONFIG := lm32_defconfig

# defines filename extension depending memory manement type.
ifeq ($(CONFIG_MMU),)
MMUEXT		:= -nommu
endif

ifeq ($(CONFIG_FRAME_POINTER),y)
KBUILD_CFLAGS		+=-fno-omit-frame-pointer #-mno-sched-prolog -mapcs
endif

#ifeq ($(CONFIG_CPU_BIG_ENDIAN),y)
#KBUILD_CPPFLAGS	+= -mbig-endian
#AS		+= -EB
#LD		+= -EB
#else
#KBUILD_CPPFLAGS	+= -mlittle-endian
#AS		+= -EL
#LD		+= -EL
#endif

comma = ,

# This selects which instruction set is used.
# Note that GCC does not numerically define an architecture version
# macro, but instead defines a whole series of macros which makes
# testing for a specific architecture or later rather impossible.
#arch-$(CONFIG_CPU_32v3)		:=-D__LINUX_ARM_ARCH__=3 -march=armv3

# This selects how we optimise for the processor.
#tune-$(CONFIG_CPU_ARM610)	:=-mtune=arm610

# Need -Uarm for gcc < 3.x
#KBUILD_CFLAGS		+=$(CFLAGS_ABI) $(arch-y) $(tune-y) $(call cc-option,-mshort-load-bytes,$(call cc-option,-malignment-traps,)) -msoft-float -Uarm
#KBUILD_AFLAGS		+=$(CFLAGS_ABI) $(arch-y) $(tune-y) -msoft-float

CHECKFLAGS	+= -D__arm__

#Default value
head-y		:= arch/lm32/kernel/head$(MMUEXT).o arch/lm32/kernel/init_task.o
textofs-y	:= 0x00008000

 machine-$(CONFIG_ARCH_LM32)	   := lm32
 # incdir-$(CONFIG_ARCH_LM32)	   := lm32
 #textaddr-$(CONFIG_ARCH_LPC22xx)	   := $(textofs-y)

# The byte offset of the kernel image in RAM from the start of RAM.
TEXT_OFFSET := $(textofs-y)

ifeq ($(incdir-y),)
incdir-y := $(machine-y)
endif
INCDIR   := arch-$(incdir-y)

ifneq ($(machine-y),)
MACHINE  := arch/lm32/mach-$(machine-y)/
else
MACHINE  :=
endif

export	TEXT_OFFSET GZFLAGS MMUEXT

# Do we have FASTFPE?
#FASTFPE		:=arch/lm32/fastfpe
#ifeq ($(FASTFPE),$(wildcard $(FASTFPE)))
#FASTFPE_OBJ	:=$(FASTFPE)/
#endif

# If we have a machine-specific directory, then include it in the build.
core-y				+= arch/lm32/kernel/ arch/lm32/mm/ arch/lm32/common/
core-y				+= $(MACHINE)
#core-$(CONFIG_FPE_NWFPE)	+= arch/arm/nwfpe/
#core-$(CONFIG_FPE_FASTFPE)	+= $(FASTFPE_OBJ)
#core-$(CONFIG_VFP)		+= arch/arm/vfp/

# If we have a common platform directory, then include it in the build.
#core-$(CONFIG_ARCH_OMAP)	+= arch/lm32/plat-omap/

#drivers-$(CONFIG_OPROFILE)      += arch/lm32/oprofile/

libs-y				:= arch/lm32/lib/ $(libs-y)
LIBGCC  			:= `$(CC) --print-libgcc-file-name`
libs-y				+= $(LIBGCC)

# Default target when executing plain make
ifeq ($(CONFIG_XIP_KERNEL),y)
KBUILD_IMAGE := xipImage
else
KBUILD_IMAGE := zImage
endif

all:   $(KBUILD_IMAGE)

boot := arch/lm32/boot

#	Update machine arch and proc symlinks if something which affects
#	them changed.  We use .arch to indicate when they were updated
#	last, otherwise make uses the target directory mtime.

include/asm-lm32/.arch: $(wildcard include/config/arch/*.h) include/config/auto.conf
	@echo '  SYMLINK include/asm-lm32/arch -> include/asm-lm32/$(INCDIR)'
ifneq ($(KBUILD_SRC),)
	$(Q)mkdir -p include/asm-lm32
	$(Q)ln -fsn $(srctree)/include/asm-lm32/$(INCDIR) include/asm-lm32/arch
else
	$(Q)ln -fsn $(INCDIR) include/asm-lm32/arch
endif
	@touch $@

#archprepare: maketools

#PHONY += maketools FORCE
#maketools: include/linux/version.h include/asm-lm32/.arch FORCE
#	$(Q)$(MAKE) $(build)=arch/lm32/tools include/asm-lm32/mach-types.h

# Convert bzImage to zImage
bzImage: zImage

zImage Image xipImage bootpImage uImage: vmlinux
	$(Q)$(MAKE) $(build)=$(boot) MACHINE=$(MACHINE) $(boot)/$@

zinstall install: vmlinux
	$(Q)$(MAKE) $(build)=$(boot) MACHINE=$(MACHINE) $@

#CLEAN_FILES += include/asm-lm32/mach-types.h \
	       include/asm-lm32/arch include/asm-lm32/.arch

# We use MRPROPER_FILES and CLEAN_FILES now
archclean:
	$(Q)$(MAKE) $(clean)=$(boot)

# My testing targets (bypasses dependencies)
bp:;	$(Q)$(MAKE) $(build)=$(boot) MACHINE=$(MACHINE) $(boot)/bootpImage
i zi:;	$(Q)$(MAKE) $(build)=$(boot) MACHINE=$(MACHINE) $@


define archhelp
  echo  '* zImage        - Compressed kernel image (arch/$(ARCH)/boot/zImage)'
  echo  '  Image         - Uncompressed kernel image (arch/$(ARCH)/boot/Image)'
  echo  '* xipImage      - XIP kernel image, if configured (arch/$(ARCH)/boot/xipImage)'
  echo  '  bootpImage    - Combined zImage and initial RAM disk' 
  echo  '                  (supply initrd image via make variable INITRD=<path>)'
  echo  '  install       - Install uncompressed kernel'
  echo  '  zinstall      - Install compressed kernel'
  echo  '                  Install using (your) ~/bin/installkernel or'
  echo  '                  (distribution) /sbin/installkernel or'
  echo  '                  install to $$(INSTALL_PATH) and run lilo'
endef
