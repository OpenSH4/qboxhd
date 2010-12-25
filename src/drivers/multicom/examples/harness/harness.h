/*
 * harness.h
 *
 * Copyright (C) STMicroelectronics Limited 2003. All rights reserved.
 *
 * Runtime support files
 */

#ifndef HARNESS_H
#define HARNESS_H

#if defined(__STRPC__)
#define ENABLE_RPC 1
#endif

#include <embx.h>
#include <embxmailbox.h>
#include <embxshm.h>


#if defined(__OS21__)
#include "os_os21.h"
#elif defined(__LINUX__)
#include "os_linux.h"
#elif defined(__WINCE__)
#include "os_wince.h"
#else
#error Unknown operating system
#endif

#if defined(BOARD_MB411) || defined(BOARD_MB442)
#include "board_mb411.h"
#elif defined(BOARD_MB519)
#include "board_mb519.h"
#elif defined(BOARD_MB618)
#include "board_mb618.h"
#else
#error Unknown board/platform
#endif


static void examplesRuntimeInit(void)
{
	osRuntimeInit();

        embxPlatformInit();
}


#if !defined(ENABLE_RPC)

static void embxRuntimeInit(void)
{
	examplesRuntimeInit();
}

#else

extern int rpcStubsInit(void *);

static void rpcRuntimeInit(void)
{
	examplesRuntimeInit();

        printf("Waiting for other CPUS ..."); fflush(stdout);
        if (0 != rpcStubsInit(NULL)) {
                printf(" FAILED\n");
                exit(1);
        }
        printf(" done\n");
}

#endif /* ENABLE_RPC */


#endif /* RUNTIME_H */
