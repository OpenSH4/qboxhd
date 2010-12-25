/*
 * linux/arch/sh/kernel/cpu/irq/st40_ilc_sti5528.c
 *
 * Copyright (C) 2004 STMicroelectronics Limited
 *
 * May be copied or modified under the terms of the GNU General Public
 * License.  See linux/COPYING for more information.
 *
 * Interrupt handling for Interrupt Level Controler (ILC) on the STi5528.
 *
 * This is simply used to route external interrupt pins to the ST40's
 * interrupt controller, optionally inverting them.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/irq.h>

#include <asm/system.h>
#include <asm/io.h>

#include "st40_ilc.h"

/*
 * STi5528 initialisation function to set up the ILC
 * The ST40 external interrupt management has a dependency on the
 * ST20 ILC interrupt controller.
 * Briefly on STi5528 interrupts:
 * - interrupts rising from on chip devices are delivered to both CPUs,
 *   ST40 via INTC2, and ST20 via ILC.  It's matter of software coherency
 *   to guarantee that a specific device is "owned" by a single CPU
 * - external interrupts (from PCI, STEMs, STi4629,...) come cross three
 *   stages before reaching the ST40 INTC (IRL pins):
 *   1) an EPLD, which provides:
 *       -  Eval board (MB376): masking/unmasking control registers (like
 *      		        other harp architectures) and multiplexes
 *      		        the independent IRQs signals into 4 encoded
 *      		        lines (input pins of STi5528).
 *       -  Espresso board    : does nothing (currently EPLD is a pass through)
 *
 *   2) the 4 external lines go first to the ILC device because their polarity
 *      could require to be inverted before reaching the ST40 INTC (it triggers
 *      the interrupts on High level).
 *      Currently this doesn't happen ... and all seems to work!! at least the
 *      Ethernet)
 *   3) finally the INTC manages the external interrupt (IRLs) as:
 *      - on Eval board (MB376): 16 different encoded level
 *      - on Espresso          : 4 independent signals
 *
 * An issue: it's likely that in the future linux will be in charge of
 *           dealing with PCI devices while the STi4629 will probably be
 *           managed by ST20 code.... concerning the interrupts this will
 *           generate conflict between differnt CPUs when accessing the
 *           ILC
 */

void __init ilc_route_external(int ilc_irq, int ext_out, int invert)
{
	ILC_SET_PRI(ilc_irq, 0x8000 | ext_out);
	ILC_SET_TRIGMODE(ilc_irq, invert ? ILC_TRIGGERMODE_LOW : ILC_TRIGGERMODE_HIGH);
	ILC_SET_ENABLE(ilc_irq);
}
