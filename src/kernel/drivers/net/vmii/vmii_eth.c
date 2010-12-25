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
#include <linux/device.h>
#include <linux/etherdevice.h>
#include <linux/vmii.h>
#include <linux/debugfs.h>

#include "vmii_eth_dev.h"

#define miiwb_name __FILE__
#define LOG_ERROR(string, args...) \
	printk("%s: " string, miiwb_name, ##args)

#ifdef CONFIG_DEBUG
#define LOG_DEBUG(string, args...) printk("%s: " string, miiwb_name, ##args)
#else
#define LOG_DEBUG(string, args...)
#endif

int vmii_eth_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd);

static int inline set_phy_status(struct vmii_dev *dev, int status)
{
	if(dev->set_phy_status)
		return dev->set_phy_status(status);
	return -EINVAL;
}

static int init_debugfs(struct vmii_eth_dev *dev)
{
	int err;

	dev->debug_dir = debugfs_create_dir("vmmii_eth", NULL);
	if (IS_ERR(dev->debug_dir)) {
		printk("%s:%d: failed", __FUNCTION__, __LINE__);
		return PTR_ERR(dev->debug_dir);
	}
	dev->debug_child = debugfs_create_u8("phy_port_status", S_IRUSR | S_IWUSR,
					dev->debug_dir, (u8 *)&(dev->phy_port_status));
	if (IS_ERR(dev->debug_child)) {
		printk("%s:%d: failed", __FUNCTION__, __LINE__);
		err = PTR_ERR(dev->debug_child);
		goto err_out;
	}
	return 0;
err_out:
	debugfs_remove(dev->debug_dir);
	return err;
}

static void exit_debugfs(struct vmii_eth_dev *dev)
{
	if(dev->debug_child && !IS_ERR(dev->debug_child))
		debugfs_remove(dev->debug_child);

	if(dev->debug_dir && !IS_ERR(dev->debug_dir))
		debugfs_remove(dev->debug_dir);
}

struct sk_buff* vmii_eth_receive(struct vmii_dev *vdev, struct sk_buff *skb)
{
	struct net_device_stats *stats;
	struct net_device *dev = vmii_get_netdev(vdev);

	rcu_read_lock();

	stats = vmii_eth_dev_get_stats(dev);

	stats->rx_packets++;
	stats->rx_bytes += skb->len;
	LOG_DEBUG("rc_packets = %d, rx_bytes = %d", stats->rx_packets, stats->rx_bytes);
	switch (skb->pkt_type) {
	case PACKET_BROADCAST: /* Yeah, stats collect these together.. */
		/* stats->broadcast ++; // no such counter :-( */
		LOG_DEBUG(" type : broadcast");
		break;

	case PACKET_MULTICAST:
		LOG_DEBUG(" type : multicast");
		stats->multicast++;
		break;

	case PACKET_OTHERHOST:
		LOG_DEBUG(" type : otherhost");
		/* Our lower layer thinks this is not local, let's make sure.
		 * This allows the ethx to have a different MAC than the
		 * underlying device, and still route correctly.
		 */
		if (!compare_ether_addr(eth_hdr(skb)->h_dest,
						skb->dev->dev_addr))
			skb->pkt_type = PACKET_HOST;
		break;
	default:
		LOG_DEBUG(" type : default");
		skb->pkt_type = PACKET_HOST;
		break;
	}
	netif_receive_skb(skb);
	LOG_DEBUG(" receive_skb\n");
	rcu_read_unlock();
	return NULL;
}

#ifdef CONFIG_VMII_MDIO
/**
 * vmii_adjust_link
 * @dev: net device structure
 * Description: it adjusts the link parameters.
 */
static void vmii_adjust_link(struct net_device *dev)
{
}

/**
 * vmii_init_phy - PHY initialization
 * @dev: net device structure
 * Description: it initializes driver's PHY state, and attaches to the PHY.
 *  Return value:
 *  0 on success
 */
static int vmii_init_phy(struct net_device *dev)
{
	struct phy_device *phydev;
	char phy_id[BUS_ID_SIZE];
	struct vmii_eth_dev *lp = netdev_priv(dev);

	/* The phy id is build from virtual device information:
	 * bus id and phy addr */
	snprintf(phy_id, BUS_ID_SIZE, PHY_ID_FMT,
		 lp->dev->bus_id, lp->dev->phy_addr);

	phydev = phy_connect(dev, phy_id, &vmii_adjust_link, 0,
			     PHY_INTERFACE_MODE_MII);

	/* The phy structure is saved to be able to detach the phy
	 * when we will close the net devive */
	lp->attached_phy = phydev;

	return 0;
}
#endif	/* CONFIG_VMII_MDIO */

static int vmii_eth_open(struct net_device *dev)
{
	struct vmii_eth_dev *lp = netdev_priv(dev);
	struct vmii_dev *vdev = lp->dev;
#ifdef CONFIG_VMII_MDIO
	int ret = 0;
#endif

	vmii_device_set_active(vdev, 1);

#ifdef CONFIG_VMII_MDIO
	ret = vmii_init_phy(dev);
	if (ret) {
		printk(KERN_ERR "%s: Cannot attach to PHY (error: %d)\n",
		       __FUNCTION__, ret);
		return -ENODEV;
	}
#endif
	if (netif_msg_ifup(lp))
		LOG_DEBUG("%s: enabling interface\n", dev->name);

	if(lp->phy_port_status)
		set_phy_status(vdev, 1);
	else
		set_phy_status(vdev, 3);

	if (dev->flags & IFF_ALLMULTI)
		dev_set_allmulti(vdev->master_device, 1);

#ifndef CONFIG_VMII_MDIO
	netif_carrier_on(dev);
#endif
	netif_start_queue(dev);

	return 0;
}

#ifdef CONFIG_VMII_MDIO
/**
 * vmii_close_phy - Close the Etherne PHY.
 * @dev: net device structure
 * Description: it stop the driver's PHY, and detach the net device to the PHY.
 * Return value:
 * None
 */
static void vmii_close_phy(struct net_device *dev)
{
	struct vmii_eth_dev *lp = netdev_priv(dev);
	struct phy_device *phydev = lp->attached_phy;

	if(phydev != NULL) {
		phy_stop(phydev);
		phy_disconnect(phydev);
	}

	lp->attached_phy = NULL;
}
#endif	/* CONFIG_VMII_MDIO */

static int vmii_eth_close (struct net_device *dev)
{
	struct vmii_eth_dev *lp = netdev_priv(dev);
	struct vmii_dev *vdev = lp->dev;

	vmii_device_set_active(vdev, 0);

	if (netif_msg_ifdown(lp))
		LOG_DEBUG("%s: disabling interface\n", dev->name);

	if(lp->phy_port_status)
		set_phy_status(vdev, 0);
	else
		set_phy_status(vdev, 3);

	dev_mc_unsync(vdev->master_device, dev);
	if (dev->flags & IFF_ALLMULTI)
		dev_set_allmulti(vdev->master_device, -1);

#ifdef CONFIG_VMII_MDIO
	/* Close the attached phy device */
	vmii_close_phy(dev);
#endif

	netif_stop_queue(dev);
	netif_carrier_off(dev);

	return 0;
}

static int vmii_eth_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct net_device_stats *stats;
	struct vmii_eth_dev *lp = netdev_priv(dev);
	struct vmii_dev *vdev = lp->dev;

	stats = vmii_eth_dev_get_stats(skb->dev);

	return vmii_device_xmit(vdev, skb, stats);
}

static inline void print_mac_addr(u8 addr[6])
{
	int i;
	for (i = 0; i < 5; i++)
		printk("%2.2x:", addr[i]);
	printk("%2.2x\n", addr[5]);
	return;
}

static void vmii_eth_set_multicast_list(struct net_device *dev)
{
	struct vmii_eth_dev *lp = netdev_priv(dev);

	vmii_dev_set_multicast_list(lp->dev);
}

static int vmii_eth_change_mtu(struct net_device *dev, int new_mtu)
{
	struct vmii_eth_dev *lp = netdev_priv(dev);

	return vmii_dev_change_mtu(lp->dev, new_mtu);
}

static int init_data(struct vmii_dev *vdev)
{
	int i, rc;
#ifdef CONFIG_VMII_MDIO
	int rcp = 0;
#endif
	char name[8];
	struct net_device *dev, *device;
	struct vmii_eth_dev *adapt = NULL;

	dev = alloc_etherdev(sizeof(struct vmii_eth_dev));
	if (!dev)
	{
		printk("Can't alloc net device!\n");
		return -ENOMEM;
	}

	SET_MODULE_OWNER(dev);

	SET_NETDEV_DEV(dev, &(vdev->dev));

	adapt = netdev_priv(dev);

	vmii_set_drvdata(vdev, adapt);

	vmii_set_netdev(vdev, dev);

	if(vmii_dev_init_master(vdev) != 0) {
		rc = -EIO;
		goto err_out_free;
	}

	if(vmii_dev_init_mac_address(vdev) != 0) {
		rc = -EIO;
		goto err_out_free;
	}

	adapt->dev = vdev;
	adapt->phy_port_status = 1;

	spin_lock_init (&adapt->lock);

	dev->open = vmii_eth_open;
	dev->do_ioctl = vmii_eth_ioctl;
	dev->stop = vmii_eth_close;
	dev->hard_start_xmit = vmii_eth_xmit;
	dev->get_stats = vmii_eth_dev_get_stats;
	dev->set_multicast_list = vmii_eth_set_multicast_list;
	dev->change_mtu = vmii_eth_change_mtu;
	dev->weight = 64;	/* arbitrary? from NAPI_HOWTO.txt. */

	for (i = 0; i <32; i++)
	{
		sprintf(name, "%s%d", vmii_get_prefix(vdev, "eth"), i);

		device = dev_get_by_name(name);
		if (device == NULL)
			break;
		dev_put(device);
	}
	if (i == 32)
	{
		printk(KERN_ERR "No available dev name\n");
		goto err_out_free;
	}

	sprintf(dev->name, "%s%d", vmii_get_prefix(vdev, "eth"), i);

	print_mac_addr(dev->dev_addr);

	if (!is_valid_ether_addr(dev->dev_addr)) {
		printk(KERN_WARNING "\tno valid MAC address; "
			"please, set using ifconfig or nwhwconfig!\n");
	}

	if ((rc = register_netdev(dev))) {
		printk(KERN_ERR "%s: ERROR %i registering the device\n",
			__FUNCTION__, rc);
		return (-ENODEV);
	}
	set_phy_status(vdev, 0);
	init_debugfs(adapt);

#ifdef CONFIG_VMII_MDIO
	if ((rcp = vmii_mdio_register(vdev)) != 0) {
		printk(KERN_ERR "%s: ERROR %i creating a vmdio instance\n",
		       __FUNCTION__, rcp);
	}
#endif
	return rc;

err_out_free:
	free_netdev(dev);
	return rc;
}

static int vmii_eth_probe(struct vmii_dev *dev)
{
	return init_data(dev);
}

int vmii_eth_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	unsigned char status = 0;
	struct mii_ioctl_data *miidata = if_mii(ifr);

	switch (cmd) {
	case SIOCGMIIPHY:
		miidata->phy_id = 0;
	break;
	case SIOCGMIIREG:
		if(miidata->reg_num == MII_BMSR)
			miidata->val_out = 0;
		else
			status = -EOPNOTSUPP;
	break;
	default:
		printk(KERN_WARNING "%s(%d) DEFAULT : %d\n",__FUNCTION__,__LINE__,cmd);
		status = -EOPNOTSUPP;
	}
	return status;
}

static void exit_data (struct vmii_dev *vdev)
{
	struct vmii_eth_dev *lp = vmii_get_drvdata(vdev);
	struct net_device *dev = vmii_get_netdev(vdev);

	if (!dev)
		BUG();

	printk("unregister netdev %s\n", dev->name);

	exit_debugfs(lp);
#ifdef CONFIG_VMII_MDIO
	vmii_mdio_unregister(vdev);
#endif
	unregister_netdev(dev);
	free_netdev(dev);
}

static int vmii_eth_remove(struct vmii_dev *dev)
{
	exit_data(dev);
	return 0;
}

static struct vmii_driver vmii_eth_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name = "generic",
	},
	.devid = VMII_GENERIC,
	.probe = vmii_eth_probe,
	.remove = vmii_eth_remove,
	.receive = vmii_eth_receive,
};

static int __init vmii_eth_init(void)
{
	printk("vmii_generic_driver is coming\n");

	return vmii_driver_register(&vmii_eth_driver);
}

static void __exit vmii_eth_exit(void)
{
	printk("vmii_generic_driver is leaving\n");

	return vmii_driver_unregister(&vmii_eth_driver);
}

#ifdef CONFIFG_VMII_ETH_MODULE
module_init(vmii_eth_init);
#else
late_initcall(vmii_eth_init);
#endif
module_exit(vmii_eth_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jean-Christophe PLAGNIOL-VILLARD <jcplagniol@wyplay.com>");
MODULE_DESCRIPTION("Virtual MII Device Generic");
