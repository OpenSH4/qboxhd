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
#include <asm/stb7100reg.h>
#include <asm/io.h>
#include <asm/pio.h>

void stb7100_reset(void);
void stb7100_clocks(void);

#define EPLD_FLASH *(volatile unsigned char *)(0xa3400000)
#define EPLD_ATAPI *(volatile unsigned char *)(0xa3900000)

#define LED *(volatile unsigned char *)(0xa2000000 + 0x00100010)

void flashWriteEnable(void)
{
  /*  Enable vpp for writing to flash */
//  EPLD_FLASH = 3;	/* bits: 0 = VPP ON; 1 = RESET	*/
}

void flashWriteDisable(void)
{
  /*  Disable vpp for writing to flash */
//  EPLD_FLASH = 2;	/* bits: 0 = VPP ON; 1 = RESET	*/
}

#define PIO_BASE  0xb8020000  

static void configPIO(void)
{
  /*  Setup PIO of ASC device */
  SET_PIO_ASC(PIO_PORT(4), 3, 2, 4, 5);  /* UART2 - AS0 */
//  SET_PIO_ASC(PIO_PORT(5), 0, 1, 2, 3);  /* UART3 - AS1 */
  
  /*  Setup up ethernet reset */
/*
#ifdef CONFIG_DRIVER_SMC91111
  SET_PIO_PIN(PIO_PORT(2), 6, STPIO_OUT);  
#endif
#ifdef CONFIG_DRIVER_NETSTMAC
  SET_PIO_PIN(PIO_PORT(2), 4, STPIO_OUT);  
#endif
*/
}

#if (CONFIG_COMMANDS & CFG_CMD_IDE)

#ifdef CONFIG_SH_STB7100_IDE
static void stb7100ref_init_ide()
{
//  EPLD_ATAPI = 1; /* Enable ATAPI mode of EMI */
}
#endif

#ifdef CONFIG_SH_STB7100_SATA
extern void stb7100_sata_init(void);
#endif

#endif

int board_init(void)
{
	unsigned long sysconf;
	/* Route UART2 instead of SCI to PIO4 */
	/* Set ssc2_mux_sel = 0 */
	sysconf = *STB7100_SYSCONF_SYS_CFG07;
	sysconf &= ~(1<<3);
	*STB7100_SYSCONF_SYS_CFG07 = sysconf;
	
	configPIO();

#ifdef CONFIG_DRIVER_SMC91111
	/*  Reset ethernet chip */
/*	STPIO_SET_PIN(PIO_PORT(2), 6, 0);
	udelay(1000);
	STPIO_SET_PIN(PIO_PORT(2), 6, 1);
	udelay(1000);
	STPIO_SET_PIN(PIO_PORT(2), 6, 0);    */            
#endif

#ifdef CONFIG_DRIVER_NETSTMAC
	/*  Reset ethernet chip */
/*	STPIO_SET_PIN(PIO_PORT(2), 4, 1);
	udelay(1000);
	STPIO_SET_PIN(PIO_PORT(2), 4, 0);
	udelay(2000);
	STPIO_SET_PIN(PIO_PORT(2), 4, 1);     */           
#endif

//Duolabs
#ifndef CONFIG_QBOXHD_mini
	//Set 1 the "popnoise_suppress"
	SET_PIO_PIN(PIO_PORT(3), 2, STPIO_OUT);
	STPIO_SET_PIN(PIO_PORT(3), 2, 1);
#endif

#if (CONFIG_COMMANDS & CFG_CMD_IDE)
#ifdef CONFIG_SH_STB7100_IDE
	stb7100ref_init_ide();
#endif
#ifdef CONFIG_SH_STB7100_SATA
	stb7100_sata_init();
#endif
#endif

	return 0;
}

int checkboard (void)
{
//	printf ("\n\nBoard: STb7100ref\n");
#ifdef CONFIG_QBOXHD_mini
	printf ("\nBoard: QBOX-HD mini\n");
#else
	printf ("\nBoard: QBOX-HD\n");
#endif
//	LED = 1;
	
	return 0;
}

int do_reset (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	stb7100_reset();
	 /*NOTREACHED*/ return (0);
}
