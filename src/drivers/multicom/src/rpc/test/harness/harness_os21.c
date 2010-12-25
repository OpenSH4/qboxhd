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

static char *transportName = "shm";

void harness_boot(void)
{
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

#ifdef __STRPC__
#ifdef ENABLE_RPC_SERVER
	rpcServerInit();
#else
	EMBX(Init());
	rpcStubsInit(NULL);
#endif
#else
	EMBX(Init());
#endif
	errno = 0;

	printf("******** "MACHINE" booted OS21 %s ********\n", kernel_version());
}

char *harness_getTransportName(void)
{
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

#elif defined BOARD_MB519

#define INTERRUPT_MBOX_SH	OS21_INTERRUPT_MBOX
#define INTERRUPT_MBOX_LX	OS21_INTERRUPT_MBOX_SH4_AUD0

#define MAILBOX0_ADDR		(void *) 0xfd800000	/* Audio0 LX mailbox */
#define MAILBOX1_ADDR		(void *) 0xfd801000	/* Video0 LX mailbox */

#elif defined BOARD_MB618

#define INTERRUPT_MBOX_SH	OS21_INTERRUPT_MBOX_SH4
#define INTERRUPT_MBOX_LX	OS21_INTERRUPT_MB_LX_AUDIO

#define MAILBOX0_ADDR		(void *) 0xfe212000	/* Audio LX mailbox */
#define MAILBOX1_ADDR		(void *) 0xfe211000	/* Video LX mailbox */

#endif

extern interrupt_name_t INTERRUPT_MBOX_SH;
extern interrupt_name_t INTERRUPT_MBOX_LX;

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
		16,			/* maxObjects */
		16,			/* freeListSize */
		(void *) 0x00000000,	/* sharedAddr */
		2 * 1024 * 1024,	/* sharedSize */
		NULL, 0,		/* No Primary Warp */
		NULL, 0,		/* No Secondary Warp */
	};

	EMBX(Mailbox_Init());
	EMBX(Mailbox_Register(MAILBOX0_ADDR, INTERRUPT_MBOX_LX, -1,
                              EMBX_MAILBOX_FLAGS_SET2 | EMBX_MAILBOX_FLAGS_LOCKS));
	EMBX(Mailbox_Register(MAILBOX1_ADDR, -1, -1, EMBX_MAILBOX_FLAGS_LOCKS));

	EMBX(RegisterTransport(EMBXSHM_mailbox_factory,
	                       &config, sizeof(config), &hFactory));
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

	EMBX(Mailbox_Init());
	EMBX(Mailbox_Register(MAILBOX1_ADDR, -1, -1, EMBX_MAILBOX_FLAGS_LOCKS));
	EMBX(Mailbox_Register(MAILBOX0_ADDR, INTERRUPT_MBOX_SH, -1,
	                      EMBX_MAILBOX_FLAGS_SET1 | EMBX_MAILBOX_FLAGS_LOCKS));

	EMBX(RegisterTransport(EMBXSHM_mailbox_factory,
	                       &config, sizeof(config), &hFactory));
}
#endif /* __ST231__ */


#else /* __OS21__ */

extern void warning_suppression(void);

#endif /* __OS21__ */
