#
# Makefile for Phoenix-RTOS loader (ARMv7A HAL i.MX 6ULL)
#
# Copyright 2021 Phoenix Systems
#
# %LICENSE%
#

INIT_RAM = 0x907000
INIT_FLASH = 0x0

LDFLAGS:=$(filter-out -Tbss% , $(LDFLAGS))
LDFLAGS:=$(filter-out -Tdata% , $(LDFLAGS))
LDFLAGS+=--defsym _plo_bss=__bss_start

CFLAGS:=$(filter-out -mfloat-abi% , $(CFLAGS))
CFLAGS:=$(filter-out -mfpu% , $(CFLAGS))

CFLAGS += -DVADDR_KERNEL_INIT=$(VADDR_KERNEL_INIT)

GCCLIB := $(shell $(CC) $(CFLAGS) -print-libgcc-file-name)

PLO_COMMANDS = alias app call console copy dump echo go help kernel map phfs script test-ddr wait

include devices/uart-imx6ull/Makefile
include devices/usbc-cdc/Makefile

OBJS += $(addprefix $(PREFIX_O)hal/$(TARGET_SUFF)/$(TARGET_SUBFAMILY)/, _init.o imx6ull.o hal.o timer.o\
		interrupts.o console.o)
