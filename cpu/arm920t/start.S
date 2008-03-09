/*
 *  armboot - Startup Code for ARM920 CPU-core
 *
 *  Copyright (c) 2001	Marius Gröger <mag@sysgo.de>
 *  Copyright (c) 2002	Alex Züpke <azu@sysgo.de>
 *  Copyright (c) 2002	Gary Jennejohn <gj@denx.de>
 *
 * S3C2410 NAND portions
 *  Copyright (c) 2001  MIZI Research, Inc.
 *  Copyright (c) 2006  OpenMoko, Inc. (Harald Welte <laforge@openmmoko.org>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */


#include <config.h>
#include <version.h>
#include <s3c2410.h>
#include <status_led.h>

/*
 *************************************************************************
 *
 * Jump vector table as in table 3.1 in [1]
 *
 *************************************************************************
 */


.globl _start
_start:	b       start_code
	ldr	pc, _undefined_instruction
	ldr	pc, _software_interrupt
	ldr	pc, _prefetch_abort
	ldr	pc, _data_abort
	ldr	pc, _not_used
	ldr	pc, _irq
	ldr	pc, _fiq

_undefined_instruction:	.word undefined_instruction
_software_interrupt:	.word software_interrupt
_prefetch_abort:	.word prefetch_abort
_data_abort:		.word data_abort
_not_used:		.word not_used
_irq:			.word irq
_fiq:			.word fiq

	.balignl 16,0xdeadbeef


/*
 *************************************************************************
 *
 * Startup Code (called from the ARM reset exception vector)
 *
 * do important init only if we don't start from memory!
 * relocate armboot to ram
 * setup stack
 * jump to second stage
 *
 *************************************************************************
 */


/* Must follow the .balign above, so we get a well-known address ! */
#ifdef CFG_PREBOOT_OVERRIDE
.globl	preboot_override
preboot_override:
	.word	0
#endif

/* Must follow preboot_override , so we get a well-known address ! */
#ifdef CFG_ENV_OVERRIDE
.globl	env_override
env_override:
	.word	0
#endif

#ifdef CONFIG_S3C2410_NAND_BOOT
.globl	booted_from_nand
booted_from_nand:
	.word	0
_booted_from_nand:
	.word	booted_from_nand
#endif /* CONFIG_S3C2410_NAND_BOOT */

_TEXT_BASE:
	.word	TEXT_BASE

.globl _armboot_start
_armboot_start:
	.word _start

/*
 * These are defined in the board-specific linker script.
 */
.globl _bss_start
_bss_start:
	.word __bss_start

.globl _bss_end
_bss_end:
	.word _end

#ifdef CONFIG_USE_IRQ
/* IRQ stack memory (calculated at run-time) */
.globl IRQ_STACK_START
IRQ_STACK_START:
	.word	0x0badc0de

/* IRQ stack memory (calculated at run-time) */
.globl FIQ_STACK_START
FIQ_STACK_START:
	.word 0x0badc0de
#endif


/*
 * the actual start code
 */

start_code:
	/*
	 * set the cpu to SVC32 mode
	 */
	mrs	r0,cpsr
	bic	r0,r0,#0x1f
	orr	r0,r0,#0xd3
	msr	cpsr,r0

	/* in case we run from the s3c24xx NAND stepping stone, the symbols
	 * for LED support are in lib_arm/board.o, i.e. outside of the
	 * steppingstone */
#ifndef CONFIG_S3C2410_NAND_BOOT
	bl coloured_LED_init
	bl red_LED_on
#endif

#if	defined(CONFIG_AT91RM9200DK) || defined(CONFIG_AT91RM9200EK) || defined(CONFIG_AT91RM9200DF)
	/*
	 * relocate exception table
	 */
	ldr	r0, =_start
	ldr	r1, =0x0
	mov	r2, #16
copyex:
	subs	r2, r2, #1
	ldr	r3, [r0], #4
	str	r3, [r1], #4
	bne	copyex
#endif

#if defined(CONFIG_S3C2400) || defined(CONFIG_S3C2410)
	/* turn off the watchdog */

# if defined(CONFIG_S3C2400)
#  define pWTCON		0x15300000
#  define INTMSK		0x14400008	/* Interupt-Controller base addresses */
#  define CLKDIVN	0x14800014	/* clock divisor register */
#else
#  define pWTCON		0x53000000
#  define INTMSK		0x4A000008	/* Interupt-Controller base addresses */
#  define INTSUBMSK	0x4A00001C
#  define CLKDIVN	0x4C000014	/* clock divisor register */
# endif

	ldr     r0, =pWTCON
	mov     r1, #0x0
	str     r1, [r0]

	/*
	 * mask all IRQs by setting all bits in the INTMR - default
	 */
	mov	r1, #0xffffffff
	ldr	r0, =INTMSK
	str	r1, [r0]
# if defined(CONFIG_S3C2410)
	ldr	r1, =0x3ff
	ldr	r0, =INTSUBMSK
	str	r1, [r0]
# endif

	/* default FCLK is 202 MHz ! */
#define LOCKTIME	0x4c000000
#define UPLLCON		0x4c000008
//#define MPLLCFG		((0x90 << 12) + (0x2 << 4) + 0x2)
#define MPLLCFG		((0x90 << 12) + (0x7 << 4) + 0x0)
#define UPLLCFG		((0x78 << 12) + (0x2 << 4) + 0x3)
	ldr	r0, =LOCKTIME
	mov	r1, #0xffffff
	str	r1, [r0]

	ldr	r0, =UPLLCON
	ldr	r1, =UPLLCFG
	str	r1, [r0]

	/* Page 7-19, seven nops between UPLL and MPLL */
	nop
	nop
	nop
	nop
	nop
	nop
	nop

	ldr	r1, =MPLLCFG
	str	r1, [r0, #-4]		/* MPLLCON */

	/* FCLK:HCLK:PCLK = 1:2:4 */
	ldr	r0, =CLKDIVN
	mov	r1, #3
	str	r1, [r0]

#if 1
	/* enable uart */
	ldr	r0, =0x4c00000c		/* clkcon */
	ldr	r1, =0x7fff0		/* all clocks on */
	str	r1, [r0]

	/* gpio UART0 init */
	ldr	r0, =0x56000070
	mov	r1, #0xaa
	str	r1, [r0]

	/* init uart */
	ldr	r0, =0x50000000
	mov	r1, #0x03
	str	r1, [r0]
	ldr	r1, =0x245
	str	r1, [r0, #0x04]
	mov	r1, #0x01
	str	r1, [r0, #0x08]
	mov	r1, #0x00
	str	r1, [r0, #0x0c]
	mov	r1, #0x1a
	str	r1, [r0, #0x28]
#endif

#endif	/* CONFIG_S3C2400 || CONFIG_S3C2410 */

#ifndef CONFIG_SKIP_LOWLEVEL_INIT
#ifndef CONFIG_LL_INIT_NAND_ONLY
	bl	cpu_init_crit
#endif

#if defined(CONFIG_AT91RM9200) || defined(CONFIG_S3C2410)

#ifndef CONFIG_SKIP_RELOCATE_UBOOT
	adr	r0, _start		/* r0 <- current position of code   */

#ifdef CONFIG_S3C2410_NAND_BOOT
					/* are we running from NAND ?	    */
#define	BWSCON	0x48000000
	ldr	r1, =BWSCON		/* Z = CPU booted from NAND	    */
	ldr	r1, [r1]
	tst	r1, #6			/* BWSCON[2:1] = OM[1:0]	    */
	teqeq	r0, #0			/* Z &= running at address 0	    */
	beq	nand_load
#endif /* CONFIG_S3C2410_NAND_BOOT */

relocate:				/* relocate U-Boot to RAM	    */
	teq	r0, #0			/* running at address 0 ?	    */
	bleq	may_resume		/* yes -> do low-level setup	    */

	adr	r0, _start		/* the above may have clobbered r0  */

	ldr	r1, _TEXT_BASE		/* test if we run from flash or RAM */
	cmp     r0, r1                  /* don't reloc during debug         */
	beq     done_relocate

	ldr	r2, _armboot_start
	ldr	r3, _bss_start
	sub	r2, r3, r2		/* r2 <- size of armboot            */
	add	r2, r0, r2		/* r2 <- source end address         */

copy_loop:
	ldmia	r0!, {r3-r10}		/* copy from source address [r0]    */
	stmia	r1!, {r3-r10}		/* copy to   target address [r1]    */
	cmp	r0, r2			/* until source end address [r2]    */
	ble	copy_loop
	mov	r0, #0			/* flush v3/v4 cache */
	mcr	p15, 0, r0, c7, c7, 0
	ldr	pc, _done_relocate	/* jump to relocated code */
_done_relocate:
	.word	done_relocate

#ifdef CONFIG_S3C2410_NAND_BOOT
nand_load:
	bl	may_resume		/* low-level setup and resume */

	@ reset NAND
	mov	r1, #S3C2410_NAND_BASE
	ldr	r2, =0xf842		@ initial value enable tacls=3,rph0=6,rph1=0
	str	r2, [r1, #oNFCONF]
	ldr	r2, [r1, #oNFCONF]
	bic	r2, r2, #0x800		@ enable chip
	str	r2, [r1, #oNFCONF]
	mov	r2, #0xff		@ RESET command
	strb	r2, [r1, #oNFCMD]
	mov	r3, #0			@ wait
1:	add	r3, r3, #0x1
	cmp	r3, #0xa
	blt	1b
2:	ldr	r2, [r1, #oNFSTAT]	@ wait ready
	tst	r2, #0x1
	beq	2b
	ldr	r2, [r1, #oNFCONF]
	orr	r2, r2, #0x800		@ disable chip
	str	r2, [r1, #oNFCONF]

	ldr	r0, _TEXT_BASE		/* upper 128 KiB: relocated uboot   */
	sub	r0, r0, #CFG_MALLOC_LEN	/* malloc area                      */
	sub	r0, r0, #CFG_GBL_DATA_SIZE /* bdinfo                        */
#ifdef CONFIG_USE_IRQ
	sub	r0, r0, #(CONFIG_STACKSIZE_IRQ+CONFIG_STACKSIZE_FIQ)
#endif
	sub	sp, r0, #12		/* leave 3 words for abort-stack    */

	@ copy u-boot to RAM
	ldr	r0, _TEXT_BASE
	mov     r1, #0x0
	mov	r2, #CFG_UBOOT_SIZE
	bl	nand_read_ll

	tst	r0, #0x0
	beq	ok_nand_read
#ifdef CONFIG_DEBUG_LL
bad_nand_read:
	ldr	r0, STR_FAIL
	ldr	r1, SerBase
	bl	PrintWord
1:	b	1b		@ infinite loop
#endif

ok_nand_read:
#ifdef CONFIG_DEBUG_LL
	ldr	r0, STR_OK
	ldr	r1, SerBase
	bl	PrintWord
#endif

	@ verify
	mov	r0, #0
	@ldr	r1, =0x33f00000
	ldr	r1, _TEXT_BASE
	mov	r2, #0x400	@ 4 bytes * 1024 = 4K-bytes
go_next:
	ldr	r3, [r0], #4
	ldr	r4, [r1], #4
	teq	r3, r4
	bne	notmatch
	subs	r2, r2, #4
	beq	done_nand_read
	bne	go_next
notmatch:
#ifdef CONFIG_DEBUG_LL
	sub	r0, r0, #4
	ldr	r1, SerBase
	bl	PrintHexWord
	ldr	r0, STR_FAIL
	ldr	r1, SerBase
	bl	PrintWord
#endif
1:	b	1b
done_nand_read:
	ldr	r0, _booted_from_nand
	mov	r1, #1
	strb	r1, [r0]
#endif /* CONFIG_S3C2410_NAND_BOOT */
done_relocate:

#if defined(CONFIG_USE_IRQ) && defined(CONFIG_S3C2410)
	/* In the case of the S3C2410, if we've somehow magically (JTAG, ...)
	   ended up in RAM, then that ram is mapped to 0x30000000 and not 0.
	   So we need to copy the interrupt vectors, etc.  */

	mov	r0, #0
	ldr	r1, _TEXT_BASE
	mov	r2, #0x40
irqvec_cpy_next:
	ldr	r3, [r1], #4
	str	r3, [r0], #4
	subs	r2, r2, #4
	bne	irqvec_cpy_next
#endif /* CONFIG_USE_IRQ */

#endif /* CONFIG_SKIP_RELOCATE_UBOOT */
#endif
	/* Set up the stack						    */
stack_setup:
	ldr	r0, _TEXT_BASE		/* upper 128 KiB: relocated uboot   */
	sub	r0, r0, #CFG_MALLOC_LEN	/* malloc area                      */
	sub	r0, r0, #CFG_GBL_DATA_SIZE /* bdinfo                        */
#ifdef CONFIG_USE_IRQ
	sub	r0, r0, #(CONFIG_STACKSIZE_IRQ+CONFIG_STACKSIZE_FIQ)
#endif
	sub	sp, r0, #12		/* leave 3 words for abort-stack    */

clear_bss:
	ldr	r0, _bss_start		/* find start of bss segment        */
	ldr	r1, _bss_end		/* stop here                        */
	mov 	r2, #0x00000000		/* clear                            */

clbss_l:str	r2, [r0]		/* clear loop...                    */
	add	r0, r0, #4
	cmp	r0, r1
	ble	clbss_l

	ldr	pc, _start_armboot

_start_armboot:	.word start_armboot


/*
 *************************************************************************
 *
 * CPU_init_critical registers
 *
 * setup important registers
 * setup memory timing
 *
 *************************************************************************
 */


#ifndef CONFIG_SKIP_LOWLEVEL_INIT
cpu_init_crit:
	/*
	 * flush v4 I/D caches
	 */
	mov	r0, #0
	mcr	p15, 0, r0, c7, c7, 0	/* flush v3/v4 cache */
	mcr	p15, 0, r0, c8, c7, 0	/* flush v4 TLB */

	/*
	 * disable MMU stuff and caches
	 */
	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, r0, #0x00002300	@ clear bits 13, 9:8 (--V- --RS)
	bic	r0, r0, #0x00000087	@ clear bits 7, 2:0 (B--- -CAM)
	orr	r0, r0, #0x00000002	@ set bit 2 (A) Align
	orr	r0, r0, #0x00001000	@ set bit 12 (I) I-Cache
	mcr	p15, 0, r0, c1, c0, 0

	/*
	 * before relocating, we have to setup RAM timing
	 * because memory timing is board-dependend, you will
	 * find a lowlevel_init.S in your board directory.
	 */
	mov	ip, lr
#if	defined(CONFIG_AT91RM9200DK) || defined(CONFIG_AT91RM9200EK) || defined(CONFIG_AT91RM9200DF)

#else
	bl	lowlevel_init
#endif
	mov	lr, ip
	mov	pc, lr
#endif /* CONFIG_SKIP_LOWLEVEL_INIT */


/*
 *************************************************************************
 *
 * may_resume
 *
 * Bring up memory and check if we're coming out of suspend.
 *
 *************************************************************************
 */


may_resume:
	mov	r10, lr			/* we may call cpu_init_crit */

	/* take sdram out of power down */
	ldr	r0, =0x56000080		/* misccr */
	ldr	r1, [ r0 ]
	bic	r1, r1, #(S3C2410_MISCCR_nEN_SCLK0 | S3C2410_MISCCR_nEN_SCLK1 | S3C2410_MISCCR_nEN_SCLKE)
	str	r1, [ r0 ]

	/* ensure signals stabalise */
	mov	r1, #128
1:	subs	r1, r1, #1
	bpl	1b

#if !defined(CONFIG_SKIP_LOWLEVEL_INIT) && defined(CONFIG_LL_INIT_NAND_ONLY)
	bl	cpu_init_crit
#endif
#if defined(CONFIG_S3C2410)
	/* ensure some refresh has happened */
	ldr	r1, =0xfffff
1:	subs	r1, r1, #1
	bpl	1b

	/* test for resume */
	ldr	r1, =0x560000B4		/* gstatus2 */
	ldr	r0, [ r1 ]
	tst	r0, #0x02		/* is this resume from power down */
	ldrne	pc, [r1, #4]		/* gstatus3 */
#endif /* CONFIG_S3C2410 */
#endif /* CONFIG_SKIP_LOWLEVEL_INIT */

	mov	pc, r10

/*
 *************************************************************************
 *
 * Interrupt handling
 *
 *************************************************************************
 */

@
@ IRQ stack frame.
@
#define S_FRAME_SIZE	72

#define S_OLD_R0	68
#define S_PSR		64
#define S_PC		60
#define S_LR		56
#define S_SP		52

#define S_IP		48
#define S_FP		44
#define S_R10		40
#define S_R9		36
#define S_R8		32
#define S_R7		28
#define S_R6		24
#define S_R5		20
#define S_R4		16
#define S_R3		12
#define S_R2		8
#define S_R1		4
#define S_R0		0

#define MODE_SVC 0x13
#define I_BIT	 0x80

/*
 * use bad_save_user_regs for abort/prefetch/undef/swi ...
 * use irq_save_user_regs / irq_restore_user_regs for IRQ/FIQ handling
 */

	.macro	bad_save_user_regs
	sub	sp, sp, #S_FRAME_SIZE
	stmia	sp, {r0 - r12}			@ Calling r0-r12
	ldr	r2, _armboot_start
	sub	r2, r2, #(CONFIG_STACKSIZE+CFG_MALLOC_LEN)
	sub	r2, r2, #(CFG_GBL_DATA_SIZE+8)  @ set base 2 words into abort stack
	ldmia	r2, {r2 - r3}			@ get pc, cpsr
	add	r0, sp, #S_FRAME_SIZE		@ restore sp_SVC

	add	r5, sp, #S_SP
	mov	r1, lr
	stmia	r5, {r0 - r3}			@ save sp_SVC, lr_SVC, pc, cpsr
	mov	r0, sp
	.endm

	.macro	irq_save_user_regs
	sub	sp, sp, #S_FRAME_SIZE
	stmia	sp, {r0 - r12}			@ Calling r0-r12
	add     r7, sp, #S_PC
	stmdb   r7, {sp, lr}^                   @ Calling SP, LR
	str     lr, [r7, #0]                    @ Save calling PC
	mrs     r6, spsr
	str     r6, [r7, #4]                    @ Save CPSR
	str     r0, [r7, #8]                    @ Save OLD_R0
	mov	r0, sp
	.endm

	.macro	irq_restore_user_regs
	ldmia	sp, {r0 - lr}^			@ Calling r0 - lr
	mov	r0, r0
	ldr	lr, [sp, #S_PC]			@ Get PC
	add	sp, sp, #S_FRAME_SIZE
	subs	pc, lr, #4			@ return & move spsr_svc into cpsr
	.endm

	.macro get_bad_stack
	ldr	r13, _armboot_start		@ setup our mode stack
	sub	r13, r13, #(CONFIG_STACKSIZE+CFG_MALLOC_LEN)
	sub	r13, r13, #(CFG_GBL_DATA_SIZE+8) @ reserved a couple spots in abort stack

	str	lr, [r13]			@ save caller lr / spsr
	mrs	lr, spsr
	str     lr, [r13, #4]

	mov	r13, #MODE_SVC			@ prepare SVC-Mode
	@ msr	spsr_c, r13
	msr	spsr, r13
	mov	lr, pc
	movs	pc, lr
	.endm

	.macro get_irq_stack			@ setup IRQ stack
	ldr	sp, IRQ_STACK_START
	.endm

	.macro get_fiq_stack			@ setup FIQ stack
	ldr	sp, FIQ_STACK_START
	.endm

/*
 * exception handlers
 */
	.align  5
undefined_instruction:
	get_bad_stack
	bad_save_user_regs
	bl 	do_undefined_instruction

	.align	5
software_interrupt:
	get_bad_stack
	bad_save_user_regs
	bl 	do_software_interrupt

	.align	5
prefetch_abort:
	get_bad_stack
	bad_save_user_regs
	bl 	do_prefetch_abort

	.align	5
data_abort:
	get_bad_stack
	bad_save_user_regs
	bl 	do_data_abort

	.align	5
not_used:
	get_bad_stack
	bad_save_user_regs
	bl 	do_not_used

#ifdef CONFIG_USE_IRQ

	.align	5
irq:
	get_irq_stack
	irq_save_user_regs
	bl 	do_irq
	irq_restore_user_regs

	.align	5
fiq:
	get_fiq_stack
	/* someone ought to write a more effiction fiq_save_user_regs */
	irq_save_user_regs
	bl 	do_fiq
	irq_restore_user_regs

#else

	.align	5
irq:
	get_bad_stack
	bad_save_user_regs
	bl 	do_irq

	.align	5
fiq:
	get_bad_stack
	bad_save_user_regs
	bl 	do_fiq

#endif
