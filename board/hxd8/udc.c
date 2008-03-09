
#include <common.h>
#include <usbdcore.h>
#include <s3c2440.h>

#if defined(CONFIG_USB_DEVICE)

void udc_ctrl(enum usbd_event event, int param)
{
	S3C24X0_GPIO * const gpio = S3C24X0_GetBase_GPIO();

	switch (event) {
	case UDC_CTRL_PULLUP_ENABLE:
		if (param)
			gpio->GPBDAT |= (1 << 9);	/* GPB9 */
		else
			gpio->GPBDAT &= ~(1 << 9);	/* GPB9 */
		break;
	case UDC_CTRL_500mA_ENABLE:
		if (param)
			gpio->GPADAT |= (1 << 0);	/* GPA0 */
		else
			gpio->GPADAT &= ~(1 << 0);	/* GPA0 */
		break;
	default:
		break;
	}
}

#endif /* CONFIG_USB_DEVICE */
