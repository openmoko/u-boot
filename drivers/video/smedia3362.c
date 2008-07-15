/*
 * (C) Copyright 2007 by OpenMoko, Inc.
 * Author: Harald Welte <laforge@openmoko.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>

#if defined(CONFIG_VIDEO_GLAMO3362)

#include <video_fb.h>
#include "videomodes.h"
#include <s3c2410.h>
#include "smedia3362.h"
#ifdef CONFIG_GTA02_REVISION
#include "../../board/neo1973/common/jbt6k74.h"
#endif

#define ARRAY_SIZE(x)           (sizeof(x) / sizeof((x)[0]))

/* Export Graphic Device */
GraphicDevice smi;

#define GLAMO_REG(x)	(*(volatile unsigned short *)(CONFIG_GLAMO_BASE + x))

static inline void
glamo_reg_write(u_int16_t reg, u_int16_t val)
{
	GLAMO_REG(reg) = val;
}

static inline u_int16_t
glamo_reg_read(u_int16_t reg)
{
	return GLAMO_REG(reg);
}

/* these are called by jbt6k74 driver to do LCM bitbang SPI via Glamo */

void smedia3362_spi_cs(int b)
{
	glamo_reg_write(GLAMO_REG_GPIO_GEN4,
	(glamo_reg_read(GLAMO_REG_GPIO_GEN4) & 0xffef) | (b << 4));
}

void smedia3362_spi_sda(int b)
{
	glamo_reg_write(GLAMO_REG_GPIO_GEN3,
		(glamo_reg_read(GLAMO_REG_GPIO_GEN3) & 0xff7f) | (b << 7));
}

void smedia3362_spi_scl(int b)
{
	glamo_reg_write(GLAMO_REG_GPIO_GEN3,
		(glamo_reg_read(GLAMO_REG_GPIO_GEN3) & 0xffbf) | (b << 6));
}

void smedia3362_lcm_reset(int b)
{
	glamo_reg_write(GLAMO_REG_GPIO_GEN2,
		(glamo_reg_read(GLAMO_REG_GPIO_GEN2) & 0xffef) | (b << 4));
}

/*
 * these are dumps of Glamo register ranges from working Linux
 * framebuffer
 */
static u16 u16a_lcd_init[] = {
	0x0020, 0x1020, 0x0B40, 0x01E0, 0x0280, 0x440C, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x03C0, 0x0000, 0x0258, 0x0000,
	0x0000, 0x0000, 0x0008, 0x0000, 0x0010, 0x0000, 0x01F0, 0x0000,
	0x0294, 0x0000, 0x0000, 0x0000, 0x0002, 0x0000, 0x0004, 0x0000,
	0x0284, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x8023, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
};

static u16 u16a_gen_init_0x0000[] = {
	0x2020, 0x3650, 0x0002, 0x01FF, 0x0000, 0x0000, 0x0000, 0x0000,
	0x000D, 0x000B, 0x00EE, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x1839, 0x0000, 0x2000, 0x0001, 0x0100, 0x0000, 0x0000, 0x0000,
	0x05DB, 0x5231, 0x09C3, 0x8261, 0x0003, 0x0000, 0x0000, 0x0000,
	0x000F, 0x101E, 0xC0C3, 0x101E, 0x000F, 0x0001, 0x030F, 0x020F,
	0x080F, 0x0F0F
};

static u16 u16a_gen_init_0x0200[] = {
	0x0EF0, 0x07FF, 0x0000, 0x0080, 0x0344, 0x0600, 0x0000, 0x0000,
	0x0000, 0x0000, 0x4000, 0xF00E, 0x00C0, 0x00C0, 0x00C0, 0x00C0,
	0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0,
	0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0,
	0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0,
	0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0,
	0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0,
	0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0,
	0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0,
	0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0,
	0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0,
	0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0,
	0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0,
	0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0,
	0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0,
	0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0,
	0x0873, 0xAFAF, 0x0108, 0x0010, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x1002, 0x6006, 0x00FF, 0x0001, 0x0020, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x3210, 0x5432, 0xE100, 0x01D6
};

#define glamofb_cmdq_empty() (glamo_reg_read(GLAMO_REGOFS_LCD + \
					GLAMO_REG_LCD_STATUS1) & (1 << 15))

void glamofb_cmd_mode(int on)
{
	if (on) {
		while (!glamofb_cmdq_empty())
			;
		/* display the entire frame then switch to command */
		glamo_reg_write(GLAMO_REGOFS_LCD + GLAMO_REG_LCD_COMMAND1,
			  GLAMO_LCD_CMD_TYPE_DISP |
			  GLAMO_LCD_CMD_DATA_FIRE_VSYNC);

		while (!(glamo_reg_read(GLAMO_REGOFS_LCD +
			 GLAMO_REG_LCD_STATUS2) & (1 << 12)))
			;
		udelay(5000); /* you really need this ;-) */
	} else {
		/* RGB interface needs vsync/hsync */
		if (glamo_reg_read(GLAMO_REGOFS_LCD + GLAMO_REG_LCD_MODE3) &
		    GLAMO_LCD_MODE3_RGB)
			glamo_reg_write(GLAMO_REGOFS_LCD +
				  GLAMO_REG_LCD_COMMAND1,
				  GLAMO_LCD_CMD_TYPE_DISP |
				  GLAMO_LCD_CMD_DATA_DISP_SYNC);

		glamo_reg_write(GLAMO_REGOFS_LCD + GLAMO_REG_LCD_COMMAND1,
			  GLAMO_LCD_CMD_TYPE_DISP |
			  GLAMO_LCD_CMD_DATA_DISP_FIRE);
	}
}

void glamofb_cmd_write(u_int16_t val)
{
	while (!glamofb_cmdq_empty())
		;
	glamo_reg_write(GLAMO_REGOFS_LCD + GLAMO_REG_LCD_COMMAND1, val);
}

void glamo_core_init(void)
{
	int bp;

	/* power up PLL1 and PLL2 */
	glamo_reg_write(GLAMO_REG_PLL_GEN7, 0x0000);
	glamo_reg_write(GLAMO_REG_PLL_GEN3, 0x0400);

	/* enable memory clock and get it out of deep pwrdown */
	glamo_reg_write(GLAMO_REG_CLOCK_MEMORY,
		glamo_reg_read(GLAMO_REG_CLOCK_MEMORY) |
		GLAMO_CLOCK_MEM_EN_MOCACLK);
	glamo_reg_write(GLAMO_REG_MEM_DRAM2,
			glamo_reg_read(GLAMO_REG_MEM_DRAM2) &
			(~GLAMO_MEM_DRAM2_DEEP_PWRDOWN));
	glamo_reg_write(GLAMO_REG_MEM_DRAM1,
			glamo_reg_read(GLAMO_REG_MEM_DRAM1) &
			(~GLAMO_MEM_DRAM1_SELF_REFRESH));
	/*
	 * we just fill up the general hostbus and LCD register sets
	 * with magic values taken from the Linux framebuffer init action
	 */
	for (bp = 0; bp < ARRAY_SIZE(u16a_gen_init_0x0000); bp++)
		glamo_reg_write(GLAMO_REGOFS_GENERIC | (bp << 1),
				u16a_gen_init_0x0000[bp]);

	for (bp = 0; bp < ARRAY_SIZE(u16a_gen_init_0x0200); bp++)
		glamo_reg_write(GLAMO_REGOFS_HOSTBUS | (bp << 1),
				u16a_gen_init_0x0200[bp]);
	
	/* spin until PLL1 lock */
	while (!(glamo_reg_read(GLAMO_REG_PLL_GEN5) & 1))
		;

	glamofb_cmd_mode(1);
	/* LCD registers */
	for (bp = 0; bp < ARRAY_SIZE(u16a_lcd_init); bp++)
		glamo_reg_write(GLAMO_REGOFS_LCD + (bp << 1),
			u16a_lcd_init[bp]);
	glamofb_cmd_mode(0);
}

void * video_hw_init(void)
{
	GraphicDevice *pGD = (GraphicDevice *)&smi;

	printf("Glamo core device ID: 0x%04x, Revision 0x%04x\n",
		glamo_reg_read(GLAMO_REG_DEVICE_ID),
		glamo_reg_read(GLAMO_REG_REVISION_ID));

	pGD->winSizeX = pGD->plnSizeX = 480;
	pGD->winSizeY = pGD->plnSizeY = 640;
	pGD->gdfBytesPP = 2;
	pGD->gdfIndex = GDF_16BIT_565RGB;

	pGD->frameAdrs = CONFIG_GLAMO_BASE + 0x00800000;
	pGD->memSize = 0x200000; /* 480x640x16bit = 614400 bytes */

	return &smi;
}

void video_set_lut(unsigned int index, unsigned char r,
	      unsigned char g, unsigned char b)
{
	/* FIXME: we don't support any palletized formats */
}

#endif /* CONFIG_VIDEO_GLAMO3362 */
