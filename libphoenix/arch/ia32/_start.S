/*
 * Phoenix-RTOS
 *
 * libphoenix
 *
 * Entrypoint (ia32)
 *
 * Copyright 2018 Phoenix Systems
 * Author: Michal Miroslaw
 *
 * This file is part of Phoenix-RTOS.
 *
 * %LICENSE%
 */

#define __ASSEMBLY__

.text

.globl _start
.type _start, %function
_start:
	jmp _startc
.size _start, .-_start
