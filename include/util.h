/*
 * util.h - Convenience functions
 *
 * (C) Copyright 2006-2007 OpenMoko, Inc.
 * Author: Werner Almesberger <werner@openmoko.org>
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

#ifndef UTIL_H
#define UTIL_H

#include "nand.h"


/* common/cmd_nand.c */
int arg_off_size(int argc, char *argv[], nand_info_t *nand, ulong *off,
  ulong *size, int net);

#endif /* UTIL_H */
