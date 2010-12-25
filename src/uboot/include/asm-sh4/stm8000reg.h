/*
 * Copyright (C) STMicroelectronics Ltd. 2003.
 * andy.sturges@st.com
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

#ifndef __STM8000REG_H
#define __STM8000REG_H

#include "asm/sh4regtype.h"

/*----------------------------------------------------------------------------*/

/*
 * Peripheral versions
 */

#ifndef ST40_LMI_VERSION
#define ST40_LMI_VERSION 3
#endif

/*----------------------------------------------------------------------------*/

/*
 * Base addresses for control register banks
 */

/* Generic SH4 control registers */
#ifndef SH4_TMU_REGS_BASE
#define SH4_TMU_REGS_BASE 0xffd80000
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

/* DVD Platform Architecture Volume 1: Cores */
#ifndef ST40_ILC_REGS_BASE
#define ST40_ILC_REGS_BASE 0xb8300000
#endif
#ifndef ST40_MAILBOX0_REGS_BASE
#define ST40_MAILBOX0_REGS_BASE 0xb0200000
#endif
#ifndef ST40_MAILBOX1_REGS_BASE
#define ST40_MAILBOX1_REGS_BASE 0xb0210000
#endif

/* DVD Platform Architecture Volume 2: System Services */
#ifndef ST40_DMAC_REGS_BASE
#define ST40_DMAC_REGS_BASE 0xb8100000
#endif
#ifndef ST40_CLOCKGENA_REGS_BASE
#define ST40_CLOCKGENA_REGS_BASE 0xb0400000
#endif
#ifndef STM8000_FSA_REGS_BASE
#define STM8000_FSA_REGS_BASE 0xb0410000
#endif
#ifndef STM8000_FSB_REGS_BASE
#define STM8000_FSB_REGS_BASE 0xb0420000
#endif
#ifndef ST40_LMI_REGS_BASE
#define ST40_LMI_REGS_BASE 0xaf000000
#endif
#ifndef ST40_EMI_REGS_BASE
#define ST40_EMI_REGS_BASE 0xa7f00000
#endif
#ifndef ST40_SYSCONF_REGS_BASE
#define ST40_SYSCONF_REGS_BASE 0xb0300000
#endif

/* DVD Platform Architecture Volume 4: I/O Devices */
#ifndef ST40_PIO0_REGS_BASE
#define ST40_PIO0_REGS_BASE 0xb8320000
#endif
#ifndef ST40_PIO1_REGS_BASE
#define ST40_PIO1_REGS_BASE 0xb8321000
#endif
#ifndef ST40_PIO2_REGS_BASE
#define ST40_PIO2_REGS_BASE 0xb8322000
#endif
#ifndef ST40_PIO3_REGS_BASE
#define ST40_PIO3_REGS_BASE 0xb8323000
#endif
#ifndef ST40_PIO4_REGS_BASE
#define ST40_PIO4_REGS_BASE 0xb8324000
#endif
#ifndef ST40_PIO5_REGS_BASE
#define ST40_PIO5_REGS_BASE 0xb8325000
#endif
#ifndef ST40_PIO6_REGS_BASE
#define ST40_PIO6_REGS_BASE 0xb8326000
#endif
#ifndef ST40_PIO7_REGS_BASE
#define ST40_PIO7_REGS_BASE 0xb8327000
#endif
#ifndef ST40_ASC0_REGS_BASE
#define ST40_ASC0_REGS_BASE 0xb8330000
#endif
#ifndef ST40_ASC1_REGS_BASE
#define ST40_ASC1_REGS_BASE 0xb8331000
#endif
#ifndef ST40_ASC2_REGS_BASE
#define ST40_ASC2_REGS_BASE 0xb8332000
#endif
#ifndef ST40_ASC3_REGS_BASE
#define ST40_ASC3_REGS_BASE 0xb8333000
#endif
#ifndef ST40_ASC4_REGS_BASE
#define ST40_ASC4_REGS_BASE 0xb8334000
#endif
#ifndef ST40_SSC0_REGS_BASE
#define ST40_SSC0_REGS_BASE 0xb8340000
#endif
#ifndef ST40_SSC1_REGS_BASE
#define ST40_SSC1_REGS_BASE 0xb8341000
#endif

/* DVD Platform Architecture Volume 6: Video/Audio Encoding */
#ifndef STM8000_LX_GLUE_REGS_BASE
#define STM8000_LX_GLUE_REGS_BASE 0xb4112000
#endif
#ifndef STM8000_SHE_REGS_BASE
#define STM8000_SHE_REGS_BASE 0xb4114000
#endif

/*----------------------------------------------------------------------------*/

#include "asm/st40reg.h"

/*----------------------------------------------------------------------------*/

/*
 * STm8000 control registers
 */

/* Frequency Synthesiser control registers (STm8000 variant) */
#define STM8000_FS_CONFIG_GENERIC_INFO(n) SH4_DWORD_REG(STM8000_FS##n##_REGS_BASE + 0x00)
#define STM8000_FS_CONFIG_CLK_1(n) SH4_DWORD_REG(STM8000_FS##n##_REGS_BASE + 0x08)
#define STM8000_FS_CONFIG_CLK_2(n) SH4_DWORD_REG(STM8000_FS##n##_REGS_BASE + 0x10)
#define STM8000_FS_CONFIG_CLK_3(n) SH4_DWORD_REG(STM8000_FS##n##_REGS_BASE + 0x18)
#define STM8000_FS_CONFIG_CLK_4(n) SH4_DWORD_REG(STM8000_FS##n##_REGS_BASE + 0x20)

/* Lx Glue control registers (STm8000 variant) */
#define STM8000_LX_GLUE_VCR_STATUS SH4_DWORD_REG(STM8000_LX_GLUE_REGS_BASE + 0x00)
#define STM8000_LX_GLUE_VCR_VERSION SH4_DWORD_REG(STM8000_LX_GLUE_REGS_BASE + 0x08)
#define STM8000_LX_GLUE_CONTROL_REQ SH4_DWORD_REG(STM8000_LX_GLUE_REGS_BASE + 0x10)
#define STM8000_LX_GLUE_CONTROL_ACK SH4_DWORD_REG(STM8000_LX_GLUE_REGS_BASE + 0x18)
#define STM8000_LX_GLUE_IRQ_STATUS SH4_DWORD_REG(STM8000_LX_GLUE_REGS_BASE + 0x20)
#define STM8000_LX_GLUE_IRQ_CLEAR SH4_DWORD_REG(STM8000_LX_GLUE_REGS_BASE + 0x28)
#define STM8000_LX_GLUE_CH1_THRESHOLD SH4_DWORD_REG(STM8000_LX_GLUE_REGS_BASE + 0x30)
#define STM8000_LX_GLUE_CH2_THRESHOLD SH4_DWORD_REG(STM8000_LX_GLUE_REGS_BASE + 0x38)
#define STM8000_LX_GLUE_CH3_THRESHOLD SH4_DWORD_REG(STM8000_LX_GLUE_REGS_BASE + 0x40)

/* SHE control registers (STm8000 variant) */
#define STM8000_SHE_VCR_STATUS SH4_DWORD_REG(STM8000_SHE_REGS_BASE + 0x0000)
#define STM8000_SHE_VCR_VERSION SH4_DWORD_REG(STM8000_SHE_REGS_BASE + 0x0008)
#define STM8000_SHE_RESET_PD_REQ SH4_DWORD_REG(STM8000_SHE_REGS_BASE + 0x0010)
#define STM8000_SHE_RESET_PD_ACK SH4_DWORD_REG(STM8000_SHE_REGS_BASE + 0x0018)
#define STM8000_SHE_HOR_SIZE SH4_DWORD_REG(STM8000_SHE_REGS_BASE + 0x0100)
#define STM8000_SHE_ADDR_PREFIX SH4_DWORD_REG(STM8000_SHE_REGS_BASE + 0x0108)
#define STM8000_SHE_CURR_CHROMA SH4_DWORD_REG(STM8000_SHE_REGS_BASE + 0x0110)
#define STM8000_SHE_FWD_CHROMA SH4_DWORD_REG(STM8000_SHE_REGS_BASE + 0x0118)
#define STM8000_SHE_BKW_CHROMA SH4_DWORD_REG(STM8000_SHE_REGS_BASE + 0x0120)
#define STM8000_SHE_MAE_OFFS_FIELD SH4_DWORD_REG(STM8000_SHE_REGS_BASE + 0x0128)
#define STM8000_SHE_MAE_OFFS_INTERP SH4_DWORD_REG(STM8000_SHE_REGS_BASE + 0x0130)
#define STM8000_SHE_MAD_THRES_INTRA SH4_DWORD_REG(STM8000_SHE_REGS_BASE + 0x0138)
#define STM8000_SHE_MAD_OFFS_INTRA SH4_DWORD_REG(STM8000_SHE_REGS_BASE + 0x0140)
#define STM8000_SHE_MAD_THRES_MC SH4_DWORD_REG(STM8000_SHE_REGS_BASE + 0x0148)
#define STM8000_SHE_MAD_OFFS_MC SH4_DWORD_REG(STM8000_SHE_REGS_BASE + 0x0150)
#define STM8000_SHE_CURR_COARSE SH4_DWORD_REG(STM8000_SHE_REGS_BASE + 0x0180)
#define STM8000_SHE_FWD_COARSE SH4_DWORD_REG(STM8000_SHE_REGS_BASE + 0x0188)
#define STM8000_SHE_CURR_FINE SH4_DWORD_REG(STM8000_SHE_REGS_BASE + 0x0190)
#define STM8000_SHE_FWD_FINE SH4_DWORD_REG(STM8000_SHE_REGS_BASE + 0x0198)
#define STM8000_SHE_BKW_FINE SH4_DWORD_REG(STM8000_SHE_REGS_BASE + 0x01a0)
#define STM8000_SHE_CAP_COARSE SH4_DWORD_REG(STM8000_SHE_REGS_BASE + 0x01a8)
#define STM8000_SHE_CAP_FINE SH4_DWORD_REG(STM8000_SHE_REGS_BASE + 0x01b0)
#define STM8000_SHE_T1_DMA_CONTROL SH4_DWORD_REG(STM8000_SHE_REGS_BASE + 0x01e8)
#define STM8000_SHE_T1_DMA_BASE SH4_DWORD_REG(STM8000_SHE_REGS_BASE + 0x01f0)
#define STM8000_SHE_T1_DMA_CURRENT SH4_DWORD_REG(STM8000_SHE_REGS_BASE + 0x01f8)

#endif /* __STM8000REG_H */
