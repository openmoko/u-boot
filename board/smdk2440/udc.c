
#include <common.h>
#include <usbdcore.h>
#include <s3c2440.h>

void udc_ctrl(enum usbd_event event, int param)
{
	S3C24X0_GPIO * const gpio = S3C24X0_GetBase_GPIO();

	switch (event) {
	case UDC_CTRL_PULLUP_ENABLE:
		if (param)
			gpio->GPGDAT |= (1 << 12);
		else
			gpio->GPGDAT &= ~(1 << 12);
		break;
	case UDC_CTRL_500mA_ENABLE:
		/* IGNORE */
		break;
	default:
		break;
	}
}
