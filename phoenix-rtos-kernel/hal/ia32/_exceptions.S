/*
 * Phoenix-RTOS
 *
 * Operating system kernel
 *
 * Exception stubs
 *
 * Copyright 2012, 2016 Phoenix Systems
 * Copyright 2001, 2005 Pawel Pisarczyk
 * Author: Pawel Pisarczyk
 *
 * This file is part of Phoenix-RTOS.
 *
 * %LICENSE%
 */

#define __ASSEMBLY__

#include <arch/cpu.h>

.text

/* Exception stub function definition */
#define EXCDEF(sym) \
.globl sym; \
.align 4, 0x90; \
.type sym, @function; \
sym:


/* Exception handling macro */
#define EXCSTUB(exc)\
	cli;\
	pushw %ds;\
	pushw %es;\
	pushw %fs;\
	pushw %gs;\
	pushl %eax;\
	pushl %ebx;\
	pushl %ecx;\
	pushl %edx;\
	pushl %ebp;\
	pushl %esi;\
	pushl %edi;\
	;\
	movl %dr5, %eax;\
	pushl %eax;\
	movl %dr4, %eax;\
	pushl %eax;\
	movl %dr3, %eax;\
	pushl %eax;\
	movl %dr2, %eax;\
	pushl %eax;\
	movl %dr1, %eax;\
	pushl %eax;\
	movl %dr0, %eax;\
	pushl %eax;\
	;\
	movl $SEL_KDATA, %eax;\
	movw %ax, %ds;\
	movw %ax, %es;\
	movw %ax, %fs;\
	movw %ax, %gs;\
	;\
	/* Call exception handler */ ;\
	leal 0(%esp), %eax;\
	pushl %eax;\
	movl exceptions + (exc) * 4, %eax;\
	pushl $exc;\
	call *%eax;\
	addl $8, %esp;\
	;\
	popl %eax;\
	movl %eax, %dr0;\
	popl %eax;\
	movl %eax, %dr1;\
	popl %eax;\
	movl %eax, %dr2;\
	popl %eax;\
	movl %eax, %dr3;\
	popl %eax;\
	movl %eax, %dr4;\
	popl %eax;\
	movl %eax, %dr5;\
	;\
	popl %edi;\
	popl %esi;\
	popl %ebp;\
	popl %edx;\
	popl %ecx;\
	popl %ebx;\
	popl %eax;\
	popw %gs;\
	popw %fs;\
	popw %es;\
	popw %ds;\
	addl $4, %esp;\
	iret;


/* Exception stubs */

EXCDEF(_exceptions_exc0)
	pushl $0;
	EXCSTUB(0);
.size _exceptions_exc0, .-_exceptions_exc0

EXCDEF(_exceptions_exc1)
	pushl $0;
	EXCSTUB(1);
.size _exceptions_exc1, .-_exceptions_exc1

EXCDEF(_exceptions_exc2)
	pushl $0;
	EXCSTUB(2);
.size _exceptions_exc2, .-_exceptions_exc2

EXCDEF(_exceptions_exc3)
	pushl $0
	EXCSTUB(3);
.size _exceptions_exc3, .-_exceptions_exc3

EXCDEF(_exceptions_exc4)
	pushl $0;
	EXCSTUB(4);
.size _exceptions_exc4, .-_exceptions_exc4

EXCDEF(_exceptions_exc5);
	pushl $0
	EXCSTUB(5);
.size _exceptions_exc5, .-_exceptions_exc5

EXCDEF(_exceptions_exc6);
	pushl $0
	EXCSTUB(6);
.size _exceptions_exc6, .-_exceptions_exc6

EXCDEF(_exceptions_exc7);
	pushl $0
	EXCSTUB(7);
.size _exceptions_exc7, .-_exceptions_exc7

EXCDEF(_exceptions_exc8);
	EXCSTUB(8);
.size _exceptions_exc8, .-_exceptions_exc8

EXCDEF(_exceptions_exc9);
	pushl $0
	EXCSTUB(9);
.size _exceptions_exc9, .-_exceptions_exc9

EXCDEF(_exceptions_exc10);
	EXCSTUB(10);
.size _exceptions_exc10, .-_exceptions_exc10

EXCDEF(_exceptions_exc11);
	EXCSTUB(11);
.size _exceptions_exc11, .-_exceptions_exc11

EXCDEF(_exceptions_exc12);
	EXCSTUB(12);
.size _exceptions_exc12, .-_exceptions_exc12

EXCDEF(_exceptions_exc13);
	EXCSTUB(13);
.size _exceptions_exc13, .-_exceptions_exc13

EXCDEF(_exceptions_exc14);
	EXCSTUB(14);
.size _exceptions_exc14, .-_exceptions_exc14

EXCDEF(_exceptions_exc15);
	pushl $0
	EXCSTUB(15);
.size _exceptions_exc15, .-_exceptions_exc15

EXCDEF(_exceptions_exc16);
	pushl $0
	EXCSTUB(16);
.size _exceptions_exc16, .-_exceptions_exc16

EXCDEF(_exceptions_exc17);
	EXCSTUB(17);
.size _exceptions_exc17, .-_exceptions_exc17

EXCDEF(_exceptions_exc18);
	pushl $0
	EXCSTUB(18);
.size _exceptions_exc18, .-_exceptions_exc18

EXCDEF(_exceptions_exc19);
	EXCSTUB(19);
.size _exceptions_exc19, .-_exceptions_exc19

EXCDEF(_exceptions_exc20);
	EXCSTUB(20);
.size _exceptions_exc20, .-_exceptions_exc20

EXCDEF(_exceptions_exc21);
	EXCSTUB(21);
.size _exceptions_exc21, .-_exceptions_exc21

EXCDEF(_exceptions_exc22);
	EXCSTUB(22);
.size _exceptions_exc22, .-_exceptions_exc22

EXCDEF(_exceptions_exc23);
	EXCSTUB(23);
.size _exceptions_exc23, .-_exceptions_exc23

EXCDEF(_exceptions_exc24);
	EXCSTUB(24);
.size _exceptions_exc24, .-_exceptions_exc24

EXCDEF(_exceptions_exc25);
	EXCSTUB(25);
.size _exceptions_exc25, .-_exceptions_exc25

EXCDEF(_exceptions_exc26);
	EXCSTUB(26);
.size _exceptions_exc26, .-_exceptions_exc26

EXCDEF(_exceptions_exc27);
	EXCSTUB(27);
.size _exceptions_exc27, .-_exceptions_exc27

EXCDEF(_exceptions_exc28);
	EXCSTUB(28);
.size _exceptions_exc28, .-_exceptions_exc28

EXCDEF(_exceptions_exc29);
	EXCSTUB(29);
.size _exceptions_exc29, .-_exceptions_exc29

EXCDEF(_exceptions_exc30);
	EXCSTUB(30);
.size _exceptions_exc30, .-_exceptions_exc30

EXCDEF(_exceptions_exc31);
	EXCSTUB(31);
.size _exceptions_exc31, .-_exceptions_exc31
