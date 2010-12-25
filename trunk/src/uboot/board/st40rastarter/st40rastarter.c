/*
 * (C) Copyright 2004 STMicroelectronics.
 *
 * Andy Sturges <andy.sturges@st.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>
#include <asm/io.h>
#include <asm/hardware.h>

#define EPLD_BASE		0xa7000000
#define EPLD_SIZE		0x34

#define EPLD_REVID		(EPLD_BASE+0x00000000)
#define EPLD_RESET		(EPLD_BASE+0x00000004)
#define EPLD_LED_SET		(EPLD_BASE+0x00000008)
#define EPLD_LED_CLR		(EPLD_BASE+0x0000000c)
#define EPLD_VPP		(EPLD_BASE+0x00000010)
#define EPLD_INTMASK0		(EPLD_BASE+0x00000014)
#define EPLD_INTMASK0SET	(EPLD_BASE+0x00000018)
#define EPLD_INTMASK0CLR	(EPLD_BASE+0x0000001c)
#define EPLD_INTMASK1		(EPLD_BASE+0x00000020)
#define EPLD_INTMASK1SET	(EPLD_BASE+0x00000024)
#define EPLD_INTMASK1CLR	(EPLD_BASE+0x00000028)
#define EPLD_INTSTAT0		(EPLD_BASE+0x0000002c)
#define EPLD_INTSTAT1		(EPLD_BASE+0x00000030)

#define EPLD_LED_ON   1
#define EPLD_LED_OFF  0

int board_init(void)
{
	return 0;
}

int checkboard (void)
{
  printf("\n\nBoard: ST40RA Starter mb374");
  printf("CPU Speed %d MHz\n", CPU_CLOCK_RATE/1000000);
  return 0;
}

void flashWriteEnable(void)
{
  ctrl_outl(1, EPLD_VPP);
}

void flashWriteDisable(void)
{
  ctrl_outl(0, EPLD_VPP);
}

int do_reset (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	ulong sr;
	asm("stc sr, %0" : "=r"(sr));
	sr |= (1 << 28); /* set block bit */
	asm("ldc %0, sr" : : "r"(sr));
	asm volatile ("trapa #0");

	/*NOTREACHED*/
	return (0);
}
