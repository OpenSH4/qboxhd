/*
 * harness_os21.c
 *
 * Copyright (C) STMicroelectronics Limited 2002. All rights reserved.
 *
 * 
 */

#ifdef __OS21__

#include <assert.h>

#include <os21.h>

#if defined __SH4__
#include <os21/st40.h>
#endif

#if defined __ST200__
#include <os21/st200.h>
#endif

#include <embx.h>

#include "harness.h"

static void harness_initialize(void);

void harness_boot(void)
{
#if defined(__SH4__) && (OS21_VERSION_MAJOR >= 3)
	extern int _st40_vmem_enhanced_mode (void);
#endif

	/* cannot use printf before the kernel is initialized since this
	 * is routed through harness_printf which does a task_lock()
	 */
	fprintf(stdout, "******** Booting "MACHINE" ********\n");

	/* boot the kernel */
	kernel_initialize(NULL);
	kernel_start();
	kernel_timeslice(OS21_TRUE);

	/* initialize any board specific components and register transports */
	harness_initialize();

	EMBX(Init());

#if defined(__SH4__) && (OS21_VERSION_MAJOR >= 3)
	printf("****** "MACHINE" booted OS21 %s '%s' ******\n", kernel_version(),
	       _st40_vmem_enhanced_mode() ? "SE (32-bit)" : "29-bit");
#else
	printf("****** "MACHINE" booted OS21 %s ******\n", kernel_version());
#endif

}

char *harness_getTransportName(void)
{
	char *transportName = "shm";

	return transportName;
}

void harness_block(void)
{
	task_suspend(NULL);
}

unsigned int harness_getTicksPerSecond(void)
{
        return time_ticks_per_sec();
}

/* use the high priority timer since it has a finer resolution */
unsigned int harness_getTime(void)
{
	/* converts a 64-bit to 32-bit type but this is OK because
	 * the tests will not run for long enough for this to roll
	 * over
	 */
	return (unsigned int) time_now();
}

void harness_sleep(int seconds)
{
	osclock_t t;

	t = time_plus(time_now(), time_ticks_per_sec() * seconds);
	VERBOSE(printf(MACHINE "sleeping until %lld\n", t));
	task_delay_until(t);
}



#include <embxmailbox.h>

#include <embxshm.h>

#define SHARED_SIZE		(2 * 1024 * 1024)	/* Size of the Shared heap */
#define SHARED_ALIGN		(32)			/* Alignment of the Shared heap */

#if defined BOARD_MB411 || defined BOARD_MB442

#define INTERRUPT_MBOX_SH	OS21_INTERRUPT_MBOX_SH4
#define INTERRUPT_MBOX_LX	OS21_INTERRUPT_MB_LX_AUDIO

#if defined(__SH4__)
#define MAILBOX0_ADDR		(void *) 0xb9212000	/* Audio LX mailbox */
#define MAILBOX1_ADDR		(void *) 0xb9211000	/* Video LX mailbox */
#endif

#if defined(__ST231__)
#define MAILBOX0_ADDR		(void *) 0x19212000	/* Audio LX mailbox */
#define MAILBOX1_ADDR		(void *) 0x19211000	/* Video LX mailbox */
#endif

#ifdef MULTICOM_32BIT_SUPPORT
#define WARP_ADDR		(void *) 0x40000000	/* Must enclose heap */
#define WARP_SIZE		0x10000000		/* 256MB */
#define WARP_ADDR2		(void *) 0x60000000
#define WARP_SIZE2		0x10000000		/* 256MB */
#else
#define WARP_ADDR		(void *) 0xa4000000
#define WARP_SIZE		0x13000000
#define WARP_ADDR2		NULL			/* No secondary warp */
#define WARP_SIZE2		0			/* No secondary warp */
#endif

#elif defined BOARD_MB519

#define INTERRUPT_MBOX_SH	OS21_INTERRUPT_MBOX
#define INTERRUPT_MBOX_LX	OS21_INTERRUPT_MBOX_SH4_AUD0

#define MAILBOX0_ADDR		(void *) 0xfd800000	/* Audio0 LX mailbox */
#define MAILBOX1_ADDR		(void *) 0xfd801000	/* Video0 LX mailbox */

#ifdef MULTICOM_32BIT_SUPPORT
#define WARP_ADDR		(void *) 0x80000000	/* Must enclose heap */
#define WARP_SIZE		0x10000000	 	/* 256MB */ 
#define WARP_ADDR2		(void *) 0x40000000
#define WARP_SIZE2		0x10000000		/* 256MB */
#else
#define WARP_ADDR		(void *) 0xa8000000
#define WARP_SIZE		0x14000000
#define WARP_ADDR2		NULL			/* No secondary warp */
#define WARP_SIZE2		0			/* No secondary warp */
#endif

#elif defined BOARD_MB618

#define INTERRUPT_MBOX_SH	OS21_INTERRUPT_MBOX_SH4
#define INTERRUPT_MBOX_LX	OS21_INTERRUPT_MB_LX_AUDIO
#define MAILBOX0_ADDR		(void *) 0xfe212000	/* Audio LX mailbox */
#define MAILBOX1_ADDR		(void *) 0xfe211000	/* Video LX mailbox */

#ifdef MULTICOM_32BIT_SUPPORT
#define WARP_ADDR		(void *) 0x40000000	/* Must enclose heap */
#define WARP_SIZE		0x10000000		/* 256MB */
#define WARP_ADDR2		NULL			/* No secondary warp */
#define WARP_SIZE2		0			/* No secondary warp */
#else
#define WARP_ADDR		(void *) 0x0c000000
#define WARP_SIZE		0x10000000
#define WARP_ADDR2		NULL			/* No secondary warp */
#define WARP_SIZE2		0			/* No secondary warp */
#endif

#endif /* BOARD_MB411 || BOARD_MB442 */

extern interrupt_name_t INTERRUPT_MBOX_SH;
extern interrupt_name_t INTERRUPT_MBOX_LX;

/* Switch the factory init function for Cached and Uncached operation */
#if defined ENABLE_EMBXSHMC
#define MAILBOX_FACTORY	EMBXSHMC_mailbox_factory;
#else
#define MAILBOX_FACTORY	EMBXSHM_mailbox_factory;
#endif /* ENABLE_EMBXSHMC */

#if defined __SH4__
static void harness_initialize(void)
{
	EMBX_FACTORY hFactory;

	EMBXSHM_MailboxConfig_t config = { 
		"shm",			/* name */
		0,			/* cpuID */
		{ 1, 1, 0, 0, 0, 0, 0, 0 },		/* participants */
#if defined ENABLE_EMBXSHMC
		0x80000000,		/* pointerWarp */
#else
		0x60000000,		/* pointerWarp */
#endif
		0,			/* maxPorts */
		64,			/* maxObjects */
		64,			/* freeListSize */
		(void *) 0x00000000,	/* sharedAddr */
		SHARED_SIZE,		/* sharedSize */

		/* INSbl24123: we need to have a warp range set up for some
		 *             processor that has an ST231 companion in order
		 *             to guard against regression
                 */
		WARP_ADDR,		/* warpRangeAddr */
		WARP_SIZE,		/* warpRangeSize */
		WARP_ADDR2,		/* warpRangeAddr2 */
		WARP_SIZE2		/* warpRangeSize2 */
	};
	EMBX_TransportFactory_fn *factory = MAILBOX_FACTORY;

	EMBX(Mailbox_Init());
	EMBX(Mailbox_Register(MAILBOX0_ADDR, INTERRUPT_MBOX_LX, -1,
                              EMBX_MAILBOX_FLAGS_SET2 | EMBX_MAILBOX_FLAGS_LOCKS));
	EMBX(Mailbox_Register(MAILBOX1_ADDR, -1, -1, EMBX_MAILBOX_FLAGS_LOCKS));

#ifdef MULTICOM_32BIT_SUPPORT
	/* Bugzilla 3666: Attempt to allocate our own shared heap */
	config.sharedAddr = memalign(SHARED_ALIGN, SHARED_SIZE);
	if (config.sharedAddr)
	{
		unsigned paddr;
		unsigned mode = VMEM_CREATE_READ|VMEM_CREATE_WRITE;

#if defined ENABLE_EMBXSHMC
		mode |= VMEM_CREATE_CACHED;
#else
		mode |= VMEM_CREATE_UNCACHED | VMEM_CREATE_NO_WRITE_BUFFER;
#endif

		vmem_virt_to_phys(config.sharedAddr, &paddr);
		config.sharedAddr = vmem_create(paddr, SHARED_SIZE, NULL, mode);
	}
#endif

	EMBX(RegisterTransport(factory, &config, sizeof(config), &hFactory));
}
#endif /* __SH4__ */
#if defined __ST231__
static void harness_initialize(void)
{
	EMBX_FACTORY hFactory;
	EMBXSHM_MailboxConfig_t config = { 
		{ "shm" },		/* name */
		1,			/* cpuID */
		{ 1, 1, 0, 0, 0, 0, 0, 0 },		/* participants */
		0 			/* pointerWarp */
	};
	EMBX_TransportFactory_fn *factory = MAILBOX_FACTORY;

	EMBX(Mailbox_Init());
	EMBX(Mailbox_Register(MAILBOX1_ADDR, -1, -1, EMBX_MAILBOX_FLAGS_LOCKS));
	EMBX(Mailbox_Register(MAILBOX0_ADDR, INTERRUPT_MBOX_SH, -1,
	                      EMBX_MAILBOX_FLAGS_SET1 | EMBX_MAILBOX_FLAGS_LOCKS));

	EMBX(RegisterTransport(factory,
	                       &config, sizeof(config), &hFactory));
}
#endif /* __ST231__ */

#ifndef HAS_DEINIT
void harness_deinit(void) {}
#endif

#else /* __OS21__ */

extern void warning_suppression(void);

#endif /* __OS21__ */

/*
 * Local Variables:
 *  tab-width: 8
 *  c-indent-level: 8
 *  c-basic-offset: 8
 * End:
 */
