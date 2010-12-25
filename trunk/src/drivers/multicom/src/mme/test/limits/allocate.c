#define DEBUG_NOTIFY_STR "  " DEBUG_FUNC_STR ": "
#define DEBUG_ERROR_STR  "**" DEBUG_FUNC_STR ": "

#ifdef DEBUG

#ifdef __KERNEL__
#define MME_Info(_y) printk _y;
#else
#define MME_Info(_y) printf _y;
#endif /* __KERNEL__ */

#else
#define MME_Info(_y)
#endif

/* Need to coordinate numbers - Nick Haydock reckons he starts at 240 */
#define ALLOCATE_MAJOR_NUM 241
#define ALLOCATE_DEV_NAME  "mmetest"

#define ALLOCATE_IOCTL_MAGIC 'k'

typedef struct AllocBuf {
	unsigned long         offset;
	void*                 allocAddr;
	unsigned              mappedAddr;
	unsigned              size;
	unsigned              cached;
	struct AllocBuf*      next;
} AllocBuf_t;

typedef struct InstanceInfo_s {
	AllocBuf_t*      allocatedBufs;
} InstanceInfo_t;


typedef struct Allocate_s {
	/* IN */
	unsigned size;
	unsigned cached;
        /* OUT */
	unsigned mapSize;
	unsigned offset;
} Allocate_t;

typedef enum IoCtlCmds_e {
        Allocate_c = 1,
        Free_c
} IoCtlCmds_t;

#define ALLOCATE_IOX_ALLOCATE _IOWR(ALLOCATE_IOCTL_MAGIC, Allocate_c, Allocate_t)
#define ALLOCATE_IOX_FREE     _IOWR(ALLOCATE_IOCTL_MAGIC, Free_c,     int)

#if defined __LINUX__ && defined __KERNEL__

#include <asm/uaccess.h>

#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/module.h>
#include <linux/highmem.h>
#include <linux/mm.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 23)
#include <linux/bigphysarea.h>
#else
#include <linux/bpa2.h>
#endif
#include <linux/version.h>

#include <linux/device.h>
#include <linux/cdev.h>

#include "embx_osinterface.h"

/* Kernel side driver */

MODULE_DESCRIPTION("MME Test Allocate Module");
MODULE_AUTHOR("STMicroelectronics Ltd");
/* MODULE_LICENSE("Copyright 2004 STMicroelectronics, All rights reserved"); */
MODULE_LICENSE("GPL");

static int major = ALLOCATE_MAJOR_NUM;	/* Can set to 0 for dynamic */
module_param     (major, int, 0);
MODULE_PARM_DESC (major, "MMETEST Major device number");

static int Allocate(struct file* filp, unsigned long arg);
static int Free    (struct file* filp, unsigned long arg);

static int allocate_open   (struct inode* inode, struct file* filp);
static int allocate_release(struct inode* inode, struct file* filp);
static int allocate_ioctl  (struct inode* inode, struct file* filp, unsigned int command, unsigned long arg);
static int allocate_mmap   (struct file* filp, struct vm_area_struct* vma);

static struct file_operations allocate_ops = {
	open:    allocate_open,
	release: allocate_release,
	ioctl:   allocate_ioctl,
	mmap:    allocate_mmap,
	owner:   THIS_MODULE
};

static struct cdev mmetest_cdev;
static dev_t       mmetest_devid;
static struct class *mmetest_class;
static struct class_device *mmetest_class_dev;

/* ==========================================================================
 * 
 * Called by the kernel when the module is loaded
 *
 * ========================================================================== 
 */

#define DEBUG_FUNC_STR "init_module"
int init_module(void)
{
	int result;

  	MME_Info((DEBUG_NOTIFY_STR "Initializing /dev/%s - major num %d\n", 
                 ALLOCATE_DEV_NAME, major));

	if (major) {
		/* Static major number allocation */
		mmetest_devid = MKDEV(major, 0);
		result =  register_chrdev_region(mmetest_devid, 1, ALLOCATE_DEV_NAME);
	}
	else {
		/* Dynamic major number allocation */
		result = alloc_chrdev_region(&mmetest_devid, 0, 1, ALLOCATE_DEV_NAME);
	}
	
	if (result) {
		printk(KERN_ERR "mmetest: register_chrdev failed : %d\n", result);
		goto err_register;
	}
	
	cdev_init(&mmetest_cdev, &allocate_ops);
	mmetest_cdev.owner = THIS_MODULE;
	result = cdev_add(&mmetest_cdev, mmetest_devid, 1);

	if (result) {
		printk(KERN_ERR "mmetest: cdev_add failed : %d\n", result);
		goto err_cdev_add;
	}
	
	/* It appears we have to create a class in order for udev to work */
	mmetest_class = class_create(THIS_MODULE, ALLOCATE_DEV_NAME);
	if((result = IS_ERR(mmetest_class))) {
		printk(KERN_ERR "mmetest: class_create failed : %d\n", result);
		goto err_class_create;
	}
	
	/* class_device_create() causes the /dev/mmetest file to appear when using udev
	 * however it is a GPL only function.
	 */
	mmetest_class_dev = class_device_create(mmetest_class, NULL, mmetest_devid, NULL, ALLOCATE_DEV_NAME);
	if((result = IS_ERR(mmetest_class_dev)))
	{
		printk(KERN_ERR "mmetest: class_device_create failed : %d\n", result);
		goto err_class_device_create;
	}
	
        MME_Info((DEBUG_NOTIFY_STR "Driver '%s' registered : major %d\n",
		  ALLOCATE_DEV_NAME, MAJOR(mmetest_devid)));
	
	return result;

err_class_device_create:
	class_destroy(mmetest_class);

err_class_create:
	cdev_del(&mmetest_cdev);
	
err_cdev_add:
	unregister_chrdev_region(mmetest_devid, 1);

err_register:

        MME_Info((DEBUG_NOTIFY_STR "Driver NOT registered : %d\n", result));

	return result;
}
#undef DEBUG_FUNC_STR

/* ==========================================================================
 *
 * Called by the kernel when the module is unloaded
 *
 * ========================================================================== 
 */
#define DEBUG_FUNC_STR "release_module"
void cleanup_module(void)
{
	MME_Info((DEBUG_NOTIFY_STR "unsregistering driver %s\n", ALLOCATE_DEV_NAME));

	unregister_chrdev(ALLOCATE_MAJOR_NUM, ALLOCATE_DEV_NAME);

	MME_Info((DEBUG_NOTIFY_STR "<<<<\n"));
}
#undef DEBUG_FUNC_STR

/* ==========================================================================
 *
 * Called by the kernel when the device is opened by an app (the mme user lib)
 *
 * ========================================================================== 
 */

#define DEBUG_FUNC_STR "allocate_open"
static int allocate_open(struct inode* inode, struct file* filp) 
{
	InstanceInfo_t* instanceInfo;

	MME_Info((DEBUG_NOTIFY_STR ">>>>\n"));

	instanceInfo = (InstanceInfo_t*) EMBX_OS_MemAlloc(sizeof(InstanceInfo_t));
	if (!instanceInfo) {
		goto nomem;
        }

	MME_Info((DEBUG_NOTIFY_STR "Instance 0x%08x\n", (int)instanceInfo));

	filp->private_data = instanceInfo;

	MME_Info((DEBUG_NOTIFY_STR "<<<< (0)\n"));
	return 0;
nomem:
	MME_Info((DEBUG_NOTIFY_STR "<<<< (-ENOTTY)\n"));
	return -ENOTTY;
}
#undef DEBUG_FUNC_STR 

/* ==========================================================================
 *
 * Called by the kernel when the device is closed by an app (the mme user lib)
 *
 * ========================================================================== 
 */
#define DEBUG_FUNC_STR "allocate_release"
static int allocate_release(struct inode* inode, struct file* filp) 
{
	EMBX_OS_MemFree(filp->private_data);

	filp->private_data = NULL;

	MME_Info((DEBUG_NOTIFY_STR "<<<< (0)\n"));
	return 0;
}
#undef DEBUG_FUNC_STR 

/* ==========================================================================
 * 
 * Called by the kernel when an ioctl sys call is made
 *
 * ========================================================================== 
 */
#define DEBUG_FUNC_STR "allocate_ioctl"
static int allocate_ioctl(struct inode* inode, struct file* filp, unsigned int command, unsigned long arg)
{
	int magic  = _IOC_TYPE(command);
	int op     = _IOC_NR(command);
	int result = -ENOTTY;
	MME_Info((DEBUG_NOTIFY_STR ">>>> command 0x%08x, op 0x%08x\n", command, op));

	if (ALLOCATE_IOCTL_MAGIC != magic) {
		MME_Info((DEBUG_ERROR_STR "<<<< -ENOTTY - command 0x%08x\n", command));
		return -ENOTTY;
	}

	switch (op) {
	case Allocate_c:
		MME_Info((DEBUG_NOTIFY_STR "Command Allocate\n"));
		result = Allocate(filp, arg);
		break;
	case Free_c:
		MME_Info((DEBUG_NOTIFY_STR "Command Free\n"));
		result = Free(filp, arg);
		break;
	default:
		MME_Info((DEBUG_ERROR_STR "Invalid ioctl command\n"));
		result =  -ENOTTY;
		break;
	}

	MME_Info((DEBUG_NOTIFY_STR "<<<< (%d)\n", result));
	return result;
}
#undef DEBUG_FUNC_STR 

/* ==========================================================================
 * 
 * allocate_mmap()
 * Called via mmap sys call from the mme user library
 *
 * ========================================================================== 
 */

#define DEBUG_FUNC_STR "allocate_mmap"
static int allocate_mmap(struct file* filp, struct vm_area_struct* vma)
{
	InstanceInfo_t*            instanceInfo = (InstanceInfo_t*)filp->private_data;
	int                        result;
	unsigned long              size         = vma->vm_end-vma->vm_start;
	unsigned long              physical     = vma->vm_pgoff * PAGE_SIZE;

	AllocBuf_t*                allocBuf;

	/* Lookup the supplied address in the allocated buffer list
	 * This has two effects;
	 *   a) It means we can stop the user from arbritarily mapping any bit of physical memory
	 *   b) We can find out the cached flag in order to create the correct mapping
	 */
	for (allocBuf = instanceInfo->allocatedBufs; allocBuf; allocBuf = allocBuf->next)
	{
		if (allocBuf->offset == physical && allocBuf->size == size)
			/* Found it! */
			break;
	}
	
	if (allocBuf == NULL)
	{
		result = -EINVAL;
		goto exit;
	}

	MME_Info((DEBUG_NOTIFY_STR
		  "ALLOC MMAP phys %x allocBuf %p vma_start %p cached %s\n",
		  physical, allocBuf, vma->vm_start, allocBuf->cached ? "YES" : "NO"));

	/* Set the VM_IO flag to signify the buffer is mmapped
           from an allocated data buffer */
	vma->vm_flags |= VM_RESERVED | VM_IO;

#if defined __LINUX__ && defined __SH4__
        if (!allocBuf->cached) {
	        /* Mark the physical area as uncached */
	        vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
        }
        MME_Info((DEBUG_NOTIFY_STR "VM Prot 0x%08x", vma->vm_page_prot));
	MME_Info((DEBUG_NOTIFY_STR "physical adress 0x%08x\n", physical));
#endif

        if (remap_pfn_range(vma, vma->vm_start, physical>>PAGE_SHIFT, size, vma->vm_page_prot) < 0)
	{
 	        MME_Info((DEBUG_ERROR_STR 
                          "Failed remap_page_range - start 0x%08x, phys 0x%08x, size %d\n", 
                          (int)(vma->vm_start), (int)physical, (int)size));
		result = -EAGAIN;
		goto exit;
	}
	
	/* Stash the mmap() address so we can easily free this memory later */
	allocBuf->mappedAddr = vma->vm_start;

	MME_Info((DEBUG_NOTIFY_STR "Mapped virt 0x%08x len %d to phys 0x%08x\n", 
                 (int)vma->vm_start, (int)size, (int)physical));

	result = 0;

exit:
	MME_Info((DEBUG_NOTIFY_STR "<<<< (%d)\n", result));		
	return result;
}
#undef DEBUG_FUNC_STR 

#define DEBUG_FUNC_STR "Allocate"
static int Allocate(struct file* filp, unsigned long arg) {
	InstanceInfo_t* instanceInfo = (InstanceInfo_t*)filp->private_data;
	Allocate_t* allocateInfoPtr = (Allocate_t*)arg; 

	Allocate_t  allocateInfo;

	AllocBuf_t* allocBuf = NULL;

	if (copy_from_user(&allocateInfo, allocateInfoPtr, sizeof(Allocate_t) )) {
		return -EFAULT;
	}
	
        unsigned int pages = (0 == allocateInfo.size?
                     0:
                     ((allocateInfo.size  - 1) / PAGE_SIZE) + 1);

       	void *alignedAddr = bigphysarea_alloc_pages(pages, 1, GFP_KERNEL);
	
        MME_Info(("bigphysarea_alloc_pages 0x%08x, pages %d pages\n", (unsigned) alignedAddr, pages));

        if (!alignedAddr) {
            return -ENOMEM;
        }

#if defined(__SH4__)
#if 0
        /* ensure there are no cache entries covering this address */
        dma_cache_wback_inv(alignedAddr, allocateInfo.size);
#endif

	allocateInfo.mapSize = pages * PAGE_SIZE;

	/* MULTICOM_32BIT_SUPPORT: Pass back the real physical address */
	(void) EMBX_OS_VirtToPhys(alignedAddr, &allocateInfo.offset);

	if (copy_to_user(allocateInfoPtr, &allocateInfo, sizeof(Allocate_t) )) {
		bigphysarea_free_pages(alignedAddr);
		return -EFAULT;
	}

	allocBuf = (AllocBuf_t *) EMBX_OS_MemAlloc(sizeof(AllocBuf_t));
	if (!allocBuf) {
		bigphysarea_free_pages(alignedAddr);
		return -ENOMEM;
	}
	
	/* Stash the allocated buffer info and save it on a linked list */
	allocBuf->offset       = allocateInfo.offset;
	allocBuf->allocAddr    = alignedAddr;
	allocBuf->mappedAddr   = 0;
	allocBuf->size         = allocateInfo.mapSize;
	allocBuf->cached       = allocateInfo.cached;
	allocBuf->next         = instanceInfo->allocatedBufs;
	
	instanceInfo->allocatedBufs = allocBuf;

	return 0;
#endif
}

#undef DEBUG_FUNC_STR 
#define DEBUG_FUNC_STR "Free"
static int Free(struct file* filp, unsigned long address)
{
	InstanceInfo_t*   instanceInfo = (InstanceInfo_t*)filp->private_data;

	AllocBuf_t      **bufp;

	int               result = -EINVAL;

	/* Find the specified buffer on the allocated list */
	for (bufp = &instanceInfo->allocatedBufs; *bufp; bufp = &(*bufp)->next)
	{
		AllocBuf_t *buf = *bufp;

		if (buf->mappedAddr == address)
		{
			/* Found a match so remove it from the list */
			(*bufp) = buf->next;
			
			/* Free off the allocBuf container */
			EMBX_OS_MemFree(buf);

			bigphysarea_free_pages(buf->allocAddr);
			
			result = 0;
			break;
		}
	}
	
	return result;
}

#elif defined __LINUX__

/* User library */

/* For mmap() */
#include <unistd.h>
#include <sys/mman.h>
/* For open() */
#include <sys/types.h>
#include <sys/stat.h>

/* For ioctl() */
#include <sys/ioctl.h>

#include <fcntl.h>


#include <stdio.h>

static int fileDescriptor = -1;

#undef  DEBUG_FUNC_STR
#define DEBUG_FUNC_STR "KernelAllocate"

void* KernelAllocate(unsigned size, unsigned cached, unsigned* mapSize) {
	int result;
	void* address = NULL;
	Allocate_t allocInfo;

	allocInfo.size = size;
	allocInfo.cached = cached;
  
	result = ioctl(fileDescriptor, ALLOCATE_IOX_ALLOCATE, &allocInfo);
	MME_Info((DEBUG_ERROR_STR "Alloc result %d\n", result));

	if (result >= 0) {
		*mapSize = allocInfo.mapSize;
		address = mmap(0, 
		               allocInfo.mapSize, 
		               PROT_READ | PROT_WRITE, MAP_SHARED,
			       fileDescriptor, 
			       allocInfo.offset);
		MME_Info((DEBUG_ERROR_STR "mmap address 0x%08x\n", address));
	        if (MAP_FAILED == address) {
	        	address = NULL;
	        }
	}
	MME_Info((DEBUG_ERROR_STR "<<<< address 0x%08x\n", address));
	return address;
}

#undef  DEBUG_FUNC_STR
#define DEBUG_FUNC_STR "KernelFree"
int KernelFree(void* address, unsigned length) {
	int result;

	result = ioctl(fileDescriptor, ALLOCATE_IOX_FREE, address);
	if (result<0) {
		return 0;
        }
	MME_Info( (DEBUG_ERROR_STR "Free ioctl result %d\n", result));

	result = munmap(address, length);
	MME_Info( (DEBUG_ERROR_STR "munmap result %d\n", result));

	return (0 == result)?1:0;
}

#undef  DEBUG_FUNC_STR
#define DEBUG_FUNC_STR "KernelAllocateInit"
int KernelAllocateInit() {
	if (fileDescriptor>=0) {
		return 0;
	}
	fileDescriptor = open("/dev/" ALLOCATE_DEV_NAME, O_RDWR);
	MME_Info( (DEBUG_ERROR_STR "Open %d\n", fileDescriptor));
	return (fileDescriptor>=0)?1:0;  
}

#undef  DEBUG_FUNC_STR
#define DEBUG_FUNC_STR "KernelAllocateDeinit"
int KernelAllocateDeinit() {
	if (fileDescriptor<0) {
		return 0;
	}
	close(fileDescriptor);
	MME_Info( (DEBUG_ERROR_STR "Closed %d\n", fileDescriptor));
	fileDescriptor = -1;
	return 1;
}
#endif

/*
 * Local Variables:
 *  tab-width: 8
 *  c-indent-level: 8
 *  c-basic-offset: 8
 * End:
 */
