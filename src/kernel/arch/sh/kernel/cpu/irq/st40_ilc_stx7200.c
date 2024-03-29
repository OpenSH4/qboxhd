/*
 * linux/arch/sh/kernel/cpu/irq/st40_ilc_stx7200.c
 *
 * Copyright (C) 2007 STMicroelectronics Limited
 * Author: Stuart Menefy <stuart.menefy@st.com>
 *
 * May be copied or modified under the terms of the GNU General Public
 * License.  See linux/COPYING for more information.
 *
 * Interrupts routed through the Interrupt Level Controller (ILC3) on the STx7200
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/errno.h>
#include <linux/platform_device.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#include <asm/hw_irq.h>
#include <asm/system.h>
#include <asm/io.h>
#include <asm/irq-ilc.h>

#include "st40_ilc.h"

struct ilc_data {
#define ilc_get_priority(_ilc)		((_ilc)->priority)
#define ilc_set_priority(_ilc, _prio)	((_ilc)->priority = (_prio))
	unsigned char priority;
#define ILC_STATE_USED			0x1
#define ilc_is_used(_ilc)		(((_ilc)->state & ILC_STATE_USED) != 0)
#define ilc_set_used(_ilc)		((_ilc)->state |= ILC_STATE_USED)
#define ilc_set_unused(_ilc)		((_ilc)->state &= ~(ILC_STATE_USED))
	unsigned char state;
};

static struct ilc_data ilc_data[ILC_NR_IRQS] =
{
	[0 ... ILC_NR_IRQS-1 ] = { .priority = 7 }
};

static DEFINE_SPINLOCK(ilc_data_lock);


#define ILC_PRIORITY_MASK_SIZE		DIV_ROUND_UP(ILC_NR_IRQS, 32)

struct pr_mask {
	/* Each priority mask needs ILC_NR_IRQS bits */
       unsigned long mask[ILC_PRIORITY_MASK_SIZE];
};

static struct pr_mask priority_mask[16];

/*
 * Debug printk macro
 */

/* #define ILC_DEBUG */
/* #define ILC_DEBUG_DEMUX */

#ifdef ILC_DEBUG
#define DPRINTK(args...)   printk(KERN_DEBUG args)
#else
#define DPRINTK(args...)
#endif

/*
 * Beware this one; the ASC has ILC ints too...
 */

#ifdef ILC_DEBUG_DEMUX
#define DPRINTK2(args...)   printk(KERN_DEBUG args)
#else
#define DPRINTK2(args...)
#endif

/*
 * The interrupt demux function. Check if this was an ILC interrupt, and
 * if so which device generated the interrupt.
 */
void ilc_irq_demux(unsigned int irq, struct irq_desc *desc)
{
#if	defined(CONFIG_CPU_SUBTYPE_STX7111) || \
	defined(CONFIG_CPU_SUBTYPE_STX7141)
	unsigned int priority = 7;
#elif	defined(CONFIG_CPU_SUBTYPE_STX5197) || \
	defined(CONFIG_CPU_SUBTYPE_STX7105) || \
	defined(CONFIG_CPU_SUBTYPE_STX7200)
	unsigned int priority = 14 - irq;
#endif
	int handled = 0;
	int idx;

	DPRINTK2("%s: irq %d\n", __FUNCTION__, irq);

	for (idx = 0; idx < ILC_PRIORITY_MASK_SIZE; ++idx) {
		unsigned long status;
		unsigned int irq_offset;
		struct irq_desc *desc;

		status = readl(ilc_base + ILC_BASE_STATUS + (idx << 2)) &
			readl(ilc_base + ILC_BASE_ENABLE + (idx << 2)) &
			priority_mask[priority].mask[idx];
		if (!status)
			continue;

		irq_offset = (idx * 32) + ffs(status) - 1;
		desc = irq_desc + ILC_IRQ(irq_offset);
		desc->handle_irq(ILC_IRQ(irq_offset), desc);
		handled = 1;
		ILC_CLR_STATUS(irq_offset);
	}

	if (likely(handled))
		return;

	atomic_inc(&irq_err_count);

	printk(KERN_DEBUG "ILC: spurious interrupt demux %d\n", irq);

	printk(KERN_DEBUG "ILC:  inputs   status  enabled    used\n");

	for (idx = 0; idx < ILC_PRIORITY_MASK_SIZE; ++idx) {
		unsigned long status, enabled, used;

		status = readl(ilc_base + ILC_BASE_STATUS + (idx << 2));
		enabled = readl(ilc_base + ILC_BASE_ENABLE + (idx << 2));
		used = 0;
		for (priority = 0; priority < 16; ++priority)
			used |= priority_mask[priority].mask[idx];

		printk(KERN_DEBUG "ILC: %3d-%3d: %08lx %08lx %08lx"
				"\n", idx * 32, (idx * 32) + 31,
				status, enabled, used);
	}
}

static unsigned int startup_ilc_irq(unsigned int irq)
{
	struct ilc_data *this;
	unsigned int priority;
	int irq_offset = irq - ILC_FIRST_IRQ;
	unsigned long flags;

	DPRINTK("%s: irq %d\n", __FUNCTION__, irq);

	WARN_ON(ilc_is_used(&ilc_data[irq_offset]));

	if ((irq_offset < 0) || (irq_offset >= ILC_NR_IRQS))
		return -ENODEV;

	this = &ilc_data[irq_offset];
	priority = this->priority;

	spin_lock_irqsave(&ilc_data_lock, flags);
	ilc_set_used(this);
	priority_mask[priority].mask[_BANK(irq_offset)] |= _BIT(irq_offset);
	spin_unlock_irqrestore(&ilc_data_lock, flags);

#if	defined(CONFIG_CPU_SUBTYPE_STX7111)
	/* ILC_EXT_OUT[4] -> IRL[0] (default priority 13 = irq  2) */
	/* ILC_EXT_OUT[5] -> IRL[1] (default priority 10 = irq  5) */
	/* ILC_EXT_OUT[6] -> IRL[2] (default priority  7 = irq  8) */
	/* ILC_EXT_OUT[7] -> IRL[3] (default priority  4 = irq 11) */
	ILC_SET_PRI(irq_offset, 0x8007);
#elif	defined(CONFIG_CPU_SUBTYPE_STX5197) || \
	defined(CONFIG_CPU_SUBTYPE_STX7105) || \
	defined(CONFIG_CPU_SUBTYPE_STX7200)
	ILC_SET_PRI(irq_offset, priority);
#elif	defined(CONFIG_CPU_SUBTYPE_STX7141)
	ILC_SET_PRI(irq_offset, 0x0);
#endif

	ILC_SET_ENABLE(irq_offset);

	return 0;
}

static void shutdown_ilc_irq(unsigned int irq)
{
	struct ilc_data *this;
	unsigned int priority;
	int irq_offset = irq - ILC_FIRST_IRQ;
	unsigned long flags;

	DPRINTK("%s: irq %d\n", __FUNCTION__, irq);

	WARN_ON(!ilc_is_used(&ilc_data[irq_offset]));

	if ((irq_offset < 0) || (irq_offset >= ILC_NR_IRQS))
		return;

	this = &ilc_data[irq_offset];
	priority = this->priority;

	ILC_CLR_ENABLE(irq_offset);
	ILC_SET_PRI(irq_offset, 0);

	spin_lock_irqsave(&ilc_data_lock, flags);
	ilc_set_unused(this);
	priority_mask[priority].mask[_BANK(irq_offset)] &= ~(_BIT(irq_offset));
	spin_unlock_irqrestore(&ilc_data_lock, flags);
}

static void unmask_ilc_irq(unsigned int irq)
{
	int irq_offset = irq - ILC_FIRST_IRQ;

	DPRINTK2("%s: irq %d\n", __FUNCTION__, irq);

	WARN_ON(!ilc_is_used(&ilc_data[irq_offset]));

	ILC_SET_ENABLE(irq_offset);
}

static void mask_ilc_irq(unsigned int irq)
{
	int irq_offset = irq - ILC_FIRST_IRQ;

	DPRINTK2("%s: irq %d\n", __FUNCTION__, irq);

	WARN_ON(!ilc_is_used(&ilc_data[irq_offset]));

	ILC_CLR_ENABLE(irq_offset);
}

static void mask_and_ack_ilc(unsigned int irq)
{
	int irq_offset = irq - ILC_FIRST_IRQ;

	DPRINTK2("%s: irq %d\n", __FUNCTION__, irq);

	WARN_ON(!ilc_is_used(&ilc_data[irq_offset]));

	ILC_CLR_ENABLE(irq_offset);
	(void)ILC_GET_ENABLE(irq_offset); /* Defeat write posting */
}

static int set_type_ilc_irq(unsigned int irq, unsigned int flow_type)
{
	int irq_offset = irq - ILC_FIRST_IRQ;
	int mode;

	switch (flow_type) {
	case IRQ_TYPE_EDGE_RISING:
		mode = ILC_TRIGGERMODE_RISING;
		break;
	case IRQ_TYPE_EDGE_FALLING:
		mode = ILC_TRIGGERMODE_FALLING;
		break;
	case IRQ_TYPE_EDGE_BOTH:
		mode = ILC_TRIGGERMODE_ANY;
		break;
	case IRQ_TYPE_LEVEL_HIGH:
		mode = ILC_TRIGGERMODE_HIGH;
		break;
	case IRQ_TYPE_LEVEL_LOW:
		mode = ILC_TRIGGERMODE_LOW;
		break;
	default:
		return -EINVAL;
	}

	ILC_SET_TRIGMODE(irq_offset, mode);

	return 0;
}

static struct irq_chip ilc_chip = {
	.name		= "ILC3",
	.startup	= startup_ilc_irq,
	.shutdown	= shutdown_ilc_irq,
	.mask		= mask_ilc_irq,
	.mask_ack	= mask_and_ack_ilc,
	.unmask		= unmask_ilc_irq,
	.set_type	= set_type_ilc_irq,
};

void __init ilc_demux_init(void)
{
	int irq;
	int irq_offset;

	/* Default all interrupts to active high. */
	for (irq_offset = 0; irq_offset < ILC_NR_IRQS; irq_offset++)
		ILC_SET_TRIGMODE(irq_offset, ILC_TRIGGERMODE_HIGH);

	for (irq = ILC_FIRST_IRQ; irq < (ILC_FIRST_IRQ+ILC_NR_IRQS); irq++)
		/* SIM: Should we do the masking etc in ilc_irq_demux and
		 * then change this to handle_simple_irq? */
		set_irq_chip_and_handler_name(irq, &ilc_chip, handle_level_irq,
					      "ILC");
}

#if defined(CONFIG_PROC_FS)

static void *ilc_seq_start(struct seq_file *s, loff_t *pos)
{
	seq_printf(s, "input irq status enabled used priority mode\n");

	if (*pos >= ILC_NR_IRQS)
		return NULL;

	return pos;
}

static void ilc_seq_stop(struct seq_file *s, void *v)
{
}

static void *ilc_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
	if (++(*pos) >= ILC_NR_IRQS)
		return NULL;

	return pos;
}

static int ilc_seq_show(struct seq_file *s, void *v)
{
	int input = *((loff_t *)v);
	int status = (ILC_GET_STATUS(input) != 0);
	int enabled = (ILC_GET_ENABLE(input) != 0);
	int used = ilc_is_used(&ilc_data[input]);

	seq_printf(s, "%3d %3d %d %d %d %d %d", input, input + ILC_FIRST_IRQ,
			status, enabled, used, readl(ILC_PRIORITY_REG(input)),
			readl(ILC_TRIGMODE_REG(input)));

	if (enabled && !used)
		seq_printf(s, " !!!");

	seq_printf(s, "\n");

	return 0;
}

static struct seq_operations ilc_seq_ops = {
	.start = ilc_seq_start,
	.next = ilc_seq_next,
	.stop = ilc_seq_stop,
	.show = ilc_seq_show,
};

static int ilc_proc_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &ilc_seq_ops);
}

static struct file_operations ilc_proc_ops = {
	.owner = THIS_MODULE,
	.open = ilc_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

/* Called from late in the kernel initialisation sequence, once the
 * normal memory allocator is available. */
static int __init ilc_proc_init(void)
{
	struct proc_dir_entry *entry = create_proc_entry("ilc", S_IRUGO, NULL);

	if (entry)
		entry->proc_fops = &ilc_proc_ops;

	return 0;
}
__initcall(ilc_proc_init);

#endif /* CONFIG_PROC_FS */
