/*
 * inode.c -- user mode filesystem api for usb gadget controllers
 *
 * Copyright (C) 2003-2004 David Brownell
 * Copyright (C) 2003 Agilent Technologies
 *
 * PLX Technology, INC. 2005-2006
 *
 * This is a modification of the original gadgetfs driver for use by the PLX MTP example
 * this was necessary to provide support for certain features like cancellation
 * and to speed up bulk file transfers. Most of the MTP enumeration support
 * is handled in this kernel driver, not in user mode.
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


#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/pagemap.h>
#include <linux/uts.h>
#include <linux/wait.h>
#include <linux/compiler.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/splice.h>

#include <linux/device.h>
#include <linux/fcntl.h>
#include <linux/file.h>

#include <linux/usb/gadgetfs.h>
#include <linux/usb_gadget.h>
#include "mtp_ioctl.h"

#undef	VERBOSE		/* extra debug messages (success too) */


/*
 * The gadgetfs API maps each endpoint to a file descriptor so that you
 * can use standard synchronous read/write calls for I/O.  There's some
 * O_NONBLOCK and O_ASYNC/FASYNC style i/o support.  Example usermode
 * drivers show how this works in practice.  You can also use AIO to
 * eliminate I/O gaps between requests, to help when streaming data.
 *
 * Key parts that must be USB-specific are protocols defining how the
 * read/write operations relate to the hardware state machines.  There
 * are two types of files.  One type is for the device, implementing ep0.
 * The other type is for each IN or OUT endpoint.  In both cases, the
 * user mode driver must configure the hardware before using it.
 *
 * - First, dev_config() is called when /dev/gadget/$CHIP is configured
 *   (by writing configuration and device descriptors).  Afterwards it
 *   may serve as a source of device events, used to handle all control
 *   requests other than basic enumeration.
 *
 * - Then either immediately, or after a SET_CONFIGURATION control request,
 *   ep_config() is called when each /dev/gadget/ep* file is configured
 *   (by writing endpoint descriptors).  Afterwards these files are used
 *   to write() IN data or to read() OUT data.  To halt the endpoint, a
 *   "wrong direction" request is issued (like reading an IN endpoint).
 *
 * Unlike "usbfs" the only ioctl()s are for things that are rare, and maybe
 * not possible on all hardware.  For example, precise fault handling with
 * respect to data left in endpoint fifos after aborted operations; or
 * selective clearing of endpoint halts, to implement SET_INTERFACE.
 */

#define	DRIVER_DESC	"USB Gadget filesystem, MTP version"
#define	DRIVER_VERSION	"2007 July 23"

static const char driver_desc [] = DRIVER_DESC;
static const char shortname [] = "gadgetfs-mtp";

MODULE_DESCRIPTION (DRIVER_DESC);
MODULE_AUTHOR ("Seth Levy");
MODULE_LICENSE ("GPL");


/*----------------------------------------------------------------------*/

#define GADGETFS_MAGIC		0xaee71ee7
#define DMA_ADDR_INVALID	(~(dma_addr_t)0)

/* /dev/gadget/$CHIP represents ep0 and the whole device */
enum ep0_state {
	/* DISBLED is the initial state.
	 */
	STATE_DEV_DISABLED = 0,

	/* Only one open() of /dev/gadget/$CHIP; only one file tracks
	 * ep0/device i/o modes and binding to the controller.  Driver
	 * must always write descriptors to initialize the device, then
	 * the device becomes UNCONNECTED until enumeration.
	 */
	STATE_OPENED,

	/* From then on, ep0 fd is in either of two basic modes:
	 * - (UN)CONNECTED: read usb_gadgetfs_event(s) from it
	 * - SETUP: read/write will transfer control data and succeed;
	 *   or if "wrong direction", performs protocol stall
	 */
	STATE_UNCONNECTED,
	STATE_CONNECTED,
	STATE_SETUP,

	/* UNBOUND means the driver closed ep0, so the device won't be
	 * accessible again (DEV_DISABLED) until all fds are closed.
	 */
	STATE_DEV_UNBOUND,
};

///////////////////////////////////////////////////////////////////////////////
// PTP/PIMA Class Specific Request Codes and Structures

#define CANCEL_REQUEST                                  0x64
// Cancel Request Data
typedef struct _CANCEL_REQUEST_DATA // received from host
{
	u16                CancellationCode;
	u32                TransactionID;
}  __attribute__ ((packed)) CANCEL_REQUEST_DATA, *PCANCEL_REQUEST_DATA;

#define GET_EXTENDED_EVENT_DATA                         0x65
// Extended Event Data
typedef struct _EXTENDED_EVENT_DATA // send to host
{
	u16                EventCode;
	u32                TransactionID;
	u16                NumberOfParameters;
}  __attribute__ ((packed)) EXTENDED_EVENT_DATA, *PEXTENDED_EVENT_DATA;

#define DEVICE_RESET_REQUEST                            0x66

#define GET_DEVICE_STATUS                               0x67
// Device Status Data
typedef struct _DEVICE_STATUS_DATA // send to host
{
	u16            wLength;
	u16            Code;
}  __attribute__ ((packed)) DEVICE_STATUS_DATA, *PDEVICE_STATUS_DATA;

struct dev_data {
	spinlock_t			lock;
	atomic_t			count;
	enum ep0_state			state;
	struct mtp_event		user_event;
	unsigned			ev_next;
	struct fasync_struct		*fasync;
	u8				current_config;

	/* drivers reading ep0 MUST handle control requests (SETUP)
	 * reported that way; else the host will time out.
	 */
	unsigned			usermode_setup : 1,
					setup_in : 1,
					setup_can_stall : 1,
					setup_out_error : 1,
					setup_abort : 1,
					did_cancel : 1, // ptp cancel request
					did_reset : 1,  // ptp reset request
					mtp_event_ready : 1;
	
	/* the rest is basically write-once */
	struct usb_config_descriptor	*config, *hs_config;
	struct usb_device_descriptor	*dev;
	struct usb_request		*req;
	struct usb_gadget		*gadget;
	struct list_head		epfiles;
	struct usb_ep			*sink_ep;
	struct usb_ep			*source_ep;	
	void				*buf;
	wait_queue_head_t		wait;
	wait_queue_head_t		user_wait;
	struct super_block		*sb;
	struct dentry			*dentry;

	/* except this scratch i/o buffer for ep0 */
	u8				rbuf [256];
	EXTENDED_EVENT_DATA ext_data;
	DEVICE_STATUS_DATA dev_status_data;
	CANCEL_REQUEST_DATA cancel_req_data;
};

static inline void get_dev (struct dev_data *data)
{
	atomic_inc (&data->count);
}

static void put_dev (struct dev_data *data)
{
	if (likely (!atomic_dec_and_test (&data->count)))
		return;
	/* needs no more cleanup */
	BUG_ON (waitqueue_active (&data->wait));
	kfree (data);
}

static struct dev_data *dev_new (void)
{
	struct dev_data		*dev;

	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev)
		return NULL;
	dev->state = STATE_DEV_DISABLED;
	atomic_set (&dev->count, 1);
	spin_lock_init (&dev->lock);
	INIT_LIST_HEAD (&dev->epfiles);
	init_waitqueue_head (&dev->user_wait);
	init_waitqueue_head (&dev->wait);
	return dev;
}

/*----------------------------------------------------------------------*/

/* other /dev/gadget/$ENDPOINT files represent endpoints */
enum ep_state {
	STATE_EP_DISABLED = 0,
	STATE_EP_READY,
	STATE_EP_DEFER_ENABLE,
	STATE_EP_ENABLED,
	STATE_EP_UNBOUND,
};

struct ep_data {
	struct semaphore		lock;
	enum ep_state			state;
	atomic_t			count;
	struct dev_data			*dev;
	/* must hold dev->lock before accessing ep or req */
	struct usb_ep			*ep;
	struct usb_request		*req;
	ssize_t				status;
	char				name [16];
	struct usb_endpoint_descriptor	desc, hs_desc;
	struct list_head		epfiles;
	wait_queue_head_t		wait;
	struct dentry			*dentry;
	struct inode			*inode;
};

static inline void get_ep (struct ep_data *data)
{
	atomic_inc (&data->count);
}

static void put_ep (struct ep_data *data)
{
	if (likely (!atomic_dec_and_test (&data->count)))
		return;
	put_dev (data->dev);
	/* needs no more cleanup */
	BUG_ON (!list_empty (&data->epfiles));
	BUG_ON (waitqueue_active (&data->wait));
	BUG_ON (down_trylock (&data->lock) != 0);
	kfree (data);
}

/*----------------------------------------------------------------------*/

/* most "how to use the hardware" policy choices are in userspace:
 * mapping endpoint roles (which the driver needs) to the capabilities
 * which the usb controller has.  most of those capabilities are exposed
 * implicitly, starting with the driver name and then endpoint names.
 */

static const char *CHIP;

/*----------------------------------------------------------------------*/

/* NOTE:  don't use dev_printk calls before binding to the gadget
 * at the end of ep0 configuration, or after unbind.
 */

/* too wordy: dev_printk(level , &(d)->gadget->dev , fmt , ## args) */
#define xprintk(d,level,fmt,args...) \
	printk(level "%s: " fmt , shortname , ## args)

#ifdef DEBUG
#define DBG(dev,fmt,args...) \
	xprintk(dev , KERN_DEBUG , fmt , ## args)
#else
#define DBG(dev,fmt,args...) \
	do { } while (0)
#endif /* DEBUG */

#ifdef VERBOSE
#define VDEBUG	DBG
#else
#define VDEBUG(dev,fmt,args...) \
	do { } while (0)
#endif /* DEBUG */

#define ERROR(dev,fmt,args...) \
	xprintk(dev , KERN_ERR , fmt , ## args)
#define WARN(dev,fmt,args...) \
	xprintk(dev , KERN_WARNING , fmt , ## args)
#define INFO(dev,fmt,args...) \
	xprintk(dev , KERN_INFO , fmt , ## args)
	
/*----------------------------------------------------------------------*/
// MTP specific static descriptors handled in kernel mode	
	
/////////////////////////////////////////////////////////////////////
// Microsoft OS String Descriptor Extensions
/////////////////////////////////////////////////////////////////////
#define MS_EXTENDED_CONFIG_DESC 0x0004
#define MS_EXTENDED_PROP_DESC   0x0005

#define GET_MS_DESCRIPTOR   0x01
#define OS_DESC             0xEE

#define STRINGID_MFGR 1
#define STRINGID_PRODUCT 2
#define STRINGID_SERIAL 3
#define STRINGID_CONFIG 4
#define STRINGID_INTERFACE 5

struct os_extended_configuration_descriptor_header
{
	u8 dwLength0;         
	u8 dwLength1;
	u8 dwLength2;
	u8 dwLength3;
	u16 bcdVersion;       
	u16 wInd;   
	u8 bCount;           
	u8 bReserved[7];
} __attribute__ ((packed));

struct os_extended_configuration_descriptor_function
{
	u8  bFirstInterfaceNumber;
	u8  bInterfaceCount;
	u8  compatibleID[8];
	u8  subCompatibleID[8];
	u8  bReserved[6];
} __attribute__ ((packed));

struct os_extended_property_descriptor_header
{
	u8 dwLength0;         
	u8 dwLength1;
	u8 dwLength2;
	u8 dwLength3;
	u8 bcdVersion0;       
	u8 bcdVersion1;
	u8 wIndex0;   
	u8 wIndex1;
	u8 wCount0;
	u8 wCount1;
} __attribute__ ((packed));
	
///////////////////////////////////////////////////////////////////////////////
// Assign index numbers to USB strings:
//  - USB strings are retrieved by the host by referencing its index number. Index 
//    numbers appear in various USB descriptors; the host fetches a string with a
//    String Descriptor request specifying an index number (See USB 2.0: 9.4.3 )
//  - NOTE: A descriptor with a non-zero string index *must* have an associated string!
typedef enum _MTP_STRING_INDEX 
{   // 
	STRING_INDEX_LANGID = 0,        // String index zero reserved for LangID
		STRING_INDEX_MANUFACTURER,      // Manufacturer string (e.g. "PLX")
		STRING_INDEX_PRODUCT,           // Product (e.g. "PSF-3000 Printer/Scanner/Fax")
		// More indices can be applied, for example:
		STRING_INDEX_SERIAL,            // Device-unique serial number string (e.g."1234-ABCD")
		STRING_INDEX_PRINTER_INTERFACE, // Interface string (e.g. "Printer interface")
		STRING_INDEX_FAX_INTERFACE,     // Interface string (e.g. "FAX interface")
		// . . .
} MTP_STRING_INDEX;

struct os_extended_configuration
{
	struct os_extended_configuration_descriptor_header ExtendedConfigHeader;
	struct os_extended_configuration_descriptor_function ExtendedConfigFunction;
} __attribute__ ((packed));

#define EXTENDED_CONFIG_RELEASE             0x0100
#define EXTENDED_CONFIG_WINDEX              0x0004
#define NUM_FUNCTIONS                       0x01

// copy in during init
static struct os_extended_configuration msdescriptor;

static struct os_extended_configuration_descriptor_header
os_extended_config_header =
{
	.dwLength0 = sizeof(struct os_extended_configuration),
	.dwLength1 = 0,
	.dwLength2 = 0,
	.dwLength3 = 0,
	.bcdVersion = __constant_cpu_to_le16(EXTENDED_CONFIG_RELEASE),
	.wInd = __constant_cpu_to_le16(EXTENDED_CONFIG_WINDEX),
	.bCount = NUM_FUNCTIONS,
	.bReserved = {0,0,0,0,0,0,0},
};

static struct os_extended_configuration_descriptor_function
os_extended_config_function =
{
	.bFirstInterfaceNumber = 0,
	.bInterfaceCount = 1,
	.compatibleID = {"MTP"},
	.subCompatibleID = {0,0,0,0,0,0,0,0},
	.bReserved = {0,0,0,0,0,0},
};

#define MTP_OS_DESCRIPTOR_SIZE 18

// MS OS Descriptor
static char MTP_OS_DESCRIPTOR[] = 
{
	0x12,               // bLength
	0x03,               // bDescriptorType
	'M',                // 14 byte signature field, Unicode "MSFT100"
	0x00,
	'S',
	0x00,
	'F',
	0x00,
	'T',
	0x00,
	'1',
	0x00,
	'0',
	0x00,
	'0',
	0x00,
	GET_MS_DESCRIPTOR,  // bMS_VendorCode, Vendor Specific code that can be
	//  used to get the Extended Configuration Descriptor
	0x00                // bPad, must be 0
};

// Add your own device specific string information here!
static struct usb_string stringtab [] =
{
	{ STRINGID_MFGR,	NULL, },
	{ STRINGID_PRODUCT,	NULL, },
	{ STRINGID_SERIAL,	NULL, },
	{ STRINGID_CONFIG,	NULL, },
	{ STRINGID_INTERFACE,	NULL, },
};

static struct usb_gadget_strings strings =
{
	/* "en-us" */
	.language = __constant_cpu_to_le16 (0x0409),
	.strings = stringtab,
};
	
/*----------------------------------------------------------------------*/

/* SYNCHRONOUS ENDPOINT OPERATIONS (bulk/intr/iso)
 *
 * After opening, configure non-control endpoints.  Then use normal
 * stream read() and write() requests; and maybe ioctl() to get more
 * precise FIFO status when recovering from cancellation.
 */

static void epio_complete (struct usb_ep *ep, struct usb_request *req)
{
	struct ep_data	*epdata = ep->driver_data;

	if (!req->context)
		return;
	if (req->status)
		epdata->status = req->status;
	else
		epdata->status = req->actual;
	complete ((struct completion *)req->context);
}

/* tasklock endpoint, returning when it's connected.
 * still need dev->lock to use epdata->ep.
 */
static int
get_ready_ep (unsigned f_flags, struct ep_data *epdata)
{
	int	val;

	if (f_flags & O_NONBLOCK) {
		if (down_trylock (&epdata->lock) != 0)
			goto nonblock;
		if (epdata->state != STATE_EP_ENABLED) {
			up (&epdata->lock);
nonblock:
			val = -EAGAIN;
		} else
			val = 0;
		return val;
	}

	if ((val = down_interruptible (&epdata->lock)) < 0)
		return val;
newstate:
	switch (epdata->state) {
	case STATE_EP_ENABLED:
		break;
	case STATE_EP_DEFER_ENABLE:
		DBG (epdata->dev, "%s wait for host\n", epdata->name);
		if ((val = wait_event_interruptible (epdata->wait, 
				epdata->state != STATE_EP_DEFER_ENABLE
				|| epdata->dev->state == STATE_DEV_UNBOUND
				)) < 0)
			goto fail;
		goto newstate;
	// case STATE_EP_DISABLED:		/* "can't happen" */
	// case STATE_EP_READY:			/* "can't happen" */
	default:				/* error! */
		pr_debug ("%s: ep %p not available, state %d\n",
				shortname, epdata, epdata->state);
		// FALLTHROUGH
	case STATE_EP_UNBOUND:			/* clean disconnect */
		val = -ENODEV;
fail:
		up (&epdata->lock);
	}
	return val;
}

static ssize_t
ep_io (struct ep_data *epdata, void *buf, unsigned len)
{
	DECLARE_COMPLETION (done);
	int value;

	spin_lock_irq (&epdata->dev->lock);
	if (likely (epdata->ep != NULL)) {
		struct usb_request	*req = epdata->req;

		req->context = &done;
		req->complete = epio_complete;
		req->buf = buf;
		req->length = len;
		value = usb_ep_queue (epdata->ep, req, GFP_ATOMIC);
	} else if (epdata->dev->did_cancel)
		value = -ECANCELED;
	else
		value = -ENODEV;
	spin_unlock_irq (&epdata->dev->lock);

	if (likely (value == 0)) {
		value = wait_event_interruptible (done.wait, done.done);
		if (value != 0) {
			spin_lock_irq (&epdata->dev->lock);
			if (likely (epdata->ep != NULL)) {
				DBG (epdata->dev, "%s i/o interrupted\n",
						epdata->name);
				usb_ep_dequeue (epdata->ep, epdata->req);
				spin_unlock_irq (&epdata->dev->lock);

				wait_event (done.wait, done.done);
				if (epdata->status == -ECONNRESET)
					epdata->status = -EINTR;
			} else {
				spin_unlock_irq (&epdata->dev->lock);

				DBG (epdata->dev, "endpoint gone\n");
				epdata->status = -ENODEV;
			}
		}
		if (epdata->status == -ECONNRESET)
			epdata->status = -ECANCELED;
		return epdata->status;
	}
	return value;
}


/* handle a synchronous OUT bulk/intr/iso transfer */
static ssize_t
ep_read (struct file *fd, char __user *buf, size_t len, loff_t *ptr)
{
	struct ep_data		*data = fd->private_data;
	void			*kbuf;
	ssize_t			value;

	if ((value = get_ready_ep (fd->f_flags, data)) < 0)
		return value;

	/* halt any endpoint by doing a "wrong direction" i/o call */
	if (data->desc.bEndpointAddress & USB_DIR_IN) {
		if ((data->desc.bmAttributes & USB_ENDPOINT_XFERTYPE_MASK)
				== USB_ENDPOINT_XFER_ISOC)
			return -EINVAL;
		DBG (data->dev, "%s halt\n", data->name);
		spin_lock_irq (&data->dev->lock);
		if (likely (data->ep != NULL))
			usb_ep_set_halt (data->ep);
		spin_unlock_irq (&data->dev->lock);
		up (&data->lock);
		return -EBADMSG;
	}

	/* FIXME readahead for O_NONBLOCK and poll(); careful with ZLPs */

	VDEBUG (data->dev, "%s read req %d OUT\n",
		data->name, len);
	value = -ENOMEM;
	kbuf = kmalloc (len, GFP_KERNEL);
	if (unlikely (!kbuf))
		goto free1;

	value = ep_io (data, kbuf, len);
	VDEBUG (data->dev, "%s read %zu OUT, status %d\n",
		data->name, len, (int) value);
	if (value >= 0 && copy_to_user (buf, kbuf, value))
		value = -EFAULT;

free1:
	up (&data->lock);
	kfree (kbuf);
	return value;
}

/* handle a synchronous IN bulk/intr/iso transfer */
static ssize_t
ep_write (struct file *fd, const char __user *buf, size_t len, loff_t *ptr)
{
	struct ep_data		*data = fd->private_data;
	void			*kbuf;
	ssize_t			value;

	if ((value = get_ready_ep (fd->f_flags, data)) < 0)
		return value;

	/* halt any endpoint by doing a "wrong direction" i/o call */
	if (!(data->desc.bEndpointAddress & USB_DIR_IN)) {
		if ((data->desc.bmAttributes & USB_ENDPOINT_XFERTYPE_MASK)
				== USB_ENDPOINT_XFER_ISOC)
			return -EINVAL;
		DBG (data->dev, "%s halt\n", data->name);
		spin_lock_irq (&data->dev->lock);
		if (likely (data->ep != NULL))
			usb_ep_set_halt (data->ep);
		spin_unlock_irq (&data->dev->lock);
		up (&data->lock);
		return -EBADMSG;
	}

	/* FIXME writebehind for O_NONBLOCK and poll(), qlen = 1 */

	value = -ENOMEM;
	kbuf = kmalloc (len, GFP_KERNEL);
	if (!kbuf)
		goto free1;
	if (copy_from_user (kbuf, buf, len)) {
		value = -EFAULT;
		goto free1;
	}

	value = ep_io (data, kbuf, len);
	VDEBUG (data->dev, "%s write %zu IN, status %d\n",
		data->name, len, (int) value);
free1:
	up (&data->lock);
	kfree (kbuf);
	return value;
}

static int
ep_release (struct inode *inode, struct file *fd)
{
	struct ep_data		*data = fd->private_data;

	/* clean up if this can be reopened */
	if (data->state != STATE_EP_UNBOUND) {
		data->state = STATE_EP_DISABLED;
		data->desc.bDescriptorType = 0;
		data->hs_desc.bDescriptorType = 0;
		usb_ep_disable(data->ep);
	}
	put_ep (data);
	return 0;
}

static int ep_to_file(struct ep_data *data, unsigned int fd, size_t count)
{
	// loop read from endpoint then write to file
	unsigned long int total = 0;
	struct file *dstf;
	int retval,orgfsuid,orgfsgid;
	mm_segment_t orgfs;
	char *buffer;
	unsigned long page;
	int index, bufsize;

	/* Save uid and gid used for filesystem access.
	 * Set user and group to 0 (root)
	 */
	orgfsuid=current->fsuid;
	orgfsgid=current->fsgid;
	current->fsuid=current->fsgid=0;
	/* save FS register and set FS register to kernel
	 * space, needed for read and write to accept
	 * buffer in kernel space.
	 */
	orgfs=get_fs();
	set_fs(KERNEL_DS);

	/* Allocate one page for buffer */
	page = __get_free_page(GFP_KERNEL);
	if (page)
	{
		buffer=(char*)page;
		// dstf = filp_open(dst, O_WRONLY|O_TRUNC|O_CREAT , 0644);
		dstf = fget(fd);
		if (IS_ERR(dstf))
		{
			printk("Bad file structure: Error %ld\n",-PTR_ERR(dstf));
		} 
		else 
		{
			do
			{
				/* Read as much as posible into the buffer,
				 * at most one page.
				 */
				retval = ep_io (data, buffer, PAGE_SIZE);			
				if (retval < 0) 
				{
					printk("ep_to_file: usb error %d\n", retval);
					break;
				}
				else if (retval < PAGE_SIZE)
				{
					// check to see if terminated short
					if (total + retval < count) 
					{
						VDEBUG(data->dev, "file xfer canceled short\n");
						break;
					}					
				}
				index=0;
				bufsize=retval;
				/* Continue writing until error or everything
				 * written.
				 */
				while ((index<bufsize) && ((retval=dstf->f_op->write
					 (dstf,buffer+index,bufsize-index,&dstf->f_pos)) > 0))
					index+=retval;
				if (index < bufsize)
				{
					printk("ep_to_file Write error %d\n",-retval);
					break;
				}
				else total = total + bufsize;
			} while (total < count);
			/* Now clean up everything that was actually allocated
			 */
			// retval=filp_close(dstf,NULL);
			// if (retval) printk("kcp: Error %d closing %s\n",-retval,dst);
			fput(dstf);  // flush any buffers to disk
		}
		free_page(page);
	}
	else
	{
		printk("kcp: Out of memory\n");
	}
	// restore...
	set_fs(orgfs);
	current->fsuid=orgfsuid;
	current->fsgid=orgfsgid;

	return total;
}

static int ep_ioctl (struct inode *inode, struct file *fd,
		unsigned code, unsigned long value)
{
	void *arg = (void *) value;
	struct ep_data		*data = fd->private_data;
	int			status;
	struct mtp_file_io mtp_req;

	if ((status = get_ready_ep (fd->f_flags, data)) < 0)
		return status;

	spin_lock_irq (&data->dev->lock);
	if (likely (data->ep != NULL)) {
		switch (code) {
		case GADGETFS_FIFO_STATUS:
			status = usb_ep_fifo_status (data->ep);
			break;
		case GADGETFS_FIFO_FLUSH:
			usb_ep_fifo_flush (data->ep);
			break;
		case GADGETFS_CLEAR_HALT:
			status = usb_ep_clear_halt (data->ep);
			break;
		case GADGETFS_CANCEL:
			// signal to break out of io
			// fasync_helper (-1, fd, 0, &data->dev->fasync);
			// flush any data left
			// usb_ep_fifo_flush (data->ep);	
			// try and dequeue any outstanding requests, completion handler will be called
			// spin_unlock_irq (&data->dev->lock);
			// usb_ep_dequeue (data->ep, data->req);	
			// spin_lock_irq (&data->dev->lock);
			// status = usb_ep_clear_halt (data->ep);		
			break;
		case GADGETFS_SET_SINK:
			data->dev->sink_ep = data->ep;
			break;
		case GADGETFS_SET_SOURCE:
			data->dev->source_ep = data->ep;
			break;		
		case GADGETFS_WRITE_FILE:
			// get write request from user mode
			if (copy_from_user(&mtp_req, (struct mtp_file_io *) arg, sizeof (struct mtp_file_io))) {
				status = -EFAULT;
				break;
			}
			status = ep_to_file(data, mtp_req.fd, mtp_req.count);
			break;
		default:
			status = -ENOTTY;
		}
	} else
		status = -ENODEV;
	spin_unlock_irq (&data->dev->lock);
	up (&data->lock);
	return status;
}


static void mtp_pipe_buf_release(struct pipe_inode_info *pipe,
				 struct pipe_buffer *buf)
{
	put_page(buf->page);
	buf->flags &= ~PIPE_BUF_FLAG_LRU;
}

static struct pipe_buf_operations mtp_pipe_buf_ops = {
	.can_merge	= 0,
	.map		= generic_pipe_buf_map,
	.unmap		= generic_pipe_buf_unmap,
	.confirm	= generic_pipe_buf_confirm,
	.release	= mtp_pipe_buf_release,
	.steal		= generic_pipe_buf_steal,
	.get		= generic_pipe_buf_get,
};

static ssize_t ep_splice_read(struct file *in, loff_t *ppos,
			      struct pipe_inode_info *pipe, size_t len,
			      unsigned int flags)
{
	struct page *pages[PIPE_BUFFERS];
	struct partial_page partial[PIPE_BUFFERS];
	struct page *page;
	size_t this_len;
	int rv;

	struct splice_pipe_desc spd = {
		.pages = pages,
		.partial = partial,
		.nr_pages = 0,
		.flags = flags,
		.ops = &mtp_pipe_buf_ops,
        };

	if (flags & SPLICE_F_NONBLOCK)
		return -EINVAL;

	for (; len; len -= this_len) {

		page = alloc_page(GFP_KERNEL);
		if (page == NULL)
			goto err_nomem;

		this_len = min_t(size_t, len, PAGE_SIZE);
		rv = ep_io(in->private_data, page_address(page), this_len);
		if (rv < 0)
			goto err_io;

		spd.pages[spd.nr_pages] = page;
		spd.partial[spd.nr_pages].offset = 0;
		spd.partial[spd.nr_pages].len = rv;
		spd.nr_pages++;

		if (rv < this_len)
			break;
		if (spd.nr_pages > PIPE_BUFFERS - 1)
			break;
	}

	return splice_to_pipe(pipe, &spd);

err_nomem:
	rv = -ENOMEM;
err_io:
	while (spd.nr_pages--)
		__free_page(spd.pages[spd.nr_pages]);
	if (page)
		__free_page(page);
	return rv;
}

static int pipe_to_gadget(struct pipe_inode_info *pipe, struct pipe_buffer *buf,
			  struct splice_desc *sd)
{
	char *src = buf->ops->map(pipe, buf, 1) + buf->offset;
	int rv = ep_io(sd->u.file->private_data, src, sd->len);
	buf->ops->unmap(pipe, buf, src);
	return rv;
}

static ssize_t ep_splice_write(struct pipe_inode_info *pipe, struct file *out,
			       loff_t *ppos, size_t len, unsigned int flags)
{
	return splice_from_pipe(pipe, out, ppos, len, flags, pipe_to_gadget);
}


/*----------------------------------------------------------------------*/

/* used after endpoint configuration */
static struct file_operations ep_io_operations = {
	.owner =	THIS_MODULE,
	.llseek =	no_llseek,

	.read =		ep_read,
	.write =	ep_write,
	.ioctl =	ep_ioctl,
	.release =	ep_release,
	.splice_read	= ep_splice_read,
	.splice_write	= ep_splice_write,
};

/* ENDPOINT INITIALIZATION
 *
 *     fd = open ("/dev/gadget/$ENDPOINT", O_RDWR)
 *     status = write (fd, descriptors, sizeof descriptors)
 *
 * That write establishes the endpoint configuration, configuring
 * the controller to process bulk, interrupt, or isochronous transfers
 * at the right maxpacket size, and so on.
 *
 * The descriptors are message type 1, identified by a host order u32
 * at the beginning of what's written.  Descriptor order is: full/low
 * speed descriptor, then optional high speed descriptor.
 */
static ssize_t
ep_config (struct file *fd, const char __user *buf, size_t len, loff_t *ptr)
{
	struct ep_data		*data = fd->private_data;
	struct usb_ep		*ep;
	u32			tag;
	int			value;

	if ((value = down_interruptible (&data->lock)) < 0)
		return value;

	if (data->state != STATE_EP_READY) {
		value = -EL2HLT;
		goto fail;
	}

	value = len;
	if (len < USB_DT_ENDPOINT_SIZE + 4)
		goto fail0;

	/* we might need to change message format someday */
	if (copy_from_user (&tag, buf, 4)) {
		goto fail1;
	}
	if (tag != 1) {
		DBG(data->dev, "config %s, bad tag %d\n", data->name, tag);
		goto fail0;
	}
	buf += 4;
	len -= 4;

	/* NOTE:  audio endpoint extensions not accepted here;
	 * just don't include the extra bytes.
	 */

	/* full/low speed descriptor, then high speed */
	if (copy_from_user (&data->desc, buf, USB_DT_ENDPOINT_SIZE)) {
		goto fail1;
	}
	if (data->desc.bLength != USB_DT_ENDPOINT_SIZE
			|| data->desc.bDescriptorType != USB_DT_ENDPOINT)
		goto fail0;
	if (len != USB_DT_ENDPOINT_SIZE) {
		if (len != 2 * USB_DT_ENDPOINT_SIZE)
			goto fail0;
		if (copy_from_user (&data->hs_desc, buf + USB_DT_ENDPOINT_SIZE,
					USB_DT_ENDPOINT_SIZE)) {
			goto fail1;
		}
		if (data->hs_desc.bLength != USB_DT_ENDPOINT_SIZE
				|| data->hs_desc.bDescriptorType
					!= USB_DT_ENDPOINT) {
			DBG(data->dev, "config %s, bad hs length or type\n",
					data->name);
			goto fail0;
		}
	}
	value = len;

	spin_lock_irq (&data->dev->lock);
	if (data->dev->state == STATE_DEV_UNBOUND) {
		value = -ENOENT;
		goto gone;
	} else if ((ep = data->ep) == NULL) {
		value = -ENODEV;
		goto gone;
	}
	switch (data->dev->gadget->speed) {
	case USB_SPEED_LOW:
	case USB_SPEED_FULL:
		value = usb_ep_enable (ep, &data->desc);
		if (value == 0)
			data->state = STATE_EP_ENABLED;
		break;
#ifdef	CONFIG_USB_GADGET_DUALSPEED
	case USB_SPEED_HIGH:
		/* fails if caller didn't provide that descriptor... */
		value = usb_ep_enable (ep, &data->hs_desc);
		if (value == 0)
			data->state = STATE_EP_ENABLED;
		break;
#endif
	default:
		DBG (data->dev, "unconnected, %s init deferred\n",
				data->name);
		data->state = STATE_EP_DEFER_ENABLE;
	}
	if (value == 0)
		fd->f_op = &ep_io_operations;
gone:
	spin_unlock_irq (&data->dev->lock);
	if (value < 0) {
fail:
		data->desc.bDescriptorType = 0;
		data->hs_desc.bDescriptorType = 0;
	}
	up (&data->lock);
	return value;
fail0:
	value = -EINVAL;
	goto fail;
fail1:
	value = -EFAULT;
	goto fail;
}

static int
ep_open (struct inode *inode, struct file *fd)
{
	struct ep_data		*data = inode->i_private;
	int			value = -EBUSY;

	if (down_interruptible (&data->lock) != 0)
		return -EINTR;
	spin_lock_irq (&data->dev->lock);
	if (data->dev->state == STATE_DEV_UNBOUND)
		value = -ENOENT;
	else if (data->state == STATE_EP_DISABLED) {
		value = 0;
		data->state = STATE_EP_READY;
		get_ep (data);
		fd->private_data = data;
		VDEBUG (data->dev, "%s ready\n", data->name);
	} else
		DBG (data->dev, "%s state %d\n",
			data->name, data->state);
	spin_unlock_irq (&data->dev->lock);
	up (&data->lock);
	return value;
}

/* used before endpoint configuration */
static struct file_operations ep_config_operations = {
	.owner =	THIS_MODULE,
	.llseek =	no_llseek,

	.open =		ep_open,
	.write =	ep_config,
	.release =	ep_release,
};

/*----------------------------------------------------------------------*/
static void clean_req (struct usb_ep *ep, struct usb_request *req)
{
	struct dev_data		*dev = ep->driver_data;

	if (req->buf != dev->rbuf) {
		kfree(req->buf);
		req->buf = dev->rbuf;
		req->dma = DMA_ADDR_INVALID;
	}
	req->complete = epio_complete;
}

static int setup_req (struct usb_ep *ep, struct usb_request *req, u16 len);
static ssize_t
ep0_ack (struct dev_data *dev)
{
	int retval;
	struct usb_ep		*ep = dev->gadget->ep0;
	struct usb_request	*req = dev->req;

	if ((retval = setup_req (ep, req, 0)) == 0)
		retval = usb_ep_queue (ep, req, GFP_ATOMIC);

	return retval;
}

static void ep0_complete (struct usb_ep *ep, struct usb_request *req)
{
	struct dev_data		*dev = ep->driver_data;
	int			free = 1;

	/* for control OUT, data must still get to userspace */
	if (!dev->setup_in) {
		dev->setup_out_error = (req->status != 0);
		if (!dev->setup_out_error)
		{
			if (dev->user_event.type == CANCEL_REQUEST)
			{
				// spin_lock_irq (&dev->lock);
				ep0_ack(dev);  // status phase
				// get transaction id, store in event param1 so we can pass to user space
				dev->user_event.param1 = le32_to_cpu(dev->cancel_req_data.TransactionID);
				dev->mtp_event_ready = 1;
				wake_up (&dev->user_wait);				
				dev->did_cancel = 1;
				free = 0;
			}
		}
	}
		
	/* clean up as appropriate */
	if (free && req->buf != &dev->rbuf)
		clean_req (ep, req);
	req->complete = epio_complete;
}

static int setup_req (struct usb_ep *ep, struct usb_request *req, u16 len)
{
	struct dev_data	*dev = ep->driver_data;

	if (len > sizeof (dev->rbuf))
		req->buf = kmalloc(len, GFP_ATOMIC);
	if (req->buf == 0) {
		req->buf = dev->rbuf;
		return -ENOMEM;
	}
	req->complete = ep0_complete;
	req->length = len;
	req->zero = 0;
	return 0;
}

// This is the user space file read of the endpoint 0 handle
static ssize_t
ep0_read (struct file *fd, char __user *buf, size_t len, loff_t *ptr)
{
	struct dev_data			*dev = fd->private_data;
	ssize_t				retval = 0;
	spin_lock_irq (&dev->lock);

	/* report fd mode change before acting on it */
	if (dev->setup_abort) {
		dev->setup_abort = 0;
		retval = -EIDRM;
		goto done;
	}

	/* control DATA stage */
	if (len == 0) {		/* ack SET_CONFIGURATION etc */
		struct usb_ep		*ep = dev->gadget->ep0;
		struct usb_request	*req = dev->req;

		if ((retval = setup_req (ep, req, 0)) == 0)
			retval = usb_ep_queue (ep, req, GFP_ATOMIC);
	} else {			/* collect OUT data */
		// no user read of ep0 
		retval = -EFAULT;
	}

done:
	spin_unlock_irq (&dev->lock);
	return retval;
}

static ssize_t
ep0_write (struct file *fd, const char __user *buf, size_t len, loff_t *ptr)
{
	struct dev_data		*dev = fd->private_data;
	ssize_t			retval = -ESRCH;

	spin_lock_irq (&dev->lock);

	/* report fd mode change before acting on it */
	if (dev->setup_abort) {
		dev->setup_abort = 0;
		retval = -EIDRM;

	/* data and/or status stage for control request */
	} else if (dev->state == STATE_SETUP) {

		/* IN DATA+STATUS caller makes len <= wLength */
		if (dev->setup_in) {
			retval = setup_req (dev->gadget->ep0, dev->req, len);
			if (retval == 0) {
				spin_unlock_irq (&dev->lock);
				if (copy_from_user (dev->req->buf, buf, len))
					retval = -EFAULT;
				else {
					retval = usb_ep_queue (
						dev->gadget->ep0, dev->req,
						GFP_KERNEL);
				}
				if (retval < 0) {
					spin_lock_irq (&dev->lock);
					clean_req (dev->gadget->ep0, dev->req);
					spin_unlock_irq (&dev->lock);
				} else
					retval = len;

				return retval;
			}

		/* can stall some OUT transfers */
		} else if (dev->setup_can_stall) {
			VDEBUG(dev, "ep0out stall\n");
			(void) usb_ep_set_halt (dev->gadget->ep0);
			retval = -EL2HLT;
			dev->state = STATE_CONNECTED;
		} else {
			DBG(dev, "bogus ep0out stall!\n");
		}
	} else
		DBG (dev, "fail %s, state %d\n", __FUNCTION__, dev->state);

	spin_unlock_irq (&dev->lock);
	return retval;
}

static int
ep0_fasync (int f, struct file *fd, int on)
{
	struct dev_data		*dev = fd->private_data;
	// caller must F_SETOWN before signal delivery happens
	VDEBUG (dev, "%s %s\n", __FUNCTION__, on ? "on" : "off");
	return fasync_helper (f, fd, on, &dev->fasync);
}

static struct usb_gadget_driver gadgetfs_driver;

static int
dev_release (struct inode *inode, struct file *fd)
{
	struct dev_data		*dev = fd->private_data;

	/* closing ep0 === shutdown all */

	usb_gadget_unregister_driver (&gadgetfs_driver);

	/* at this point "good" hardware has disconnected the
	 * device from USB; the host won't see it any more.
	 * alternatively, all host requests will time out.
	 */

	fasync_helper (-1, fd, 0, &dev->fasync);
	kfree (dev->buf);
	dev->buf = NULL;
	put_dev (dev);

	/* other endpoints were all decoupled from this device */
	dev->state = STATE_DEV_DISABLED;
	return 0;
}

static void destroy_ep_files (struct dev_data *dev);

static int dev_ioctl (struct inode *inode, struct file *fd,
		unsigned code, unsigned long value)
{
	void *arg = (void *) value;
	struct dev_data		*dev = fd->private_data;
	struct usb_gadget	*gadget = dev->gadget;
	ssize_t				retval;
		
	switch (code)
	{
		case GADGETFS_EV_WAIT:
			// Wait for next user event
			retval = wait_event_interruptible (dev->user_wait, dev->mtp_event_ready != 0);
			dev->mtp_event_ready = 0;
			if (copy_to_user (arg, &dev->user_event, sizeof (struct usb_gadgetfs_event)))
				return -EFAULT;
			return 0;
		case GADGETFS_CLOSE_FILES:
                        // close file links to kernel and user
			destroy_ep_files (dev);
			return 0;
		case GADGETFS_CANCEL_DONE:
			if (dev->did_cancel == 0)
				return -EINVAL;
			dev->did_cancel = 0;
			return 0;
		default:
			if (gadget->ops->ioctl)
				return gadget->ops->ioctl (gadget, code, value);
			break;
	}
	return -ENOTTY;
}

/* used after device configuration */
static struct file_operations ep0_io_operations = {
	.owner =	THIS_MODULE,
	.llseek =	no_llseek,

	.read =		ep0_read,
	.write =	ep0_write,
	.fasync =	ep0_fasync,
	// .poll =	ep0_poll,
	.ioctl =	dev_ioctl,
	.release =	dev_release,
};

/*----------------------------------------------------------------------*/

/* The in-kernel gadget driver handles most ep0 issues, in particular
 * enumerating the single configuration (as provided from user space).
 *
 * Unrecognized ep0 requests may be handled in user space.
 */

#ifdef	CONFIG_USB_GADGET_DUALSPEED
static void make_qualifier (struct dev_data *dev)
{
	struct usb_qualifier_descriptor		qual;
	struct usb_device_descriptor		*desc;

	qual.bLength = sizeof qual;
	qual.bDescriptorType = USB_DT_DEVICE_QUALIFIER;
	qual.bcdUSB = __constant_cpu_to_le16 (0x0200);

	desc = dev->dev;
	qual.bDeviceClass = desc->bDeviceClass;
	qual.bDeviceSubClass = desc->bDeviceSubClass;
	qual.bDeviceProtocol = desc->bDeviceProtocol;

	/* assumes ep0 uses the same value for both speeds ... */
	qual.bMaxPacketSize0 = desc->bMaxPacketSize0;

	qual.bNumConfigurations = 1;
	qual.bRESERVED = 0;

	memcpy (dev->rbuf, &qual, sizeof qual);
}
#endif

static int
config_buf (struct dev_data *dev, u8 type, unsigned index)
{
	int		len;
#ifdef CONFIG_USB_GADGET_DUALSPEED
	int		hs;
#endif

	/* only one configuration */
	if (index > 0)
		return -EINVAL;

#ifdef CONFIG_USB_GADGET_DUALSPEED
	hs = (dev->gadget->speed == USB_SPEED_HIGH);
	if (type == USB_DT_OTHER_SPEED_CONFIG)
		hs = !hs;
	if (hs) {
		dev->req->buf = dev->hs_config;
		len = le16_to_cpup (&dev->hs_config->wTotalLength);
	} else
#endif
	{
		dev->req->buf = dev->config;
		len = le16_to_cpup (&dev->config->wTotalLength);
	}
	((u8 *)dev->req->buf) [1] = type;
	return len;
}

// MTP set configuration
#define CONFIG_VALUE 3

static int
gadgetfs_setup (struct usb_gadget *gadget, const struct usb_ctrlrequest *ctrl)
{
	struct dev_data			*dev = get_gadget_data (gadget);
	struct usb_request		*req = dev->req;
	struct usb_ep *ep;
	int				value = -EOPNOTSUPP;
	int tmp;
	u8 *bptr;
	u8	config, power;

	spin_lock (&dev->lock);
	dev->setup_abort = 0;
	if (dev->state == STATE_UNCONNECTED) {
		struct usb_ep	*ep;
		struct ep_data	*data;

		dev->state = STATE_CONNECTED;
		dev->dev->bMaxPacketSize0 = gadget->ep0->maxpacket;

#ifdef	CONFIG_USB_GADGET_DUALSPEED
		if (gadget->speed == USB_SPEED_HIGH && dev->hs_config == 0) {
			ERROR (dev, "no high speed config??\n");
			return -EINVAL;
		}
#endif	/* CONFIG_USB_GADGET_DUALSPEED */
                // Save speed to send to user later
                dev->user_event.param1 = gadget->speed;				
		list_for_each_entry (ep, &gadget->ep_list, ep_list) {
			data = ep->driver_data;
			/* ... down_trylock (&data->lock) ... */
			if (data->state != STATE_EP_DEFER_ENABLE)
				continue;
#ifdef	CONFIG_USB_GADGET_DUALSPEED
			if (gadget->speed == USB_SPEED_HIGH)
				value = usb_ep_enable (ep, &data->hs_desc);
			else
#endif	/* CONFIG_USB_GADGET_DUALSPEED */
				value = usb_ep_enable (ep, &data->desc);
			if (value) {
				ERROR (dev, "deferred %s enable --> %d\n",
					data->name, value);
				continue;
			}
			data->state = STATE_EP_ENABLED;
			wake_up (&data->wait);
			DBG (dev, "woke up %s waiters\n", data->name);
		}

	/* host may have given up waiting for response.  we can miss control
	 * requests handled lower down (device/endpoint status and features);
	 * then ep0_{read,write} will report the wrong status. controller
	 * driver will have aborted pending i/o.
	 */
	} else if (dev->state == STATE_SETUP)
		dev->setup_abort = 1;

	req->buf = dev->rbuf;
	req->dma = DMA_ADDR_INVALID;
	req->context = NULL;
	// Any requests which aren't handled will stall if value isn't >= 0
	value = -EOPNOTSUPP;
	switch (ctrl->bRequest) {

	case USB_REQ_GET_DESCRIPTOR:
		if (ctrl->bRequestType != USB_DIR_IN)
			goto unrecognized;
		switch (ctrl->wValue >> 8) {

		case USB_DT_DEVICE:
			value = min (ctrl->wLength, (u16) sizeof *dev->dev);
			req->buf = dev->dev;
			break;
#ifdef	CONFIG_USB_GADGET_DUALSPEED
		case USB_DT_DEVICE_QUALIFIER:
			if (!dev->hs_config)
				break;
			value = min (ctrl->wLength, (u16)
				sizeof (struct usb_qualifier_descriptor));
			make_qualifier (dev);
			break;
		case USB_DT_OTHER_SPEED_CONFIG:
			// FALLTHROUGH
#endif
		case USB_DT_CONFIG:
			value = config_buf (dev,
					ctrl->wValue >> 8,
					ctrl->wValue & 0xff);
			if (value >= 0)
				value = min (ctrl->wLength, (u16) value);
			break;
		case USB_DT_STRING:
			tmp = ctrl->wValue & 0x0ff;
			if (tmp == OS_DESC)
			{
				memcpy(req->buf, (u8 *) &MTP_OS_DESCRIPTOR, MTP_OS_DESCRIPTOR_SIZE);				
                                value = min(ctrl->wLength, (u16) MTP_OS_DESCRIPTOR_SIZE);
                                break;                                                            
			}
			if (tmp != 0 && ctrl->wIndex != strings.language)
			{
				break;
			}
			/* wIndex == language code.
			 * this driver only handles one language, you can
			 * add others even if they don't use iso8859/1
			 */
			value = usb_gadget_get_string(&strings, tmp, req->buf);
			if (value >= 0)
				value = min (ctrl->wLength, (u16) value);
                        break;    
		default:		// all others are errors
			break;
		}
		break;
        case GET_MS_DESCRIPTOR:
		if (ctrl->bRequestType != (USB_DIR_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE))
		{
			break;
		}
		// Complete special msdescriptor setup
		memcpy(&msdescriptor.ExtendedConfigHeader, &os_extended_config_header, sizeof(struct os_extended_configuration_descriptor_header));
		memcpy(&msdescriptor.ExtendedConfigFunction, &os_extended_config_function, sizeof(struct os_extended_configuration_descriptor_function));
		memcpy(req->buf, (u8 *) &msdescriptor, sizeof(struct os_extended_configuration));				
                value = min(ctrl->wLength, (u16) sizeof(struct os_extended_configuration));		
                break;
	case USB_REQ_GET_INTERFACE:
		if (ctrl->bRequestType == (USB_DIR_IN|USB_RECIP_INTERFACE)
		    && ctrl->wIndex == 0 && ctrl->wLength <= 1)
		{
			/* only one altsetting in this driver */
			bptr = (u8 *) req->buf;
			bptr[0] = 0;
			value = 1;
		}
		else
		{
			printk("Bad get interface request type!\n");
		}
		break;
	case USB_REQ_SET_INTERFACE:
		if (ctrl->bRequestType == USB_RECIP_INTERFACE
			    && ctrl->wIndex == 0 && ctrl->wValue == 0)
		{
			// send event to user land to just reset toggle/halt for the interface's endpoints 	
			dev->user_event.type = GADGETFS_SET_INTERFACE;
			dev->mtp_event_ready = 1;
			wake_up (&dev->user_wait);	
			// then send an ack
			ep0_ack (dev);
		}
		else
		{
			printk("Bad set interface request type!\n");
		}
		break;
	case USB_REQ_GET_CONFIGURATION:
		if (ctrl->bRequestType != USB_DIR_IN)
			break; // unknown
		*(u8 *)req->buf = dev->current_config;
		value = min (ctrl->wLength, (u16) 1);
		break;		
	/* currently one config, two speeds */
	case USB_REQ_SET_CONFIGURATION:
		if (ctrl->bRequestType != 0)
			break;
		/* report SET_CONFIGURATION like any other control request,
		 * except that usermode may not stall this.  the next
		 * request mustn't be allowed start until this finishes:
		 * endpoints and threads set up, etc.
		 */
		INFO (dev, "configuration #%d\n", dev->current_config);
		if (ctrl->bRequestType != (USB_DIR_OUT |  USB_TYPE_STANDARD | USB_RECIP_DEVICE))
		{
			break;
		}				
		switch (ctrl->wValue)
		{
			case CONFIG_VALUE:
#ifdef	CONFIG_USB_GADGET_DUALSPEED
				if (gadget->speed == USB_SPEED_HIGH) {
					config = dev->hs_config->bConfigurationValue;
					power = dev->hs_config->bMaxPower;
				} else
#endif
				{
					config = dev->config->bConfigurationValue;
					power = dev->config->bMaxPower;
				}

				if (config == (u8) ctrl->wValue) {
					value = 0;
					dev->current_config = config;
					// usb_gadget_vbus_draw(gadget, 2 * power);
				}					
				// Send MTP start event to user space
				dev->user_event.type = GADGETFS_CONNECT;
				dev->mtp_event_ready = 1;
				wake_up (&dev->user_wait);	
				ep0_ack (dev); 
				break;
			case 0:
				dev->current_config = 0;
				// usb_gadget_vbus_draw(gadget, 8 /* mA */ );					
				// Send MTP stop event to user space
				dev->user_event.type = GADGETFS_DISCONNECT;
				dev->mtp_event_ready = 1;
				wake_up (&dev->user_wait);						
				ep0_ack (dev);
				break;
			default:
				/* kernel bug -- "can't happen" */
				ERROR (dev, "Bad set configuration %d??\n", ctrl->wValue);
				break;
		}
		break;
	// PTP class requests
	case GET_EXTENDED_EVENT_DATA:
		if (dev->did_reset)
		{
			dev->ext_data.EventCode =  __constant_cpu_to_le16(0x400B);  // device reset event
			dev->did_reset = 0;
		}
		else
			dev->ext_data.EventCode =  __constant_cpu_to_le16(0x4000); // Undefined event
		dev->ext_data.TransactionID = 0;
		dev->ext_data.NumberOfParameters = 0;
		value = min (ctrl->wLength, (u16) sizeof(EXTENDED_EVENT_DATA));		
		req->buf = (u8 *) &dev->ext_data;
		break;
	case DEVICE_RESET_REQUEST:
		dev->user_event.type = DEVICE_RESET_REQUEST;	
		dev->mtp_event_ready = 1;
		wake_up (&dev->user_wait);						
		ep0_ack (dev);		// send ack to host
		dev->did_reset = 1;
		break;
	case CANCEL_REQUEST:
		VDEBUG(dev, "Host MTP cancel request\n");
		gadget_for_each_ep(ep, gadget) {
			struct ep_data *data = ep->driver_data;
			usb_ep_dequeue(ep, data->req);
		}
		dev->user_event.type = CANCEL_REQUEST;
		req->buf = (u8 *) &dev->cancel_req_data;
		break;
	case GET_DEVICE_STATUS:
		/*
		 * This hack just adds support for host initiated cancel
		 */
		tmp = dev->did_cancel ? 0x2019 : 0x2001;
		dev->dev_status_data.Code = __constant_cpu_to_le16(tmp);
		dev->dev_status_data.wLength = __constant_cpu_to_le16(4); // total size of device status data
		req->buf = (u8 *) &dev->dev_status_data;
		value = 4;
		break;
	default:
unrecognized:
		VDEBUG (dev, "unrec %s req%02x.%02x v%04x i%04x l%d\n",
			dev->usermode_setup ? "delegate" : "fail",
			ctrl->bRequestType, ctrl->bRequest,
			ctrl->wValue, ctrl->wIndex, ctrl->wLength);
		VDEBUG(dev, "ep0in stall\n");
			(void) usb_ep_set_halt (dev->gadget->ep0);
			dev->state = STATE_CONNECTED;
	}

	dev->setup_in = (ctrl->bRequestType & USB_DIR_IN) ? 1 : 0;
	if (!dev->setup_in)
	{
		// Set up OUT direction data phase on ep0 to get rest of data
		// ep0 complete is called when the req is complete
		req->complete = ep0_complete;
		req->length =  ctrl->wLength;
		value = usb_ep_queue (gadget->ep0, req, GFP_ATOMIC);
		if (value < 0) clean_req (gadget->ep0, req);
		else value = 0;
	}
	/* proceed with in data transfer and status phases */
	else if (value >= 0 && dev->state != STATE_SETUP) {
		req->length = value;
		req->zero = value < ctrl->wLength
				&& (value % gadget->ep0->maxpacket) == 0;
		value = usb_ep_queue (gadget->ep0, req, GFP_ATOMIC);
		if (value < 0) {
			DBG (dev, "ep_queue --> %d\n", value);
			req->status = 0;
		}
	}

	/* device stalls when value < 0 */
	spin_unlock (&dev->lock);
	return value;
}

static void destroy_ep_files (struct dev_data *dev)
{
	struct list_head	*entry, *tmp;

	DBG (dev, "%s %d\n", __FUNCTION__, dev->state);

	/* dev->state must prevent interference */
restart:
	spin_lock_irq (&dev->lock);
	list_for_each_safe (entry, tmp, &dev->epfiles) {
		struct ep_data	*ep;
		struct inode	*parent;
		struct dentry	*dentry;

		/* break link to FS */
		ep = list_entry (entry, struct ep_data, epfiles);
		list_del_init (&ep->epfiles);
		dentry = ep->dentry;
		ep->dentry = NULL;
		parent = dentry->d_parent->d_inode;

		/* break link to controller */
		if (ep->state == STATE_EP_ENABLED)
			(void) usb_ep_disable (ep->ep);
		ep->state = STATE_EP_UNBOUND;
		usb_ep_free_request (ep->ep, ep->req);
		ep->ep = NULL;
		wake_up (&ep->wait);
		put_ep (ep);

		spin_unlock_irq (&dev->lock);

		/* break link to dcache */
		mutex_lock (&parent->i_mutex);
		d_delete (dentry);
		dput (dentry);
		mutex_unlock (&parent->i_mutex);

		/* fds may still be open */
		goto restart;
	}
	spin_unlock_irq (&dev->lock);
}


static struct inode *
gadgetfs_create_file (struct super_block *sb, char const *name,
		void *data, const struct file_operations *fops,
		struct dentry **dentry_p);

static int activate_ep_files (struct dev_data *dev)
{
	struct usb_ep	*ep;
	struct ep_data	*data;

	gadget_for_each_ep (ep, dev->gadget) {

		data = kzalloc(sizeof(*data), GFP_KERNEL);
		if (!data)
			goto enomem0;
		data->state = STATE_EP_DISABLED;
		init_MUTEX (&data->lock);
		init_waitqueue_head (&data->wait);

		strncpy (data->name, ep->name, sizeof (data->name) - 1);
		atomic_set (&data->count, 1);
		data->dev = dev;
		get_dev (dev);

		data->ep = ep;
		ep->driver_data = data;

		data->req = usb_ep_alloc_request (ep, GFP_KERNEL);
		if (!data->req)
			goto enomem1;

		data->inode = gadgetfs_create_file (dev->sb, data->name,
				data, &ep_config_operations,
				&data->dentry);
		if (!data->inode)
			goto enomem2;
		list_add_tail (&data->epfiles, &dev->epfiles);
	}
	return 0;

enomem2:
	usb_ep_free_request (ep, data->req);
enomem1:
	put_dev (dev);
	kfree (data);
enomem0:
	DBG (dev, "%s enomem\n", __FUNCTION__);
	destroy_ep_files (dev);
	return -ENOMEM;
}

static void
gadgetfs_unbind (struct usb_gadget *gadget)
{
	struct dev_data		*dev = get_gadget_data (gadget);

	DBG (dev, "%s\n", __FUNCTION__);

	spin_lock_irq (&dev->lock);
	dev->state = STATE_DEV_UNBOUND;
	spin_unlock_irq (&dev->lock);

	destroy_ep_files (dev);
	gadget->ep0->driver_data = NULL;
	set_gadget_data (gadget, NULL);

	/* we've already been disconnected ... no i/o is active */
	if (dev->req)
		usb_ep_free_request (gadget->ep0, dev->req);
	DBG (dev, "%s done\n", __FUNCTION__);
	put_dev (dev);
}

static struct dev_data		*the_device;

static int
gadgetfs_bind (struct usb_gadget *gadget)
{
	struct dev_data		*dev = the_device;

	if (!dev)
		return -ESRCH;
	if (0 != strcmp (CHIP, gadget->name)) {
		printk (KERN_ERR "%s expected %s controller not %s\n",
			shortname, CHIP, gadget->name);
		return -ENODEV;
	}

	set_gadget_data (gadget, dev);
	dev->gadget = gadget;
	gadget->ep0->driver_data = dev;
	dev->dev->bMaxPacketSize0 = gadget->ep0->maxpacket;

	/* preallocate control response and buffer */
	dev->req = usb_ep_alloc_request (gadget->ep0, GFP_KERNEL);
	if (!dev->req)
		goto enomem;
	dev->req->context = NULL;
	dev->req->complete = epio_complete;

	if (activate_ep_files (dev) < 0)
		goto enomem;

	INFO (dev, "bound to %s driver\n", gadget->name);
	dev->state = STATE_UNCONNECTED;
	get_dev (dev);
	return 0;

enomem:
	gadgetfs_unbind (gadget);
	return -ENOMEM;
}

static void
gadgetfs_disconnect (struct usb_gadget *gadget)
{
	struct dev_data		*dev = get_gadget_data (gadget);

	if (dev->state == STATE_UNCONNECTED) {
		DBG (dev, "already unconnected\n");
		return;
	}
	dev->state = STATE_UNCONNECTED;

	dev->user_event.type = GADGETFS_DISCONNECT;
	dev->mtp_event_ready = 1;
	wake_up (&dev->user_wait);	
	INFO (dev, "disconnected\n");
}

static void
gadgetfs_suspend (struct usb_gadget *gadget)
{
	struct dev_data		*dev = get_gadget_data (gadget);

	INFO (dev, "suspended from state %d\n", dev->state);
	spin_lock (&dev->lock);
	switch (dev->state) {
	case STATE_SETUP:		// VERY odd... host died??
	case STATE_CONNECTED:
	case STATE_UNCONNECTED:
		/* FALLTHROUGH */
	default:
		break;
	}
	dev->user_event.type = GADGETFS_SUSPEND;
	dev->mtp_event_ready = 1;
	wake_up (&dev->user_wait);		
	spin_unlock (&dev->lock);
}

static struct usb_gadget_driver gadgetfs_driver = {
#ifdef	CONFIG_USB_GADGET_DUALSPEED
	.speed		= USB_SPEED_HIGH,
#else
	.speed		= USB_SPEED_FULL,
#endif
	.function	= (char *) driver_desc,
	.bind		= gadgetfs_bind,
	.unbind		= gadgetfs_unbind,
	.setup		= gadgetfs_setup,
	.disconnect	= gadgetfs_disconnect,
	.suspend	= gadgetfs_suspend,

	.driver 	= {
		.name		= (char *) shortname,
	},
};

/*----------------------------------------------------------------------*/

static void gadgetfs_nop(struct usb_gadget *arg) { }

static int gadgetfs_probe (struct usb_gadget *gadget)
{
	CHIP = gadget->name;
	return -EISNAM;
}

static struct usb_gadget_driver probe_driver = {
	.speed		= USB_SPEED_HIGH,
	.bind		= gadgetfs_probe,
	.unbind		= gadgetfs_nop,
	.setup		= (void *)gadgetfs_nop,
	.disconnect	= gadgetfs_nop,
	.driver 	= {
		.name		= "nop",
	},
};


/* DEVICE INITIALIZATION
 *
 *     fd = open ("/dev/gadget/$CHIP", O_RDWR)
 *     status = write (fd, descriptors, sizeof descriptors)
 *
 * That write establishes the device configuration, so the kernel can
 * bind to the controller ... guaranteeing it can handle enumeration
 * at all necessary speeds.  Descriptor order is:
 *
 * . message tag (u32, host order) ... for now, must be zero; it
 *	would change to support features like multi-config devices
 * . full/low speed config ... all wTotalLength bytes (with interface,
 *	class, altsetting, endpoint, and other descriptors)
 * . high speed config ... all descriptors, for high speed operation;
 * 	this one's optional except for high-speed hardware
 * . device descriptor
 * . string table
 *
 * Endpoints are not yet enabled. Drivers may want to immediately
 * initialize them, using the /dev/gadget/ep* files that are available
 * as soon as the kernel sees the configuration, or they can wait
 * until device configuration and interface altsetting changes create
 * the need to configure (or unconfigure) them.
 *
 * After initialization, the device stays active for as long as that
 * $CHIP file is open.  Events may then be read from that descriptor,
 * such as configuration notifications.  More complex drivers will handle
 * some control requests in user space.
 */

static int is_valid_config (struct usb_config_descriptor *config)
{
	return config->bDescriptorType == USB_DT_CONFIG
		&& config->bLength == USB_DT_CONFIG_SIZE
		&& config->bConfigurationValue != 0
		&& (config->bmAttributes & USB_CONFIG_ATT_ONE) != 0
		&& (config->bmAttributes & USB_CONFIG_ATT_WAKEUP) == 0;
	/* FIXME if gadget->is_otg, _must_ include an otg descriptor */
	/* FIXME check lengths: walk to end */
}

static ssize_t
dev_config (struct file *fd, const char __user *buf, size_t len, loff_t *ptr)
{
	struct dev_data		*dev = fd->private_data;
	ssize_t			value = len, length = len;
	unsigned		total;
	u32			tag;
	char			*kbuf;
	size_t tmp;
	int i;

	if (dev->state != STATE_OPENED)
		return -EEXIST;

	if (len < (USB_DT_CONFIG_SIZE + USB_DT_DEVICE_SIZE + 4))
		return -EINVAL;

	/* we might need to change message format someday */
	if (copy_from_user (&tag, buf, 4))
		return -EFAULT;
	if (tag != 0)
		return -EINVAL;
	buf += 4;
	length -= 4;

	kbuf = kmalloc (length, GFP_KERNEL);
	if (!kbuf)
		return -ENOMEM;
	if (copy_from_user (kbuf, buf, length)) {
		kfree (kbuf);
		return -EFAULT;
	}

	spin_lock_irq (&dev->lock);
	value = -EINVAL;
	if (dev->buf)
		goto fail;
	dev->buf = kbuf;

	/* full or low speed config */
	dev->config = (void *) kbuf;
	total = le16_to_cpu(dev->config->wTotalLength);
	// total = le16_to_cpup (&dev->config->wTotalLength);
	if (!is_valid_config (dev->config) || total >= length)
		goto fail;
	kbuf += total;
	length -= total;

	/* optional high speed config */
	if (kbuf [1] == USB_DT_CONFIG) {
		dev->hs_config = (void *) kbuf;
		// total = le16_to_cpup (&dev->hs_config->wTotalLength);
		total = le16_to_cpu (dev->hs_config->wTotalLength);
		if (!is_valid_config (dev->hs_config) || total >= length)
			goto fail;
		kbuf += total;
		length -= total;
	}

	/* could support multiple configs, using another encoding! */

	/* device descriptor (tweaked for paranoia) */
	if (length < USB_DT_DEVICE_SIZE)
		goto fail;
	dev->dev = (void *)kbuf;
	if (dev->dev->bLength != USB_DT_DEVICE_SIZE
			|| dev->dev->bDescriptorType != USB_DT_DEVICE
			|| dev->dev->bNumConfigurations != 1)
		goto fail;
	dev->dev->bNumConfigurations = 1;
	dev->dev->bcdUSB = __constant_cpu_to_le16 (0x0200);
	kbuf += dev->dev->bLength;
	length -= dev->dev->bLength;

	/* import the string table */
	for (i = 0; i < ARRAY_SIZE(stringtab) && length > 0; i++) {
		tmp = strlen(kbuf) + 1;
		if (tmp > length)
			goto fail;
		length -= tmp;
		stringtab[i].s = kbuf;
		kbuf += tmp;
	}

	/* triggers gadgetfs_bind(); then we can enumerate. */
	spin_unlock_irq (&dev->lock);
	value = usb_gadget_register_driver (&gadgetfs_driver);
	if (value != 0) {
		kfree (dev->buf);
		dev->buf = NULL;
	} else {
		/* at this point "good" hardware has for the first time
		 * let the USB the host see us.  alternatively, if users
		 * unplug/replug that will clear all the error state.
		 *
		 * note:  everything running before here was guaranteed
		 * to choke driver model style diagnostics.  from here
		 * on, they can work ... except in cleanup paths that
		 * kick in after the ep0 descriptor is closed.
		 */
		fd->f_op = &ep0_io_operations;
		value = len;
	}
	return value;

fail:
	spin_unlock_irq (&dev->lock);
	pr_debug ("%s: %s fail %Zd, %p\n", shortname, __FUNCTION__, value, dev);
	kfree (dev->buf);
	dev->buf = NULL;
	return value;
}

static int
dev_open (struct inode *inode, struct file *fd)
{
	struct dev_data		*dev = inode->i_private;
	int			value = -EBUSY;

	if (dev->state == STATE_DEV_DISABLED) {
		dev->ev_next = 0;
		dev->state = STATE_OPENED;
		fd->private_data = dev;
		get_dev (dev);
		value = 0;
	}
	return value;
}

static struct file_operations dev_init_operations = {
	.owner =	THIS_MODULE,
	.llseek =	no_llseek,

	.open =		dev_open,
	.write =	dev_config,
	.fasync =	ep0_fasync,
	.ioctl =	dev_ioctl,
	.release =	dev_release,
};

/*----------------------------------------------------------------------*/

/* FILESYSTEM AND SUPERBLOCK OPERATIONS
 *
 * Mounting the filesystem creates a controller file, used first for
 * device configuration then later for event monitoring.
 */


/* FIXME PAM etc could set this security policy without mount options
 * if epfiles inherited ownership and permissons from ep0 ...
 */

static unsigned default_uid = 0644;
static unsigned default_gid = 0644;
static unsigned default_perm = S_IRUSR | S_IWUSR;

// module_param (default_uid, uint, 0644);
// module_param (default_gid, uint, 0644);
// module_param (default_perm, uint, 0644);


static struct inode *
gadgetfs_make_inode (struct super_block *sb,
		void *data, const struct file_operations *fops,
		int mode)
{
	struct inode *inode = new_inode (sb);

	if (inode) {
		inode->i_mode = mode;
		inode->i_uid = default_uid;
		inode->i_gid = default_gid;
		inode->i_blocks = 0;
		inode->i_atime = inode->i_mtime = inode->i_ctime
				= CURRENT_TIME;
		inode->i_private = data;
		inode->i_fop = fops;
	}
	return inode;
}

/* creates in fs root directory, so non-renamable and non-linkable.
 * so inode and dentry are paired, until device reconfig.
 */
static struct inode *
gadgetfs_create_file (struct super_block *sb, char const *name,
		void *data, const struct file_operations *fops,
		struct dentry **dentry_p)
{
	struct dentry	*dentry;
	struct inode	*inode;

	dentry = d_alloc_name(sb->s_root, name);
	if (!dentry)
		return NULL;

	inode = gadgetfs_make_inode (sb, data, fops,
			S_IFREG | (default_perm & S_IRWXUGO));
	if (!inode) {
		dput(dentry);
		return NULL;
	}
	d_add (dentry, inode);
	*dentry_p = dentry;
	return inode;
}

// See website on virtual file systems
static struct super_operations gadget_fs_operations = {
	.statfs		= simple_statfs,
	.drop_inode	= generic_delete_inode,
};

static int
gadgetfs_fill_super (struct super_block *sb, void *opts, int silent)
{
	struct inode	*inode;
	struct dentry	*d;
	struct dev_data	*dev;

	if (the_device)
		return -ESRCH;

	/* fake probe to determine $CHIP */
	(void) usb_gadget_register_driver (&probe_driver);
	if (!CHIP)
		return -ENODEV;

	/* superblock */
	sb->s_blocksize = PAGE_CACHE_SIZE;
	sb->s_blocksize_bits = PAGE_CACHE_SHIFT;
	sb->s_magic = GADGETFS_MAGIC;
	sb->s_op = &gadget_fs_operations;
	sb->s_time_gran = 1;

	/* root inode */
	inode = gadgetfs_make_inode (sb,
			NULL, &simple_dir_operations,
			S_IFDIR | S_IRUGO | S_IXUGO);
	if (!inode)
		goto enomem0;
	inode->i_op = &simple_dir_inode_operations;
	if (!(d = d_alloc_root (inode)))
		goto enomem1;
	sb->s_root = d;

	/* the ep0 file is named after the controller we expect;
	 * user mode code can use it for sanity checks, like we do.
	 */
	dev = dev_new ();
	if (!dev)
		goto enomem2;

	dev->sb = sb;
	if (!gadgetfs_create_file (sb, CHIP,
				dev, &dev_init_operations,
				&dev->dentry))
		goto enomem3;

	/* other endpoint files are available after hardware setup,
	 * from binding to a controller.
	 */
	the_device = dev;
	return 0;

enomem3:
	put_dev (dev);
enomem2:
	dput (d);
enomem1:
	iput (inode);
enomem0:
	return -ENOMEM;
}

/* "mount -t gadgetfs path /dev/gadget" ends up here */
static int
gadgetfs_get_sb (struct file_system_type *t, int flags,
		 const char *path, void *opts, struct vfsmount *mnt)
{
	return get_sb_single (t, flags, opts, gadgetfs_fill_super, mnt);
}

static void
gadgetfs_kill_sb (struct super_block *sb)
{
	kill_litter_super (sb);
	if (the_device) {
		put_dev (the_device);
		the_device = NULL;
	}
}

/*----------------------------------------------------------------------*/

static struct file_system_type gadgetfs_type = {
	.owner		= THIS_MODULE,
	.name		= shortname,
	.get_sb		= gadgetfs_get_sb,
	.kill_sb	= gadgetfs_kill_sb,
};

/*----------------------------------------------------------------------*/

static int __init init (void)
{
	int status;

	status = register_filesystem (&gadgetfs_type);
	if (status == 0)
		pr_info ("%s: %s, version " DRIVER_VERSION "\n",
			shortname, driver_desc);
	return status;
}
module_init (init);

static void __exit cleanup (void)
{
	pr_debug ("unregister %s\n", shortname);
	unregister_filesystem (&gadgetfs_type);
}
module_exit (cleanup);

