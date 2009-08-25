/*
 * (C) 2006-2007 by OpenMoko, Inc.
 * Author: Harald Welte <laforge@openmoko.org>
 *
 * based on existing S3C2410 startup code in u-boot:
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
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

#include <common.h>
#include <s3c2440.h>
#include <i2c.h>
#include <bootmenu.h>
#include <asm/atomic.h>

#include "../common/neo1973.h"
#include "../common/jbt6k74.h"

#include "pcf50633.h"

DECLARE_GLOBAL_DATA_PTR;

/* That many seconds the power key needs to be pressed to power up */
#define POWER_KEY_SECONDS	1

/* If the battery voltage is below this, we can't provide stable power */
#define	SAFE_POWER_MILLIVOLT	3400

#if defined(CONFIG_ARCH_GTA02_v1)
//#define M_MDIV	0x7f		/* Fout = 405.00MHz */
#define M_MDIV	0x7d		/* Fout = 399.00MHz */
#define M_PDIV	0x2
#define M_SDIV	0x1

#define U_M_MDIV	0x38
#define U_M_PDIV	0x2
#define U_M_SDIV	0x2
#else
#define M_MDIV 42
#define M_PDIV 1
#define M_SDIV 0
#define U_M_MDIV 88
#define U_M_PDIV 4
#define U_M_SDIV 2
#endif

extern void smedia3362_lcm_reset(int);
extern void glamo_core_init(void);

unsigned int neo1973_wakeup_cause;
extern unsigned char booted_from_nand;
extern unsigned char booted_from_nor;
extern int nobootdelay;
extern int udc_usb_maxcurrent;

char __cfg_prompt[20] = "GTA02vXX # ";

/*
 * In >GTA02v5, use gta02_revision to test for features, not
 * CONFIG_GTA02_REVISION or CONFIG_ARCH_GTA02_vX !
 */
int gta02_revision;

static uint16_t gpb_shadow = 0; /* to work around GTA02v5 LED bug */

int gta02_get_pcb_revision(void);

static inline void delay (unsigned long loops)
{
	__asm__ volatile ("1:\n"
	  "subs %0, %1, #1\n"
	  "bne 1b":"=r" (loops):"0" (loops));
}

enum gta02_led {
	GTA02_LED_PWR_ORANGE	= 0,
	GTA02_LED_PWR_BLUE	= 1,
	GTA02_LED_AUX_RED	= 2,
};

/*
 * Miscellaneous platform dependent initialisations
 */

static void cpu_speed(int mdiv, int pdiv, int sdiv, int clkdivn)
{
	S3C24X0_CLOCK_POWER * const clk_power = S3C24X0_GetBase_CLOCK_POWER();

	/* clock divide */
	clk_power->CLKDIVN = clkdivn;

	/* to reduce PLL lock time, adjust the LOCKTIME register */
	clk_power->LOCKTIME = 0xFFFFFF;

	/* configure MPLL */
	clk_power->MPLLCON = ((mdiv << 12) + (pdiv << 4) + sdiv);

	/* some delay between MPLL and UPLL */
	delay (4000);

	/* configure UPLL */
	clk_power->UPLLCON = ((U_M_MDIV << 12) + (U_M_PDIV << 4) + U_M_SDIV);

	/* some delay between MPLL and UPLL */
	delay (8000);
}

int board_init(void)
{
	S3C24X0_GPIO * const gpio = S3C24X0_GetBase_GPIO();

	/* FCLK = 200MHz values from cpu/arm920t/start.S */
	cpu_speed(142, 7, 1, 3); /* 200MHZ, 1:2:4 */

	/* set up the I/O ports */
#if CONFIG_GTA02_REVISION == 1
	gpio->GPACON = 0x007E1FFF;
	gpio->GPADAT |= (1 << 16);      /* Set GPA16 to high (nNAND_WP) */

	gpio->GPBCON = 0x00155555;
	gpio->GPBUP = 0x000007FF;

	gpio->GPCCON = 0x55551155;
	gpio->GPCUP = 0x0000FFFF;

	gpio->GPDCON = 0x55555555;
	gpio->GPDUP = 0x0000FFFF;

	gpio->GPECON = 0xAAAAAAAA;
	gpio->GPEUP = 0x0000FFFF;

	gpio->GPFCON = 0x0000AAAA;
	gpio->GPFUP = 0x000000FF;

	gpio->GPGCON = 0x013DFDFA;
	gpio->GPGUP = 0x0000FFFF;

	gpio->GPHCON = 0x0028AAAA;
	gpio->GPHUP = 0x000007FF;

	gpio->GPJCON = 0x1545541;
#elif CONFIG_GTA02_REVISION == 2
	gpio->GPACON = 0x007E1FFF;
	gpio->GPADAT |= (1 << 16);      /* Set GPA16 to high (nNAND_WP) */

	gpio->GPBCON = 0x00155555;
	gpio->GPBUP = 0x000007FF;

	gpio->GPCCON = 0x55415155;
	gpio->GPCUP = 0x0000FFFF;

	gpio->GPDCON = 0x55555555;
	gpio->GPDUP = 0x0000FFFF;

	gpio->GPECON = 0xAAAAAAAA;
	gpio->GPEUP = 0x0000FFFF;

	gpio->GPFCON = 0x0000AAAA;
	gpio->GPFUP = 0x000000FF;

	gpio->GPGCON = 0x0156FE7A;
	gpio->GPGUP = 0x0000FFFF;

	gpio->GPHCON = 0x001AAAAA;
	gpio->GPHUP = 0x000007FF;

	gpio->GPJCON = 0x1551544;
	gpio->GPJUP = 0x1ffff;
	gpio->GPJDAT |= (1 << 4);	/* Set GPJ4 to high (nGSM_EN) */
#elif CONFIG_GTA02_REVISION >= 3
	gpio->GPACON = 0x007E5FFF;
	gpio->GPADAT |= (1 << 16);      /* Set GPA16 to high (nNAND_WP) */

	gpio->GPBCON = 0x00155555;
	gpio->GPBUP = 0x000007FF;

	/*
	 * PCB rev index found on C13, C15, D0, D3 and D4.  These are NC or
	 * pulled up by 10K.  Therefore to ensure no current flows when they
	 * are not interrogated, we drive them high.  When we interrogate them
	 * we make them pulled them down inputs briefly and set them high op
	 * again afterwards.
	 */

	/* pulldown on "PIO_5" BT module to stop float when unpowered
	 * C13 and C15 are b0 and b1 of PCB rev index
	 */
	gpio->GPCCON = 0x55555155;
	gpio->GPCUP = 0x0000FFFF & ~(1 << 5);
	gpio->GPCDAT |= (1 << 13) | (1 << 15); /* index detect -> hi */

	/* D0, D3 and D4 are b2, b3 and b4 of PCB rev index */
	gpio->GPDCON = 0x55555555;
	gpio->GPDUP = 0x0000FFFF;
	gpio->GPDDAT |= (1 << 0) | (1 << 3) | (1 << 4); /* index detect -> hi */

	/* pulldown on GPE11 / SPIMISO0 - goes to debug board and will float */
	gpio->GPECON = 0xAAAAAAAA;
	gpio->GPEUP = 0x0000FFFF & ~(1 << 11);

	/* pulldown on GPF03: TP-4705+debug - debug conn will float */
	gpio->GPFCON = 0x0000AAAA;
	gpio->GPFUP = 0x000000FF & ~(1 << 3);

	gpio->GPGCON = 0x01AAFE79;
	gpio->GPGUP = 0x0000FFFF;

	/* pulldown on GPH08: UEXTCLK, just floats!
	 * pulldown GPH0 -- nCTS0 / RTS_MODEM -- floats when GSM off
	 * pulldown GPH3 -- RXD[0] / TX_MODEM -- floats when GSM off
	 */
	gpio->GPHCON = 0x001AAAAA;
	gpio->GPHUP = 0x000007FF & ~(1 << 8) & ~(1 << 0) & ~(1 << 3);

	/* pulldown on GPJ00: input, just floats! */
	/* pulldown on GPJ07: WLAN module WLAN_GPIO0, no ext pull */
	gpio->GPJCON = 0x1551544;
	gpio->GPJUP = 0x1ffff & ~(1 << 0) & ~(1 << 7);
	gpio->GPJDAT |= (1 << 4) | (1 << 6);
					/* Set GPJ4 to high (nGSM_EN) */
					/* Set GPJ6 to high (nDL_GSM) */
	gpio->GPJDAT &= ~(1 << 5);	/* Set GPJ5 to low 3D RST */
	gpio->GPJDAT &= ~(1 << 5);	/* Set GPJ5 to low 3D RST */

	/* leaving Glamo forced to Reset# active here killed
	 * U-Boot when you touched the memory region
	 */

	gpio->GPJDAT |= (1 << 5);	/* Set GPJ5 to high 3D RST */
#else
#error Please define GTA02 version
#endif

	/* arch number of SMDK2410-Board */
	gd->bd->bi_arch_number = MACH_TYPE_NEO1973_GTA02;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0x30000100;

	icache_enable();
	dcache_enable();

	/*
	 * Since the NOR at address 0 is replaced by SteppingStone when the AUX
	 * button is released, we would crash when an interrupt arrives (e.g.,
	 * on USB insertion).
	 *
	 * We solve this as follows: we copy the vector table to RAM at address
	 * 0x30000000 and then use the PID feature in the 920T MMU to map all
	 * addresses in the range 0x0....... to 0x3....... without actually
	 * setting up page mappings in the MMU. Thus, vectors are then
	 * retrieved from their location in RAM.
	 *
	 * Note that the mapping is done in lib_arm/interrupts.c, so that it
	 * automatically tracks whether we allow interrupts or not. This is
	 * particularly necessary when we boot, since the operating system may
	 * not expect to find this sort of mapping to be active.
	 */
#ifdef CONFIG_GTA02_REVISION
	{
		extern char _start;

		memcpy((void *) 0x30000000, &_start, 0x40);
	}
#endif
	return 0;
}

static void set_revision(void)
{
	int rev = gta02_get_pcb_revision();
	char buf[32];

	if (CONFIG_GTA02_REVISION < 5)
		gta02_revision = CONFIG_GTA02_REVISION;
	else {
		switch (rev) {
		case 0x000:
			gta02_revision = 5;
			break;
		case 0x001:
			gta02_revision = 6;
			break;
		default:
			printf("Unrecognized hardware revision 0x%03x. "
			    "Defaulting to GTA02v6.\n", rev);
			gta02_revision = 6;
		}
	}
	sprintf(__cfg_prompt, "GTA02v%d # ", gta02_revision);

#if 1 /* remove these after checking that Andy doesn't need them anymore */
	printf("PCB rev: 0x%03X\n", rev);
	/* expose in the env so we can add to kernel commandline */
	sprintf(buf, "0x%03X", rev);
	setenv("pcb_rev", buf);
#endif
}

static void poll_charger(void)
{
	if (pcf50633_read_charger_type() == 1000)
		pcf50633_usb_maxcurrent(1000);
	else /* track what the time-critical udc callback allows us */
		if (pcf50633_usb_last_maxcurrent != udc_usb_maxcurrent)
			pcf50633_usb_maxcurrent(udc_usb_maxcurrent);
}

static int have_int(uint8_t mask1, uint8_t mask2);

static void clear_pmu_int(void)
{
	S3C24X0_INTERRUPT * const intr = S3C24X0_GetBase_INTERRUPT();
	S3C24X0_GPIO * const gpio = S3C24X0_GetBase_GPIO();

	/* read the PMU's interrupt register and store what we found for later
	   use */
	have_int(0, 0);

	/* clear EINT9/GPG1 in the MCU's interrupt path */
	gpio->EINTPEND = 1 << 9;
	intr->SRCPND = BIT_EINT8_23;
	intr->INTPND = BIT_EINT8_23;
}

static void cpu_idle(void)
{
	S3C24X0_INTERRUPT * const intr = S3C24X0_GetBase_INTERRUPT();
	S3C24X0_GPIO * const gpio = S3C24X0_GetBase_GPIO();
	S3C24X0_CLOCK_POWER * const clk = S3C24X0_GetBase_CLOCK_POWER();
	unsigned long flags;

	/*
	 * We don't want to execute interrupts throughout all this, since
	 * u-boot's interrupt handling code isn't modular, and getting a "real"
	 * interrupt without clearing it in the interrupt handler would cause
	 * us to loop permanently.
	 */
	local_irq_save(flags);

	/* enable PMU interrupts */
	intr->INTMSK &= ~BIT_EINT8_23;
	gpio->EINTMASK &= ~(1 << 9);

	/* go idle */
	clk->CLKCON |= 1 << 2;

	 /* disable PMU interrupts */
	intr->INTMSK |= BIT_EINT8_23;
	gpio->EINTMASK |= 1 << 9;

	/* collect PMU interrupts and clear them */
	clear_pmu_int();

	/* we're safe now */
	local_irq_restore(flags);
}

static int charger_is_present(void)
{
	/* is charger or power adapter present? */
	return  !!(pcf50633_reg_read(PCF50633_REG_MBCS1) & 3);
}

static int battery_is_present(void)
{
	/* battery less than bvmlvl -> don't boot */
	return !(pcf50633_reg_read(PCF50633_REG_BVMCTL) & 1);
}

static int battery_is_good(void)
{
	/* battery is absent -> don't boot */
	if (!battery_is_present())
		return 0;

	/* we could try to boot, but we'll probably die on the way */
	if (pcf50633_read_battvolt() < SAFE_POWER_MILLIVOLT)
		return 0;

	return 1;
}

static int wait_for_power(void)
{
	/*
	 * TODO: this function should also check if charger is still attached
	 * it makes no sense to wait otherwise.
	*/

	int seconds = 0;
	int led_cycle = 1;
	int power = 1;

	while (1) {
		poll_charger();

		/* we have plenty of external power but no visible battery ->
		 * don't hang around trying to charge, try to boot */
		if (!battery_is_present() && (pcf50633_usb_last_maxcurrent >= 500))
			break;

		/* cpu_idle sits with interrupts off destroying USB operation
		 * don't run it unless we are in trouble
		 */
		if (!battery_is_good())
			cpu_idle();
		else
			udelay(1000000);

		if (neo1973_new_second()) {
			/*
			 * Probe the battery only if the current LED cycle is
			 * about to end, so that it had time to discharge.
			 */
			if (led_cycle && battery_is_good())
				break;

			/* check if charger is present, otherwise stop start up */
			if (!charger_is_present()) {
				power = 0;
				break;
			}
			seconds++;
		}

		led_cycle = !seconds || (seconds & 1);

		/*
		 * Blink the AUX LED, unless it's broken (which is the case in
		 * GTA02v5 it is) and draws excessive current, which we just
		 * can't afford in this delicate situation.
		 */
		if (gta02_revision > 5)
			neo1973_led(GTA02_LED_AUX_RED, led_cycle);

		/* alternate LED and charger cycles */
		pcf50633_reg_set_bit_mask(PCF50633_REG_MBCC1, 1, !led_cycle);

                /* cancel shutdown timer to keep charging
		 * it can get triggered by lowvsys along the way but if it
		 * didn't kill us then don't let it kill us later
		 */
                pcf50633_reg_write(PCF50633_REG_OOCSHDWN, 4);
	}

	pcf50633_reg_set_bit_mask(PCF50633_REG_MBCC1, 1, 1); /* charge ! */

	/* switch off the AUX LED */
	neo1973_led(GTA02_LED_AUX_RED, 0);

	/* do we have power now? */
	return power;
}

static void pcf50633_late_init(void)
{
#ifdef CONFIG_ARCH_GTA02_v1
	uint8_t pwren = 1;	/* always on */
	uint8_t recent = 0;	/* antiques don't have that */
#else
	uint8_t pwren = 2;	/* enabled if GPIO1 = HIGH */
	uint8_t recent = 1;	/* always on */
#endif

	pcf50633_reg_write(PCF50633_REG_LDO1ENA, pwren);
	pcf50633_reg_write(PCF50633_REG_LDO2ENA, 2); /* enabled if GPIO1 = H */
	pcf50633_reg_write(PCF50633_REG_LDO5ENA, recent);
	pcf50633_reg_write(PCF50633_REG_LDO6ENA, recent);

	pcf50633_reg_write(PCF50633_REG_MBCC5, 0xff); /* 1A USB fast charge */
}

int board_late_init(void)
{
	S3C24X0_GPIO * const gpio = S3C24X0_GetBase_GPIO();
	uint8_t int1, int2;
	char buf[32];
	int menu_vote = 0; /* <= 0: no, > 0: yes */
	int seconds = 0;
	int enter_bootmenu;
	char *env_stop_in_menu;

	set_revision();

	/* Initialize the Power Management Unit with a safe register set */
	pcf50633_init();

	/* obtain wake-up reason */
	int1 = pcf50633_reg_read(PCF50633_REG_INT1);
	int2 = pcf50633_reg_read(PCF50633_REG_INT2);

	/* if there's no other reason, must be regular reset */
	neo1973_wakeup_cause = NEO1973_WAKEUP_RESET;

	if (!booted_from_nand && !booted_from_nor)
		goto woken_by_reset;

	/* save wake-up reason in environment */
	sprintf(buf, "0x%02x", int1);
	setenv("pcf50633_int1", buf);
	sprintf(buf, "0x%02x", int2);
	setenv("pcf50633_int2", buf);

	if (int1 & PCF50633_INT1_ALARM) {
		/* we've been woken up by RTC alarm, boot */
		neo1973_wakeup_cause = NEO1973_WAKEUP_ALARM;
		goto continue_boot;
	}
	if (int1 & PCF50633_INT1_USBINS) {
		/* we've been woken up by charger insert */
		neo1973_wakeup_cause = NEO1973_WAKEUP_CHARGER;
	}

	if (int2 & PCF50633_INT2_ONKEYF) {
		/* we've been woken up by a falling edge of the onkey */
		neo1973_wakeup_cause = NEO1973_WAKEUP_POWER_KEY;
	}

	if (neo1973_wakeup_cause == NEO1973_WAKEUP_CHARGER) {
		/* if we still think it was only a charger insert, boot */
		goto continue_boot;
	}

woken_by_reset:

	while (neo1973_wakeup_cause == NEO1973_WAKEUP_RESET ||
	    neo1973_on_key_pressed()) {

        if (neo1973_aux_key_pressed())
			menu_vote++;
		else
			menu_vote--;

		if (neo1973_new_second())
			seconds++;
		if (seconds >= POWER_KEY_SECONDS)
			goto continue_boot;
	}
	/* Power off if minimum number of seconds not reached */
	neo1973_poweroff();

continue_boot:
	/* Power off if no battery is present and only 100mA is available */
	if (!wait_for_power())
		neo1973_poweroff();

	pcf50633_late_init();
	cpu_speed(M_MDIV, M_PDIV, M_SDIV, 5); /* 400MHZ, 1:4:8 */

	/* issue a short pulse with the vibrator */
	neo1973_led(GTA02_LED_AUX_RED, 1);
	neo1973_vibrator(1);
	udelay(20000);
	neo1973_led(GTA02_LED_AUX_RED, 0);
	neo1973_vibrator(0);

#if defined(CONFIG_ARCH_GTA02_v1)
	/* Glamo3362 reset and power cycle */
	gpio->GPJDAT &= ~0x000000001;	/* GTA02v1_GPIO_3D_RESET */
	pcf50633_reg_write(PCF50633_REG_DOWN2ENA, 0);
	udelay(50*1000);
	pcf50633_reg_write(PCF50633_REG_DOWN2ENA, 0x2);
	gpio->GPJDAT |= 0x000000001;	/* GTA02v1_GPIO_3D_RESET */
#endif

	env_stop_in_menu = getenv("stop_in_menu");
	/* If the stop_in_menu environment variable is set, enter the
	 * boot menu */
	if (env_stop_in_menu && strcmp(env_stop_in_menu, "yes") == 0)
		menu_vote = 1;

	enter_bootmenu = menu_vote > 0 || booted_from_nor;
	glamo_core_init();
	smedia3362_lcm_reset(1);
	if (!enter_bootmenu && getenv("splashimage"))
		run_command(getenv("splashimage"), 0);
	jbt6k74_init();
	jbt6k74_enter_state(JBT_STATE_NORMAL);
	jbt6k74_display_onoff(1);
	/* switch on the backlight */
	neo1973_backlight(1);

#if 0
	{
		/* check if sd card is inserted, and power-up if it is */
		S3C24X0_GPIO * const gpio = S3C24X0_GetBase_GPIO();
		if (!(gpio->GPFDAT & (1 << 5)))
			gpio->GPBDAT &= ~(1 << 2);
	}
#endif

	if (enter_bootmenu) {
		extern struct bootmenu_setup bootmenu_setup;

		if (booted_from_nand)
			bootmenu_setup.comment = "NAND";
		if (booted_from_nor)
			bootmenu_setup.comment = "NOR";
		neo1973_bootmenu();
		nobootdelay = 1;
	}

	return 0;
}

int dram_init (void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	return 0;
}

u_int32_t get_board_rev(void)
{
	return 0x300+0x10*gta02_revision;
}

void neo1973_poweroff(void)
{
	printf("poweroff\n");
	udc_disconnect();
	pcf50633_reg_write(PCF50633_REG_OOCSHDWN, 0x01);
	/* don't return to caller */
	while (1) ;
}

void neo1973_backlight(int on)
{
	if (on) {
		/* pcf50633 manual p60
		 * "led_out should never be set to 000000, as this would result
		 * in a deadlock making it impossible to program another value.
		 * If led_out should be inadvertently set to 000000, the
		 * LEDOUT register can be reset by disabling and enabling the
		 * LED converter via control bit led_on in the LEDENA register"
		 */
		pcf50633_reg_write(PCF50633_REG_LEDENA, 0x00);
		pcf50633_reg_write(PCF50633_REG_LEDENA, 0x01);
		pcf50633_reg_write(PCF50633_REG_LEDOUT, 0x3f);
	} else
		pcf50633_reg_write(PCF50633_REG_LEDENA, 0x00);
}

/* FIXME: shared */
void neo1973_vibrator(int on)
{
	S3C24X0_GPIO * const gpio = S3C24X0_GetBase_GPIO();
	if (on)
#if defined(CONFIG_ARCH_GTA01_v3) || defined(CONFIG_ARCH_GTA01_v4)
		gpio->GPGDAT |= (1 << 11);	/* GPG11 */
#elif defined(CONFIG_ARCH_GTA01B_v2) || defined(CONFIG_ARCH_GTA01B_v3)
		gpio->GPBDAT |= (1 << 10);	/* GPB10 */
#else
		gpio->GPBDAT |= (1 << 3);	/* GPB3 */
#endif
	else
#if defined(CONFIG_ARCH_GTA01_v3) || defined(CONFIG_ARCH_GTA01_v4)
		gpio->GPGDAT &= ~(1 << 11);	/* GPG11 */
#elif defined(CONFIG_ARCH_GTA01B_v2) || defined(CONFIG_ARCH_GTA01B_v3)
		gpio->GPBDAT &= ~(1 << 10);	/* GPB10 */
#else
		gpio->GPBDAT &= ~(1 << 3);	/* GPB3 */
#endif
	gpio->GPBDAT |= gpb_shadow;
}

void neo1973_gsm(int on)
{
	S3C24X0_GPIO * const gpio = S3C24X0_GetBase_GPIO();

	/* GPIO2 of PMU, GPB7(MODEM_ON)=1 and GPB5(MODEM_RST)=0 */
	if (on) {
#if !defined(CONFIG_ARCH_GTA02_v1)
		pcf50633_reg_write(PCF50633_REG_GPIO2CFG, 0x07);
#endif
		gpio->GPBDAT &= ~(1 << 5);	/* GTA02_GPIO_MODEM_RST */
		gpio->GPBDAT |= (1 << 7);	/* GTA02_GPIO_MODEM_ON */
		gpio->GPJDAT &= ~(1 << 6);	/* GTA02_GPIO_nDL_GSM */
	} else {
		gpio->GPBDAT &= ~(1 << 7);	/* GTA02_GPIO_MODEM_ON */
#if !defined(CONFIG_ARCH_GTA02_v1)
		pcf50633_reg_write(PCF50633_REG_GPIO2CFG, 0x00);
#endif
		gpio->GPJDAT |= (1 << 6);	/* GTA02_GPIO_nDL_GSM */
	}
}

void neo1973_gps(int on)
{
	if (on)
		pcf50633_reg_write(PCF50633_REG_LDO5ENA, 0x01);
	else
		pcf50633_reg_write(PCF50633_REG_LDO5ENA, 0x00);
}

static int pwr_int_pending(void)
{
	S3C24X0_GPIO * const gpio = S3C24X0_GetBase_GPIO();

	return !(gpio->GPGDAT & (1 << 1));	/* EINT9/GPG1 */
}

static int have_int(uint8_t mask1, uint8_t mask2)
{
	static uint8_t pending1 = 0, pending2 = 0;

	if (pwr_int_pending()) {
		/*
		 * We retrieve all interupts, so that we clear any stray ones
		 * in INT2 and INT3.
		 */
		uint8_t ints[5];
		int i;

		for (i = 0; i != 5; i++)
			ints[i] = pcf50633_reg_read(PCF50633_REG_INT1+i);
		pending1 |= ints[0];
		pending2 |= ints[1];
	}
	if (pending1 & mask1) {
		pending1 &= ~mask1;
		return 1;
	}
	if (pending2 & mask2) {
		pending1 &= ~mask2;
		return 1;
	}
	return 0;
}

int neo1973_new_second(void)
{
	return have_int(PCF50633_INT1_SECOND, 0);
}

int neo1973_on_key_pressed(void)
{
	static int pressed = -1;

	if (pressed == -1 ||
	    have_int(0, PCF50633_INT2_ONKEYF | PCF50633_INT2_ONKEYR))
		pressed = !(pcf50633_reg_read(PCF50633_REG_OOCSTAT) &
		    PCF50633_OOCSTAT_ONKEY);
	return pressed;
}

int neo1973_aux_key_pressed(void)
{
	S3C24X0_GPIO * const gpio = S3C24X0_GetBase_GPIO();

	return !!(gpio->GPFDAT & (1 << 6));
}

/* The sum of all part_size[]s must equal to or greater than the NAND size,
   i.e., 0x10000000. */

unsigned int dynpart_size[] = {
    CFG_UBOOT_SIZE, CFG_ENV_SIZE, 0x800000, 0xa0000, 0x40000, 0x10000000, 0 };
char *dynpart_names[] = {
    "u-boot", "u-boot_env", "kernel", "splash", "factory", "rootfs", NULL };


const char *neo1973_get_charge_status(void)
{
	/* FIXME */
	return pcf50633_charger_state();
}

int neo1973_set_charge_mode(enum neo1973_charger_cmd cmd)
{
	/* FIXME */
	puts("not implemented yet\n");
	return -1;
}

void neo1973_led(int led, int on)
{
	S3C24X0_GPIO * const gpio = S3C24X0_GetBase_GPIO();

	if (led > 2)
		return;

	if (on) {
		gpb_shadow |= (1 << led);
		gpio->GPBDAT |= gpb_shadow;
	}
	else {
		gpb_shadow &= ~(1 << led);
		gpio->GPBDAT = (gpio->GPBDAT | gpb_shadow) & ~(1 << led);
	}
}

/**
 * returns PCB revision information in b9,b8 and b2,b1,b0
 * Pre-GTA02 A6 returns 0x000
 *     GTA02 A6 returns 0x001
 */

int gta02_get_pcb_revision(void)
{
	S3C24X0_GPIO * const gpio = S3C24X0_GetBase_GPIO();
	int n;
	u32 u;

	/* make C13 and C15 pulled-down inputs */
	gpio->GPCCON &= ~0xcc000000;
	gpio->GPCUP  &= ~((1 << 13) | (1 << 15));
	/* D0, D3 and D4 pulled-down inputs */
	gpio->GPDCON &= ~0x000003c3;
	gpio->GPDUP  &= ~((1 << 0) | (1 << 3) | (1 << 4));

	/* delay after changing pulldowns */
	u = gpio->GPCDAT;
	u = gpio->GPDDAT;

	/* read the version info */
	u = gpio->GPCDAT;
	n =  (u >> (13 - 0)) & 0x001;
	n |= (u >> (15 - 1)) & 0x002;
	u = gpio->GPDDAT;
	n |= (u << (0 + 2))  & 0x004;

	n |= (u << (8 - 3))  & 0x100;
	n |= (u << (9 - 4))  & 0x200;

	/*
	 * when not being interrogated, all of the revision GPIO
	 * are set to output HIGH without pulldown so no current flows
	 * if they are NC or pulled up.
	 */
	/* make C13 and C15 high ouputs with no pulldowns */
	gpio->GPCCON |= 0x44000000;
	gpio->GPCUP  |= (1 << 13) | (1 << 15);
	gpio->GPCDAT |= (1 << 13) | (1 << 15);
	/* D0, D3 and D4 high ouputs with no pulldowns */
	gpio->GPDCON |= 0x00000141;
	gpio->GPDUP  |= (1 << 0) | (1 << 3) | (1 << 4);
	gpio->GPDDAT |= (1 << 0) | (1 << 3) | (1 << 4);

	return n;
}
