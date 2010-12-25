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

#include <linux/device.h>
#include <linux/vmii.h>
#include <linux/etherdevice.h>

int vmii_dev_set_mac_address(struct vmii_dev *dev, u8 *addr)
{
	struct net_device *master;

	if (!dev || !addr)
		return -EINVAL;
	if(!is_valid_ether_addr(addr))
		return -EINVAL;
	if (memcpy(dev->addr, addr, ETH_ALEN) == NULL)
		return -EIO;
	master = vmii_get_master_netdev(dev);
	if(master->set_mac_address)
		return master->set_mac_address(master, addr);

	return 0;
}

void vmii_dev_set_multicast_list(struct vmii_dev *dev)
{
	struct net_device *netdev = vmii_get_netdev(dev);

	dev_mc_sync(dev->master_device, netdev);
}

int vmii_dev_change_mtu(struct vmii_dev *dev, int new_mtu)
{
	struct net_device *netdev = vmii_get_netdev(dev);
	struct net_device *master_netdev = vmii_get_master_netdev(dev);

	if (new_mtu < 68 || master_netdev->mtu < new_mtu)
		return -EINVAL;
	netdev->mtu = new_mtu;
	return 0;
}

int vmii_dev_init_master(struct vmii_dev *dev)
{
	dev->master_device = dev_get_by_name(dev->master);
	if(!dev->master_device) {
		printk(KERN_DEBUG "vmii: mii master '%s' not found", dev->master);
		return -EIO;
	}

	return 0;
}

int vmii_dev_init_mac_address(struct vmii_dev *dev)
{
	if(!dev->master_device) {
		printk(KERN_DEBUG "vmii: mii master '%s' not found", dev->master);
		return -EIO;
	}

	if (memcpy(dev->netdev->dev_addr, dev->master_device->dev_addr, ETH_ALEN) == NULL)
		return -EIO;

	return 0;
}

EXPORT_SYMBOL(vmii_dev_set_mac_address);
EXPORT_SYMBOL(vmii_dev_change_mtu);
EXPORT_SYMBOL(vmii_dev_set_multicast_list);
EXPORT_SYMBOL(vmii_dev_init_master);
EXPORT_SYMBOL(vmii_dev_init_mac_address);
