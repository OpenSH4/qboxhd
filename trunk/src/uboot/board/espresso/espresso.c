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
#include <asm/io.h>
#include <asm/pio.h>

#if (CONFIG_COMMANDS & CFG_CMD_IDE)
extern void sti5528_ide_init(void);
#endif

void flashWriteEnable(void)
{
  ;
}

void flashWriteDisable(void)
{
  ;
}

#define PIO_BASE	0xba025000 /*  PIO port 5 */

static void configPIO(void)
{
  SET_PIO_ASC(PIO_BASE, 4, 5, 6, 7) ;
  SET_PIO_ASC(PIO_BASE, 0, 1, 2, 3) ;
}

int board_init(void)
{
	configPIO();
#if (CONFIG_COMMANDS & CFG_CMD_IDE)
	sti5528_ide_init();
#endif
	return 0;
}

int checkboard (void)
{
	printf ("\n\nBoard: STI5528-Espresso\n");
	printf ("CPU Speed %d MHz\n", CPU_CLOCK_RATE / 1000000);
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
