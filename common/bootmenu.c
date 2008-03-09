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

#ifdef CFG_BOOTMENU

#include <malloc.h>
#include <devices.h>
#include <console.h>
#include <bootmenu.h>


extern const char version_string[];


#define ANSI_CLEAR	"\e[2J"
#define ANSI_REVERSE	"\e[7m"
#define	ANSI_NORMAL	"\e[m"
#define ANSI_GOTOYX	"\e[%d;%dH"

/*
 * MIN_BOOT_MENU_TIMEOUT ensures that users can't by accident set the timeout
 * unusably short.
 */
#define MIN_BOOT_MENU_TIMEOUT	10	/* 10 seconds */
#define BOOT_MENU_TIMEOUT	60	/* 60 seconds */
#define AFTER_COMMAND_WAIT	3	/* wait (2,3] after running commands */
#define	MAX_MENU_ITEMS		10	/* cut off after that many */

#define	TOP_ROW		2
#define	MENU_0_ROW	(TOP_ROW+5)


struct option {
	const char *label;
	void (*fn)(void *user);	/* run_command if NULL */
	void *user;
};


static const struct bootmenu_setup *setup;
static struct option options[MAX_MENU_ITEMS];
static int num_options = 0;
static int max_width = 0;

static device_t *bm_con;


static void bm_printf(const char *fmt, ...)
{
	va_list args;
	char printbuffer[CFG_PBSIZE];

	va_start(args, fmt);
	vsprintf(printbuffer, fmt, args);
	va_end(args);

	bm_con->puts(printbuffer);
}


static char *get_option(int n)
{
	char name[] = "menu_XX";

	sprintf(name+5, "%d", n);
	return getenv(name);
}


static void print_option(const struct option *option, int reverse)
{
	int n = option-options;

	bm_printf(ANSI_GOTOYX, MENU_0_ROW+n, 1);
	if (reverse)
		bm_printf(ANSI_REVERSE);
	bm_printf("  %-*s  ", max_width, option->label);
	if (reverse)
		bm_printf(ANSI_NORMAL);
}


static int get_var_positive_int(char *var, int default_value)
{
	const char *s;
	char *end;
	int n;

	s = getenv(var);
	if (!s)
		return default_value;
        n = simple_strtoul(s, &end, 0);
	if (!*s || *end || n < 1)
		return default_value;
	return n;
}


static void show_bootmenu(void)
{
	const struct option *option;

	bm_printf(ANSI_CLEAR ANSI_GOTOYX "%s", TOP_ROW, 1, version_string);
	if (setup->comment)
		bm_printf(ANSI_GOTOYX "*** BOOT MENU (%s) ***",
		    TOP_ROW+3, 1, setup->comment);
	else
		bm_printf(ANSI_GOTOYX "*** BOOT MENU ***", TOP_ROW+3, 1);
	bm_printf(ANSI_GOTOYX, MENU_0_ROW, 1);

	for (option = options; option != options+num_options; option++)
		print_option(option, option == options);

	bm_printf("\n\n%s to select, %s to execute.\n",
	    setup->next_key_action, setup->enter_key_name);
}


static void redirect_console(int grab)
{
	static device_t *orig_stdout, *orig_stderr;

	if (grab) {
		orig_stdout = stdio_devices[stdout];
		orig_stderr = stdio_devices[stderr];
		stdio_devices[stdout] = bm_con;
		stdio_devices[stderr] = bm_con;
	}
	else {
		/*
		 * Make this conditional, because the command may also change
		 * the console.
		 */
		if (stdio_devices[stdout] == bm_con)
			stdio_devices[stdout] = orig_stdout;
		if (stdio_devices[stderr] == bm_con)
			stdio_devices[stderr] = orig_stderr;
	}
}


static void do_option(const struct option *option)
{
	int seconds, next;

	bm_printf(ANSI_CLEAR ANSI_GOTOYX, 1, 1);
	redirect_console(1);

	if (option->fn)
		option->fn(option->user);
	else
		run_command(option->user, 0);

	redirect_console(0);
	seconds = get_var_positive_int("after_command_wait",
	    AFTER_COMMAND_WAIT);
	if (seconds)
		bm_printf("\n%s to %s.", setup->next_key_action,
		    option ? "return to boot menu" : "power off");
	next = 1; /* require up-down transition */
	while (seconds) {
		int tmp;

		tmp = setup->next_key(setup->user);
		if (tmp && !next)
			break;
		next = tmp;
		if (setup->seconds(setup->user))
			seconds--;
	}
	if (!option)
		setup->idle_action(setup->idle_action);
	show_bootmenu();
}


static void bootmenu_hook(int activity)
{
	static int next = 1, on = 1;
	static const struct option *option = options;
	static int seconds = 0;
	int tmp;

	if (activity)
		seconds = 0;
	tmp = setup->next_key(setup->user);
	if (tmp && !next) {
		print_option(option, 0);
		option++;
		if (option == options+num_options)
			option = options;
		print_option(option, 1);
		seconds = 0;
	}
	next = tmp;
	tmp = setup->enter_key(setup->user);
	if (tmp && !on) {
		do_option(option);
		option = options;
		seconds = 0;
	}
	on = tmp;
	if (setup->seconds(setup->user)) {
		int timeout;

		timeout = get_var_positive_int("boot_menu_timeout",
		    BOOT_MENU_TIMEOUT);
		if (timeout < MIN_BOOT_MENU_TIMEOUT)
			timeout = MIN_BOOT_MENU_TIMEOUT;
		if (++seconds > timeout) {
			setup->idle_action(setup->idle_action);
			seconds = 0;
		}
	}
}


static device_t *find_console(const char *name)
{
	int i;

	for (i = 1; i != ListNumItems(devlist); i++) {
		device_t *dev = ListGetPtrToItem(devlist, i);

		if (!strcmp(name, dev->name))
			if (dev->flags & DEV_FLAGS_OUTPUT)
				return dev;
	}
	return NULL;
}


void bootmenu_add(const char *label, void (*fn)(void *user), void *user)
{
	int len;

	options[num_options].label = label;
	options[num_options].fn = fn;
	options[num_options].user = user;
	num_options++;

	len = strlen(label);
	if (len > max_width)
		max_width = len;
}


void bootmenu_init(struct bootmenu_setup *__setup)
{
	int n;

	setup = __setup;
	for (n = 1; n != MAX_MENU_ITEMS+1; n++) {
		const char *spec, *colon;

		spec = get_option(n);
		if (!spec)
			continue;
		colon = strchr(spec, ':');
		if (!colon)
			bootmenu_add(spec, NULL, (char *) spec);
		else {
			char *label;
			int len = colon-spec;

			label = malloc(len+1);
			if (!label)
				return;
			memcpy(label, spec, len);
			label[len] = 0;
			bootmenu_add(label, NULL, (char *) colon+1);
		}
	}
}


void bootmenu(void)
{
	bm_con = find_console("vga");
	if (bm_con && bm_con->start && bm_con->start() < 0)
		bm_con = NULL;
	if (!bm_con)
		bm_con = stdio_devices[stdout];
	if (!bm_con)
		return;
#if 0
	console_assign(stdout, "vga");
	console_assign(stderr, "vga");
#endif
	show_bootmenu();
	console_poll_hook = bootmenu_hook;
}

#endif /* CFG_BOOTMENU */
