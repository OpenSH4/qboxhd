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
#include <asm/hardware.h>

static char *ratio_str[] = {
	"1/1 ",
	"1/2 ",
	"1/3 ",
	"1/4 ",
	"1/6 ",
	"1/8 ",
	"1/16",
	"??"
};

static char *ick_code[] = {
	"ICK=1   ",
	"ICK=1/2 ",
	"ICK=1/3 ",
	"ICK=1/4 ",
	"ICK=1/6 ",
	"ICK=1/8 "
};

static char *bfc_ratio[] = {
	"BCK=1   ",
	"BCK=1/2 ",
	"BCK=1/3 ",
	"BCK=1/4 ",
	"BCK=1/6 ",
	"BCK=1/8 "
};

static char *pfc_ratio[] = {
	"PCK=1/2 ",
	"PCK=1/3 ",
	"PCK=1/4 ",
	"PCK=1/6 ",
	"PCK=1/8 ",
	"PCK=??  "
};

static int init_femi (void)
{
	*FEMI_A0MCR = 0x0b777110;
	*FEMI_A1MCR = 0x0b777108;
	*FEMI_A2MCR = 0x03777510;
	*FEMI_A3MCR = 0x0b777110;
	*FEMI_A4MCR = 0x0b777108;
}

long int initdram (int board_type)
{
	/* This has been done for us */
	return CFG_SDRAM_SIZE;
}

void flashWriteEnable(void)
{
  ;
}

void flashWriteDisable(void)
{
  ;
}

int board_init(void)
{
	init_femi ();
	return 0;
}

int checkboard (void)
{
	int frq_ratio = (*CPG_FRQCR & 0x1FF);
	int ifc_code = ((frq_ratio >> 6) & 0x07);
	int bfc = ((frq_ratio >> 3) & 0x07);
	int pfc = (frq_ratio & 0x07);

	printf ("\n\nBoard: Microdev sh4-202\n");
	printf ("CPU Speed %d MHz\n", CPU_CLOCK_RATE / 1000000);
	printf ("FRQCR  (0x%x) => %s %s %s\n", *CPG_FRQCR, ick_code[ifc_code],
		bfc_ratio[bfc], pfc_ratio[pfc]);

	return 0;
}

int do_reset (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	ulong sr;
      asm ("stc sr, %0":"=r" (sr));
	sr |= (1 << 28);	/* set block bit */
      asm ("ldc %0, sr": :"r" (sr));
	asm volatile ("trapa #0");

	 /*NOTREACHED*/ return (0);
}
