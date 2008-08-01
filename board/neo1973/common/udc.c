
#include <common.h>
#include <usbdcore.h>
#include <s3c2410.h>
#include <pcf50606.h>
#include <pcf50633.h>

int udc_usb_maxcurrent = 0;

void udc_ctrl(enum usbd_event event, int param)
{
	S3C24X0_GPIO * const gpio = S3C24X0_GetBase_GPIO();

	switch (event) {
	case UDC_CTRL_PULLUP_ENABLE:
#if defined(CONFIG_ARCH_GTA01_v4) || defined(CONFIG_ARCH_GTA01B_v2) || \
    defined(CONFIG_ARCH_GTA01B_v3) || defined(CONFIG_ARCH_GTA01B_v4) || \
    defined(CONFIG_GTA02_REVISION)
		if (param)
			gpio->GPBDAT |= (1 << 9);
		else
			gpio->GPBDAT &= ~(1 << 9);
#endif
		break;
	case UDC_CTRL_500mA_ENABLE:
#if defined(CONFIG_ARCH_GTA01_v3) || defined(CONFIG_ARCH_GTA01_v4) || \
    defined(CONFIG_ARCH_GTA01B_v2) || defined(CONFIG_ARCH_GTA01B_v3) || \
    defined(CONFIG_ARCH_GTA01B_v4)
		pcf50606_charge_autofast(param);
#elif defined(CONFIG_GTA02_REVISION)
		/* if we take time out here to do the excrutiatingly slow
		 * I2C transaction to the PMU to change current limit, it
		 * gives us 50:50 chance of trashing the USB connection for
		 * the whole session, including through Linux.
		 *
		 * Therefore we track what we're allowed and update it on the
		 * I2C bus elsewhere when it changes.
		 */
		if (param)
			udc_usb_maxcurrent = 500;
		else
			udc_usb_maxcurrent = 0;
#endif
		break;
	default:
		break;
	}
}
