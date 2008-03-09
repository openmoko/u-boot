/*
 * (C) Copyright 2006-2007 OpenMoko, Inc.
 * Author: Harald Welte <laforge@openmoko.org>
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

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <environment.h>
#include <nand.h>
#include <asm/errno.h>

#if defined(CFG_ENV_OFFSET_OOB)

int do_dynenv(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	struct mtd_info *mtd = &nand_info[0];
	int ret, size = 8;
	uint8_t *buf;

	char *cmd = argv[1];

	buf = malloc(mtd->oobsize);
	if (!buf)
		return -ENOMEM;

	if (!strcmp(cmd, "get")) {
		ret = mtd->read_oob(mtd, 8, size, (size_t *) &size, (u_char *) buf);

		if (buf[0] == 'E' && buf[1] == 'N' &&
		    buf[2] == 'V' && buf[3] == '0')
			printf("0x%08x\n", *((u_int32_t *) &buf[4]));
		else
			printf("No dynamic environment marker in OOB block 0\n");

	} else if (!strcmp(cmd, "set")) {
		unsigned long addr;
		if (argc < 3)
			goto usage;

		buf[0] = 'E';
		buf[1] = 'N';
		buf[2] = 'V';
		buf[3] = '0';
		addr = simple_strtoul(argv[2], NULL, 16);
		memcpy(buf+4, &addr, 4);

		printf("%02x %02x %02x %02x - %02x %02x %02x %02x\n",
			buf[0], buf[1], buf[2], buf[3],
			buf[4], buf[5], buf[6], buf[7]);

		ret = mtd->write_oob(mtd, 8, size, (size_t *) &size, (u_char *) buf);
	} else
		goto usage;

	free(buf);
	return ret;

usage:
	free(buf);
	printf("Usage:\n%s\n", cmdtp->usage);
	return 1;
}

U_BOOT_CMD(dynenv, 3, 1, do_dynenv,
	"dynenv  - dynamically placed (NAND) environment\n",
	"dynenv set off	- set enviromnent offset\n"
	"dynenv get	- get environment offset\n");

#endif /* CFG_ENV_OFFSET_OOB */
