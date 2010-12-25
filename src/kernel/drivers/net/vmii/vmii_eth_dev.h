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

#ifndef _VMII_ETH_DEV_H_
#define _VMII_ETH_DEV_H_

typedef struct vmii_eth_dev {
	int bus_id;
	struct net_device_stats stats;
	u32 msg_enable;
	spinlock_t lock;

#if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)
	struct vlan_group *vlgrp;
#endif

	struct dentry *debug_dir;
	struct dentry *debug_child;
	u8 phy_port_status;
	struct vmii_dev *dev;
	struct phy_device *attached_phy;

}END_DEVICE;

#define VMII_ETH_DEV_INFO(x) ((struct vmii_eth_dev *)(x->priv))

static inline struct net_device_stats *vmii_eth_dev_get_stats(struct net_device *dev)
{
	return &(VMII_ETH_DEV_INFO(dev)->stats);
}

#endif /* _VMII_ETH_DEV_H_ */
