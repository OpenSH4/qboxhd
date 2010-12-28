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

/* 
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_SH4    1	              /* This is an SH4 CPU	               */
#define CONFIG_CPU_SUBTYPE_1XX    /* its an SH4-103 (ST40) */

#define CPU_CLOCK_RATE	166000000            /* 166 MHz clock for the ST40RA core */
#define P_CLOCK_RATE    (CPU_CLOCK_RATE/3)   /* Peripher clock for SH4 corclocke CW - Changed from 1/4*/
#define B_CLOCK_RATE    (CPU_CLOCK_RATE/2)   /* Bus clock for SH4 core         */

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 */
 
#define CFG_SDRAM_BASE		0x88000000      /* SDRAM in P1 region CW - Modified        */
#define CFG_SDRAM_SIZE		0x03F00000      /* CW - Only 63 of 64M visible? see DSheet. */
#define CFG_FLASH_BASE		0xA0000000
#define CFG_RESET_ADDRESS	0xA0000000

#define CFG_MONITOR_LEN		(128 << 10)	/* Reserve 256 kB for Monitor */
#define CFG_MONITOR_BASE        CFG_FLASH_BASE 
#define CFG_MALLOC_LEN		(1 << 20)	/* Reserve 1MB kB for malloc */
#define CFG_BOOTPARAMS_LEN	(128 << 10)
#define CFG_GBL_DATA_SIZE	1024		/* Global data structures */

#define CFG_MEMTEST_START   	CFG_SDRAM_BASE
#define CFG_MEMTEST_END     	(CFG_SDRAM_BASE + CFG_SDRAM_SIZE - (2 << 20))

#define CONFIG_BAUDRATE		115200
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/*
#define	CONFIG_EXTRA_ENV_SETTINGS \
		"ethaddr=00:0d:88:a2:41:de\0" \
		"update=protect off 1:0;erase 1:0;cp.b 0xa4000000 0xa0000000 40000;protect on 1:0\0"
*/
#define	CONFIG_EXTRA_ENV_SETTINGS


#define CONFIG_COMMANDS	((CONFIG_CMD_DFL | CFG_CMD_NFS | CFG_CMD_PING | CFG_CMD_DHCP) & ~CFG_CMD_NET)


/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

/*
 * Serial console info 
 */

#define CONFIG_CONS_INDEX 0

#define CFG_SCONSOLE_ADDR      (CFG_FLASH_BASE	+ (2048 << 10))
#define CFG_SCONSOLE_SIZE      (1024 << 10)

/* #define CFG_SH_ASC_SERIAL 1 */
#define CFG_SH_SCIF_SERIAL 1

/*
 *
 * Ethernet driver
 *
 */

/*
#define CONFIG_DRIVER_SMC91111 0
#define	CONFIG_SMC91111_BASE	0xa3820300ul 
*/

/*
 * Miscellaneous configurable options
 */
#define CFG_HUSH_PARSER         1 
#define	CFG_LONGHELP		1		/* undef to save memory		*/
#define CFG_PROMPT		"ST40-RA> "	/* Monitor Command Prompt	*/
#define CFG_PROMPT_HUSH_PS2     "ST40-RA> "
#define CFG_CBSIZE		256
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size	*/
#define CFG_MAXARGS		16		/* max number of command args	*/
#define CFG_HZ			(P_CLOCK_RATE/1024) /* HZ for timer ticks	*/
#define CFG_LOAD_ADDR		CFG_SDRAM_BASE	/* default load address		*/
#define CFG_BOOTMAPSZ           (16 << 20)      /* initial linux memory size 	*/
#define CONFIG_BOOTDELAY	20		/* default delay before executing bootcmd */

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CFG_FLASH_CFI_DRIVER
#define CFG_FLASH_CFI
#define CFG_FLASH_PROTECTION    1	/* use hardware flash protection        */
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CFG_MAX_FLASH_SECT	64	/* max number of sectors on one chip 	*/

#if 0
#define CFG_FLASH_PROTECTION    1	/* use hardware flash protection        */
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CFG_MAX_FLASH_SECT	64	/* max number of sectors on one chip 	*/
#endif

/*-----------------------------------------------------------------------
 * NVRAM organization
 */

/* Address and size of Primary Environment Sector	*/

#define	CFG_ENV_IS_IN_FLASH	1 
#define CFG_ENV_ADDR		(CFG_FLASH_BASE + 0x20000) /* 2nd sector */
#define CFG_ENV_SIZE		0x20000 /* 128K */
#define CFG_ENV_SECT_SIZE       0x20000

#if 0
#define	CFG_ENV_IS_NOWHERE
#define CFG_ENV_SIZE		0x20000 /* 128K */
#define CFG_ENV_SECT_SIZE       0x40000
#endif

#endif	/* __CONFIG_H */