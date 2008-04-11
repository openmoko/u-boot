/*
 * (C) Copyright 2004
 *  Samsung Electronics  : SW.LEE <hitchcar@samsung.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */

#ifndef __SMDK2440_NAND_H
#define __SMDK2440_NAND_H

#define CFG_ENV_NAND_BLOCK     8

#if 0 //old flash
#define NAND_OOB_SIZE           (16)
#define NAND_PAGES_IN_BLOCK     (32)
#define NAND_PAGE_SIZE          (512)

#define NAND_BLOCK_SIZE         (NAND_PAGE_SIZE*NAND_PAGES_IN_BLOCK)
#define NAND_BLOCK_MASK         (NAND_BLOCK_SIZE - 1)
#define NAND_PAGE_MASK          (NAND_PAGE_SIZE - 1)
#else	//new flash
#define NAND_OOB_SIZE           (64)
#define NAND_PAGES_IN_BLOCK     (64)
#define NAND_PAGE_SIZE          (2048)

#define NAND_BLOCK_SIZE         (NAND_PAGE_SIZE*NAND_PAGES_IN_BLOCK)
#define NAND_BLOCK_MASK         (NAND_BLOCK_SIZE - 1)
#define NAND_PAGE_MASK          (NAND_PAGE_SIZE - 1)

#endif



//#define NAND_3_ADDR_CYCLE	1
//#define S3C24X0_16BIT_NAND	1

#ifdef KINGFISH
#undef S3C24X0_16BIT_NAND	
#define S3C24X0_16BIT_NAND	1
#endif

#endif	

