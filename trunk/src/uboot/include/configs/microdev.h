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
#define CONFIG_CPU_SUBTYPE_SH4_2XX    /* its an SH4-202                        */
#define CONFIG_SH_SH4202_MICRODEV 1   /* This is an SH4-202 MICRODEV CPU BOARD */

#define CPU_CLOCK_RATE	266000000            /* 266 MHz clock for the SH4 core */
#define P_CLOCK_RATE    (CPU_CLOCK_RATE/4)   /* Peripheral clock for SH4 core  */
#define B_CLOCK_RATE    (CPU_CLOCK_RATE/2)   /* Bus clock for SH4 core         */

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * Assume we run out of uncached memory for the moment
 */
 
#define CFG_SDRAM_BASE		0x88000000      /* SDRAM in P1 region         */
#define CFG_SDRAM_SIZE		0x04000000
#define CFG_FLASH_BASE		0xA0000000
#define CFG_RESET_ADDRESS	0xA0000000

#define CFG_MONITOR_LEN		0x00040000	/* Reserve 256 kB for Monitor */
#define CFG_MONITOR_BASE        CFG_FLASH_BASE 
#define CFG_MALLOC_LEN		(1 << 20)	/* Reserve 1MB kB for malloc */
#define CFG_BOOTPARAMS_LEN	(128 << 10)
#define CFG_GBL_DATA_SIZE	1024		/* Global data structures */

#define CFG_MEMTEST_START   	CFG_SDRAM_BASE
#define CFG_MEMTEST_END     	(CFG_SDRAM_BASE + CFG_SDRAM_SIZE - (2 << 20))

#define CONFIG_BAUDRATE		38400
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

#define XSTR(s) STR(s)
#define STR(s) #s

#define	CONFIG_EXTRA_ENV_SETTINGS \
		"board=microdev\0" \
		"monitor_base=" XSTR(CFG_MONITOR_BASE) "\0" \
		"monitor_len=" XSTR(CFG_MONITOR_LEN) "\0" \
		"monitor_sec=1:0\0" \
		"load_addr=" XSTR(CFG_LOAD_ADDR) "\0" \
		"unprot=" \
		  "protect off $monitor_sec\0" \
		"update=" \
		  "erase $monitor_sec;" \
		  "cp.b $load_addr $monitor_base $monitor_len;" \
		  "protect on $monitor_sec\0" 


#define CONFIG_COMMANDS	(CONFIG_CMD_DFL | CFG_CMD_ASKENV  | CFG_CMD_NFS | CFG_CMD_PING | CFG_CMD_DHCP)

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

/*
 * Serial console info 
 */
#define CFG_SH_SCIF_SERIAL 
#define CONFIG_CONS_INDEX 0

#if 0
#define CFG_SCONSOLE_ADDR      (CFG_FLASH_BASE	+ (2048 << 10))
#define CFG_SCONSOLE_SIZE      (1024 << 10)
#endif

/*
 *
 * Ethernet driver
 *
 */

#define CONFIG_DRIVER_SMC91111 1
#define	CONFIG_SMC91111_BASE	0xa7500000ul

/*
 * Miscellaneous configurable options
 */
#define CFG_HUSH_PARSER         1 
#define	CFG_LONGHELP		1		/* undef to save memory		*/
#define CFG_PROMPT		"MICRODEV> "	/* Monitor Command Prompt	*/
#define CFG_PROMPT_HUSH_PS2     "MICRODEV> "
#define CFG_CBSIZE		256
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size	*/
#define CFG_MAXARGS		16		/* max number of command args	*/
#define CFG_HZ			(P_CLOCK_RATE/1024) /* HZ for timer ticks	*/
#define CFG_LOAD_ADDR		CFG_SDRAM_BASE	/* default load address		*/
#define CFG_BOOTMAPSZ           (16 << 20)      /* initial linux memory size 	*/
#define CONFIG_BOOTDELAY	10		/* default delay before executing bootcmd */

/*-----------------------------------------------------------------------
 * FLASH organization
 */

/* Microdev organised as 32MB flash with 256k blocks */
#define CFG_FLASH_CFI_DRIVER
#define CFG_FLASH_CFI
#define CFG_FLASH_PROTECTION    1	/* use hardware flash protection        */
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CFG_MAX_FLASH_SECT	128	/* max number of sectors on one chip 	*/


/*-----------------------------------------------------------------------
 * NVRAM organization
 */

/* Address and size of Primary Environment Sector	*/

#define	CFG_ENV_IS_IN_FLASH	1 
#define CFG_ENV_OFFSET          0x40000
#define CFG_ENV_ADDR            (CFG_FLASH_BASE + CFG_ENV_OFFSET)
#define CFG_ENV_SIZE            0x20000
#define CFG_ENV_SECT_SIZE       0x40000

#endif	/* __CONFIG_H */
