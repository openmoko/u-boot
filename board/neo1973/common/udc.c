
#include <common.h>
#include <usbdcore.h>
#include <s3c2410.h>

void udc_ctrl(enum usbd_event event, int param)
{
	S3C24X0_GPIO * const gpio = S3C24X0_GetBase_GPIO();

	switch (event) {
	case UDC_CTRL_PULLUP_ENABLE:
#if defined(CONFIG_ARCH_GTA01_v4) || defined(CONFIG_ARCH_GTA01B_v2) || \
    defined(CONFIG_ARCH_GTA01B_v3) || defined(CONFIG_ARCH_GTA01B_v4)
		if (param)
			gpio->GPBDAT |= (1 << 9);
		else
			gpio->GPBDAT &= ~(1 << 9);
#endif
		break;
	default:
		break;
	}
}
