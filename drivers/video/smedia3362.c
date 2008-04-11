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

#define ARRAY_SIZE(x)           (sizeof(x) / sizeof((x)[0]))

/* Export Graphic Device */
GraphicDevice smi;

#define GLAMO_REG(x)	(*(volatile unsigned short *)(CONFIG_GLAMO_BASE + x))

static inline void glamo_reg_write(u_int16_t reg, u_int16_t val)
{
	GLAMO_REG(reg) = val;
}

static inline u_int16_t glamo_reg_read(u_int16_t reg)
{
	return GLAMO_REG(reg);
}

struct glamo_script {
	u_int16_t reg;
	u_int16_t val;
}; // __attribute__((packed));

/* from 'initial glamo 3365 script' */
static struct glamo_script gl3362_init_script[] = {
	/* clock */
	{ GLAMO_REG_CLOCK_MEMORY, 	0x300a },
	{ GLAMO_REG_CLOCK_LCD,		0x10aa },
	{ GLAMO_REG_CLOCK_MMC,		0x100a },
	{ GLAMO_REG_CLOCK_ISP,		0x32aa },
	{ GLAMO_REG_CLOCK_JPEG,		0x100a },
	{ GLAMO_REG_CLOCK_3D,		0x302a },
	{ GLAMO_REG_CLOCK_2D,		0x302a },
	//{ GLAMO_REG_CLOCK_RISC1,	0x1aaa },
	//{ GLAMO_REG_CLOCK_RISC2,	0x002a },
	{ GLAMO_REG_CLOCK_MPEG,		0x3aaa },
	{ GLAMO_REG_CLOCK_MPROC,	0x12aa },
		{ 0xfffe, 5 },
	{ GLAMO_REG_CLOCK_HOST,		0x000d },
	{ GLAMO_REG_CLOCK_MEMORY,	0x000a },
	{ GLAMO_REG_CLOCK_LCD,		0x00ee },
	{ GLAMO_REG_CLOCK_MMC,		0x000a },
	{ GLAMO_REG_CLOCK_ISP,		0x02aa },
	{ GLAMO_REG_CLOCK_JPEG,		0x000a },
	{ GLAMO_REG_CLOCK_3D,		0x002a },
	{ GLAMO_REG_CLOCK_2D,		0x002a },
	//{ GLAMO_REG_CLOCK_RISC1,	0x0aaa },
	//{ GLAMO_REG_CLOCK_RISC2,	0x002a },
	{ GLAMO_REG_CLOCK_MPEG,		0x0aaa },
	{ GLAMO_REG_CLOCK_MPROC,	0x02aa },
		{ 0xfffe, 5 },
	{ GLAMO_REG_PLL_GEN1,		0x061a }, /* PLL1=50MHz, OSCI=32kHz */
	{ GLAMO_REG_PLL_GEN3,		0x09c3 }, /* PLL2=80MHz, OSCI=32kHz */
		{ 0xfffe, 5 },
	{ GLAMO_REG_CLOCK_GEN5_1,	0x18ff },
	{ GLAMO_REG_CLOCK_GEN5_2,	0x051f },
	{ GLAMO_REG_CLOCK_GEN6,		0x2000 },
	{ GLAMO_REG_CLOCK_GEN7,		0x0105 },
	{ GLAMO_REG_CLOCK_GEN8,		0x0100 },
	{ GLAMO_REG_CLOCK_GEN10,	0x0017 },
	{ GLAMO_REG_CLOCK_GEN11,	0x0017 },

	/* hostbus interface */
	{ GLAMO_REG_HOSTBUS(1),		0x0e00 },
	{ GLAMO_REG_HOSTBUS(2),		0x07ff },
	{ GLAMO_REG_HOSTBUS(4),		0x0080 },
	{ GLAMO_REG_HOSTBUS(5),		0x0244 },
	{ GLAMO_REG_HOSTBUS(6),		0x0600 },
	{ GLAMO_REG_HOSTBUS(12),	0xf00e },

	/* memory */
	{ GLAMO_REG_MEM_TYPE,		0x0874 }, /* VRAM 8Mbyte */
	{ GLAMO_REG_MEM_GEN,		0xafaf },
	{ GLAMO_REG_MEM_TIMING(1),	0x0108 },
	{ GLAMO_REG_MEM_TIMING(2),	0x0010 },
	{ GLAMO_REG_MEM_TIMING(3),	0x0000 },
	{ GLAMO_REG_MEM_TIMING(4),	0x0000 },
	{ GLAMO_REG_MEM_TIMING(5),	0x0000 },
	{ GLAMO_REG_MEM_TIMING(6),	0x0000 },
	{ GLAMO_REG_MEM_TIMING(7),	0x0000 },
	{ GLAMO_REG_MEM_TIMING(8),	0x1002 },
	{ GLAMO_REG_MEM_TIMING(9),	0x6006 },
	{ GLAMO_REG_MEM_TIMING(10),	0x00ff },
	{ GLAMO_REG_MEM_TIMING(11),	0x0001 },
	{ GLAMO_REG_MEM_POWER1,		0x0020 },
	{ GLAMO_REG_MEM_POWER2,		0x0000 },
	{ GLAMO_REG_MEM_DRAM1,		0x0000 },
		{ 0xfffe, 1 },
	{ GLAMO_REG_MEM_DRAM1,		0xc100 },
	{ GLAMO_REG_MEM_DRAM2,		0x01d6 },
};

#if 0
static struct glamo_script gl3362_init_script[] = {
	/* clock */
	{ GLAMO_REG_CLOCK_MEMORY, 	0x300a },
};
#endif

static void glamo_run_script(struct glamo_script *script, int num)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(gl3362_init_script); i++) {
		struct glamo_script *reg = script + i;
		printf("reg=0x%04x, val=0x%04x\n", reg->reg, reg->val);

		if (reg->reg == 0xfffe)
			udelay(reg->val*1000);
		else
			glamo_reg_write(reg->reg, reg->val);
	}

}

static void glamo_core_init(void)
{
	printf("Glamo core device ID: 0x%04x, Revision 0x%04x\n",
		glamo_reg_read(GLAMO_REG_DEVICE_ID),
		glamo_reg_read(GLAMO_REG_REVISION_ID));

	glamo_run_script(gl3362_init_script, ARRAY_SIZE(gl3362_init_script));
}

void *video_hw_init(void)
{
	u_int16_t reg;
	GraphicDevice *pGD = (GraphicDevice *)&smi;

	glamo_core_init();

	printf("Video: ");

	/* FIXME: returning since vram access still locks up system */
	return NULL;

	/* FIXME: this is static */
	pGD->winSizeX = pGD->plnSizeX = 480;
	pGD->winSizeY = pGD->plnSizeY = 640;
	pGD->gdfBytesPP = 2;
	pGD->gdfIndex = GDF_16BIT_565RGB;

	pGD->frameAdrs = CONFIG_GLAMO_BASE + 0x00800000;
	pGD->memSize = 0x200000; /* 480x640x16bit = 614400 bytes */

	//printf("memset ");
	//memset(pGD->frameAdrs, 0, pGD->memSize);

	printf("END\n");

	return &smi;
}

void
video_set_lut(unsigned int index, unsigned char r,
	      unsigned char g, unsigned char b)
{
	/* FIXME: we don't support any palletized formats */
}

#endif /* CONFIG_VIDEO_GLAMO3362 */
