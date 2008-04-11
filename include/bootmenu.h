/*
 * bootmenu.h - Boot menu
 *
 * Copyright (C) 2006-2008 by OpenMoko, Inc.
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

#ifndef BOOTMENU_H
#define BOOTMENU_H

#define MIN_BOOT_MENU_TIMEOUT	10	/* 10 seconds */
#define BOOT_MENU_TIMEOUT	60	/* 60 seconds */
#define AFTER_COMMAND_WAIT	3	/* wait (2,3] after running commands */
#define MAX_MENU_ITEMS		10	/* cut off after that many */


struct bootmenu_setup {
	/* title comment, NULL if none */
	const char *comment;

	/* non-zero while the "next" key is being pressed */
	int (*next_key)(void *user);

	/* non-zero while the "enter" key is being pressed */
	int (*enter_key)(void *user);

	/* return the number of seconds that have passed since the last call
	   to "seconds". It's okay to limit the range to [0, 1]. */
	int (*seconds)(void *user);

	/* action to take if the boot menu times out */
	void (*idle_action)(void *user);

	/* user-specific data, passed "as is" to the functions above */
	void *user;

	/* Action to invoke the "next" function. Begins in upper case. E.g.,
 	   "Press [AUX]". */
	const char *next_key_action;

	/* Name of the "enter" key, optionally with an action (in lower case).
	   E.g., "[POWER]" or "tap the screen". */
	const char *enter_key_name;
};


/*
 * Initialize the menu from the environment.
 */

void bootmenu_init(struct bootmenu_setup *setup);

/*
 * To add entries on top of the boot menu, call bootmenu_add before
 * bootmenu_init. To add entries at the end, call it after bootmenu_init.
 * If "fn" is NULL, the command specified in "user" is executed.
 */

void bootmenu_add(const char *label, void (*fn)(void *user), void *user);

/*
 * Run the boot menu.
 */

void bootmenu(void);

#endif /* !BOOTMENU_H */
