/*
 * (C) Copyright 2007 OpenMoko, Inc.
 * Written by Jim Huang <jserv@openmoko.org>
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

/*
 * Boot support
 */
#include <common.h>
#include <devices.h>

#define GSM_UART_DEVICE  "s3ser0"

int neo1973_gsmver()
{
	int i;
	device_t *dev = NULL;
	int string_end_count = 0;

	/* Scan for selected output/input device */
	for (i = 1; i <= ListNumItems (devlist); i++) {
		device_t *tmp = ListGetPtrToItem (devlist, i);
		if (!strcmp(tmp->name, GSM_UART_DEVICE)) {
			dev = tmp;
			break;
		}
	}
	if (!dev)
		return -1;

#if defined(CONFIG_ARCH_GTA01_v3) || defined(CONFIG_ARCH_GTA01_v4) || \
	defined(CONFIG_ARCH_GTA01B_v2) || defined(CONFIG_ARCH_GTA01B_v3) || \
	defined(CONFIG_ARCH_GTA01B_v4)
	neo1973_gta01_serial0_gsm(1);
#endif

	/* Query GSM firmware information by AT command */
	dev->puts("AT+CGMR\r\n");
	puts("GSM firmware version: ");

	/* read from serial and display version information */
	while (1) {
		if (dev->tstc()) {
			i = dev->getc();
			putc(i);
			/* FIXME: should we just dump straightforward
			 * version string such as "moko1" or "moko4"?
			 */
			if (i == '\n' || i == '\r')
				continue;
		}
	}
	putc('\n');

#if defined(CONFIG_ARCH_GTA01_v3) || defined(CONFIG_ARCH_GTA01_v4) || \
    defined(CONFIG_ARCH_GTA01B_v2) || defined(CONFIG_ARCH_GTA01B_v3) || \
    defined(CONFIG_ARCH_GTA01B_v4)
	neo1973_gta01_serial0_gsm(0);
#endif

	return 0;
}
