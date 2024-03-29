/* $Id: init-st40common.S,v 1.4 2005/11/30 11:52:56 sturgesa Exp $
 *
 *  gdb-sh-stub/init-st40common.S
 */

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
#include "asm/linkage.h"

#define CACHE_RAM		0x7c000000

/*  NOTE:	 ALL THIS CODE MUST BE PIC !!!! */
	
ENTRY(init_ram)
	/* Stash the pr somewhere safe */
	sts     pr, r14

	MOV_CONST32_R0 0X20000000
	/* If in flash then init SDRAM */
	mov	r14, r1
	or	r0, r1			/* normalise to a P2 address		*/
	mov.l	sdram_base, r2		/* address of SDRAM			*/
	or	r0, r2			/* make sure p2				*/
	cmp/hi	r2, r1			/* are we already in SDRAM ?		*/
	bt	end


	/*
	 * I-cache & D-cache on; D-cache in write through
	 */
	mov.l   ccr_address, r0
	mov.l   ccr_mode, r1
	mov.l   r1, @r0
	nop                                     /* Required 8 cycle delay */
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	bra     caches_on
	  nop

.balign 4
ccr_address:            .long SH4_CCN_CCR
ccr_mode:               .long 0x8000090b /* caches on, O-cache write through*/
					 /* Enhanced mode on 200 series cores */

	/*
	 * We need to get the poke loop & associated data
	 * into cache. The poke loop is structured so that it is
	 * all pulled into cache on it 1st iteration.
	 * To get the poke table into Dcache, we simple read it all
	 *
	 */
caches_on:
	mova    poke_loop_address, r0           /* R6: poke loop addr */
	mov.l   @r0, r6
	add     r0, r6
	mov.l   p2_to_p1_mask, r3               /* R3: P2 -> P1 mapping */
	and     r3, r6                          /* convert to P1 addresses */
	mov.l   data_start_address, r1          /* R1 = start address */
	add     r0, r1
	mov.l   data_end_address, r2            /* R2 = end address */
	add     r0, r2
	and     r3, r1                          /* convert to a P1 addresses */
	and     r3, r2
	mov     r1, r5                          /* R5 = scratch data ptr */

1:      mov.l   @r5+, r4                        /* Load poke table */
	cmp/eq  r5, r2
	bf      1b
	bra     do_pokes
	  nop

	.balign 4
poke_loop_address:      .long   poke_loop - poke_loop_address
data_start_address:     .long   __memory_setup_table - poke_loop_address
data_end_address:       .long   __memory_setup_table_end - poke_loop_address
p2_to_p1_mask:          .long ~0x20000000

	/*
	 * Its now safe to call the poke loop with real data, since it
	 * and its associated data table are in onchip caches. Setting
	 * up the memory interfaces may cause the EMI (where this FLASH
	 * image resides) to briefly hang - but the CPU will be safely
	 * executing from cache should this happen.
	 */
do_pokes:
	jsr     @r6                             /* R6 still points to table */
	  nop

	/* Restore the PR */
	lds     r14, pr
end:
	rts
	 nop

.balign 4
sdram_base:	.long CFG_SDRAM_BASE
TTB:            .long 0xFF000008

/*
 * Routine to setup peripheral registers.
 * It is executed from within the I-cache,
 * with all its data in the D-cache
 */
poke_loop:
	mov.l   @r1+, r0        /* opcode */
	mov.l   @r1+, r2        /* address */
	mov.l   @r1+, r3        /* value */

	cmp/eq  #0, r0          /* End marker ? */
	bf      1f

	rts                     /* Return point */
	  nop

1:      cmp/eq  #4, r0          /* 4 byte write... */
	bf      1f
	mov.l   r3, @r2

1:      cmp/eq  #2, r0          /* 2 byte write... */
	bf      1f
	mov.w   r3, @r2

1:      cmp/eq  #1, r0          /* 1 byte write... */
	bf      1f
	mov.b   r3, @r2

1:      cmp/eq  #5, r0          /* 4 byte OR... */
	bf      1f
	mov.l   @r2,r4
	or      r3,r4
	mov.l   r4,@r2

1:      cmp/eq  #6, r0          /* 4 byte UPDATE... */
	bf      1f
	mov.l   @r2,r4
	and     r3,r4
	mov.l   @r1+,r3
	or      r3,r4
	mov.l   r4,@r2

1:      cmp/eq  #8, r0          /* 4 byte write UPDATE... */
	bf      1f
	mov.l   @r3,r4
	mov.l   @r1+,r3
	and     r3,r4
	mov.l   @r1+,r3
	shld    r3,r4
	mov.l   @r1+,r3
	or      r3,r4
	mov.l   r4,@r2

1:      cmp/eq  #7, r0          /* WHILE != ... */
	bf      1f
	mov.l   @r1+,r5
2:      mov.l   @r2,r4
	and     r3,r4
	cmp/eq  r4,r5
	bf      2b

1:      mov     #1,r0           /* small delay */
	swap.w  r0, r0

2:      add     #-1,r0
	cmp/eq  #0, r0
	bf      2b
	bt      poke_loop
