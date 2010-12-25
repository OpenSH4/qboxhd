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

#ifndef _CUSTOM_PLATFORM_H_
#define _CUSTOM_PLATFORM_H_

#include <linux/err.h>

#define CPLATFORM_RESFLAG_DEFAULT 0x0

#define CPLATFORM_DRVFLAG_DEFAULT 0x0

struct custom_platform_resource {
	const char* driver_name;
	u32 id;
	u32 flags;
	u32 data_size;
	const void* data;
};

struct custom_platform_driver {
	const char* name;
	int (*probe)(const struct custom_platform_resource* a_resource, u32 flags, void* driver_data);
	int (*remove)(const struct custom_platform_resource* a_resource, u32 flags, void* driver_data);
};

#ifdef CONFIG_CUSTOM_PLATFORM

extern int custom_platform_add_resource(const struct custom_platform_resource* resource);
extern int custom_platform_remove_resource(const struct custom_platform_resource* resource);

extern int custom_platform_register_driver(const struct custom_platform_driver* driver, u32 flags, void* driver_data);
extern int custom_platform_unregister_driver(const char* driver_name);

#else
static inline int custom_platform_add_resource(const struct custom_platform_resource*) { return -ENOTSUPP; }
static inline int custom_platform_remove_resource(const struct custom_platform_resource*) { return -ENOTSUPP; }
static inline int custom_platform_register_driver(const struct custom_platform_driver*, u32, void*) { return -ENOTSUPP; }
static inline int custom_platform_unregister_driver(const char*) { return -ENOTSUPP; }
#endif /* CONFIG_CUSTOM_PLATFORM */

#endif /* _CUSTOM_PLATFORM_H_ */
