#
# Makefile for Phoenix-RTOS kernel (ARMv7 HAL)
#
# Copyright 2016-2017, 2019-2020 Phoenix Systems
#


ifneq (, $(findstring l1, $(TARGET_SUBFAMILY)))
    include hal/armv7m/stm32/l1/Makefile
else ifneq (, $(findstring l4, $(TARGET_SUBFAMILY)))
    include hal/armv7m/stm32/l4/Makefile
endif

CFLAGS += -Ihal/armv7m/stm32

OBJS += $(addprefix $(PREFIX_O)hal/armv7m/stm32/, _init.o interrupts.o)
