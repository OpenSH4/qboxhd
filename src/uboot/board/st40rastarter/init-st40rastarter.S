/* $Id: init-st40rastarter.S,v 1.1 2005/05/17 10:32:12 sturgesa Exp $
 *
 *  gdb-sh-stub/init-st40rastarter.S
 */

#include "../../cpu/sh4_1xx/init-st40common.S"

	.balign 4
9:

	! FMI configuration

	! Program Bank Addresses
	.long	4, 0xa7f00860, 0x00000006	! EMI_BANKNUMBER
	.long	4, 0xa7f00800, 0x00000000	! EMI_BANK0_BASEADDRESS ## Bank 0 - Boot Flash : 0x00000000 - 0x00FFFFFF
	.long	4, 0xa7f00810, 0x00000004	! EMI_BANK1_BASEADDRESS ## Bank 1 - STEM0 32 MB Flash Module : 0x01000000 - 0x02FFFFFF
	.long	4, 0xa7f00820, 0x0000000c	! EMI_BANK2_BASEADDRESS ## Bank 2 - STEM1 reserved : 0x03000000 - 0x03FFFFFF
	.long	4, 0xa7f00830, 0x00000014	! EMI_BANK3_BASEADDRESS ## Bank 3 - Free : 0x04000000 - 0x04FFFFFF
	.long	4, 0xa7f00840, 0x00000018	! EMI_BANK4_BASEADDRESS ## Bank 4 - Free : 0x05000000 - 0x05FFFFFF
	.long	4, 0xa7f00850, 0x0000001c	! EMI_BANK5_BASEADDRESS ## Bank 5 - EPLD registers : 0x07000000 - 0x07eFFFFF

	! Program Bank Function 

	! Bank 0: Configured for Flash 8Mb
	.long	4, 0xa7f00100, 0x001016e9
	.long	4, 0xa7f00108, 0x9d200000
	.long	4, 0xa7f00110, 0x9d200000
	.long	4, 0xa7f00118, 0x00000000

	! Bank 1: STEM Module (reset configuration not changed)

	! Bank 2: STEM Module (reset configuration not changed)

	! Bank 3: Unused

	! Bank 4: Unused
	
	! Bank 5: Configured for EPLD Registers
	.long	4, 0xa7f00240, 0x001016e9 	! EMI_BANK5_EMICONFIGDATA0
	.long	4, 0xa7f00248, 0x9d200000 	! EMI_BANK5_EMICONFIGDATA1
	.long	4, 0xa7f00250, 0x9d200000 	! EMI_BANK5_EMICONFIGDATA2
	.long	4, 0xa7f00258, 0x00000000 	! EMI_BANK5_EMICONFIGDATA3

	! Program Other EMI Registers

	.long	4, 0xa7f00028, 0x00000000	! EMI_GENCFG (sdram refresh bank 1)
	.long	4, 0xa7f00050, 0x00000002	! EMI_FLASHCLKSEL (1/3 bus clock) 
	.long	4, 0xa7f00068, 0x00000001	! EMI_CLKENABLE

	! LMI configuration
	! Program system configuration registers
	.long	4, 0xbb190018, 0x00000200	! SYSCONF_SYS_CON2[0] 
	
	! LMI registers
	.long	4, 0xaf000008, 0x04000283	! LMI_MIM   
	.long	4, 0xaf000018, 0x000011db	! LMI_STR
	.long	4, 0xaf000030, 0x0c001500	! LMI_SDRA0 
	.long	4, 0xaf000038, 0x0c001500	! LMI_SDRA1 

	! Program LMI_SDRAM_CONTROL register
	.long	4, 0xaf000010, 0x00000001	! LMI_SCR 
	.long	4, 0xaf000010, 0x00000003	! LMI_SCR 
	.long	4, 0xaf000010, 0x00000001	! LMI_SCR
	.long	4, 0xaf000010, 0x00000002	! LMI_SCR
	.long	4, 0xaf800000|0x00002000, 0	! LMI_SDMR0
	.long	4, 0xaf900000|0x00002000, 0	! LMI_SDMR1
	.long	4, 0xaf800000|0x00000b10, 0	! LMI_SDMR0
	.long	4, 0xaf900000|0x00000b10, 0	! LMI_SDMR1
	.long	4, 0xaf000010, 0x00000002	! LMI_SCR 
	.long	4, 0xaf000010, 0x00000004	! LMI_SCR 
	.long	4, 0xaf000010, 0x00000004	! LMI_SCR 
	.long	4, 0xaf000010, 0x00000004	! LMI_SCR 
	.long	4, 0xaf800000|0x00000310, 0	! LMI_SDMR0
	.long	4, 0xaf900000|0x00000310, 0	! LMI_SDMR1
	.long	4, 0xaf000010, 0x00000000	! LMI_SCR 

	.long	0,0,0
19:
