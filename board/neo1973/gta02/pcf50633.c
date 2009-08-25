
#include <common.h>
#include <pcf50633.h>

/* initial register set for PCF50633 in Neo1973 GTA02 devices */
const u_int8_t pcf50633_initial_regs[__NUM_PCF50633_REGS] = {
	/* gap */
	[PCF50633_REG_INT1M]	= 0x00,
	[PCF50633_REG_INT2M]	= PCF50633_INT2_EXTON3F |
				  PCF50633_INT2_EXTON3R |
				  PCF50633_INT2_EXTON2F |
				  PCF50633_INT2_EXTON2R,
	[PCF50633_REG_INT3M]	= PCF50633_INT3_ADCRDY,
	[PCF50633_REG_INT4M]	= 0x00,
	[PCF50633_REG_INT5M]	= 0x00,

	[PCF50633_REG_OOCWAKE]	= 0xd3, /* wake from ONKEY,EXTON!,RTC,USB,ADP */
	[PCF50633_REG_OOCTIM1]	= 0xaa,	/* debounce 14ms everything */
	[PCF50633_REG_OOCTIM2]	= 0x4a,
	[PCF50633_REG_OOCMODE]	= 0x55,
	[PCF50633_REG_OOCCTL]	= 0x47,

	[PCF50633_REG_GPIOCTL]	= 0x01,	/* only GPIO1 is input */
	[PCF50633_REG_GPIO2CFG]	= 0x00,
	[PCF50633_REG_GPIO3CFG]	= 0x00,
	[PCF50633_REG_GPOCFG]	= 0x00,

	[PCF50633_REG_SVMCTL]	= 0x08,	/* 3.10V SYS voltage thresh. */
	[PCF50633_REG_BVMCTL]	= 0x02,	/* 2.80V BAT voltage thresh. */

	[PCF50633_REG_STBYCTL1]	= 0x00,
	[PCF50633_REG_STBYCTL2]	= 0x00,

	[PCF50633_REG_DEBPF1]	= 0xff,
	[PCF50633_REG_DEBPF2]	= 0xff,
	[PCF50633_REG_DEBPF2]	= 0x3f,

	[PCF50633_REG_AUTOOUT]	= 0x6b,	/* 3.300V */
	[PCF50633_REG_AUTOENA]	= 0x01,	/* always on */
	[PCF50633_REG_AUTOCTL]	= 0x00, /* automatic up/down operation */
	[PCF50633_REG_AUTOMXC]	= 0x0a,	/* 400mA at startup FIXME */

	[PCF50633_REG_DOWN1OUT]	= 0x1b, /* 1.3V (0x1b * .025V + 0.625V) */
	[PCF50633_REG_DOWN1ENA] = 0x02, /* enabled if GPIO1 = HIGH */
	[PCF50633_REG_DOWN1CTL]	= 0x00, /* no DVM */
	[PCF50633_REG_DOWN1MXC]	= 0x22,	/* limit to 510mA at startup */

	[PCF50633_REG_DOWN2OUT]	= 0x2f, /* 1.8V (0x2f * .025V + 0.625V) */
#ifdef CONFIG_ARCH_GTA02_v1
	[PCF50633_REG_DOWN2ENA]	= 0x02, /* enabled if GPIO1 = HIGH */
#else
	[PCF50633_REG_DOWN2ENA]	= 0x01, /* always enabled */
#endif
	[PCF50633_REG_DOWN2CTL]	= 0x00,	/* no DVM */
	[PCF50633_REG_DOWN2MXC]	= 0x22, /* limit to 510mA at startup */

	[PCF50633_REG_MEMLDOOUT] = 0x00,
	[PCF50633_REG_MEMLDOENA] = 0x00,

	[PCF50633_REG_LEDOUT]	= 0x2f,	/* full backlight power */
	[PCF50633_REG_LEDENA]	= 0x00,	/* disabled */
	[PCF50633_REG_LEDCTL]	= 0x05, /* ovp enabled, ocp 500mA */
	[PCF50633_REG_LEDDIM]	= 0x20,	/* dimming curve */

	[PCF50633_REG_LDO1OUT]	= 0x18,	/* 3.3V (24 * 0.1V + 0.9V) */
	[PCF50633_REG_LDO1ENA]	= 0x00,	/* GSENSOR_3V3, enable later */

	[PCF50633_REG_LDO2OUT]	= 0x18,	/* 3.3V (24 * 0.1V + 0.9V) */
	[PCF50633_REG_LDO2ENA]	= 0x00, /* CODEC_3V3, enable later */

#ifdef CONFIG_ARCH_GTA02_v1
	[PCF50633_REG_LDO3OUT]	= 0x15,	/* 3.0V (21 * 0.1V + 0.9V) */
	[PCF50633_REG_LDO3ENA]	= 0x02, /* enabled if GPIO1 = HIGH */
#else
	[PCF50633_REG_LDO3OUT]	= 0x00,
	[PCF50633_REG_LDO3ENA]	= 0x00,
#endif

	[PCF50633_REG_LDO4ENA]	= 0x00,

	[PCF50633_REG_LDO5OUT]	= 0x15, /* 3.0V (21 * 0.1V + 0.9V) */
	[PCF50633_REG_LDO5ENA]	= 0x00, /* RF_3V, enable later  */

	[PCF50633_REG_LDO6OUT]	= 0x15,	/* 3.0V (21 * 0.1V + 0.9V) */
	[PCF50633_REG_LDO6ENA]	= 0x00,	/* LCM_3V, enable later */

	[PCF50633_REG_HCLDOOUT]	= 0x18,	/* 3.3V (24 * 0.1V + 0.9V) */
	[PCF50633_REG_HCLDOENA]	= 0x00, /* off by default*/

	[PCF50633_REG_DCDCPFM]	= 0x00, /* off by default*/

	[PCF50633_REG_MBCC1]	= 0xe7, /* enable charger, 2 h limit */
	[PCF50633_REG_MBCC2]	= 0x28,	/* Vbatconid=2.7V, Vmax=4.20V */
	[PCF50633_REG_MBCC3]	= 0x19,	/* 25/255 == 98mA pre-charge */
	[PCF50633_REG_MBCC4]	= 0xff, /* 255/255 == 1A adapter fast */
	[PCF50633_REG_MBCC5]	= 0x19,	/* 25/255 == 98mA soft-start usb fast */
	[PCF50633_REG_MBCC6]	= 0x00, /* cutoff current 1/32 * Ichg */
	[PCF50633_REG_MBCC7]	= 0x00,	/* 1.6A max bat curr, USB 100mA */
	[PCF50633_REG_MBCC8]	= 0x00,

	[PCF50633_REG_BBCCTL]	= 0x19,	/* 3V, 200uA, on */
};
