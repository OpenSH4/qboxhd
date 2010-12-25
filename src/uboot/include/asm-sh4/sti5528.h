/*
 * include/asm-sh4/sh4_202.h
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2004, STMicroelectronics
 */

#ifndef __ASM_STI5528_H
#define __ASM_STI5528_H

#define TMU_BASE_ADDRESS    0xffd80000
#define RTC_BASE_ADDRESS    0xffc80000

#ifndef SH4_TMU_REGS_BASE
#define SH4_TMU_REGS_BASE 0xffd80000
#endif
#ifndef SH4_RTC_REGS_BASE
#define SH4_RTC_REGS_BASE 0xffc80000
#endif

/* Common ST40 control registers */
#ifndef ST40_CPG_REGS_BASE
#define ST40_CPG_REGS_BASE 0xffc00000
#endif
#ifndef ST40_INTC_REGS_BASE
#define ST40_INTC_REGS_BASE 0xffd00000
#endif
#ifndef ST40_INTC2_REGS_BASE
#define ST40_INTC2_REGS_BASE 0xfe080000
#endif
#ifndef ST40_SCIF1_REGS_BASE
#define ST40_SCIF1_REGS_BASE 0xffe00000
#endif
#ifndef ST40_SCIF2_REGS_BASE
#define ST40_SCIF2_REGS_BASE 0xffe80000
#endif

#endif
