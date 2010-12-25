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
extern void sti5528_ide_init (void);
#endif

#define FLASH_VPP 0xA2300000
#define FLASH_WRITE_ENABLE 0xA1300000

void flashWriteEnable (void)
{
  writeb(3, FLASH_VPP);
  writeb(3, FLASH_WRITE_ENABLE);
}

void flashWriteDisable (void)
{
	;
}

#define SYSCONF_BASE	0xf9162000 /* 19162000 */
#define SYS_DEVICEID	(SYSCONF_BASE + 0x00)
#define SYS_REVISION	(SYSCONF_BASE + 0x04)
#define SYS_REVISION2	(SYSCONF_BASE + 0x10)
#define SYS_STA0	(SYSCONF_BASE + 0x1C)
#define SYS_CFG03	(SYSCONF_BASE + 0x3C)	 /* Comms. ports     */
#define SYS_CFG07	(SYSCONF_BASE + 0x4C)	 /* ST20 boot contro */
#define SYS_CFG10	(SYSCONF_BASE + 0x58)	 /* Ext. IRQs & USB  */

#define PIO_BASE	0xba025000	/*  PIO port 5 */

static void configPIO (void)
{
	unsigned long sysreg;
	/* By default sti5528 board uses SCIF not ASC */

	/*
	 * Enable ST40 SCIF serial system console:
	 * Set System configuration registers so that the
	 * PIO7[0-3] and PIO7[4-7] are used respectively for
	 * SCI0 and SCI1 ST40 serial lines.
	 * Also set PCI_REQN[1-3]_EN to enable PCI_REQ[1-3]
	 * (connected to PIO3[4-6]).
	 */
	sysreg = ctrl_inl(SYS_CFG03);
	sysreg |= 0x00010600 | (1<<19)|(1<<20)|(1<<21);
	ctrl_outl(sysreg, SYS_CFG03);
}

int board_init (void)
{
	configPIO ();
#if (CONFIG_COMMANDS & CFG_CMD_IDE)
	sti5528_ide_init ();
#endif
	return 0;
}

int checkboard (void)
{
	printf ("\n\nBoard: STI5528-Eval\n");
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
