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

	
#include <config.h>
#include "asm/regdef.h"
#include "asm/hardware.h"
#include "asm/asmdefs.h"
#include "../lib_sh4/linkage.h"

	.text
	
ENTRY(init_ram)
	MOV_CONST32_R0 0XA0000000
	/* If in flash then init SDRAM */
	sts	pr, r1
	or	r0, r1			/* normalise to a P2 address		*/
	mov.l	sdram_base, r2		/* address of SDRAM			*/
	or	r0, r2			/* make sure p2				*/
	cmp/hi	r2, r1			/* are we already in SDRAM ?		*/
	bt	end
	
	/* set EMI to 1/2 (133MHz) CPG2_FRQCR3 = (*CPG2_FRQCR3 & 0x0F8 ) | 1 */
	mov.l	cpg2, r1
	mov.l	@r1, r0
	and	#0xf8, r0
	or	#1, r0
	mov.l	r0,@r1

	/* Init registers */
	mova	sdram_init_table, r0
	mov	r0, r1
	
loop:	mov.l	@r1+, r0
	cmp/eq	#0, r0
	bt	end
	cmp/eq	#1, r0
	bf	no_delay
	/* delay for a while */
	mov.l	delay_count, r2
dl:	dt	r2
	bf	dl
	bra	loop
	nop
no_delay:
	mov.l	@r1+, r2
	mov.l	r2, @r0
	bra	loop
	nop
end:	rts
	nop
	
	.balign 4
	
sdram_base:	.long CFG_SDRAM_BASE
cpg2:		.long CPG2_FRQCR3
delay_count:	.long 100000	/* Should be enough */
	
sdram_init_table:	
	.long \
	EMI_COC, 0x40,		/* 40 is better for wk45 boards */ \
	1, /* sleep */ \
	EMI_MIM, 0x21, \
	EMI_STR, 0x0000116B, \
	EMI_SDRA0, 0x0a001900, /* ## SDRAM address split 13x9  */ \
	EMI_SDRA1, 0x0c001900, \
	EMI_SCR, 0x00000003,   /* clock enable */ \
	1, /* sleep */ \
	EMI_SCR, 0x00000002, \
	EMI_SCR, 0x00000004, \
	EMI_SCR, 0x00000004, \
	EMI_SCR, 0x00000004, \
	EMI_SCR, 0x00000004, \
	EMI_SCR, 0x00000004, \
	EMI_SCR, 0x00000004, \
	EMI_SCR, 0x00000004, \
	EMI_SCR, 0x00000004, \
	EMI_SDMR0+0x110, 0xffffffff, /* set sdmr 110 =>  BL=010 WT=0  LTMODE=010 DLL= 0 */ \
	EMI_SDMR1+0x110, 0xffffffff,\
	EMI_MIM,  0x00900281, /*enabled,sdram,little endian, refresh enabled, 90 rfrsh interval */ \
	EMI_SCR,  0x00000000, \
	0 /* END */
