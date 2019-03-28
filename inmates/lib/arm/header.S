/*
 * Jailhouse, a Linux-based partitioning hypervisor
 *
 * Copyright (c) ARM Limited, 2014
 * Copyright (c) Siemens AG, 2016
 *
 * Authors:
 *  Jean-Philippe Brucker <jean-philippe.brucker@arm.com>
 *  Jan Kiszka <jan.kiszka@siemens.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 *
 * Alternatively, you can use or redistribute this file under the following
 * BSD license:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <asm/sysregs.h>

	.arm

	.section ".boot", "ax"
	.align 5
vectors:
	b	__reset_entry
	b	__vector_undef
	b	vector_svc
	b	vector_pabt
	b	vector_dabt
	b	vector_unused
	b	__sched_irq
	b	vector_fiq

.macro vector, name
	.weak vector_\name
	vector_\name:
	b	.
.endm
	vector undef
	vector svc
	//vector pabt
	//vector dabt
	vector unused
	//vector irq
	vector fiq

__sched_irq:
	/*lr point to the interrupted instruction*/
	sub	lr,	lr, #4
	push {r0-r12, lr}
	bl vector_irq
	ldr r1, =need_schedule
	ldr r0, [r1]
	cmp r0, #0
	beq 1f

	/*clear schedure flag*/
	mov r0, #0
	str r0, [r1]

	pop	{r0-r12, r14}
	srsdb	sp!, 0x13 /* Save LR_irq and SPSR_irq to SVC mode stack */
	cps		0x13		/* Switch to SVC mode */
	push {r0-r12, r14}

	/*save stack pointer to current TCB*/
	ldr r1, =curr_tcb
	ldr r0, [r1]
	str sp, [r0]

	/*load stack pointer from next TCB*/
	ldr r3, =next_tcb
	ldr r2, [r3]
	ldr sp, [r2]

	/* curr_tcb = next_tcb*/
	str r2, [r1]
	pop {r0-r12, r14}
	/*cannot use rfeia, because both os and app code is on svc mode,
	 * not usr or sys mode
	 */
	//rfeia sp!	/* Return using RFE from SVC mode stack */
	ldr	r6,	[sp], #4	/*pc*/
	ldr	r7,	[sp], #4	/*cpsr*/
	msr cpsr, r7
	/* check task is in thumb or ARM state*/
	tst r7, #0x20
	bne	2f
	/*To ARM state*/
	bx r6
2:
	/*add #1 to pc, exchange to thumb state*/
	add r6, r6, #1
	bx r6
1:
	pop {r0-r12, r14}
	movs pc,lr

__vector_undef:
	mov r0, lr
	mrs r1, spsr
	mrs r2, cpsr
	b	.

vector_pabt:
	mov r0, lr
	mrs r1, spsr
	b	.

vector_dabt:
	mov r0, lr
	mrs r1, spsr
	b	.

	.globl __reset_entry
__reset_entry:
	ldr	r0, =vectors
	arm_write_sysreg(VBAR, r0)

	mov	r0, #0
	ldr	r1, =bss_start
	ldr	r2, =bss_dwords
	cmp	r2, #0
	beq	2f

1:	str	r0, [r1]
	add	r1, #4
	subs	r2, #1
	bne	1b

2:	ldr	sp, =stack_top

	b	c_entry

	.ltorg
