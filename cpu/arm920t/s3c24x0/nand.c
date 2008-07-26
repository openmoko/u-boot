/*
 * (C) Copyright 2006 OpenMoko, Inc.
 * Author: Harald Welte <laforge@openmoko.org>
 *         Holger Freyther <zecke@openmoko.org>
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

#if 0
#define DEBUGN	printf
#else
#define DEBUGN(x, args ...) {}
#endif

#if defined(CONFIG_CMD_NAND)
#if !defined(CFG_NAND_LEGACY)

#include <nand.h>
#include <s3c2410.h>

#define __REGb(x)	(*(volatile unsigned char *)(x))
#define __REGi(x)	(*(volatile unsigned int *)(x))

#define	NF_BASE		0x4e000000

#define	NFCONF		__REGi(NF_BASE + 0x0)

#if defined(CONFIG_S3C2410)

#define oNFCMD		0x4
#define	oNFADDR		0x8
#define oNFDATA		0xc
#define oNFSTAT		0x10
#define NFECC0		__REGb(NF_BASE + 0x14)
#define NFECC1		__REGb(NF_BASE + 0x15)
#define NFECC2		__REGb(NF_BASE + 0x16)
#define NFCONF_nFCE	(1<<11)

#define S3C2410_NFCONF_EN          (1<<15)
#define S3C2410_NFCONF_512BYTE     (1<<14)
#define S3C2410_NFCONF_4STEP       (1<<13)
#define S3C2410_NFCONF_INITECC     (1<<12)
#define S3C2410_NFCONF_TACLS(x)    ((x)<<8)
#define S3C2410_NFCONF_TWRPH0(x)   ((x)<<4)
#define S3C2410_NFCONF_TWRPH1(x)   ((x)<<0)

#elif defined(CONFIG_S3C2440) || defined(CONFIG_S3C2442)

#define oNFCMD		0x8
#define oNFADDR		0xc
#define oNFDATA		0x10
#define oNFSTAT		0x20

#define	NFCONT		__REGi(NF_BASE + 0x04)
#define	NFMECC0		__REGi(NF_BASE + 0x2C)
#define NFCONF_nFCE	(1<<1)
#define S3C2440_NFCONF_INITECC		(1<<4)
#define S3C2440_NFCONF_MAINECCLOCK	(1<<5)
#define nand_select()		(NFCONT &= ~(1 << 1))
#define nand_deselect()		(NFCONT |= (1 << 1))
#define nand_clear_RnB()	(NFSTAT |= (1 << 2))
#define nand_detect_RB()	{ while(!(NFSTAT&(1<<2))); }
#define nand_wait()		{ while(!(NFSTAT & 0x4)); } /* RnB_TransDectect */

#endif

#define	NFCMD		__REGb(NF_BASE + oNFCMD)
#define	NFADDR		__REGb(NF_BASE + oNFADDR)
#define	NFDATA		__REGb(NF_BASE + oNFDATA)
#define	NFSTAT		__REGb(NF_BASE + oNFSTAT)

#if defined(CONFIG_HXD8)
static int hxd8_nand_dev_ready(struct mtd_info *mtd)
{
	S3C24X0_GPIO * const gpio = S3C24X0_GetBase_GPIO();
	u_int32_t val = gpio->GPCDAT;

	switch (nand_curr_device) {
		case 0:
			return (NFSTAT & 0x01);
		case 1:	/* RnB 3 */
			return ((val>>6) & 0x01);
		case 2:	/* RnB 4 */
			return ((val>>7) & 0x01);
		case 3:	/* RnB 2 */
			return  ((val>>5) & 0x01);
		default:
			return 0;
	}
}

/* 4G Nand flash chip select function */
static void hxd8_nand_select_chip(struct nand_chip *this, int chip)
{
	S3C24X0_GPIO * const gpio = S3C24X0_GetBase_GPIO();

	if (chip == 0)
		gpio->GPGDAT &=  ~(1 << 1);
	else
		gpio->GPGDAT |=  (1 << 1);

	if (chip == 1)
		gpio->GPADAT &=  ~(1 << 15);
	else
		gpio->GPADAT |= (1 << 15);

	if (chip == 2)
		gpio->GPADAT &=  ~(1 << 16);
	else
		gpio->GPADAT |=  (1 << 16);

	if (chip == 3)
		gpio->GPADAT &=  ~(1 << 14);
	else
		gpio->GPADAT |= (1 << 14);

	/* UGLY: ew don't have mtd_info pointer, but know that
	 * s3c24xx hwcontrol function does not use it for CLRNCE */
	if (chip == -1)
		this->hwcontrol(NULL, NAND_CTL_CLRNCE);
	else
		this->hwcontrol(NULL, NAND_CTL_SETNCE);
}
#endif

static void s3c2410_hwcontrol(struct mtd_info *mtd, int cmd)
{
	struct nand_chip *chip = mtd->priv;

	DEBUGN("hwcontrol(): 0x%02x: ", cmd);

	switch (cmd) {
	case NAND_CTL_SETNCE:
#if defined(CONFIG_S3C2410)
		NFCONF &= ~NFCONF_nFCE;
#elif defined(CONFIG_S3C2440) || defined(CONFIG_S3C2442)
		NFCONT &= ~NFCONF_nFCE;
#endif
		DEBUGN("NFCONF=0x%08x\n", NFCONF);
		break;
	case NAND_CTL_CLRNCE:
#if defined(CONFIG_S3C2410)
		NFCONF |= NFCONF_nFCE;
#elif defined(CONFIG_S3C2440) || defined(CONFIG_S3C2442)
		NFCONT &= ~NFCONF_nFCE;
#endif
		DEBUGN("NFCONF=0x%08x\n", NFCONF);
		break;
	case NAND_CTL_SETALE:
		chip->IO_ADDR_W = NF_BASE + oNFADDR;
		DEBUGN("SETALE\n");
		break;
	case NAND_CTL_SETCLE:
		chip->IO_ADDR_W = NF_BASE + oNFCMD;
		DEBUGN("SETCLE\n");
		break;
	default:
		chip->IO_ADDR_W = NF_BASE + oNFDATA;
		break;
	}
	return;
}

static int s3c2410_dev_ready(struct mtd_info *mtd)
{
	DEBUGN("dev_ready\n");
	return (NFSTAT & 0x01);
}

#ifdef CONFIG_S3C2410_NAND_HWECC
void s3c2410_nand_enable_hwecc(struct mtd_info *mtd, int mode)
{
	DEBUGN("s3c2410_nand_enable_hwecc(%p, %d)\n", mtd ,mode);
	NFCONF |= S3C2410_NFCONF_INITECC;
}

static int s3c2410_nand_calculate_ecc(struct mtd_info *mtd, const u_char *dat,
				      u_char *ecc_code)
{
	ecc_code[0] = NFECC0;
	ecc_code[1] = NFECC1;
	ecc_code[2] = NFECC2;
	DEBUGN("s3c2410_nand_calculate_hwecc(%p,): 0x%02x 0x%02x 0x%02x\n",
		mtd , ecc_code[0], ecc_code[1], ecc_code[2]);

	return 0;
}

#endif

#ifdef CONFIG_S3C2440_NAND_HWECC
static void s3c2440_nand_enable_hwecc(struct mtd_info *mtd, int mode)
{
	DEBUGN("%s(%p, %d)\n", __func__, mtd, mode);

	/* 
	 * In software mode, ECC module generates ECC parity code for all
	 * read / write data. So you have to reset ECC value by writing the
	 * InitECC(NFCONT[4]) bit as ‘1’ and have to clear theMainECCLock(
	 * NFCONT[5]) bit to ‘0’(Unlock) before read or write data.
	 */
	NFCONT |= S3C2440_NFCONF_INITECC;
	NFCONT &= ~S3C2440_NFCONF_MAINECCLOCK;
}

static int s3c2440_nand_calculate_ecc(struct mtd_info *mtd, const u_char *dat,
                                      u_char *ecc_code)
{
	unsigned long ecc = NFMECC0;

	ecc_code[0] = ecc;
	ecc_code[1] = ecc >> 8;
	ecc_code[2] = ecc >> 16;

	DEBUGN("%s(%p,): 0x%02x 0x%02x 0x%02x\n",
		__func__, mtd , ecc_code[0], ecc_code[1], ecc_code[2]);

	return 0;
}
#endif

static int s3c24xx_nand_correct_data_noop(struct mtd_info *mtd, u_char *dat,
				          u_char *read_ecc, u_char *calc_ecc)
{
	if (read_ecc[0] == calc_ecc[0] &&
	    read_ecc[1] == calc_ecc[1] &&
	    read_ecc[2] == calc_ecc[2])
		return 0;

	printf("%s: not implemented\n", __func__);
	return -1;
}

int board_nand_init(void) __attribute__((weak, alias("__board_nand_init")));

int __board_nand_init(struct nand_chip *nand)
{
	u_int32_t cfg;
	u_int8_t tacls, twrph0, twrph1;
	S3C24X0_CLOCK_POWER * const clk_power = S3C24X0_GetBase_CLOCK_POWER();

	DEBUGN("board_nand_init()\n");

	clk_power->CLKCON |= (1 << 4);

	/* initialize hardware */
	twrph0 = 3; twrph1 = 0; tacls = 0;

#if defined(CONFIG_S3C2410)
	cfg = S3C2410_NFCONF_EN;
	cfg |= S3C2410_NFCONF_TACLS(tacls - 1);
	cfg |= S3C2410_NFCONF_TWRPH0(twrph0 - 1);
	cfg |= S3C2410_NFCONF_TWRPH1(twrph1 - 1);

	NFCONF = cfg;
#elif defined(CONFIG_S3C2440) || defined(CONFIG_S3C2442)
	twrph0 = 7; twrph1 = 7; tacls = 7;
	NFCONF = (tacls<<12)|(twrph0<<8)|(twrph1<<4)|(0<<0);
	NFCONT = (0<<13)|(0<<12)|(0<<10)|(0<<9)|(0<<8)|(1<<6)|(1<<5)|(1<<4)|(1<<1)|(1<<0);
#endif

	/* initialize nand_chip data structure */
	nand->IO_ADDR_R = nand->IO_ADDR_W = NF_BASE + oNFDATA;

	/* read_buf and write_buf are default */
	/* read_byte and write_byte are default */

	/* hwcontrol always must be implemented */
	nand->hwcontrol = s3c2410_hwcontrol;

	nand->dev_ready = s3c2410_dev_ready;

#if defined(CONFIG_S3C2410_NAND_HWECC)
	nand->enable_hwecc = s3c2410_nand_enable_hwecc;
	nand->calculate_ecc = s3c2410_nand_calculate_ecc;
	nand->correct_data = s3c24xx_nand_correct_data_noop;
	nand->eccmode = NAND_ECC_HW3_512;
#elif defined(CONFIG_S3C2440_NAND_HWECC)
	/*
	 * Assume we have a large page flash and set the eccmode so that it 
	 * will be compatible with the linux kernel.
         */
	nand->enable_hwecc = s3c2440_nand_enable_hwecc;
	nand->calculate_ecc = s3c2440_nand_calculate_ecc;
	nand->correct_data = s3c24xx_nand_correct_data_noop;
	nand->eccmode = NAND_ECC_HW3_256;
#else
	nand->eccmode = NAND_ECC_SOFT;
#endif

#if defined(CONFIG_HXD8)
	nand->dev_ready = hxd8_nand_dev_ready;
	nand->select_chip = hxd8_nand_select_chip;
#endif

#ifdef CONFIG_S3C2410_NAND_BBT
	nand->options = NAND_USE_FLASH_BBT | NAND_DONT_CREATE_BBT;
#else
	nand->options = 0;
#endif

#if defined(CONFIG_S3C2440) || defined(CONFIG_S3C2442)
/*
	nand_select();
	nand_clear_RnB();
	NFCMD = NAND_CMD_RESET;
	{ volatile int i; for (i = 0; i < 10; i ++); }
	nand_detect_RB();
	nand_deselect();
*/
#endif

	DEBUGN("end of nand_init\n");

	return 0;
}

#else
 #error "U-Boot legacy NAND support not available for S3C24xx"
#endif
#endif
