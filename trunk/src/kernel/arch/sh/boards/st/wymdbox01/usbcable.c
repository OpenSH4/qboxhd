/*
 * version 1.0
 *
 * (C) Copyright 2006-2009 WyPlay SAS.
 * Frederic Mazuel <fmazuel@wyplay.com>
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

#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/usb/usbc.h>
#include <asm/irl.h>
#include <asm/irq-stb7100.h>

static struct usbc_device usbc;

static unsigned int select_irq(unsigned int);

struct usbc_device {
	int irq;
};

extern void __iomem *ilc_base;
#define _BIT(_int)			(1 << (_int % 32))
#define _REG_OFF(_int)			(sizeof(int) * (_int / 32))

#define ILC_INTERRUPT_REG(_int)		((unsigned long)ilc_base + 0x080 + _REG_OFF(_int))

/*
 * Macros to get/set/clear ILC registers
 */
#define ILC_GET_INTERRUPT(_int)		(readl(ILC_INTERRUPT_REG(_int)) & _BIT(_int))

/*******************************************************************************
 * USB cable services API
 ******************************************************************************/
int usbc_setup(struct platform_device *dev)
{
	int ret = 0;
	struct resource *res;
	unsigned irq_ext;


	res = platform_get_resource(dev, IORESOURCE_IRQ, 0);
	if (!res) {
		ret = -ENODEV;
		printk(KERN_ERR
			"%s: ERROR: getting IRQ resource\n",
			__FUNCTION__);
		goto out;
	}

	usbc.irq = res->start;

	printk("usb cable irq = %d\n",usbc.irq);

	if ((irq_ext = select_irq(usbc.irq)) <0) {
		printk(KERN_ERR
			"%s: ERROR: unknown EXT IRQ %d\n",
			__FUNCTION__, usbc.irq);
		ret = -1;
		goto out;
	}

out:
	return ret;
}

void usbc_freed(struct platform_device *dev)
{
}

int usbc_get_state(void)
{
	return !(ILC_GET_INTERRUPT(select_irq(usbc.irq)));
}

/******************************************************************************
 * Utilities
 *****************************************************************************/
static unsigned int select_irq(unsigned int irq)
{
	unsigned int val;

	switch (irq) {
		case IRL0_IRQ:
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
