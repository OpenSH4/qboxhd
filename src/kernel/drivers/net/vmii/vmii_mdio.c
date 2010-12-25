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

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/netdevice.h>
#include <linux/module.h>
#include <linux/mii.h>
#include <linux/phy.h>
#include <linux/vmii.h>
#include <linux/mii.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/uaccess.h>

static DEFINE_RWLOCK(vmii_switch_list_lock);
static LIST_HEAD(vmii_switch_list);

int vmii_mdio_read(struct mii_bus *bus, int phyaddr, int phyreg)
{
	int cr = 0xFFFF;
	struct vmii_dev *vdev = bus->priv;

	if ((vdev->bus_id == bus->id) &&( vdev->phy_addr == phyaddr)) {
		switch (phyreg) {
		case MII_PHYSID1:
			cr = 0;
			break;
		case MII_PHYSID2:
			cr = 7;
			break;
		default:
			cr = -EIO;
			break;
		}
	}

	return cr;
}

int vmii_mdio_write(struct mii_bus *bus, int phyaddr, int phyreg, u16 phydata)
{
	printk("%s\n",__FUNCTION__);
	return 0;
}

static int is_master_phydev(struct device *dev, void *data)
{
	struct phy_device *phydev = container_of(dev, struct phy_device, dev);
	struct vmii_dev* vdev = (struct vmii_dev*)data;

	return (phydev->attached_dev == vdev->master_device);
}

static struct phy_device* find_master_phy_device(struct vmii_dev* vdev)
{
	struct device *dev;
	struct phy_device *phydev;

	dev = bus_find_device(&mdio_bus_type, NULL, vdev, is_master_phydev);
	if(!dev)
		return NULL;
	phydev = container_of(dev, struct phy_device, dev);

	return phydev;
}

int vmii_mdio_register(struct vmii_dev *vdev)
{
	int err = 0;
	struct mii_bus *new_bus = kzalloc(sizeof(struct mii_bus), GFP_KERNEL);
	struct mii_bus *master_bus;
	int *irqlist;

	if (!new_bus)
		return -ENOMEM;

	irqlist = kzalloc(sizeof(int) * PHY_MAX_ADDR, GFP_KERNEL);

	if(!irqlist) {
		err = -ENOMEM;
		goto irq_alloc_fail;
	}

	rtnl_lock();
	if((err = vmii_open_master(vdev)) < 0) {
		err = -EIO;
		goto irq_alloc_fail;
	}
	rtnl_unlock();

	vdev->master_phy = find_master_phy_device(vdev);
	master_bus = vmii_get_master_mii_bus(vdev);

	if(!master_bus) {
		err = -EIO;
		printk(KERN_DEBUG "%s: Cannot find master mii_bus\n",
			vdev->name);
		goto bus_register_fail;
	}

	/* Assign IRQ to phy at address phy_addr */
	irqlist[vdev->phy_addr] = vdev->phy_irq;

	new_bus->name = "VMII MII Bus";
	new_bus->read = &vmii_mdio_read;
	new_bus->write = &vmii_mdio_write;
	new_bus->id = (int)vdev->bus_id;
	new_bus->priv = vdev;
	new_bus->irq = irqlist;
	new_bus->phy_mask = vdev->phy_mask;
	new_bus->dev = &vdev->dev;
	err = mdiobus_register(new_bus);
	if (err != 0) {
		printk(KERN_ERR "%s: Cannot register as MDIO bus\n",
		       new_bus->name);
		goto bus_register_fail;
	}

	vdev->mii = new_bus;

	return 0;

bus_register_fail:
	kfree(irqlist);
irq_alloc_fail:
	kfree(new_bus);
	return err;
}

int vmii_mdio_unregister(struct vmii_dev *vdev)
{
	mdiobus_unregister(vdev->mii);
	vdev->mii->priv = NULL;
	kfree(vdev->mii->irq);
	kfree(vdev->mii);

	return 0;
}

/*
 * VMII MDIO switch
 */
int vmii_switch_port_register (struct port_switch *port) {
	int err = 0;

	write_lock(&vmii_switch_list_lock);
	if(port->read_status) {
		list_add(&port->node, &vmii_switch_list);
	}
	write_unlock(&vmii_switch_list_lock);

	return err;
}

int vmii_switch_port_unregister (struct port_switch *port) {
	int err = 0;
	write_lock(&vmii_switch_list_lock);
	list_del(&port->node);
	write_unlock(&vmii_switch_list_lock);
	return err;
}

/*
 * VMII MDIO PHY driver
 */
struct port_switch *port_match(struct phy_device *phydev) {
	struct list_head *node;
	struct port_switch *candidate_port;
	struct mii_bus *bus;
	struct vmii_dev *vdev;

	bus = phydev->bus;
	vdev = (struct vmii_dev *)bus->priv;

	list_for_each(node, &vmii_switch_list) {
		candidate_port = list_entry(node, struct port_switch, node);
		if((candidate_port->switch_id == vdev->switch_id) &&
		   (candidate_port->port_id == vdev->phy_port)) {
			return candidate_port;
		}
	}
	return (struct port_switch *)NULL;
}

#ifdef CONFIG_VMII_PHY_INTERRUPT
static int vmii_phy_ack_interrupt(struct phy_device *phydev)
{
	struct port_switch *port;
	int status = -EIO;

	if ((port = port_match(phydev)) != NULL)
		status = port->ack_interrupt (port);
	else {
		printk("%s no matching port\n",__FUNCTION__);
	}

	return status;
}

static int vmii_phy_config_intr(struct phy_device *phydev)
{
	struct port_switch *port;
	int status = -EIO;

	if ((port = port_match(phydev)) != NULL)
		status = port->config_intr (port);
	else {
		printk("%s: no matching port\n",__FUNCTION__);
	}
	return status;
}
#endif

static int vmii_phy_config_aneg(struct phy_device *phydev)
{
	struct port_switch *port;
	int status = -EIO;

	if ((port = port_match(phydev)) != NULL)
		status = port->config_aneg (port);
	else {
		printk("%s: no matching port\n",__FUNCTION__);
	}
	return status;
}

static int vmii_phy_config_init(struct phy_device *phydev)
{
	struct port_switch *port;
	int status = -EIO;

	if ((port = port_match(phydev)) != NULL)
		status = port->config_init (port);
	else {
		printk("%s: no matching port\n",__FUNCTION__);
	}

	phydev->adjust_state = 0;
	phydev->speed = SPEED_100;
	phydev->duplex = DUPLEX_FULL;
	phydev->link = 0;
	phydev->pause = phydev->asym_pause = 0;
	phydev->state = PHY_NOLINK;
	netif_carrier_off(phydev->attached_dev);

	return status;
}

static int vmii_phy_read_status(struct phy_device *phydev)
{
	int status = -EIO;
	struct port_switch *port;

	if ((port = port_match(phydev)) != NULL)
		status = port->read_status (port);
	else {
		printk("%s: no matching port\n",__FUNCTION__);
	}

	phydev->speed = SPEED_100;
	phydev->duplex = DUPLEX_FULL;
	phydev->pause = phydev->asym_pause = 0;
	phydev->link = status;

#ifndef CONFIG_VMII_PHY_INTERRUPT
	phydev->irq = PHY_IGNORE_INTERRUPT;
#endif

	return 0;
}

static struct phy_driver vmii_mdio_phy_drivers=
{
	.phy_id = 0x007,
	.phy_id_mask = 0xfff,
	.name = "vmii phy",

	.features = PHY_BASIC_FEATURES,

	.flags = PHY_HAS_INTERRUPT,

	/* basic functions */
	.config_init = &vmii_phy_config_init,
	.config_aneg = &vmii_phy_config_aneg,
	.read_status = &vmii_phy_read_status,

	/* IRQ related */
#ifdef CONFIG_VMII_PHY_INTERRUPT
	.config_intr = &vmii_phy_config_intr,
	.ack_interrupt = &vmii_phy_ack_interrupt,
#endif

	.driver = {.owner = THIS_MODULE,},
};

static int __init vmii_mdio_phy_init(void)
{
	int ret;

	ret = phy_driver_register(&vmii_mdio_phy_drivers);
	if (ret) {
		phy_driver_unregister(&vmii_mdio_phy_drivers);
	}

	return ret;
}

static void __exit vmii_mdio_phy_exit(void)
{
	phy_driver_unregister(&vmii_mdio_phy_drivers);
}

module_init(vmii_mdio_phy_init);
module_exit(vmii_mdio_phy_exit);
