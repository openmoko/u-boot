/*
 * (C) Copyright 2006 by OpenMoko, Inc.
 * Author: Harald Welte <laforge@openmoko.org>
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
#include <command.h>
#include <net.h>		/* for print_IPaddr */
#include <s3c2410.h>

#include "neo1973.h"

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_CMD_BDI)

int do_neo1973 ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int i;

	if (!strcmp(argv[1], "info")) {
		printf("FIC Neo1973 Hardware Revision 0x%04x\n", get_board_rev());
	} else if (!strcmp(argv[1], "power-off")) {
		neo1973_poweroff();
	} else if (!strcmp(argv[1], "charger") || !strcmp(argv[1], "charge")) {
		if (argc < 3)
			goto out_help;
		if (!strcmp(argv[2], "status") || !strcmp(argv[2], "state")) {
			printf("%s\n", neo1973_get_charge_status());
		} else if (!strcmp(argv[2], "autofast")) {
			neo1973_set_charge_mode(NEO1973_CHGCMD_AUTOFAST);
		} else if (!strcmp(argv[2], "!autofast")) {
			neo1973_set_charge_mode(NEO1973_CHGCMD_NO_AUTOFAST);
		} else if (!strcmp(argv[2], "off")) {
			neo1973_set_charge_mode(NEO1973_CHGCMD_OFF);
		} else if (!strcmp(argv[2], "fast")) {
			neo1973_set_charge_mode(NEO1973_CHGCMD_FAST);
		} else
			goto out_help;
	} else if (!strcmp(argv[1], "backlight")) {
		if (argc < 3)
			goto out_help;
		if (!strcmp(argv[2], "on"))
			neo1973_backlight(1);
		else
			neo1973_backlight(0);
	} else if (!strcmp(argv[1], "led")) {
		long led = simple_strtol(argv[2], NULL, 10);
		if (argc < 4)
			goto out_help;
		if (!strcmp(argv[3], "on"))
			neo1973_led(led, 1);
		else
			neo1973_led(led, 0);
	} else if (!strcmp(argv[1], "vibrator")) {
		if (argc < 3)
			goto out_help;
		if (!strcmp(argv[2], "on"))
			neo1973_vibrator(1);
		else
			neo1973_vibrator(0);
	} else if (!strcmp(argv[1], "gsm")) {
		if (argc < 3)
			goto out_help;
		if (!strcmp(argv[2], "on"))
			neo1973_gsm(1);
		else if (!strcmp(argv[2], "off"))
			neo1973_gsm(0);
		else if (!strcmp(argv[2], "version"))
			neo1973_gsmver();
	} else if (!strcmp(argv[1], "gps")) {
		if (argc < 3)
			goto out_help;
		if (!strcmp(argv[2], "on"))
			neo1973_gps(1);
		else
			neo1973_gps(0);
	} else if (!strcmp(argv[1], "udc")) {
		if (argc < 3)
			goto out_help;
		if (!strcmp(argv[2], "pullup")) {
			if (argc < 4)
				goto out_help;
			if (!strcmp(argv[3], "on"))
				udc_connect();
			else
				udc_disconnect();
		} else
			goto out_help;
	} else {
out_help:
		printf("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	return 0;
}

/* -------------------------------------------------------------------- */

U_BOOT_CMD(
	neo1973,	4,	1,	do_neo1973,
	"neo1973 - phone specific commands\n",
	"neo1973 info - display phone informantion\n"
	"neo1973 power-off - switch off the phone\n"
	"neo1973 charger status - display charger status\n"
	"neo1973 charger autofast - enable automatic fast (500mA) charging\n"
	"neo1973 charger !autofast - disable automatic fast (500mA) charging\n"
	"neo1973 charger fast - enable fast (500mA) charging\n"
	"neo1973 charger off - disable charging\n"
	"neo1973 backlight (on|off) - switch backlight on or off\n"
	"neo1973 led num (on|off) - switch LED number 'num' on or off\n"
	"neo1973 vibrator (on|off) - switch vibrator on or off\n"
	"neo1973 gsm (on|off|version) - switch GSM Modem on/off or print firmware version\n"
	"neo1973 gps (on|off) - switch GPS system on or off\n"
	"neo1973 udc pullup (on|off) - switch USB device controller pull-up on or off\n"
);
#endif	/* CONFIG_CMD_BDI */
