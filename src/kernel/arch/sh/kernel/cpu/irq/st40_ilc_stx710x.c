/*
 * (C) Copyright 2004 STMicroelectronics Limited.
 * Henry Bell <henry.bell@st.com>
 *
 * (C) Copyright 2006-2008 WyPlay SAS <linux-kernel@wyplay.com>.
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
#include <linux/module.h>  /* for EXPORT_SYMBOL */

#include <asm/io.h>

#include "st40_ilc.h"


/* ripped from st40_ilc_sti5528.c */
void __init ilc_route_external(int ilc_irq, int ext_out, int mode)
{
	ILC_SET_PRI(ilc_irq, 0x8000 | ext_out);
	ILC_SET_TRIGMODE(ilc_irq, mode);
	ILC_SET_ENABLE(ilc_irq);
}

void ilc_clear_status(unsigned int ilc_irq)
{
	/* clear EXTx interrupt status in ILC_STA2
	 * using register ILC_CLR_STA2 (0xB8000288), case of ILC_EXT_IRQ0 */
	ILC_CLR_STATUS(ilc_irq);
}
EXPORT_SYMBOL(ilc_clear_status);

unsigned int ilc_get_input(unsigned int ilc_irq)
{
	return ctrl_inl(ILC_INTERRUPT_REG(ilc_irq)) & _BIT(ilc_irq);
}
EXPORT_SYMBOL(ilc_get_input);


/* XXX: FIXME */
#if 0

#define ILC_DEBUG

#ifdef ILC_DEBUG
#define DPRINTK(args...)   printk(args)
#else
#define DPRINTK(args...)
#endif

#ifdef ILC_DEBUG_DEMUX
#define DPRINTK2(args...)   printk(args)
#else
#define DPRINTK2(args...)
#endif


static hw_irq_controller *ipr_handler = NULL;

static void enable_ilc_irq(unsigned int irq);
static void disable_ilc_irq(unsigned int irq);
static unsigned int startup_ilc_irq(unsigned int irq);
static void shutdown_ilc_irq(unsigned int irq);
static void mask_and_ack_ilc(unsigned int irq);
static void end_ilc_irq(unsigned int irq);



/* Interrupt controller descriptor */
static hw_irq_controller ilc_irq_type = {
   .typename   = "ILC-IRQ",
   .startup    = startup_ilc_irq,
   .shutdown   = shutdown_ilc_irq,
   .enable     = enable_ilc_irq,
   .disable    = disable_ilc_irq,
   .ack        = mask_and_ack_ilc,
   .end        = end_ilc_irq,
   /* .set_affinity */
};



int request_ilc_irq(unsigned int irq, irqreturn_t (*handler)(int, void *, struct pt_regs *), unsigned long irqflags, const char * devname, void *dev_id)
{
	int ret;

	DPRINTK("%s: %d\n", __FUNCTION__, irq);

	ret = request_irq(irq, handler, irqflags, devname, dev_id);

	if (ret) {
		DPRINTK("%s: request_irq error %d\n", __FUNCTION__, ret);
		return ret;
	}

	switch(irq) {
		case IRL0_IRQ:
		case IRL1_IRQ:
		case IRL2_IRQ:
		case IRL3_IRQ:
			DPRINTK("%s: patching irq %d\n", __FUNCTION__, irq);
			ipr_handler = irq_desc[irq].handler;
			irq_desc[irq].handler = &ilc_irq_type;
			irq_desc[irq].handler->startup(irq);
/* ACS: FIXME other SH boards do not use the IRQ_PER_CPU status flag
 * check if really required
 * if not, undefine ARCH_HAS_IRQ_PER_CPU in include/asm-sh/irq.h as well */
			irq_desc[irq].status |= IRQ_PER_CPU;
			break;

		default:
			DPRINTK("%s: unable to patch irq %d\n",__FUNCTION__,irq);
			return -EINVAL;
	}
	return(0);
}

void free_ilc_irq(unsigned int irq, void *dev_id)
{
   DPRINTK("%s: %d\n", __FUNCTION__, irq);

   switch(irq) {
      case IRL0_IRQ:
      case IRL1_IRQ:
      case IRL2_IRQ:
      case IRL3_IRQ:
         DPRINTK("%s: un-patching irq %d\n",__FUNCTION__,irq);
         irq_desc[irq].handler->shutdown(irq);
         irq_desc[irq].handler = ipr_handler;
         break;
      default:
         DPRINTK("%s: unable to un-patch irq %d\n",__FUNCTION__,irq);
         break;
   }

   free_irq(irq, dev_id);
}

unsigned int select_irq(unsigned int irq)
{
#ifdef CONFIG_SH_WYBOX_03
	const unsigned long soc_id = ctrl_inl(SYSCONF_DEVICEID);
#endif
	unsigned int val;

	switch (irq) {
		case IRL0_IRQ:
#ifdef CONFIG_SH_WYBOX_03
			if (STM_IS_DEVICE_STX7109(soc_id))
				val = ILC_MII_MDINT;
			else
#endif
			val = ILC_EXT_IRQ0;
			break;
		case IRL1_IRQ:
			val = ILC_EXT_IRQ1;
			break;
		case IRL2_IRQ:
			val = ILC_EXT_IRQ2;
			break;
		case IRL3_IRQ:
			val = ILC_EXT_IRQ3;
			break;
		default:
			val=-1;
			break;
	}
	return val;
}

static void enable_ilc_irq(unsigned int irq)
{
        int ilc_irq;

	DPRINTK("%s: %d\n",__FUNCTION__,irq);
 	ilc_irq = select_irq(irq);;
        if ((ilc_irq < 0) || (ilc_irq >= ILC_NR_IRQS))
                return;
        ILC_SET_ENABLE(ilc_irq);
	ipr_handler->enable(irq);
}

static void disable_ilc_irq(unsigned int irq)
{
        int ilc_irq;

	DPRINTK("%s: %d\n",__FUNCTION__,irq);
 	ilc_irq = select_irq(irq);;
        if ((ilc_irq < 0) || (ilc_irq >= ILC_NR_IRQS)) {
                return;
	}
        ILC_CLR_ENABLE(ilc_irq);
	ipr_handler->disable(irq);
}


static unsigned int startup_ilc_irq(unsigned int irq)
{
        unsigned int ilc_irq;
        unsigned int val;

	DPRINTK("%s: %d\n",__FUNCTION__,irq);
	ilc_irq = select_irq(irq);;

	if ((ilc_irq < 0))
		return -ENODEV;

#if 0
	if (ipr_handler && ipr_handler->startup)
		ipr_handler->startup(irq);
	else
		printk("%s: ipr_handler=%p, startup=%p !!!\n",
			ipr_handler, (ipr_handler) ? ipr_handler->startup : NULL);
#endif
		
	ILC_SET_ENABLE(ilc_irq);
	val = ilc_get_input(ilc_irq);
	DPRINTK("%s: STATUS irq %d = %d\n",__FUNCTION__,ilc_irq,val);
	return 0;
}

static void shutdown_ilc_irq(unsigned int irq)
{
        unsigned int ilc_irq;

	DPRINTK("%s: %d\n",__FUNCTION__,irq);
 	ilc_irq = select_irq(irq);;

        if ((ilc_irq < 0) ) return;

        ILC_CLR_ENABLE(ilc_irq);
	ipr_handler->shutdown(irq);
}

static void mask_and_ack_ilc(unsigned int irq)
{
        unsigned int ilc_irq;
	unsigned int val;

	DPRINTK("%s: %d\n",__FUNCTION__,irq);

 	ilc_irq = select_irq(irq);;
        if ((ilc_irq < 0) ) return;
	val = ilc_get_input(ilc_irq);
	DPRINTK("%s: STATUS irq %d = %d\n",__FUNCTION__,ilc_irq,val);

        ILC_CLR_ENABLE(ilc_irq);
	ILC_CLR_STATUS(ilc_irq);

	val = ilc_get_input(ilc_irq);
	DPRINTK("%s: STATUS irq %d = %d\n",__FUNCTION__,ilc_irq,val);

	val = irq_desc[irq].status;
	DPRINTK("%s: irq %d status %x \n",__FUNCTION__,irq,val);
	ipr_handler->ack(irq);
	irq_desc[irq].status |= IRQ_MASKED;
}


static void end_ilc_irq(unsigned int irq)
{
        unsigned int ilc_irq;
        unsigned int val;

	DPRINTK("%s: %d\n",__FUNCTION__,irq);
 	ilc_irq = select_irq(irq);;
	val = ilc_get_input(ilc_irq);
	DPRINTK("%s: STATUS irq %d = %d\n",__FUNCTION__,ilc_irq,val);
	if ( !val ) 
		DPRINTK("%s: line status %d\n",__FUNCTION__,val);
	val = irq_desc[irq].status;
	DPRINTK("%s: irq %d status %x \n",__FUNCTION__,irq,val);
        if (!(irq_desc[irq].status & (IRQ_DISABLED|IRQ_INPROGRESS))) {
		DPRINTK("%s: reenable irq %d\n",__FUNCTION__,irq);
		ILC_SET_ENABLE(ilc_irq);
		irq_desc[irq].status &= ~IRQ_MASKED;
	}
	ipr_handler->end(irq);
}


void make_ilc_irq(unsigned int irq, unsigned int irl,unsigned int addr, int pos, int priority)
{
	DPRINTK("%s: %d\n",__FUNCTION__,irq);
	make_ipr_irq(irl , addr, pos, priority);
	ipr_handler = irq_desc[irq + ILC_FIRST_IRQ].handler;
	irq_desc[irq + ILC_FIRST_IRQ].handler = &ilc_irq_type;
}

EXPORT_SYMBOL(request_ilc_irq);
EXPORT_SYMBOL(free_ilc_irq);

#endif
