
_start:
	b 	reset
undefvec:
	b	undefvec
swivec:
	b	swivec
pabtvec:
	b	pabtvec
dabtvec:
	b	dabtvec
rsvdvec:
	b	rsvdvec
irqvec:
	b	irqvec
fiqvec:
	b	fiqvec

reset:
	/*
	 * set the cpu to SVC32 mode
	 */
	mrs	r0,cpsr
	bic	r0,r0,#0x1f
	orr	r0,r0,#0xd3
	msr	cpsr,r0

/* turn off the watchdog */
#define pWTCON		0x53000000
#define INTMSK		0x4A000008	/* Interupt-Controller base addresses */
#define INTSUBMSK	0x4A00001C
#define CLKDIVN	0x4C000014	/* clock divisor register */

	ldr     r0, =pWTCON
	mov     r1, #0x0
	str     r1, [r0]

	mov	r1, #0xffffffff
	ldr	r0, =INTMSK
	str	r1, [r0]
	ldr	r1, =0x3ff
	ldr	r0, =INTSUBMSK
	str	r1, [r0]

	/* FCLK:HCLK:PCLK = 1:2:4 */
	/* default FCLK is 120 MHz ! */
	ldr	r0, =CLKDIVN
	mov	r1, #3
	str	r1, [r0]

	bl	cpu_init_crit
	ldr	r0,=TEXT_BASE
	mov	pc, r0

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
	bl	lowlevel_init
	mov	lr, ip
	mov	pc, lr

