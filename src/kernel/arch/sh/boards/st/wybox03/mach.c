/*
 * Machine vector for the WyBox prototype board, PCB revision WYBOXA_MBRD_V0200
 *
 * (C) Copyright 2005 STMicroelectronics Limited.
 * Stuart Menefy <stuart.menefy@st.com>
 *
 * (C) Copyright 2006-2008 WyPlay SAS.
 * Aubin Constans <aconstans@wyplay.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; version 2 of
 * the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <linux/init.h>
#include <linux/irq.h>
#include <asm/system.h>
#include <asm/io.h>
#include <asm/machvec.h>
#include <asm/irq-stb7100.h>

#include "../../../kernel/cpu/irq/st40_ilc.h"	/* for ILC_TRIGGERMODE_X */

static void __iomem* wybox03_ioport_map(unsigned long port, unsigned int size)
{
#ifdef CONFIG_BLK_DEV_ST40IDE
	/*
	 * The IDE driver appears to use memory addresses with IO port
	 * calls. This needs fixing.
	 */
	return (void __iomem *)port;
#endif

	/* However picking somewhere safe isn't as easy as you might think.
	 * I used to use external ROM, but that can cause problems if you are
	 * in the middle of updating Flash. So I'm now using the processor core
	 * version register, which is guaranted to be available, and non-writable.
	 */
	return (void __iomem *)CCN_PVR;
}

static void __init wybox03_init_irq(void)
{
	/* enable individual interrupt mode for externals */
	plat_irq_setup_pins(IRQ_MODE_IRQ);

	/* Set the ILC to route external interrupts to the INTC */
	/* Outputs 0-3 are the interrupt pins, 4-7 are routed to the INTC */

	ilc_route_external(ILC_EXT_MDINT, 4, ILC_TRIGGERMODE_LOW);

	ilc_route_external(ILC_EXT_IRQ1, 5, ILC_TRIGGERMODE_HIGH);
	ilc_route_external(ILC_EXT_IRQ2, 6, ILC_TRIGGERMODE_HIGH);
	ilc_route_external(ILC_EXT_IRQ3, 7, ILC_TRIGGERMODE_FALLING);
#if 0
	make_ipr_irq(IRL0_IRQ, IRL0_IPR_ADDR, IRL0_IPR_POS, IRL0_PRIORITY);
	make_ipr_irq(IRL1_IRQ, IRL1_IPR_ADDR, IRL1_IPR_POS, IRL1_PRIORITY);
	make_ipr_irq(IRL2_IRQ, IRL2_IPR_ADDR, IRL2_IPR_POS, IRL2_PRIORITY);
	make_ipr_irq(IRL3_IRQ, IRL3_IPR_ADDR, IRL3_IPR_POS, IRL3_PRIORITY);
#endif
}

extern void __init wybox03_setup(char **cmdline_p);

struct sh_machine_vector mv_wybox03 __initmv = {
	.mv_name	= "Wyplay WyBox03 Prototype Board",
	.mv_setup	= wybox03_setup,
	.mv_nr_irqs	= NR_IRQS,
	.mv_init_irq	= wybox03_init_irq,
	.mv_ioport_map	= wybox03_ioport_map,
};
