/*-
 * An rtc/i2c driver for the Ricoh R2025 real-time clock module
 *
 * Copyright (c) 2006 Shigeyuki Fukushima.
 * All rights reserved.
 *
 * Written by Shigeyuki Fukushima.
 *
 * Port to Linux by
 * Jean-Christophe Plagniol-Villard <jcplagniol@wyplay.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _R2025_H_
#define _R2025_H_


#define	R2025_I2C_ADDR			0x32

#define	R2025_REG_BASE			0x0f

#define	R2025_REGISTER_SIZE		16

#define R2025_CLK_SIZE			7	

#define R2025_SEC			(0x0)
#define R2025_MIN			(0x1)
#define R2025_HOUR			(0x2)
#define R2025_WDAY			(0x3)
#define R2025_MDAY			(0x4)
#define R2025_MON			(0x5)
#define R2025_YEAR			(0x6)
#define R2025_CORRECTCLOCK		(0x7<<4)
#define R2025_ALARMW_MIN		(0x8<<4)
#define R2025_ALARMW_HOUR		(0x9<<4)
#define R2025_ALARMW_WDAY		(0xa<<4)
#define R2025_ALARMD_MIN		(0xb<<4)
#define R2025_ALARMD_HOUR		(0xc<<4)
#define R2025_RESERVED			(0xd<<4)
#define R2025_CTRL1			(0xe<<4)
#define R2025_CTRL2			(0xf<<4)


#define R2025_SEC_MASK			0x7f
#define R2025_MIN_MASK			0x7f
#define R2025_HOUR_MASK			0x3f
#define R2025_WDAY_MASK			0x07
#define R2025_MDAY_MASK			0x3f
#define R2025_MON_MASK			0x1f
#define R2025_YEAR_MASK			0xff
#define R2025_CORRECTCLOCK_MASK		0x7f
#define R2025_ALARMW_MIN_MASK		0x7f
#define R2025_ALARMW_HOUR_MASK		0x3f
#define R2025_ALARMW_WDAY_MASK		0x7f
#define R2025_ALARMD_MIN_MASK		0x7f
#define R2025_ALARMD_HOUR_MASK		0x3f
#define R2025_CTRL1_MASK		0xff
#define R2025_CTRL2_MASK		0xff

#define R2025_MON_Y1920			(1u << 7)

#define R2025_CTRL1_WALE		(1u << 7)
#define R2025_CTRL1_DALE		(1u << 6)
#define R2025_CTRL1_H1224		(1u << 5)
#define R2025_CTRL1_CLEN2		(1u << 4)
#define R2025_CTRL1_TEST		(1u << 3)
#define R2025_CTRL1_CT2			(1u << 2)
#define R2025_CTRL1_CT1			(1u << 1)
#define R2025_CTRL1_CT0			(1u << 0)

#define R2025_CTRL2_VDSL		(1u << 7)
#define R2025_CTRL2_VDET		(1u << 6)
#define R2025_CTRL2_XST			(1u << 5)
#define R2025_CTRL2_PON			(1u << 4)
#define R2025_CTRL2_CLEN1		(1u << 3)
#define R2025_CTRL2_CTFG		(1u << 2)
#define R2025_CTRL2_WAFG		(1u << 1)
#define R2025_CTRL2_DAFG		(1u << 0)

#endif /* _R2025_H_ */

