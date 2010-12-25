/*
 * (C) Copyright 2008 WyPlay SAS.
 * Jean-Christophe PLAGNIOL-VILLARD <jcplagniol@wyplay.com>
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

#include <linux/autoconf.h>
#include <linux/platform_device.h>
#include <linux/netdevice.h>
#include <linux/vmii.h>
#include <linux/if_vmii.h>

static struct device *current_bus;

static int vmii_close_device(struct vmii_dev *dev);

int vmii_device_set_active(struct vmii_dev *dev, u8 state)
{
	if (vmii_open_master(dev) != 0)
		return -EIO;
	return 0;
}

struct data {
	int ret;
	struct net_device *orig_dev;
	struct sk_buff *skb;
	struct vmii_dev *dev;
	struct vmii_driver *drv;
	u8 state;
};

static int _vmii_device_set_active_child(struct device *_dev, void *_data)
{
	struct vmii_dev *dev = VMII_DEV(_dev);
	struct data * data = _data;

	if (data->dev->dev.bus_id == _dev->bus_id) {
		dev->active = data->state;
	} else if (data->state) {
		dev->active = 0;
		vmii_close_device(dev);
	}
	return 0;
}

static int _vmii_device_set_active(struct vmii_dev *dev, u8 state)
{
	struct data data;
	int err = -EINVAL;

	if(!dev)
		return err;

	if (vmii_open_master(dev) != 0)
		return -EIO;

	data.state = state;
	data.dev = dev;
	dev->active = state;

	device_for_each_child(current_bus, &data, _vmii_device_set_active_child);

	return 0;
}

static int vmii_close_device(struct vmii_dev *dev)
{
	struct net_device *netdev = vmii_get_netdev(dev);

	if(!netdev || !netdev->name)
		return -EIO;

	if ((netdev->flags & IFF_UP) != IFF_UP) {
		printk(KERN_ERR "%s: closed %s\n", "VMII" ,netdev->name);
		return 0;
	}

	if (dev_change_flags(netdev, netdev->flags & ~IFF_UP) < 0) {
		printk(KERN_ERR "%s: failed to close %s\n", "VMII" ,netdev->name);
		rtnl_unlock();
		goto release;
	}

	return 0;

release:
	return -EIO;
}

int vmii_open_master(struct vmii_dev *dev)
{
	struct net_device *netdev = vmii_get_netdev(dev);
	struct net_device *master_netdev = vmii_get_master_netdev(dev);

	if ((master_netdev->flags & IFF_UP) == IFF_UP) {
		printk(KERN_ERR "%s: opened %s\n", "VMII",
				master_netdev->name);
		return 0;
	}

	if(!is_valid_ether_addr(netdev->dev_addr))
		if (memcpy(netdev->dev_addr,
				master_netdev->dev_addr, ETH_ALEN) == NULL)
			return -ENOMEM;

	if (dev_change_flags(master_netdev,
			master_netdev->flags | IFF_UP) < 0) {
		printk(KERN_ERR "%s: failed to open %s\n", "VMII",
				master_netdev->name);
		rtnl_unlock();
		goto release;
	}

	return 0;

release:
	return -EIO;

}

static int vmii_search_netdev(struct device *_dev, void *_data)
{
	struct vmii_dev *dev = VMII_DEV(_dev);
	struct data *data = _data;

	if(data->orig_dev == dev->netdev) {
		data->dev = dev;
		return 1;
	}
	return 0;
}

struct vmii_dev* vmii_get_by_netdev(struct net_device *dev)
{
	struct data data;

	data.dev = NULL;
	data.orig_dev = dev;
	device_for_each_child(current_bus, &data, vmii_search_netdev);

	return data.dev;
}

static int vmii_device_event(struct notifier_block *unused,
				unsigned long event, void *ptr)
{
	struct net_device *dev = ptr;

	switch (event) {
	case NETDEV_UP:
		_vmii_device_set_active(vmii_get_by_netdev(dev), 1);
		return NOTIFY_DONE;
	}
	return NOTIFY_DONE;
}

static struct notifier_block vmii_notifier_block __read_mostly = {
	.notifier_call	= vmii_device_event,
};

int vmii_device_xmit(struct vmii_dev *dev, struct sk_buff *skb, struct net_device_stats *stats)
{
	int ret;

	if(!dev || !skb)
		return -EINVAL;

	rcu_read_lock();

	skb->dev = vmii_get_master_netdev(dev);

	ret = dev_queue_xmit(skb);

	if(stats) {
		if (likely(ret == NET_XMIT_SUCCESS)) {

			stats->tx_packets++; /* for statics only */
			stats->tx_bytes += skb->len;
		} else {
			stats->tx_errors++;
			stats->tx_aborted_errors++;
		}
	}
	rcu_read_unlock();

	return NETDEV_TX_OK;
}

static DEFINE_SPINLOCK(ptype_lock);
static struct list_head ptype_base[16];	/* 16 way hashed list */

void vmii_add_pack(struct packet_type *pt)
{
	int hash;

	spin_lock_bh(&ptype_lock);
	if (pt->type != htons(ETH_P_ALL)) {
		hash = ntohs(pt->type) & 15;
		list_add_rcu(&pt->list, &ptype_base[hash]);
	}
	spin_unlock_bh(&ptype_lock);
}

static void __vmii_remove_pack(struct packet_type *pt)
{
	struct list_head *head;
	struct packet_type *pt1;

	spin_lock_bh(&ptype_lock);

	head = &ptype_base[ntohs(pt->type) & 15];

	list_for_each_entry(pt1, head, list) {
		if (pt == pt1) {
			list_del_rcu(&pt->list);
			goto out;
		}
	}

	printk(KERN_WARNING "vmii_remove_pack: %p not found.\n", pt);
out:
	spin_unlock_bh(&ptype_lock);
}

void vmii_remove_pack(struct packet_type *pt)
{
	__vmii_remove_pack(pt);

}

static inline int should_deliver(const struct vmii_dev *p,
				 const struct sk_buff *skb,
				 const struct net_device *orig_dev)
{
	if(orig_dev != p->master_device)
		return 0;
	return 1;
}

static __inline__ int deliver_skb(struct sk_buff *skb,
				  struct packet_type *pt_prev,
				  struct net_device *orig_dev)
{
	skb->dev = pt_prev->dev;
	return pt_prev->func(skb, skb->dev, pt_prev, orig_dev);
}

static int vmii_check_deliver(struct device *_dev, void *_data)
{
	struct vmii_dev *dev = VMII_DEV(_dev);
	struct vmii_driver *drv = VMII_DRV(_dev->driver);
	struct data *data = _data;
	int ret = 0;

	if(should_deliver(dev, data->skb, data->orig_dev)) {
		data->ret++;
		if(dev->active) {
			data->drv = drv;
			data->dev = dev;
			ret = 1;
		}
	}
	return ret;
}

static struct sk_buff *vmii_receive_hook(struct sk_buff *skb, struct net_device *dev,
		struct packet_type *pt, struct net_device *orig_dev)
{
	struct packet_type *ptype, *pt_prev;
	unsigned short type;
	struct data data;

	if (skb->pkt_type == PACKET_LOOPBACK) {
		return skb;
	}

	data.orig_dev = orig_dev;
	data.dev = NULL;
	data.drv = NULL;
	data.skb = skb;
	data.ret = 0;

	device_for_each_child(current_bus, &data, vmii_check_deliver);

	if(!data.ret)
		return skb;

	pt_prev = NULL;

	type = skb->protocol;

	list_for_each_entry_rcu(ptype, &ptype_base[ntohs(type)&15], list) {
		if (ptype->type == type) {
			if (pt_prev) {
				if(deliver_skb(skb, pt_prev, orig_dev))
					return NULL;
			}
			pt_prev = ptype;
		}
	}
	if (pt_prev) {
		if(deliver_skb(skb, pt_prev, orig_dev))
			return NULL;
	}

	if(!data.dev || !data.dev->active)
		goto free;

	skb->dev = vmii_get_netdev(data.dev);

	return data.drv->receive(data.dev, skb);

free:
	kfree_skb(skb);
	return NULL;
}

#ifdef CONFIG_PM
static int vmii_suspend(struct platform_device *dev, pm_message_t state)
{
	return 0;
}

static int vmii_resume(struct platform_device *dev)
{
	return 0;
}
#endif

static void vmii_dev_release(struct device *_dev)
{
	struct vmii_dev *dev = VMII_DEV(_dev);

	kfree(dev);
}

static int vmii_init_one_child(struct device *me,
			       struct vmii_dev *dev,
			       int index)
{
	int ret;
	char name[20];

	if (!dev) {
		ret = -ENOMEM;
		goto out;
	}

	sprintf(name, "%s%d", dev->name, index);
	strncpy(dev->dev.bus_id,name,sizeof(dev->dev.bus_id));

	if(strcmp("generic", dev->name) == 0)
		dev->devid = VMII_GENERIC;

	dev->dev.parent = me;
	dev->dev.bus = &vmii_bus_type;
	dev->dev.release = vmii_dev_release;

	ret = device_register(&dev->dev);
out:
	return ret;

}

static int vmii_remove_child(struct device *dev, void *data)
{
	device_unregister(dev);
	return 0;
}

static int vmii_probe(struct platform_device *dev)
{
	struct device *me = &dev->dev;
	static struct vmii_devs *vmii_devices;
	int i;

	vmii_devices = (struct vmii_devs*)me->platform_data;

	current_bus = me;

	for (i = 0; i < vmii_devices->nb; i++)
		vmii_init_one_child(me, &vmii_devices->devs[i], i);

	for (i = 0; i < 16; i++)
		INIT_LIST_HEAD(&ptype_base[i]);

	register_netdevice_notifier(&vmii_notifier_block);
	vmii_handle_frame_hook = vmii_receive_hook;

	return 0;
}

static int vmii_remove(struct platform_device *dev)
{
	struct device *me = &dev->dev;

	if (me)
		device_for_each_child(me, NULL, vmii_remove_child);

	vmii_handle_frame_hook = NULL;
	unregister_netdevice_notifier(&vmii_notifier_block);

	return 0;
}

/*
 *	Not sure if this should be on the system bus or not yet.
 *	We really want some way to register a system device at
 *	the per-machine level, and then have this driver pick
 *	up the registered devices.
 */
static struct platform_driver vmii_device_driver = {
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "vmii",
	},
	.probe		= vmii_probe,
	.remove		= vmii_remove,
#ifdef CONFIG_PM
	.suspend	= vmii_suspend,
	.resume		= vmii_resume,
#endif
};

/*
 *	VMII "Register Access Bus."
 *
 *	We model this as a regular bus type, and hang devices directly
 *	off this.
 */
static int vmii_match(struct device *_dev, struct device_driver *_drv)
{
	struct vmii_dev *dev = VMII_DEV(_dev);
	struct vmii_driver *drv = VMII_DRV(_drv);

	return dev->devid == drv->devid;
}

static int vmii_bus_suspend(struct device *dev, pm_message_t state)
{
	struct vmii_dev *ldev = VMII_DEV(dev);
	struct vmii_driver *drv = VMII_DRV(dev->driver);
	int ret = 0;

	if (drv && drv->suspend)
		ret = drv->suspend(ldev, state);
	return ret;
}

static int vmii_bus_resume(struct device *dev)
{
	struct vmii_dev *ldev = VMII_DEV(dev);
	struct vmii_driver *drv = VMII_DRV(dev->driver);
	int ret = 0;

	if (drv && drv->resume)
		ret = drv->resume(ldev);
	return ret;
}

static int vmii_bus_probe(struct device *dev)
{
	struct vmii_dev *ldev = VMII_DEV(dev);
	struct vmii_driver *drv = VMII_DRV(dev->driver);
	int ret = -ENODEV;

	if (drv->probe)
		ret = drv->probe(ldev);
	return ret;
}

static int vmii_bus_remove(struct device *dev)
{
	struct vmii_dev *ldev = VMII_DEV(dev);
	struct vmii_driver *drv = VMII_DRV(dev->driver);
	int ret = 0;

	if (drv->remove)
		ret = drv->remove(ldev);
	return ret;
}

struct bus_type vmii_bus_type = {
	.name		= "vmii-bus",
	.match		= vmii_match,
	.probe		= vmii_bus_probe,
	.remove		= vmii_bus_remove,
	.suspend	= vmii_bus_suspend,
	.resume		= vmii_bus_resume,
};

int vmii_driver_register(struct vmii_driver *driver)
{
	driver->driver.bus = &vmii_bus_type;
	return driver_register(&driver->driver);
}

void vmii_driver_unregister(struct vmii_driver *driver)
{
	driver_unregister(&driver->driver);
}

static int __init vmii_init(void)
{
	int ret;
	ret = bus_register(&vmii_bus_type);

	if (ret == 0)
		platform_driver_register(&vmii_device_driver);

	return ret;
}

static void __exit vmii_exit(void)
{
	platform_driver_unregister(&vmii_device_driver);
	bus_unregister(&vmii_bus_type);
}

module_init(vmii_init);
module_exit(vmii_exit);

MODULE_DESCRIPTION("Virtual MII bus");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jean-Christophe PLAGNIOL-VILLARD <jcplagniol@wyplay.com>");

EXPORT_SYMBOL(vmii_driver_register);
EXPORT_SYMBOL(vmii_driver_unregister);
EXPORT_SYMBOL(vmii_bus_type);
EXPORT_SYMBOL(vmii_device_xmit);
EXPORT_SYMBOL(vmii_device_set_active);
EXPORT_SYMBOL(vmii_add_pack);
EXPORT_SYMBOL(vmii_remove_pack);
