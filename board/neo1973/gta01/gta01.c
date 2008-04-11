/*
 * (C) 2006 by OpenMoko, Inc.
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
#include <devices.h>
#include <s3c2410.h>
#include <i2c.h>

#include "pcf50606.h"

#include "../common/neo1973.h"
#include "../common/jbt6k74.h"

DECLARE_GLOBAL_DATA_PTR;

/* That many seconds the power key needs to be pressed to power up */
#define POWER_KEY_SECONDS	2

#if defined(CONFIG_ARCH_GTA01_v3) || defined(CONFIG_ARCH_GTA01_v4)
//#define M_MDIV	0xA1		/* Fout = 202.8MHz */
//#define M_PDIV	0x3
//#define M_SDIV	0x1
#define M_MDIV	0x90		/* Fout = 202.8MHz */
#define M_PDIV	0x7
#define M_SDIV	0x0
#elif defined(CONFIG_ARCH_GTA01B_v2) || defined(CONFIG_ARCH_GTA01B_v3)
/* In case the debug board is attached, we cannot go beyond 200 MHz */
#if 0
#define M_MDIV	0x7d		/* Fout = 266MHz */
#define M_PDIV	0x1
#define M_SDIV	0x1
#else
#define M_MDIV	0x90		/* Fout = 202.8MHz */
#define M_PDIV	0x7
#define M_SDIV	0x0
#endif
#elif defined(CONFIG_ARCH_GTA01B_v4)
/* This board doesn't have bus lines at teh debug port, and we can go to 266 */
#define M_MDIV	0x7d		/* Fout = 266MHz */
#define M_PDIV	0x1
#define M_SDIV	0x1
#else
#error Please define GTA01 revision
#endif

#define U_M_MDIV	0x78
#define U_M_PDIV	0x2
#define U_M_SDIV	0x3

unsigned int neo1973_wakeup_cause;
extern int nobootdelay;

static inline void delay (unsigned long loops)
{
	__asm__ volatile ("1:\n"
	  "subs %0, %1, #1\n"
	  "bne 1b":"=r" (loops):"0" (loops));
}

/*
 * Miscellaneous platform dependent initialisations
 */

int board_init (void)
{
	S3C24X0_CLOCK_POWER * const clk_power = S3C24X0_GetBase_CLOCK_POWER();
	S3C24X0_GPIO * const gpio = S3C24X0_GetBase_GPIO();

	/* to reduce PLL lock time, adjust the LOCKTIME register */
	clk_power->LOCKTIME = 0xFFFFFF;

	/* configure MPLL */
	clk_power->MPLLCON = ((M_MDIV << 12) + (M_PDIV << 4) + M_SDIV);

	/* some delay between MPLL and UPLL */
	delay (4000);

	/* configure UPLL */
	clk_power->UPLLCON = ((U_M_MDIV << 12) + (U_M_PDIV << 4) + U_M_SDIV);

	/* some delay between MPLL and UPLL */
	delay (8000);

	/* set up the I/O ports */
#if defined(CONFIG_ARCH_GTA01_v3)
	gpio->GPACON = 0x007FFFFF;

	gpio->GPBCON = 0x00005055;
	gpio->GPBUP = 0x000007FF;

	gpio->GPCCON = 0xAAAA12A8;
	gpio->GPCUP = 0x0000FFFF;

	gpio->GPDCON = 0xAAAAAAAA;
	gpio->GPDUP = 0x0000FFFF;

	gpio->GPECON = 0xAAAAAAAA;
	gpio->GPEUP = 0x0000FFFF;

	gpio->GPFCON = 0x00002AA9;
	gpio->GPFUP = 0x000000FF;

	gpio->GPGCON = 0xA846F0C0;
	gpio->GPGUP = 0x0000AFEF;

	gpio->GPHCON = 0x0008FAAA;
	gpio->GPHUP = 0x000007FF;
#elif defined(CONFIG_ARCH_GTA01_v4)
	gpio->GPACON = 0x005E47FF;

	gpio->GPBCON = 0x00045015;
	gpio->GPBUP = 0x000007FF;
	gpio->GPBDAT |= 0x4;		/* Set GPB2 to high (Flash power-up) */

	gpio->GPCCON = 0xAAAA12A9;
	gpio->GPCUP = 0x0000FFFF;

	gpio->GPDCON = 0xAAAAAAAA;
	gpio->GPDUP = 0x0000FFFF;

	gpio->GPECON = 0xA02AAAAA;
	gpio->GPEUP = 0x0000FFFF;

	gpio->GPFCON = 0x0000aa09;
	gpio->GPFUP = 0x000000FF;

	gpio->GPGCON = 0xFF40F0C1;
	gpio->GPGUP = 0x0000AFEF;

	gpio->GPHCON = 0x0000FAAA;
	gpio->GPHUP = 0x000007FF;
#elif defined(CONFIG_ARCH_GTA01B_v2) || defined(CONFIG_ARCH_GTA01B_v3)
	gpio->GPACON = 0x005E4FFF;

	gpio->GPBCON = 0x00145415;
	gpio->GPBUP = 0x000007FF;
	gpio->GPBDAT |= 0x4;		/* Set GPB2 to high (Flash power-up) */

	gpio->GPCCON = 0xAAAA12A9;
	gpio->GPCUP = 0x0000FFFF;

	gpio->GPDCON = 0xAAAAAAAA;
	gpio->GPDUP = 0x0000FFFF;

	gpio->GPECON = 0xA02AAAAA;
	gpio->GPEUP = 0x0000FFFF;

	gpio->GPFCON = 0x0000aa19;
	gpio->GPFUP = 0x000000FF;
	gpio->GPFDAT |= 0x4;		/* Set GBF2 to high (nGSM_EN) */

	gpio->GPGCON = 0xFF40F0C1;
	gpio->GPGUP = 0x0000AFEF;

	gpio->GPHCON = 0x0000FAAA;
	gpio->GPHUP = 0x000007FF;
#elif defined(CONFIG_ARCH_GTA01B_v4)
	gpio->GPACON = 0x0005E0FFF;
	gpio->GPADAT |= (1 << 16);	/* Set GPA16 to high (nNAND_WP) */

	gpio->GPBCON = 0x00045455;
	gpio->GPBUP = 0x000007FF;
	gpio->GPBDAT |= 0x4;            /* Set GPB2 to high (SD power down) */

	gpio->GPCCON = 0xAAAA12A9;
	gpio->GPCUP = 0x0000FFFF;

	gpio->GPDCON = 0xAAAAAAAA;
	gpio->GPDUP = 0x0000FFFF;

	gpio->GPECON = 0xAAAAAAAA;
	gpio->GPEUP = 0x0000FFFF;

	gpio->GPFCON = 0x0000aa99;
	gpio->GPFUP = 0x000000FF;
	gpio->GPFDAT |= 0x4;		/* Set GBF2 to high (nGSM_EN) */

	gpio->GPGCON = 0xFF14F0F8;
	gpio->GPGUP = 0x0000AFEF;

	gpio->GPHCON = 0x0000FAAA;
	gpio->GPHUP = 0x000007FF;
#else
#error Please define GTA01 version
#endif

	/* arch number of SMDK2410-Board */
	gd->bd->bi_arch_number = MACH_TYPE_NEO1973_GTA01;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0x30000100;

	icache_enable();
	dcache_enable();

	return 0;
}

int board_late_init(void)
{
	extern unsigned char booted_from_nand;
	unsigned char tmp;
	char buf[32];
	int menu_vote = 0; /* <= 0: no, > 0: yes */
	int seconds = 0;

	/* Initialize the Power Management Unit with a safe register set */
	pcf50606_init();

	/* if there's no other reason, must be regular reset */
	neo1973_wakeup_cause = NEO1973_WAKEUP_RESET;

	if (!booted_from_nand)
		goto woken_by_reset;

	/* obtain wake-up reason, save INT1 in environment */
	tmp = pcf50606_reg_read(PCF50606_REG_INT1);
	sprintf(buf, "0x%02x", tmp);
	setenv("pcf50606_int1", buf);

	if (tmp & PCF50606_INT1_ALARM) {
		/* we've been woken up by RTC alarm, boot */
		neo1973_wakeup_cause = NEO1973_WAKEUP_ALARM;
		goto continue_boot;
	}
	if (tmp & PCF50606_INT1_EXTONR) {
		/* we've been woken up by charger insert */
		neo1973_wakeup_cause = NEO1973_WAKEUP_CHARGER;
	}

	if (tmp & PCF50606_INT1_ONKEYF) {
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
	jbt6k74_init();
	jbt6k74_enter_state(JBT_STATE_NORMAL);
	jbt6k74_display_onoff(1);

	/* issue a short pulse with the vibrator */
	neo1973_vibrator(1);
	udelay(50000);
	neo1973_vibrator(0);

	/* switch on the backlight */
	neo1973_backlight(1);

#if defined(CONFIG_ARCH_GTA01B_v4)
	{
		/* check if sd card is inserted, and power-up if it is */
		S3C24X0_GPIO * const gpio = S3C24X0_GetBase_GPIO();
		if (!(gpio->GPFDAT & (1 << 5)))
			gpio->GPBDAT &= ~(1 << 2);
	}
#endif

	if (menu_vote > 0) {
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
#if defined(CONFIG_ARCH_GTA01_v3)
	return 0x00000130;
#elif defined(CONFIG_ARCH_GTA01_v4)
	return 0x00000140;
#elif defined(CONFIG_ARCH_GTA01B_v2)
	return 0x00000220;
#elif defined(CONFIG_ARCH_GTA01B_v3)
	return 0x00000230;
#elif defined(CONFIG_ARCH_GTA01B_v4)
	return 0x00000240;
#endif
}

void neo1973_poweroff(void)
{
	serial_printf("poweroff\n");
	udc_disconnect();
	pcf50606_reg_write(PCF50606_REG_OOCC1, PCF50606_OOCC1_GOSTDBY);
	/* don't return to caller */
	while (1) ;
}

void neo1973_backlight(int on)
{
	S3C24X0_GPIO * const gpio = S3C24X0_GetBase_GPIO();
	if (on)
		gpio->GPBDAT |= 0x01;
	else
		gpio->GPBDAT &= ~0x01;
}

void neo1973_vibrator(int on)
{
	S3C24X0_GPIO * const gpio = S3C24X0_GetBase_GPIO();
	if (on)
#if defined(CONFIG_ARCH_GTA01_v3) || defined(CONFIG_ARCH_GTA01_v4)
		gpio->GPGDAT |= (1 << 11);	/* GPG11 */
#elif defined(CONFIG_ARCH_GTA01B_v2) || defined(CONFIG_ARCH_GTA01B_v3)
		gpio->GPBDAT |= (1 << 10);	/* GPB10 */
#elif defined(CONFIG_ARCH_GTA01B_v4)
		gpio->GPBDAT |= (1 << 3);	/* GPB3 */
#endif
	else
#if defined(CONFIG_ARCH_GTA01_v3) || defined(CONFIG_ARCH_GTA01_v4)
		gpio->GPGDAT &= ~(1 << 11);	/* GPG11 */
#elif defined(CONFIG_ARCH_GTA01B_v2) || defined(CONFIG_ARCH_GTA01B_v3)
		gpio->GPBDAT &= ~(1 << 10);	/* GPB10 */
#elif defined(CONFIG_ARCH_GTA01B_v4)
		gpio->GPBDAT &= ~(1 << 3);	/* GPB3 */
#endif
}

/* switch serial port 0 multiplexer */
void neo1973_gta01_serial0_gsm(int on)
{
	S3C24X0_GPIO * const gpio = S3C24X0_GetBase_GPIO();
	S3C24X0_UART * const uart = S3C24X0_GetBase_UART(0);
	int i;

	if (on) {
		for (i = 0; i < 3; i++) {
			if (!strcmp(stdio_devices[i]->name, "serial") ||
			    !strcmp(stdio_devices[i]->name, "s3ser0")) {
				puts("ERROR: serial port busy, can't enable GSM!\n");
				return;
			}
		}
		puts("switching s3ser0 from console into GSM mode\n");
		uart->UMCON |= 0x10;		/* Hardware flow control */
		gpio->GPFDAT &= ~(1 << 2);	/* GPF2: nGSM_EN */
	} else {
		gpio->GPFDAT |= (1 << 2);	/* GPF2: nGSM_EN */
		uart->UMCON &= ~0x10;		/* No Hardware flow control */
		puts("switched s3ser0 from GSM mode back into console mode\n");
	}
}

/* switch gsm power on/off */
void neo1973_gsm(int on)
{
	S3C24X0_GPIO * const gpio = S3C24X0_GetBase_GPIO();
	device_t *dev = search_device(DEV_FLAGS_INPUT, "s3ser0");
	if (!dev) {
		puts("can't find s3ser0 device ?!?\n");
		return;
	}

	if (on) {
		gpio->GPBDAT &= ~(1 << 6);	/* GPB6: MODEM_RST */
		gpio->GPBDAT |= (1 << 7);	/* GPB7: MODEM_ON */
	} else {
		/* unfortunately switching the modem off is not that easy.
		 * Ti's firmware is insisting on not switching off... */
		gpio->GPBDAT &= ~(1 << 7);	/* GPB7: MODEM_ON */
		//gpio->GPBDAT |= (1 << 6);	/* GPB6: MODEM_RST */
		if (!(gpio->GPFDAT & (1 << 2)))
			puts("Can't power modem off while serial console "
			     "in use!\n");
		else {
			S3C24X0_UART * const uart = S3C24X0_GetBase_UART(0);
			uart->UMCON |= 0x10;		/* Hardware flow control */
			gpio->GPFDAT &= ~(1 << 2);	/* GPF2: nGSM_EN */
			dev->puts("AT@POFF\r\n");
			gpio->GPFDAT |= (1 << 2);	/* GPF2: nGSM_EN */
			uart->UMCON &= ~0x10;		/* No Hardware flow control */
		}
	}
}

/* switch gps power on/off */
void neo1973_gps(int on)
{
	printf("not implemented yet!\n");
}

static int pwr_int_pending(void)
{
	S3C24X0_GPIO * const gpio = S3C24X0_GetBase_GPIO();

#if defined(CONFIG_ARCH_GTA01B_v4)
	return !(gpio->GPGDAT & (1 << 1));	/* EINT9/GPG1 */
#else
	return !(gpio->GPGDAT & (1 << 8));	/* EINT16/GPG8 */
#endif /* !CONFIG_ARCH_GTA01B_v4 */
}

static int have_int1(uint8_t mask)
{
	static uint8_t pending = 0;

	if (pwr_int_pending()) {
		/*
		 * We retrieve all interupts, so that we clear any stray ones
		 * in INT2 and INT3.
		 */
		uint8_t int1,int2,int3;

		int1 = pcf50606_reg_read(PCF50606_REG_INT1);
		int2 = pcf50606_reg_read(PCF50606_REG_INT2);
		int3 = pcf50606_reg_read(PCF50606_REG_INT3);
		pending |= int1;
	}
	if (!(pending & mask))
		return 0;
	pending &= ~mask;
	return 1;
}

int neo1973_new_second(void)
{
	return have_int1(PCF50606_INT1_SECOND);
}

int neo1973_on_key_pressed(void)
{
	static int pressed = -1;

	if (pressed == -1 ||
	    have_int1(PCF50606_INT1_ONKEYF | PCF50606_INT1_ONKEYR)) {
		pressed = !(pcf50606_reg_read(PCF50606_REG_OOCS) &
		     PFC50606_OOCS_ONKEY);
}
	return pressed;
}

int neo1973_aux_key_pressed(void)
{
	S3C24X0_GPIO * const gpio = S3C24X0_GetBase_GPIO();

	if (gpio->GPFDAT & (1 << 6))
		return 0;
	return 1;
}

static const char *chgstate_names[] = {
	[PCF50606_MBCC1_CHGMOD_QUAL]		= "qualification",
	[PCF50606_MBCC1_CHGMOD_PRE]		= "pre",
	[PCF50606_MBCC1_CHGMOD_TRICKLE]		= "trickle",
	[PCF50606_MBCC1_CHGMOD_FAST_CCCV]	= "fast_cccv",
	[PCF50606_MBCC1_CHGMOD_FAST_NOCC]	= "fast_nocc",
	[PCF50606_MBCC1_CHGMOD_FAST_NOCV]	= "fast_nocv",
	[PCF50606_MBCC1_CHGMOD_FAST_SW]		= "fast_switch",
	[PCF50606_MBCC1_CHGMOD_IDLE]		= "idle",
};

const char *neo1973_get_charge_status(void)
{
	u_int8_t mbcc1 = pcf50606_reg_read(PCF50606_REG_MBCC1);
	u_int8_t chgmod = (mbcc1 & PCF50606_MBCC1_CHGMOD_MASK);
	return chgstate_names[chgmod];
}

int neo1973_set_charge_mode(enum neo1973_charger_cmd cmd)
{
	switch (cmd) {
	case NEO1973_CHGCMD_NONE:
		break;
	case NEO1973_CHGCMD_AUTOFAST:
		pcf50606_reg_set_bit_mask(PCF50606_REG_MBCC1,
					  PCF50606_MBCC1_AUTOFST,
					  PCF50606_MBCC1_AUTOFST);
		break;
	case NEO1973_CHGCMD_NO_AUTOFAST:
		pcf50606_reg_set_bit_mask(PCF50606_REG_MBCC1,
					  PCF50606_MBCC1_AUTOFST, 0);
		break;
	case NEO1973_CHGCMD_OFF:
		pcf50606_reg_set_bit_mask(PCF50606_REG_MBCC1,
					  PCF50606_MBCC1_CHGMOD_MASK,
					  PCF50606_MBCC1_CHGMOD_IDLE);
		break;

	case NEO1973_CHGCMD_FAST:
	case NEO1973_CHGCMD_FASTER:
		pcf50606_reg_set_bit_mask(PCF50606_REG_MBCC1,
					  PCF50606_MBCC1_CHGMOD_MASK,
					  PCF50606_MBCC1_CHGMOD_FAST_CCCV);
		break;
	}
	return 0;
}

void neo1973_led(int led, int on)
{
	printf("No LED's in this Neo1973 hardware revision\n");
}



/* The sum of all part_size[]s must equal to the NAND size, i.e., 0x4000000.
   "initrd" is sized such that it can hold two uncompressed 16 bit 640*480
   images: 640*480*2*2 = 1228800 < 1245184. */

unsigned int dynpart_size[] = {
    CFG_UBOOT_SIZE, 0x4000, 0x200000, 0xa0000, 0x3d5c000-CFG_UBOOT_SIZE, 0 };
char *dynpart_names[] = {
    "u-boot", "u-boot_env", "kernel", "splash", "rootfs", NULL };


