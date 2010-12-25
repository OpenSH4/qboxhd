/*
 * Copyright (C) 2003-2004 David Brownell
 * Copyright (C) 2003 Agilent Technologies
 *
 * PLX Technology, INC. 2005-2006
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _MTP_IOCTL_H
#define _MTP_IOCTL_H

#define GADGETFS_EV_WAIT _IO('g',4)

/* Cancels any current requests in progress then flush the fifo
 * 
 */
#define	GADGETFS_CANCEL	_IO('g',5)
#define GADGETFS_SET_SINK _IO('g', 6)
#define GADGETFS_SET_SOURCE _IO('g', 7)
#define GADGETFS_WRITE_FILE _IO('g', 8)
#define GADGETFS_CLOSE_FILES _IO('g', 9)
#define GADGETFS_CANCEL_DONE _IO('g', 10)

#define GADGETFS_SET_INTERFACE 10

struct mtp_event {
    u16 type;
    u32 param1;
    u32 param2;
};

struct mtp_file_io {
	unsigned int fd;
    size_t count;
};


#endif
