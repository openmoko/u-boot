#include <common.h>

#ifdef CONFIG_DRIVER_PCF50633

#include <i2c.h>
#include <pcf50633.h>
#include <asm/atomic.h>
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))


#define ADC_NOMINAL_RES_1A 6
#define ADC_NOMINAL_RES_NC_R_USB 43

#define PCF50633_I2C_ADDR		0x73

void __pcf50633_reg_write(u_int8_t reg, u_int8_t val)
{
	i2c_write(PCF50633_I2C_ADDR, reg, 1, &val, 1);
}

u_int8_t __pcf50633_reg_read(u_int8_t reg)
{
	u_int8_t tmp;
	i2c_read(PCF50633_I2C_ADDR, reg, 1, &tmp, 1);
	return tmp;
}

void pcf50633_reg_write(u_int8_t reg, u_int8_t val)
{
	unsigned long flags;

	local_irq_save(flags);
	__pcf50633_reg_write(reg, val);
	local_irq_restore(flags);
}

u_int8_t pcf50633_reg_read(u_int8_t reg)
{
	unsigned long flags;
	u_int8_t tmp;

	local_irq_save(flags);
	tmp = __pcf50633_reg_read(reg);
	local_irq_restore(flags);

	return tmp;
}

void pcf50633_reg_set_bit_mask(u_int8_t reg, u_int8_t mask, u_int8_t val)
{
	unsigned long flags;
	u_int8_t tmp;

	local_irq_save(flags);
	tmp = __pcf50633_reg_read(reg);
	__pcf50633_reg_write(reg, (val & mask) | (tmp & ~mask));
	local_irq_restore(flags);
}

void pcf50633_reg_clear_bits(u_int8_t reg, u_int8_t bits)
{
	unsigned long flags;
	u_int8_t tmp;

	local_irq_save(flags);
	tmp = pcf50633_reg_read(reg);
	pcf50633_reg_write(reg, (tmp & ~bits));
	local_irq_restore(flags);
}

static const u_int8_t regs_invalid[] = {
	PCF50633_REG_VERSION,
	PCF50633_REG_VARIANT,
	PCF50633_REG_OOCSHDWN,
	PCF50633_REG_INT1,
	PCF50633_REG_INT2,
	PCF50633_REG_INT3,
	PCF50633_REG_INT4,
	PCF50633_REG_INT5,
	PCF50633_REG_OOCSTAT,
	0x2c,
	PCF50633_REG_DCDCSTAT,
	PCF50633_REG_LDOSTAT,
	PCF50633_REG_MBCS1,
	PCF50633_REG_MBCS2,
	PCF50633_REG_MBCS3,
	PCF50633_REG_ALMDATA,
	0x51,
	/* 0x55 ... 0x6e: don't write */
	/* 0x6f ... 0x83: reserved */
};
#define PCF50633_LAST_REG	0x55

static int reg_is_invalid(u_int8_t reg)
{
	int i;

	/* all registers above 0x55 (ADCS1) except 0x84 */
	if (reg == PCF50633_REG_DCDCPFM)
		return 0;
	if (reg >= 0x55)
		return 1;

	for (i = 0; i < ARRAY_SIZE(regs_invalid); i++) {
		if (regs_invalid[i] > reg)
			return 0;
		if (regs_invalid[i] == reg)
			return 1;
	}

	return 0;
}

static u_int16_t pcf50633_adc_read(u_int8_t channel, u_int8_t avg)
{
	u_int16_t ret;

	/* start ADC conversion of selected channel */
	pcf50633_reg_write(PCF50633_REG_ADCC1, channel |
					       avg |
					       PCF50633_ADCC1_ADCSTART |
					       PCF50633_ADCC1_RES_10BIT);

	/* spin until completed */
	while (!(pcf50633_reg_read(PCF50633_REG_ADCS3) & 0x80))
		;

	/* grab the result */
	ret = (pcf50633_reg_read(PCF50633_REG_ADCS1) << 2) |
	      (pcf50633_reg_read(PCF50633_REG_ADCS3) &
	      PCF50633_ADCS3_ADCDAT1L_MASK);

	return ret;
}

/* figure out our charger situation */
int pcf50633_read_charger_type(void)
{
	u_int16_t ret;

	if ((pcf50633_reg_read(PCF50633_REG_MBCS1) & 0x3) != 0x3)
		return 0; /* no power, just battery */

	/* kill ratiometric, but enable ACCSW biasing */
	pcf50633_reg_write(PCF50633_REG_ADCC2, 0x00);
	pcf50633_reg_write(PCF50633_REG_ADCC3, 0x01);

	ret = pcf50633_adc_read(PCF50633_ADCC1_MUX_ADCIN1,
	    PCF50633_ADCC1_AVERAGE_16);

	/* well it is nearest to the 1A resistor */
	if (ret < ((ADC_NOMINAL_RES_1A + ADC_NOMINAL_RES_NC_R_USB) / 2))
		return 1000;

	/* there is no resistor, so it must be USB pwr */
	return 100; /* USB power then */

}

u_int16_t pcf50633_read_battvolt(void)
{
	u_int16_t ret;

	ret = pcf50633_adc_read(PCF50633_ADCC1_MUX_BATSNS_RES, 0);

	return (ret * 6000) / 1024;
}

/* initialize PCF50633 register set */
void pcf50633_init(void)
{
	unsigned long flags;
	u_int8_t i;
	int limit;

	local_irq_save(flags);
	for (i = 0; i < PCF50633_LAST_REG; i++) {
		if (reg_is_invalid(i))
			continue;
		__pcf50633_reg_write(i, pcf50633_initial_regs[i]);
	}
	local_irq_restore(flags);

	printf("Power: ");
	limit = pcf50633_read_charger_type();
	/*
	 * If we're on real USB, don't change the setting to avoid racing with
	 * USB signaling.
	 */
	if (limit != 100) {
		printf("%dmA\n", limit);
		pcf50633_usb_maxcurrent(limit);
	}
}

int pcf50633_usb_last_maxcurrent = -1;

void pcf50633_usb_maxcurrent(unsigned int ma)
{
	u_int8_t val;

	pcf50633_usb_last_maxcurrent = ma;
	if (ma < 100)
		val = PCF50633_MBCC7_USB_SUSPEND;
	else if (ma < 500)
		val = PCF50633_MBCC7_USB_100mA;
	else if (ma < 1000)
		val = PCF50633_MBCC7_USB_500mA;
	else
		val = PCF50633_MBCC7_USB_1000mA;

	return pcf50633_reg_set_bit_mask(PCF50633_REG_MBCC7, 0x03, val);
}


static const char *charger_states[] = {
	[0]	= "play_only",
	[1]	= "usb_precharge",
	[2]	= "usb_precharge_wait",
	[3]	= "usb_fast_charge",
	[4]	= "usb_fast_charge_wait",
	[5]	= "usb_suspend",
	[6]	= "adapter_precharge",
	[7]	= "adapter_precharge_wait",
	[8]	= "adapter_fast_charge",
	[9]	= "adapter_fast_charge_wait",
	[10]	= "battery_full",
	[11]	= "halt",
};

const char *pcf50633_charger_state(void)
{
	u_int8_t val = pcf50633_reg_read(PCF50633_REG_MBCS2);

	val &= 0x0f;
	if (val > 11)
		return "error";

	return charger_states[val];
}

#if defined(CONFIG_RTC_PCF50633) && defined(CONFIG_CMD_DATE)

#include <rtc.h>

static unsigned bcd2bin (uchar n)
{
	return ((((n >> 4) & 0x0F) * 10) + (n & 0x0F));
}

static unsigned char bin2bcd (unsigned int n)
{
	return (((n / 10) << 4) | (n % 10));
}


int rtc_get(struct rtc_time *tmp)
{
	tmp->tm_sec = bcd2bin(pcf50633_reg_read(PCF50633_REG_RTCSC));
	tmp->tm_min = bcd2bin(pcf50633_reg_read(PCF50633_REG_RTCMN));
	tmp->tm_hour = bcd2bin(pcf50633_reg_read(PCF50633_REG_RTCHR));
	tmp->tm_wday = bcd2bin(pcf50633_reg_read(PCF50633_REG_RTCWD));
	tmp->tm_mday = bcd2bin(pcf50633_reg_read(PCF50633_REG_RTCDT));
	tmp->tm_mon = bcd2bin(pcf50633_reg_read(PCF50633_REG_RTCMT));
	tmp->tm_year = bcd2bin(pcf50633_reg_read(PCF50633_REG_RTCYR));
	if (tmp->tm_year < 70)
		tmp->tm_year += 2000;
	else
		tmp->tm_year += 1900;
	tmp->tm_yday = 0;
	tmp->tm_isdst = 0;

	return 0;
}

void rtc_set(struct rtc_time *tmp)
{
	pcf50633_reg_write(PCF50633_REG_RTCSC, bin2bcd(tmp->tm_sec));
	pcf50633_reg_write(PCF50633_REG_RTCMN, bin2bcd(tmp->tm_min));
	pcf50633_reg_write(PCF50633_REG_RTCHR, bin2bcd(tmp->tm_hour));
	pcf50633_reg_write(PCF50633_REG_RTCWD, bin2bcd(tmp->tm_wday));
	pcf50633_reg_write(PCF50633_REG_RTCDT, bin2bcd(tmp->tm_mday));
	pcf50633_reg_write(PCF50633_REG_RTCMN, bin2bcd(tmp->tm_mon));
	pcf50633_reg_write(PCF50633_REG_RTCYR, bin2bcd(tmp->tm_year % 100));
}

void rtc_reset(void)
{
	/* FIXME */
}
#endif /* CONFIG_RTC_PCF50633 && CONFIG_CMD_DATE */


#endif /* CONFIG DRIVER_PCF50633 */
