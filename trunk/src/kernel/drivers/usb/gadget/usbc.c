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
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/usb/usbc.h>

dev_t usbc_dev_number;		/* Allocated device number */
struct class *usbc_class;	/* Tie with the device model */
struct class_device *usbc_class_dev;
struct usbc_data *usbcd;

static ssize_t show_carrier(struct class_device *cdev, char *buf)
{
	char *pos = buf;

	if(usbcd == NULL)
		goto out;

	pos += sprintf(pos, "%d\n",usbcd->get_state());
out:
	return pos - buf;
}
static CLASS_DEVICE_ATTR(carrier, S_IRUGO, show_carrier, NULL);

static int usbc_probe(struct platform_device *dev)
{
	int ret;
	struct device *me = &dev->dev;

	if (usbcd != NULL) {
		ret = -EBUSY;
		goto out;
	}

	usbcd = me->platform_data;

	/* Initalize the board specific part */
	ret = (usbcd->init)(dev);

out:
	return ret;
}

static int usbc_remove(struct platform_device *dev)
{
	if(usbcd == NULL)
		goto out;

	/* Initalize the board specific part */
	(usbcd->remove)(dev);
	usbcd = NULL;
out:
	return 0;
}

static struct platform_driver usbc_device_driver = {
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "usbcable",
	},
	.probe		= usbc_probe,
	.remove		= usbc_remove,
};

static int __init usbc_init(void)
{
	int ret = 0;

	printk("%s\n",__FUNCTION__);

	usbcd = NULL;

	ret = platform_driver_register(&usbc_device_driver);
	if (ret < 0) {
		printk(KERN_ERR
			"%s: ERROR: registering usbcable driver on platfomr bus error: 0x%x)\n",
			__FUNCTION__, ret);
		goto out;
	}

	ret = alloc_chrdev_region (&usbc_dev_number, 0, 1, "usbc");
	if (ret < 0) {
		printk (KERN_ERR
			"%s: usbcable can't register device err: 0x%x\n",
			__FUNCTION__, ret);
		goto out;
	}

	usbc_class = class_create (THIS_MODULE, "usbc");
	usbc_class_dev = class_device_create (usbc_class, NULL, usbc_dev_number, NULL, "usbc0");
	ret = class_device_create_file(usbc_class_dev, &class_device_attr_carrier);
	if (ret) {
		printk( KERN_ERR
			"%s: ERROR: Creating usbcable carrier file: 0x%x)\n",
			__FUNCTION__, ret);
	}

out:
	return ret;
}

static void __exit usbc_exit(void)
{
	printk("%s\n",__FUNCTION__);
	class_device_remove_file(usbc_class_dev, &class_device_attr_carrier);
	class_device_destroy(usbc_class, usbc_dev_number);
	class_destroy(usbc_class);
	platform_driver_unregister(&usbc_device_driver);
}

module_init(usbc_init);
module_exit(usbc_exit);

MODULE_DESCRIPTION("usbcable");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Frederic Mazuel <fmazuel@wyplay.com>");
