/*
 * nand_read.c: Simple NAND read functions for booting from NAND
 *
 * This is used by cpu/arm920/start.S assembler code,
 * and the board-specific linker script must make sure this
 * file is linked within the first 4kB of NAND flash.
 *
 * Taken from GPLv2 licensed vivi bootloader,
 * Copyright (C) 2002 MIZI Research, Inc.
 *
 * Author: Hwang, Chideok <hwang@mizi.com>
 * Date  : $Date: 2004/02/04 10:37:37 $
 *
 * u-boot integration and bad-block skipping (C) 2006 by OpenMoko, Inc.
 * Author: Harald Welte <laforge@openmoko.org>
 */

#include <common.h>

#ifdef CONFIG_S3C2410_NAND_BOOT

#define __REGb(x)	(*(volatile unsigned char *)(x))
#define __REGi(x)	(*(volatile unsigned int *)(x))
#define NF_BASE		0x4e000000
#define NFCONF		__REGi(NF_BASE + 0x0)
#define NFCMD		__REGb(NF_BASE + 0x4)
#define NFADDR		__REGb(NF_BASE + 0x8)
#define NFDATA		__REGb(NF_BASE + 0xc)
#define NFSTAT		__REGb(NF_BASE + 0x10)

#define BUSY 1
inline void wait_idle(void)
{
	int i;

	while (!(NFSTAT & BUSY))
		for (i=0; i<10; i++);
}

#define NAND_SECTOR_SIZE	512
#define NAND_BLOCK_MASK		(NAND_SECTOR_SIZE - 1)
#define NAND_PAGE_SIZE		0x4000

/* low level nand read function */
int nand_read_ll(unsigned char *buf, unsigned long start_addr, int size)
{
	int i, j;

	if ((start_addr & NAND_BLOCK_MASK) || (size & NAND_BLOCK_MASK))
		return -1;	/* invalid alignment */

	/* chip Enable */
	NFCONF &= ~0x800;
	for (i=0; i<10; i++);

	for (i=start_addr; i < (start_addr + size);) {
#ifdef CONFIG_S3C2410_NAND_SKIP_BAD
		if (start_addr % NAND_PAGE_SIZE == 0) {
			unsigned char data;
			NFCMD = 0x50;
			NFADDR = 517&0xf;
			NFADDR = (i >> 9) & 0xff;
			NFADDR = (i >> 17) & 0xff;
			NFADDR = (i >> 25) & 0xff;
			wait_idle();
			data = (NFDATA & 0xff);
			if (data != 0xff) {
				/* Bad block */
				i += NAND_PAGE_SIZE;
				size += NAND_PAGE_SIZE;
				continue;
			}
		}
#endif
		/* READ0 */
		NFCMD = 0;

		/* Write Address */
		NFADDR = i & 0xff;
		NFADDR = (i >> 9) & 0xff;
		NFADDR = (i >> 17) & 0xff;
		NFADDR = (i >> 25) & 0xff;

		wait_idle();

		for (j=0; j < NAND_SECTOR_SIZE; j++, i++) {
			*buf = (NFDATA & 0xff);
			buf++;
		}
	}

	/* chip Disable */
	NFCONF |= 0x800;	/* chip disable */

	return 0;
}

#endif /* CONFIG_S3C2410_NAND_BOOT */
