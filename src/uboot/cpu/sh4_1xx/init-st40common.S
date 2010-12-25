/* $Id: init-st40common.S,v 1.8 2005/11/30 11:52:56 sturgesa Exp $
 *
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

	MOV_CONST32_R0 0XA0000000
	/* If in flash then init SDRAM */
	mov	r14, r1
	or	r0, r1			/* normalise to a P2 address		*/
	mov.l	sdram_base, r2		/* address of SDRAM			*/
	or	r0, r2			/* make sure p2				*/
	cmp/hi	r2, r1			/* are we already in SDRAM ?		*/
	bt	end

	/* First turn half the Ocache into RAM, and enable the I cache */
	mov.l	CCR_RAM_D, r4	/* CCR Data */
	CALL    67f

	/* Next install the code into the cache */
	mova	CODE_SRC, r0
	mov.l	CODE_SRC, r1
	add	r0, r1
	mov.l	CODE_SRC_END, r4
	add	r0, r4
	mov.l	ITLB_MASK, r0
	and	r1, r0
	mov.l	ITLB_ADDR, r2
	add	r0, r2
	mov.l	ITLB_DATA, r3
	add	r0, r3
1:	mov	r1, r0
	add	#1, r0
	mov.l	r0, @r2
	mov.l	@(0x00, r1), r0
	mov.l	r0, @(0x00, r3)
	mov.l	@(0x04, r1), r0
	mov.l	r0, @(0x04, r3)
	mov.l	@(0x08, r1), r0
	mov.l	r0, @(0x08, r3)
	mov.l	@(0x0c, r1), r0
	mov.l	r0, @(0x0c, r3)
	mov.l	@(0x10, r1), r0
	mov.l	r0, @(0x10, r3)
	mov.l	@(0x14, r1), r0
	mov.l	r0, @(0x14, r3)
	mov.l	@(0x18, r1), r0
	mov.l	r0, @(0x18, r3)
	mov.l	@(0x1c, r1), r0
	mov.l	r0, @(0x1c, r3)
	add	#0x20, r1
	add	#0x20, r2
	add	#0x20, r3
	/* Need to cope with cache wrapping around */
	cmp/ge	r4, r1
	bf	1b

	/* Next copy the data we need into the cache RAM */
	mova	DATA_SRC, r0
	mov	r0, r3
	mov.l	DATA_SRC, r0
	add	r3, r0
	mov.l	DATA_DEST, r1
	mov.l	DATA_SRC_END, r2
	add	r3, r2
1:	mov.l	@r0, r3
	mov.l	r3, @r1
	add	#4, r0
	add	#4, r1
	cmp/eq	r0, r2
	bf	1b

/* load address of scratch reg */
	mov.l   TTB, r5

#if 1
	/* Finally call into the cached code */
	mova	CODE_SRC, r0
	mov.l	CODE_SRC, r1
	add	r1, r0
	mov.l	P1_MASK, r1
	and	r1, r0
	mov.l	DATA_DEST, r1
	jsr	@r0
	nop
#endif

	/* Turn the cache RAM back into cache and disable and invalidate the I cache */
	mov.l	CCR_NONE_D, r4	/* CCR Data */
	CALL	67f

	/* Restore the PR */
	lds     r14, pr
end:
	rts
	 nop

	.align	2
67:	.long	68f-67b
CCR_RAM_D:
	.long	CCR_ORA | CCR_OCE | CCR_ICE
CCR_NONE_D:
	.long	CCR_ICI | CCR_OCI

68:
	mov.l	CCR_A, r0
	mov.l	r4, @r0
	rts
	 nop

	.align	2
CCR_A:	.long   CCN_CCR

	.align	2
CODE_SRC:
	.long	7f-CODE_SRC
CODE_SRC_END:
	.long	17f-CODE_SRC
ITLB_ADDR:
	.long	0xf0000000
ITLB_DATA:
	.long	0xf1000000
ITLB_MASK:
	.long	0x1fe0

DATA_SRC:
	.long	9f-DATA_SRC
DATA_SRC_END:
	.long	19f-DATA_SRC
DATA_DEST:
	.long	CACHE_RAM

P1_MASK:
	.long	~0x20000000

	/*
	 * This is the code which is loaded into the cache and performs the pokes.
	 * Take extra code when writing here:
	 *   - it must be position independant (compiled for P2, run from P1).
	 *   - it must not do any data accesses from anywhere other than cache RAM,
	 *     (watch for 'mov.x label' instructions especially).
	 */
	 
#define WHILE_NE(A, AND, VAL)	.long 0x44, A, AND, VAL
#define UPDATE_LONG(A, AND, OR)	.long 0x24, A, AND, OR
#define OR_LONG(A, V)		.long 0x14, A, V
#define POKE_LONG(A, V)		.long 0x04, A, V
#define POKE_SHORT(A, V)	.long 0x02, A, V
#define POKE_CHAR(A, V)		.long 0x01, A, V
#define END_MARKER		.long 0x00, 0, 0
		 
	.balign	32	/* Align to a cache line */
7:
	mov.l	@r1+, r0	! Size
	mov.l	@r1+, r3	! Addr
	mov.l	@r1+, r4	! Data

	mov.l   r4, @r5

	! 0 - exit
	cmp/eq	#0, r0
	bf	1f
	rts
	 nop

1:	! 1 - poke byte
	cmp/eq	#1, r0
	bf	1f
	bra	6f
	 mov.b	r4, @r3

1:	! 2 - poke word
	cmp/eq	#2, r0
	bf	1f
	bra	6f
	 mov.w	r4, @r3

1:	! 4 - poke long
	cmp/eq	#4, r0
	bf	1f
	bra	6f
	 mov.l	r4, @r3

 1:	! 0x14 - OR long
	cmp/eq	#0x14, r0
	bf	1f
	mov.l	@r3, r0
	or	r4, r0
	bra	6f
	 mov.l	r0, @r3

 1:	! 0x24 - UPDATE long (needs extra param)
	cmp/eq	#0x24, r0
	bf	1f
	mov.l	@r3, r0
	and	r4, r0		! Mask
	mov.l	@r1+, r4	! Data
	or	r4, r0
	bra	6f
	 mov.l	r0, @r3

1:	! 0x34 - wait long mask set
	cmp/eq	#0x34, r0
	bf	1f
2:	mov.l	@r3, r0
	tst	r4, r0	! T if 0
	bt	2b
	bra	6f
	 nop
1:	! 0x44 - wait long mask ne
	cmp/eq	#0x44, r0
	bf	1f
	mov.l   @r1+, r5
2:	mov.l	@r3, r0
	and	r4,  r0
	cmp/eq  r0, r5
	bf	2b
	bra	6f
	 nop

1:

6:

	/* Lets waste some time */
	mov	#0x01, r0
	/* swap.b	r0, r0 */
	swap.w	r0, r0
22:	add	#-1, r0
	cmp/eq	#0, r0
	bf	22b

	bra	7b
	 nop
17:

.balign 4
sdram_base:	.long CFG_SDRAM_BASE
TTB:            .long 0xFF000008
