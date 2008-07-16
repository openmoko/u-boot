
#include <common.h>

#ifdef CONFIG_DRIVER_PCF50606

#include <i2c.h>
#include <pcf50606.h>
#include <asm/atomic.h>
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define PCF50606_I2C_ADDR		0x08

void __pcf50606_reg_write(u_int8_t reg, u_int8_t val)
{
	i2c_write(PCF50606_I2C_ADDR, reg, 1, &val, 1);
}

u_int8_t __pcf50606_reg_read(u_int8_t reg)
{
	u_int8_t tmp;
	i2c_read(PCF50606_I2C_ADDR, reg, 1, &tmp, 1);
	return tmp;
}

void pcf50606_reg_write(u_int8_t reg, u_int8_t val)
{
	unsigned long flags;

	local_irq_save(flags);
	__pcf50606_reg_write(reg, val);
	local_irq_restore(flags);
}

u_int8_t pcf50606_reg_read(u_int8_t reg)
{
	unsigned long flags;
	u_int8_t tmp;

	local_irq_save(flags);
	tmp = __pcf50606_reg_read(reg);
	local_irq_restore(flags);

	return tmp;
}

void pcf50606_reg_set_bit_mask(u_int8_t reg, u_int8_t mask, u_int8_t val)
{
	unsigned long flags;
	u_int8_t tmp;

	local_irq_save(flags);
	tmp = __pcf50606_reg_read(reg);
	__pcf50606_reg_write(reg, (val & mask) | (tmp & ~mask));
	local_irq_restore(flags);
}

void pcf50606_reg_clear_bits(u_int8_t reg, u_int8_t bits)
{
	unsigned long flags;
	u_int8_t tmp;

	local_irq_save(flags);
	tmp = pcf50606_reg_read(reg);
	pcf50606_reg_write(reg, (tmp & ~bits));
	local_irq_restore(flags);
}

static const u_int8_t regs_valid[] = {
	PCF50606_REG_OOCS, PCF50606_REG_INT1M, PCF50606_REG_INT2M,
	PCF50606_REG_INT3M, PCF50606_REG_OOCC1, PCF50606_REG_OOCC2,
	PCF50606_REG_PSSC, PCF50606_REG_PWROKM, PCF50606_REG_DCDC1,
	PCF50606_REG_DCDC2, PCF50606_REG_DCDC3, PCF50606_REG_DCDC4,
	PCF50606_REG_DCDEC1, PCF50606_REG_DCDEC2, PCF50606_REG_DCUDC1,
	PCF50606_REG_DCUDC2, PCF50606_REG_IOREGC, PCF50606_REG_D1REGC1,
	PCF50606_REG_D2REGC1, PCF50606_REG_D3REGC1, PCF50606_REG_LPREGC1,
	PCF50606_REG_LPREGC2, PCF50606_REG_MBCC1, PCF50606_REG_MBCC2,
	PCF50606_REG_MBCC3, PCF50606_REG_BBCC, PCF50606_REG_ADCC1,
	PCF50606_REG_ADCC2, PCF50606_REG_ACDC1, PCF50606_REG_BVMC,
	PCF50606_REG_PWMC1, PCF50606_REG_LEDC1, PCF50606_REG_LEDC2,
	PCF50606_REG_GPOC1, PCF50606_REG_GPOC2, PCF50606_REG_GPOC3,
	PCF50606_REG_GPOC4, PCF50606_REG_GPOC5,
};


/* initialize PCF50606 register set */
void pcf50606_init(void)
{
	unsigned long flags;
	int i;

	local_irq_save(flags);
	for (i = 0; i < ARRAY_SIZE(regs_valid); i++) {
		__pcf50606_reg_write(regs_valid[i],
				     pcf50606_initial_regs[regs_valid[i]]);
	}
	local_irq_restore(flags);
}

void pcf50606_charge_autofast(int on)
{
	if (on) {
		printf("Enabling automatic fast charge\n");
		pcf50606_reg_set_bit_mask(PCF50606_REG_MBCC1,
					  PCF50606_MBCC1_AUTOFST,
					  PCF50606_MBCC1_AUTOFST);
	} else {
		printf("Disabling fast charge\n");
		pcf50606_reg_write(PCF50606_REG_MBCC1, 0x00);
	}
}

#if defined(CONFIG_RTC_PCF50606) && defined(CONFIG_CMD_DATE)

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
	tmp->tm_sec = bcd2bin(pcf50606_reg_read(PCF50606_REG_RTCSC));
	tmp->tm_min = bcd2bin(pcf50606_reg_read(PCF50606_REG_RTCMN));
	tmp->tm_hour = bcd2bin(pcf50606_reg_read(PCF50606_REG_RTCHR));
	tmp->tm_wday = bcd2bin(pcf50606_reg_read(PCF50606_REG_RTCWD));
	tmp->tm_mday = bcd2bin(pcf50606_reg_read(PCF50606_REG_RTCDT));
	tmp->tm_mon = bcd2bin(pcf50606_reg_read(PCF50606_REG_RTCMT));
	tmp->tm_year = bcd2bin(pcf50606_reg_read(PCF50606_REG_RTCYR));
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
	pcf50606_reg_write(PCF50606_REG_RTCSC, bin2bcd(tmp->tm_sec));
	pcf50606_reg_write(PCF50606_REG_RTCMN, bin2bcd(tmp->tm_min));
	pcf50606_reg_write(PCF50606_REG_RTCHR, bin2bcd(tmp->tm_hour));
	pcf50606_reg_write(PCF50606_REG_RTCWD, bin2bcd(tmp->tm_wday));
	pcf50606_reg_write(PCF50606_REG_RTCDT, bin2bcd(tmp->tm_mday));
	pcf50606_reg_write(PCF50606_REG_RTCMT, bin2bcd(tmp->tm_mon));
	pcf50606_reg_write(PCF50606_REG_RTCYR, bin2bcd(tmp->tm_year % 100));
}

void rtc_reset(void)
{
	/* FIXME */
}
#endif /* CONFIG_RTC_PCF50606 && CONFIG_CMD_DATE */

#endif /* CONFIG DRIVER_PCF50606 */
