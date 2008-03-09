/*
 * (C) Copyright 2007 OpenMoko, Inc.
 * Author: Harald Welte <laforge@openmoko.org>
 *
 * Configuation settings for the FIC HXD8
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

#ifndef __CONFIG_H
#define __CONFIG_H

/* we want to be able to start u-boot directly from within NAND flash */
#define CONFIG_LL_INIT_NAND_ONLY
#define CONFIG_S3C2410_NAND_BOOT	1
#define CONFIG_S3C2410_NAND_SKIP_BAD	1

#define CFG_UBOOT_SIZE		0x40000 /* size of u-boot, for NAND loading */

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_ARM920T		1	/* This is an ARM920T Core	*/
#define	CONFIG_S3C2440		1	/* in a SAMSUNG S3C2440 SoC     */
#define CONFIG_SMDK2440		1	/* on a SAMSUNG SMDK2440 Board  */
#define CONFIG_HXD8		1	/* on a FIC HXD8 Board  */

/* input clock of PLL */
#define CONFIG_SYS_CLK_FREQ	16934400/* the HXD8 has this input clock */


#define USE_920T_MMU		1
#define CONFIG_USE_IRQ		1

/*
 * Size of malloc() pool
 */
#define CFG_MALLOC_LEN		(CFG_ENV_SIZE + 2048*1024)
					/* >> CFG_VIDEO_LOGO_MAX_SIZE */
#define CFG_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */

/*
 * Hardware drivers
 */

/*
 * select serial console configuration
 */
#define CONFIG_SERIAL3		1	/* we use SERIAL 3 on HXD8 */

/************************************************************
 * RTC
 ************************************************************/
#define	CONFIG_RTC_S3C24X0	1

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_BAUDRATE		115200

/***********************************************************
 * Command definition
 ***********************************************************/

#define CONFIG_CMD_BDI
#define CONFIG_CMD_LOADS
#define CONFIG_CMD_LOADB
#define CONFIG_CMD_IMI
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_MEMORY
#define CONFIG_CMD_ENV
			/* CFG_CMD_IRQ	 | */
#define CONFIG_CMD_BOOTD
#define CONFIG_CMD_CONSOLE
			/* CFG_CMD_BMP	 | */
#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_RUN
#define CONFIG_CMD_ECHO
#define CONFIG_CMD_I2C
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_IMMAP
#define CONFIG_CMD_DATE
#define CONFIG_CMD_AUTOSCRIPT
#define CONFIG_CMD_BSP
#define CONFIG_CMD_ELF
#define CONFIG_CMD_MISC
			/* CFG_CMD_USB	 | */
#define CONFIG_CMD_JFFS2
#define CONFIG_CMD_DIAG
			/* CFG_CMD_HWFLOW | */
#define CONFIG_CMD_SAVES
#define CONFIG_CMD_NAND
#define CONFIG_CMD_PORTIO
#define CONFIG_CMD_MMC
#define CONFIG_CMD_FAT
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_LICENSE

#define CONFIG_BOOTDELAY	3
#define CONFIG_BOOTARGS    	""
#define CONFIG_BOOTCOMMAND	"setenv bootargs ${bootargs_base} ${mtdparts}; nand read.e 0x32000000 kernel; bootm 0x32000000"

#define CONFIG_DOS_PARTITION	1

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	115200		/* speed to run kgdb serial port */
/* what's this ? it's not used anywhere */
#define CONFIG_KGDB_SER_INDEX	3		/* which serial port to use */
#endif

/*
 * Miscellaneous configurable options
 */
#define	CFG_LONGHELP				/* undef to save memory		*/
#define	CFG_PROMPT		"HXD8 # "	/* Monitor Command Prompt	*/
#define	CFG_CBSIZE		256		/* Console I/O Buffer Size	*/
#define	CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define	CFG_MAXARGS		64		/* max number of command args	*/
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x30000000	/* memtest works on	*/
#define CFG_MEMTEST_END		0x33F00000	/* 63 MB in DRAM	*/

#undef  CFG_CLKS_IN_HZ		/* everything, incl board info, in Hz */

#define	CFG_LOAD_ADDR		0x33000000	/* default load address	*/

/* the PWM TImer 4 uses a counter of 15625 for 10 ms, so we need */
/* it to wrap 100 times (total 1562500) to get 1 sec. */
#define	CFG_HZ			1562500

/* valid baudrates */
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/*-----------------------------------------------------------------------
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	(512*1024)	/* regular stack */
#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ	(8*1024)	/* IRQ stack */
#define CONFIG_STACKSIZE_FIQ	(4*1024)	/* FIQ stack */
#endif

#if 0
#define CONFIG_USB_OHCI_NEW	1
#define CFG_USB_OHCI_CPU_INIT	1
#define CFG_USB_OHCI_REGS_BASE	0x49000000 /* S3C24X0_USB_HOST_BASE */
#define CFG_USB_OHCI_SLOT_NAME	"s3c2440"
#define CFG_USB_OHCI_MAX_ROOT_PORTS 	2
#endif

#if 1
#define CONFIG_USB_DEVICE	1
#define CONFIG_USB_TTY		1
#define CFG_CONSOLE_IS_IN_ENV	1
#define CONFIG_USBD_VENDORID 		0x1457     /* Linux/NetChip */
#define CONFIG_USBD_PRODUCTID_GSERIAL	0x5120    /* gserial */
#define CONFIG_USBD_PRODUCTID_CDCACM 	0x511a    /* CDC ACM */
#define CONFIG_USBD_MANUFACTURER	"OpenMoko, Inc"
#define CONFIG_USBD_PRODUCT_NAME	"HXD8 Bootloader " U_BOOT_VERSION
#define CONFIG_USBD_DFU			1
#define CONFIG_USBD_DFU_XFER_SIZE 	4096	/* 0x4000 */
#define CONFIG_USBD_DFU_INTERFACE	2
#endif
#define CFG_CONSOLE_IS_IN_ENV	1

#define CONFIG_EXTRA_ENV_SETTINGS 					\
	"usbtty=cdc_acm\0"						\
	"bootargs_base=rootfstype=jffs2 root=/dev/mtdblock4 console=ttySAC2,115200 console=tty0 loglevel=8\0" \
	""

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	1	   /* we have 1 bank of DRAM */
#define PHYS_SDRAM_1		0x30000000 /* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	0x08000000 /* 128 MB */
#define PHYS_SDRAM_RES_SIZE	0x00200000 /* 2 MB for frame buffer */

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */

/* No NOR flash in this device */
#define CFG_NO_FLASH		1

#define CFG_ENV_SIZE		0x20000		/* 128k Total Size of Environment Sector */
#define	CFG_ENV_IS_IN_NAND	1
#define CFG_ENV_OFFSET_OOB   	1               /* Location of ENV stored in block 0 OOB */
#define	CFG_PREBOOT_OVERRIDE	1	/* allow preboot from memory */

#define NAND_MAX_CHIPS		3
#define CFG_NAND_BASE		0x4e000000
#define CFG_MAX_NAND_DEVICE	NAND_MAX_CHIPS
#define CFG_NAND_BASE_LIST	{ CFG_NAND_BASE, CFG_NAND_BASE, CFG_NAND_BASE }

#define CONFIG_MMC		1
#define CFG_MMC_BASE		0xff000000

/* EXT2 driver */
#define CONFIG_EXT2		1

#define CONFIG_FAT		1
#define CONFIG_SUPPORT_VFAT

#if 1
/* JFFS2 driver */
#define CONFIG_JFFS2_CMDLINE	1
#define CONFIG_JFFS2_NAND	1
#define CONFIG_JFFS2_NAND_DEV	0
//#define CONFIG_JFFS2_NAND_OFF	0x634000
//#define CONFIG_JFFS2_NAND_SIZE	0x39cc000
#endif

/* ATAG configuration */
#define CONFIG_INITRD_TAG		1
#define CONFIG_SETUP_MEMORY_TAGS	1
#define CONFIG_CMDLINE_TAG		1
#define CONFIG_REVISION_TAG		1
#if 0
#define CONFIG_SERIAL_TAG		1
#endif

#define CONFIG_DRIVER_S3C24X0_I2C	1
#define CONFIG_HARD_I2C			1
#define CFG_I2C_SPEED			400000	/* 400kHz according to PCF50606 data sheet */
#define CFG_I2C_SLAVE			0x7f

/* we have a board_late_init() function */
#define BOARD_LATE_INIT			1

#if 1
#define CONFIG_VIDEO
#define CONFIG_VIDEO_S3C2410
#define CONFIG_CFB_CONSOLE
#define CONFIG_VIDEO_LOGO
#define CONFIG_SPLASH_SCREEN
#define CFG_VIDEO_LOGO_MAX_SIZE	(640*480+1024+100) /* 100 = slack */
#define CONFIG_VIDEO_BMP_GZIP
#define CONFIG_VGA_AS_SINGLE_DEVICE
#define CONFIG_CMD_UNZIP

#define VIDEO_KBD_INIT_FCT	0
#define VIDEO_TSTC_FCT		serial_tstc
#define VIDEO_GETC_FCT		serial_getc

#define LCD_VIDEO_ADDR		0x33d00000
#endif

#define CONFIG_S3C2410_NAND_BBT                1
//#define CONFIG_S3C2410_NAND_HWECC              1

#define CONFIG_DRIVER_PCF50606		1

#define MTDIDS_DEFAULT	"nand0=hxd8-nand"
#define MTPARTS_DEFAULT	"hxd8-nand:256k(u-boot),128k(u-boot_env),2M(kernel),640k(splash),0x3fd00000(jffs2)"
#define CFG_NAND_DYNPART_MTD_KERNEL_NAME "hxd8-nand"
#define CONFIG_NAND_DYNPART

#endif	/* __CONFIG_H */
