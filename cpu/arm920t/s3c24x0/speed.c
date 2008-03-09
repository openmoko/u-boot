/*
 * (C) Copyright 2001-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2002
 * David Mueller, ELSOFT AG, d.mueller@elsoft.ch
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

/* This code should work for both the S3C2400 and the S3C2410
 * as they seem to have the same PLL and clock machinery inside.
 * The different address mapping is handled by the s3c24xx.h files below.
 */

#include <common.h>
#if defined(CONFIG_S3C2400) || defined (CONFIG_S3C2410) || \
    defined (CONFIG_S3C2440) || defined(CONFIG_S3C2442) || defined (CONFIG_TRAB)

#if defined(CONFIG_S3C2400)
#include <s3c2400.h>
#elif defined(CONFIG_S3C2410)
#include <s3c2410.h>
#elif defined(CONFIG_S3C2440) || defined(CONFIG_S3C2442)
#include <s3c2440.h>
#endif

#define MPLL 0
#define UPLL 1

/* ------------------------------------------------------------------------- */
/* NOTE: This describes the proper use of this file.
 *
 * CONFIG_SYS_CLK_FREQ should be defined as the input frequency of the PLL.
 *
 * get_FCLK(), get_HCLK(), get_PCLK() and get_UCLK() return the clock of
 * the specified bus in HZ.
 */
/* ------------------------------------------------------------------------- */

static ulong get_PLLCLK(int pllreg)
{
    S3C24X0_CLOCK_POWER * const clk_power = S3C24X0_GetBase_CLOCK_POWER();
    ulong r, m, p, s;

    if (pllreg == MPLL)
	r = clk_power->MPLLCON;
    else if (pllreg == UPLL)
	r = clk_power->UPLLCON;
    else
	hang();

    m = ((r & 0xFF000) >> 12) + 8;
    p = ((r & 0x003F0) >> 4) + 2;
    s = r & 0x3;
#if defined(CONFIG_S3C2400) || defined(CONFIG_S3C2410)
    return((CONFIG_SYS_CLK_FREQ * m) / (p << s));
#elif defined(CONFIG_S3C2440) || defined(CONFIG_S3C2442)
    /* To avoid integer overflow, changed the calc order */
    if (pllreg == MPLL)
    	return ( 2 * m * (CONFIG_SYS_CLK_FREQ / (p << s )) );
    else
    	return ( m * (CONFIG_SYS_CLK_FREQ / (p << s )) );
#else
#error "get_PLLCLK not implemented for CPU type"
#endif
}

/* return FCLK frequency */
ulong get_FCLK(void)
{
    return(get_PLLCLK(MPLL));
}

/* return HCLK frequency */
ulong get_HCLK(void)
{
    S3C24X0_CLOCK_POWER * const clk_power = S3C24X0_GetBase_CLOCK_POWER();

#if defined(CONFIG_S3C2400) || defined(CONFIG_S3C2410)
    return((clk_power->CLKDIVN & 0x2) ? get_FCLK()/2 : get_FCLK());
#elif defined(CONFIG_S3C2440) || defined(CONFIG_S3C2442)
    switch (clk_power->CLKDIVN & 0x6) {
        case 0x0:
	    return get_FCLK();
	case 0x2:
	    return get_FCLK()/2;
	case 0x4:
	    return (clk_power->CAMDIVN & 0x200) ? get_FCLK()/8 : get_FCLK()/4;
	case 0x6:
	    return (clk_power->CAMDIVN & 0x100) ? get_FCLK()/6 : get_FCLK()/3;
    }
    return 0;
#else
#error "get_HCLK not implemented for CPU type"
#endif
}

/* return PCLK frequency */
ulong get_PCLK(void)
{
    S3C24X0_CLOCK_POWER * const clk_power = S3C24X0_GetBase_CLOCK_POWER();

    return((clk_power->CLKDIVN & 0x1) ? get_HCLK()/2 : get_HCLK());
}

/* return UCLK frequency */
ulong get_UCLK(void)
{
    return(get_PLLCLK(UPLL));
}

#endif /* defined(CONFIG_S3C2400) || defined (CONFIG_S3C2410) ||
          defined(CONFIG_S3C2440) || defined (CONFIG_S3C2442) ||
	  defined (CONFIG_TRAB) */
