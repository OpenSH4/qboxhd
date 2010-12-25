/*
 * Generic, dynamic Board Support foundation
 * for proprietary device drivers
 *
 * (C) Copyright 2007-2008 WyPlay SAS.
 * Aubin Constans <aconstans@wyplay.com>
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

#include <linux/module.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/list.h>
#include <linux/custom_platform.h>
#include <asm/string.h>

#define CPLATFORM_SBIT_NONE     0x00
#define CPLATFORM_SBIT_RES      0x01
#define CPLATFORM_SBIT_DRV      0x02
#define CPLATFORM_SBIT_ATTACHED 0x04

struct custom_platform_node {
	struct list_head node;
	struct custom_platform_driver driver;
	void* driverdata;
	const struct custom_platform_resource* resource;
	unsigned int state;
};

struct custom_platform_base {
	struct mutex lock;
	struct list_head objectlist;
};

static const struct custom_platform_driver driver_template = {
	.name                        = NULL,
	.probe                       = NULL,
	.remove                      = NULL,
};

static struct custom_platform_base cplatform = {
	.lock                        = __MUTEX_INITIALIZER(cplatform.lock),
	.objectlist                  = LIST_HEAD_INIT(cplatform.objectlist),
};

/**
 * custom_platform_add_resource - Add one resource.
 * @resource:	resource structure.
 *
 * The resource is then accessed by the matching, registered driver, if any.
 * The resource data tree must remain in memory, at the provided address,
 * until custom_platform_remove_resource() is called.
 */

int custom_platform_add_resource(const struct custom_platform_resource* resource)
{
	struct custom_platform_node* object = NULL;
	int resultcode = 0;

	if (resource == NULL)
		return -EINVAL;
	if (resource->driver_name == NULL)
		return -EINVAL;

	if (mutex_lock_interruptible(&cplatform.lock) != 0)
		return -ERESTARTSYS;

	list_for_each_entry(object, &cplatform.objectlist, node)
	{
		if (object->state & CPLATFORM_SBIT_RES) {
			if (0 == strcmp(object->resource->driver_name, resource->driver_name)) {
				/* conflicts with resource name registered already */
				resultcode = -EADDRINUSE;
				break;
			}
		}
		else if (object->state & CPLATFORM_SBIT_DRV) {
			if (0 == strcmp(object->driver.name, resource->driver_name)) {
				/* found a matching driver, attaching to it */
				resultcode = 1;
				break;
			}
		}
	}
	if (resultcode == 0)
	{
		/* no matching driver registered yet, creating new entry */
		object = kzalloc(sizeof(*object), GFP_KERNEL);
		if (object == NULL) {
			resultcode = -ENOMEM;
			goto Exit;
		}
		INIT_LIST_HEAD(&object->node);
		object->driver = driver_template;
		object->driverdata = NULL;
		object->resource = resource;
		object->state = CPLATFORM_SBIT_RES;
		list_add(&object->node, &cplatform.objectlist);
	}
	else if (resultcode == 1)
	{
		/* attaching to existing entry with matching driver */
		object->resource = resource;
		object->state |= CPLATFORM_SBIT_RES;
		resultcode = object->driver.probe(object->resource, CPLATFORM_RESFLAG_DEFAULT, object->driverdata);
		if (resultcode == 0)
			object->state |= CPLATFORM_SBIT_ATTACHED;
		else {
			printk(KERN_ERR "custom platform \"%s\" driver failed to install, with error %d.\n", object->driver.name, resultcode);
			resultcode = 0;
		}
	}

Exit:
	mutex_unlock(&cplatform.lock);
	return resultcode;	
}

EXPORT_SYMBOL(custom_platform_add_resource);

/**
 * custom_platform_remove_resource - detach the designated resource.
 * @resource:	resource structure.
 *
 * Stop sharing the designated resource, previously registered with
 * custom_platform_add_resource().
 * May fail, with result != 0, if the attached driver fails to detach. In this
 * event the resource remains shared, and the caller should retry later.
 */

int custom_platform_remove_resource(const struct custom_platform_resource* resource)
{
	struct custom_platform_node* object = NULL;
	int resultcode = -EINVAL;

	if (resource == NULL)
		return -EINVAL;

	if (mutex_lock_interruptible(&cplatform.lock) != 0)
		return -ERESTARTSYS;

	/* search for the entry with the matching resource */
	list_for_each_entry(object, &cplatform.objectlist, node)
	{
		if (object->state & CPLATFORM_SBIT_RES) {
			if (object->resource == resource) {
				/* found the right entry */
				resultcode = 0;
				break;
			}
		}
	}
	if (resultcode != 0)
		goto Exit;  /* no matching entry */
	if (object->state & CPLATFORM_SBIT_ATTACHED) {
		/* try detaching the driver */
		resultcode = object->driver.remove(object->resource, CPLATFORM_RESFLAG_DEFAULT, object->driverdata);
		if (resultcode != 0)
			goto Exit;  /* the driver may have refused to detach right now */
		object->state &= ~CPLATFORM_SBIT_ATTACHED;
	}
	if (object->state & CPLATFORM_SBIT_DRV) {
		/* remove the resource */
		object->state &= ~CPLATFORM_SBIT_RES;
		object->resource = NULL;
	}
	else {
		/* there is no associated, registered driver, remove the entry */
		list_del(&object->node);
		kfree(object);
	}

Exit:
	mutex_unlock(&cplatform.lock);
	return resultcode;	
}

EXPORT_SYMBOL(custom_platform_remove_resource);

/**
 * custom_platform_register_driver - Add one driver.
 * @driver:	driver description
 * @flags:	registration conditions
 * @driver_data:	optional private pointer passed back to driver callbacks.
 *
 * The driver probe() callback is then called, when a matching resource is made
 * available by a 3rd-party provider. If probe() returns a non-zero result,
 * the resource is not attached, however it does not prevent the driver from
 * registering, with the present function accordingly returning 0.
 * A successfully attached resource may be later unregistered, with the driver
 * remove() callback being called then.
 * The driver description must remain valid, until
 * custom_platform_unregister_driver() is called.
 */

int custom_platform_register_driver(const struct custom_platform_driver* driver, u32 flags, void* driver_data)
{
	struct custom_platform_node* object = NULL;
	int resultcode = 0;

	if (driver == NULL)
		return -EINVAL;
	if (driver->name == NULL || driver->probe == NULL || driver->remove == NULL)
		return -EINVAL;

	if (mutex_lock_interruptible(&cplatform.lock) != 0)
		return -ERESTARTSYS;

	list_for_each_entry(object, &cplatform.objectlist, node)
	{
		if (object->state & CPLATFORM_SBIT_DRV) {
			if (0 == strcmp(object->driver.name, driver->name)) {
				/* conflicts with driver name registered already */
				resultcode = -EADDRINUSE;
				break;
			}
		}
		else if (object->state & CPLATFORM_SBIT_RES) {
			if (0 == strcmp(object->resource->driver_name, driver->name)) {
				/* found a matching resource, attaching to it */
				resultcode = 1;
				break;
			}
		}
	}
	if (resultcode == 0)
	{
		/* no matching resource registered yet, creating new entry */
		object = kzalloc(sizeof(*object), GFP_KERNEL);
		if (object == NULL) {
			resultcode = -ENOMEM;
			goto Exit;
		}
		INIT_LIST_HEAD(&object->node);
		object->driver = *driver;
		object->driverdata = driver_data;
		object->resource = NULL;
		object->state = CPLATFORM_SBIT_DRV;
		list_add(&object->node, &cplatform.objectlist);
	}
	else if (resultcode == 1)
	{
		/* attaching to existing entry with matching resource */
		object->driver = *driver;
		object->driverdata = driver_data;
		object->state |= CPLATFORM_SBIT_DRV;
		resultcode = object->driver.probe(object->resource, CPLATFORM_RESFLAG_DEFAULT, object->driverdata);
		if (resultcode == 0)
			object->state |= CPLATFORM_SBIT_ATTACHED;
		else {
			printk(KERN_ERR "custom platform \"%s\" driver failed to install, with error %d.\n", object->driver.name, resultcode);
			resultcode = 0;
		}
	}

Exit:
	mutex_unlock(&cplatform.lock);
	return resultcode;	
}

EXPORT_SYMBOL(custom_platform_register_driver);

/**
 * custom_platform_unregister_driver - detach the designated driver.
 * @driver_name:	driver name.
 *
 * Unregister the designated driver, previously registered with
 * custom_platform_register_driver(), with the same driver name.
 * If a matching resource had been successfully attached, the driver remove()
 * callback is called. May this request fail, and
 * custom_platform_unregister_driver() is aborted, forwarding the non-zero
 * result code returned by remove(). In this event, the driver remains
 * registered, and the caller should retry later.
 */

int custom_platform_unregister_driver(const char* driver_name)
{
	struct custom_platform_node* object = NULL;
	int resultcode = -EINVAL;

	if (driver_name == NULL)
		return -EINVAL;

	if (mutex_lock_interruptible(&cplatform.lock) != 0)
		return -ERESTARTSYS;

	/* search for the entry with the matching driver */
	list_for_each_entry(object, &cplatform.objectlist, node)
	{
		if (object->state & CPLATFORM_SBIT_DRV) {
			if (0 == strcmp(object->driver.name, driver_name)) {
				/* found the right entry */
				resultcode = 0;
				break;
			}
		}
	}
	if (resultcode != 0)
		goto Exit;  /* no matching entry */
	if (object->state & CPLATFORM_SBIT_ATTACHED) {
		/* try detaching the driver */
		resultcode = object->driver.remove(object->resource, CPLATFORM_RESFLAG_DEFAULT, object->driverdata);
		if (resultcode != 0)
			goto Exit;  /* the driver may have refused to detach right now */
		object->state &= ~CPLATFORM_SBIT_ATTACHED;
	}
	if (object->state & CPLATFORM_SBIT_RES) {
		/* unregister the driver */
		object->state &= ~CPLATFORM_SBIT_DRV;
		object->driver = driver_template;
		object->driverdata = NULL;
	}
	else {
		/* there is no associated, registered resource, remove the entry */
		list_del(&object->node);
		kfree(object);
	}

Exit:
	mutex_unlock(&cplatform.lock);
	return resultcode;	
}

EXPORT_SYMBOL(custom_platform_unregister_driver);
