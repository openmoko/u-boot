/*
 * nand.c - Board-specific NAND setup
 *
 * Copyright (C) 2007 by OpenMoko, Inc.
 * Written by Werner Almesberger <werner@openmoko.org>
 * All Rights Reserved
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


#include "config.h" /* nand.h needs NAND_MAX_CHIPS */
#include "linux/mtd/mtd.h"
#include "linux/mtd/nand.h"
#include "s3c2440.h"


/* Derived from drivers/nand/nand_bbt.c:largepage_flashbased */

static uint8_t scan_ff_pattern[] = { 0xff, 0xff };

static struct nand_bbt_descr badblock_pattern = {
	.options = NAND_BBT_SCAN2NDPAGE,
	.offs = 0,
	.len = 1,
	.pattern = scan_ff_pattern
};


int board_nand_init(struct nand_chip *nand)
{
	nand->badblock_pattern = &badblock_pattern;
	return __board_nand_init(nand);
}
