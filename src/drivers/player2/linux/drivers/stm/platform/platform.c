/*
 * Player2 Platform registration
 *
 * Copyright (C) 2006 STMicroelectronics Limited
 * Author: Peter Bennett <peter.bennett@st.com>
 *
 * May be copied or modified under the terms of the GNU General Public
 * License.  See linux/COPYING for more information.
 */

#include <linux/io.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/stm/slim.h>
#include <linux/autoconf.h>
#include <asm-sh/processor.h>

#if defined (CONFIG_KERNELVERSION)
#include <asm-sh/irq-ilc.h>
#include <asm-sh/irq.h>
#else /* STLinux 2.2 kernel */
#define ILC_IRQ(x) (x + MUXED_IRQ_BASE)
#endif 

static struct plat_slim tkdma_core = {
	.name        = "tkdma",
	.version     = 2,
	.imem_offset = 0x6000,
	.dmem_offset = 0x4000,
	.pers_offset = 0x5000,
	.regs_offset = 0x2000,
	.imem_size   = (1536 * 4),
	.dmem_size   = (512 * 4) 
};

#ifdef __TDT__
int tkdma = 0;
#else
int tkdma = 1;
#endif
module_param(tkdma, bool, S_IRUGO|S_IWUSR);

#if defined(CONFIG_CPU_SUBTYPE_STB7100)
#if defined(CONFIG_SH_QBOXHD_1_0) || defined(CONFIG_SH_QBOXHD_MINI_1_0)
#include "qboxhd.h"
#else
#include "mb442.h"
#include "hms1.h"
#endif
#include "platform_710x.h"
#elif defined(CONFIG_CPU_SUBTYPE_STX7200)
#include "mb520.h"
#include "platform_7200.h"
#elif defined(CONFIG_CPU_SUBTYPE_STX7141)
#include "platform_7141.h"
#elif defined(CONFIG_CPU_SUBTYPE_STX7111)
#include "platform_7111.h"
#elif defined(CONFIG_CPU_SUBTYPE_STX7105)
#include "platform_7105.h"
#endif


#ifdef CONFIG_SH_QBOXHD_1_0
#define MOD               "-HD"
#elif  CONFIG_SH_QBOXHD_MINI_1_0
#define MOD               "-Mini"
#else
#define MOD               ""
#endif

#define PLATFORM_VERSION       "0.9"MOD
MODULE_VERSION(PLATFORM_VERSION);

MODULE_DESCRIPTION("Player2 Platform Driver");
MODULE_AUTHOR("STMicroelectronics Limited");
// MODULE_VERSION("0.9");
MODULE_LICENSE("GPL");
