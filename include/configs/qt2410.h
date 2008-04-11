/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 * Gary Jennejohn <gj@denx.de>
 * David Mueller <d.mueller@elsoft.ch>
 *
 * Configuation settings for the Armzone QT2410 board.
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

#if 0
/* If we want to start u-boot from usb bootloader in NOR flash */
#define CONFIG_SKIP_RELOCATE_UBOOT	1
#define	CONFIG_SKIP_LOWLEVEL_INIT	1
#else
/* If we want to start u-boot directly from within NAND flash */
#define CONFIG_S3C2410_NAND_BOOT	1
#define CONFIG_S3C2410_NAND_SKIP_BAD	1
#endif

#define CFG_UBOOT_SIZE            0x40000

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_ARM920T		1	/* This is an ARM920T Core	*/
#define	CONFIG_S3C2410		1	/* in a SAMSUNG S3C2410 SoC     */
#define CONFIG_SMDK2410		1	/* on a SAMSUNG SMDK2410 Board  */

/* input clock of PLL */
#define CONFIG_SYS_CLK_FREQ	12000000/* the SMDK2410 has 12MHz input clock */


#define USE_920T_MMU		1
#define CONFIG_USE_IRQ		1

/*
 * Size of malloc() pool
 */
#define CFG_MALLOC_LEN		(CFG_ENV_SIZE + 400*1024)
#define CFG_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */

/*
 * Hardware drivers
 */
#if 0
#define CONFIG_DRIVER_CS8900	1	/* we have a CS8900 on-board */
#define CS8900_BASE		0x19000300
#define CS8900_BUS16		1 /* the Linux driver does accesses as shorts */
#endif

/*
 * select serial console configuration
 */
#define CONFIG_SERIAL1          1	/* we use SERIAL 1 on SMDK2410 */
#define CONFIG_HWFLOW		1

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
#define CONFIG_CMD_ENV
#define CONFIG_CMD_BOOTD
#define CONFIG_CMD_CONSOLE
#define CONFIG_CMD_BPM
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
#define CONFIG_CMD_JFFS2
#define CONFIG_CMD_DIAG
#define CONFIG_CMD_HWFLOW
#define CONFIG_CMD_SAVES
#define CONFIG_CMD_NAND
#define CONFIG_CMD_PORTIO
#define CONFIG_CMD_MMC
#define CONFIG_CMD_FAT
#define CONFIG_CMD_EXT2

#if 0
			CFG_CMD_DHCP	 | \
			CFG_CMD_PING	 | \
			CFG_CMD_NET	 | \

#endif

#define CONFIG_BOOTDELAY	3
#define CONFIG_BOOTARGS    	"rootfstype=jffs2 root=/dev/mtdblock4 console=ttySAC0,115200 console=tty0 loglevel=8"
#define CONFIG_ETHADDR		01:ab:cd:ef:fe:dc
#define CONFIG_NETMASK          255.255.255.0
#define CONFIG_IPADDR		10.0.0.110
#define CONFIG_SERVERIP		10.0.0.1
/*#define CONFIG_BOOTFILE	"elinos-lart" */
#define CONFIG_BOOTCOMMAND	"nand load 0x32000000 0x34000 0x200000; bootm 0x32000000"

#define CONFIG_DOS_PARTITION	1

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	115200		/* speed to run kgdb serial port */
/* what's this ? it's not used anywhere */
#define CONFIG_KGDB_SER_INDEX	1		/* which serial port to use */
#endif

/*
 * Miscellaneous configurable options
 */
#define	CFG_LONGHELP				/* undef to save memory		*/
#define	CFG_PROMPT		"QT2410 # "	/* Monitor Command Prompt	*/
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
#define CONFIG_STACKSIZE	(128*1024)	/* regular stack */
#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ	(4*1024)	/* IRQ stack */
#define CONFIG_STACKSIZE_FIQ	(4*1024)	/* FIQ stack */
#endif

#define CONFIG_USB_OHCI		1

#define CONFIG_USB_DEVICE	1
#define CONFIG_USB_TTY		1
#define CFG_CONSOLE_IS_IN_ENV	1
#define CONFIG_USBD_VENDORID		0x1457     /* Linux/NetChip */
#define CONFIG_USBD_PRODUCTID_GSERIAL	0x5120    /* gserial */
#define CONFIG_USBD_PRODUCTID_CDCACM	0x511d    /* CDC ACM */
#define CONFIG_USBD_MANUFACTURER	"Armzone"
#define CONFIG_USBD_PRODUCT_NAME	"QT2410 Bootloader " U_BOOT_VERSION
#define CONFIG_EXTRA_ENV_SETTINGS	"usbtty=cdc_acm\0"
#define CONFIG_USBD_DFU			1
#define CONFIG_USBD_DFU_XFER_SIZE	4096
#define CONFIG_USBD_DFU_INTERFACE	2

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	1	   /* we have 1 bank of DRAM */
#define PHYS_SDRAM_1		0x30000000 /* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	0x04000000 /* 64 MB */
#define PHYS_SDRAM_RES_SIZE	0x00200000 /* 2 MB for frame buffer */

#define PHYS_FLASH_1		0x00000000 /* Flash Bank #1 */

#define CFG_FLASH_BASE		PHYS_FLASH_1

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */

#define CONFIG_AMD_LV400	1	/* uncomment this if you have a LV400 flash */

#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks */
#define PHYS_FLASH_SIZE		0x00080000 /* 512KB */
#define CFG_MAX_FLASH_SECT	(11)	/* max number of sectors on one chip */

/* timeout values are in ticks */
#define CFG_FLASH_ERASE_TOUT	(5*CFG_HZ) /* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT	(5*CFG_HZ) /* Timeout for Flash Write */

#define	CFG_ENV_IS_IN_NAND	1
#define CFG_ENV_SIZE		0x4000		/* 16k Total Size of Environment Sector */
#define CFG_ENV_OFFSET_OOB	1		/* Location of ENV stored in block 0 OOB */

#define NAND_MAX_CHIPS		1
#define CFG_NAND_BASE		0x4e000000
#define CFG_MAX_NAND_DEVICE	1

#define CONFIG_MMC		1
#define CFG_MMC_BASE		0xff000000

#define CONFIG_EXT2		1

/* FAT driver in u-boot is broken currently */
#define CONFIG_FAT		1
#define CONFIG_SUPPORT_VFAT

#if 1
/* JFFS2 driver */
#define CONFIG_JFFS2_CMDLINE	1
#define CONFIG_JFFS2_NAND	1
#define CONFIG_JFFS2_NAND_DEV	0
//#define CONFIG_JFFS2_NAND_OFF		0x634000
//#define CONFIG_JFFS2_NAND_SIZE	0x39cc000
#endif

/* ATAG configuration */
#define CONFIG_INITRD_TAG		1
#define CONFIG_SETUP_MEMORY_TAGS	1
#define CONFIG_CMDLINE_TAG		1

#define CONFIG_DRIVER_S3C24X0_I2C	1
#define CONFIG_HARD_I2C			1
#define CFG_I2C_SPEED			400000  /* 400kHz according to PCF50606 data sheet */
#define CFG_I2C_SLAVE			0x7f

#define CONFIG_VIDEO
#define CONFIG_VIDEO_S3C2410
#define CONFIG_CFB_CONSOLE
#define CONFIG_VIDEO_LOGO
#define CONFIG_SPLASH_SCREEN
#define CFG_VIDEO_LOGO_MAX_SIZE		(640*480+1024+100) /* 100 = slack */
#define CONFIG_VIDEO_BMP_GZIP
#define CONFIG_VGA_AS_SINGLE_DEVICE
#define CONFIG_CMD_UNZIP

#define VIDEO_KBD_INIT_FCT     0
#define VIDEO_TSTC_FCT         serial_tstc
#define VIDEO_GETC_FCT         serial_getc

#define LCD_VIDEO_ADDR         0x33d00000

#define CONFIG_S3C2410_NAND_BBT	1

#define MTDIDS_DEFAULT		"nand0=qt2410-nand"
#define MTPARTS_DEFAULT		"qt2410-nand:192k(u-boot),8k(u-boot_env),2M(kernel),2M(splash),-(jffs2)"
#define CFG_NAND_DYNPART_MTD_KERNEL_NAME "qt2410-nand"
#define CONFIG_NAND_DYNPART

#endif	/* __CONFIG_H */
