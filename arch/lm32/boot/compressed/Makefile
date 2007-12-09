#
# linux/arch/lm32/boot/compressed/Makefile
#
# create a compressed vmlinuz image from the original vmlinux
#

HEAD	= head.o
OBJS	= misc.o
FONTC	= drivers/video/console/font_acorn_8x8.c

FONT = $(addprefix ../../../../drivers/video/console/, font_acorn_8x8.o)

#
# Architecture dependencies
#
ifeq ($(CONFIG_ARCH_LM32),y)
OBJS		+= head-lm32.o
endif

ifeq ($(CONFIG_CPU_BIG_ENDIAN),y)
OBJS		+= big-endian.o
endif

#
# We now have a PIC decompressor implementation.  Decompressors running
# from RAM should not define ZTEXTADDR.  Decompressors running directly
# from ROM or Flash must define ZTEXTADDR (preferably via the config)
# FIXME: Previous assignment to ztextaddr-y is lost here. See SHARK
ifeq ($(CONFIG_ZBOOT_ROM),y)
ZTEXTADDR	:= $(CONFIG_ZBOOT_ROM_TEXT)
ZBSSADDR	:= $(CONFIG_ZBOOT_ROM_BSS)
else
ZTEXTADDR	:= 0
ZBSSADDR	:= ALIGN(4)
endif

SEDFLAGS	= s/TEXT_START/$(ZTEXTADDR)/;s/BSS_START/$(ZBSSADDR)/

targets       := vmlinux vmlinux.lds piggy.gz piggy.o $(FONT) \
		 head.o misc.o $(OBJS)
EXTRA_CFLAGS  := -fpic
EXTRA_AFLAGS  :=

# Supply ZRELADDR, INITRD_PHYS and PARAMS_PHYS to the decompressor via
# linker symbols.  We only define initrd_phys and params_phys if the
# machine class defined the corresponding makefile variable.
LDFLAGS_vmlinux := --defsym zreladdr=$(ZRELADDR)
ifneq ($(INITRD_PHYS),)
LDFLAGS_vmlinux += --defsym initrd_phys=$(INITRD_PHYS)
endif
ifneq ($(PARAMS_PHYS),)
LDFLAGS_vmlinux += --defsym params_phys=$(PARAMS_PHYS)
endif
LDFLAGS_vmlinux += -p --no-undefined -X \
	$(shell $(CC) $(CFLAGS) --print-libgcc-file-name) -T

# Don't allow any static data in misc.o, which
# would otherwise mess up our GOT table
CFLAGS_misc.o := -Dstatic=

$(obj)/vmlinux: $(obj)/vmlinux.lds $(obj)/$(HEAD) $(obj)/piggy.o \
	 	$(addprefix $(obj)/, $(OBJS)) FORCE
	$(call if_changed,ld)
	@:

$(obj)/piggy.gz: $(obj)/../Image FORCE
	$(call if_changed,gzip)

$(obj)/piggy.o:  $(obj)/piggy.gz FORCE

CFLAGS_font_acorn_8x8.o := -Dstatic=

$(obj)/vmlinux.lds: $(obj)/vmlinux.lds.in arch/lm32/boot/Makefile .config
	@sed "$(SEDFLAGS)" < $< > $@

$(obj)/misc.o: $(obj)/misc.c include/asm/arch/uncompress.h lib/inflate.c

