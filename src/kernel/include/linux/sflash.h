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

#ifndef _LINUX_SFLASH_H_
#define _LINUX_SFLASH_H_

struct sflash_info {
	int (*set_vpp)(int);
};

#endif /* _LINUX_SFLASH_H_ */
