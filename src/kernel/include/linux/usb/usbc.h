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

#ifndef __USBC_H__
#define __USBC_H__

/*
 * init		: callback used to setup the usb cable gadget driver
 * remove	: callback used to remove the usb cable gadget driver
 * get_state	: callback used to get the usb carrier state
 *			0: unplugged
 *			1: plugged
 */
struct usbc_data {
	int (*init)(struct platform_device *);
	void (*remove)(struct platform_device *);
	int (*get_state)(void);
};

#endif /* __USBC_H__ */
