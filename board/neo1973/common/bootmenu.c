/*
 * bootmenu.c - Boot menu
 *
 * Copyright (C) 2006-2007 by OpenMoko, Inc.
 * Written by Werner Almesberger <werner@openmoko.org>
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


#include <common.h>
#include <environment.h>
#include <bootmenu.h>
#include <asm/atomic.h>

#ifdef CONFIG_USBD_DFU
#include "usbdcore.h"
#include "usb_dfu.h"
#endif

#include "neo1973.h"


#define DEBOUNCE_LOOPS		1000	/* wild guess */


static int debounce(int (*fn)(void), int *last)
{
	int on, i;

again:
	on = fn();
	if (on != *last)
		for (i = DEBOUNCE_LOOPS; i; i--)
			if (on != fn())
				goto again;
	*last = on;
	return on;
}


static int aux_key(void *user)
{
	static int last_aux = -1;

	return debounce(neo1973_aux_key_pressed, &last_aux);
}


static int on_key(void *user)
{
	static int last_on = -1;

	return debounce(neo1973_on_key_pressed, &last_on);
}


static int seconds(void *user)
{
	return neo1973_new_second();
}


static int system_idle(void)
{
#ifdef  CONFIG_USBD_DFU
	if (system_dfu_state)
		return *system_dfu_state == DFU_STATE_appIDLE;
#endif
        return 1;
}


static void poweroff_if_idle(void *user)
{
	unsigned long flags;

	local_irq_save(flags);
	if (system_idle())
		neo1973_poweroff();
	local_irq_restore(flags);
}


/* "bootmenu_setup" is extern, so platform can tweak it */

struct bootmenu_setup bootmenu_setup = {
	.next_key = aux_key,
	.enter_key = on_key,
	.seconds = seconds,
	.idle_action = poweroff_if_idle,
	.next_key_action = "Press [AUX]",
	.enter_key_name = "[POWER]",
};


void neo1973_bootmenu(void)
{
	bootmenu_add("Boot", NULL, "bootd");
	bootmenu_init(&bootmenu_setup);
	bootmenu();
}
