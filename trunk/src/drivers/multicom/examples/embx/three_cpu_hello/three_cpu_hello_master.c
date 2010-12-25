/*******************************************************************/ 
/* Copyright 2003 STMicroelectronics R&D Ltd. All rights reserved. */ 
/*                                                                 */ 
/* File: three_cpu_hello_master.c                                  */
/*                                                                 */ 
/* Description:                                                    */ 
/*    ST40 transport master for three cpu hello example            */
/*                                                                 */
/*******************************************************************/ 

/*
 * Standard OS21 and ST40 board support headers
 */
#include <os21.h>
#include <os21/st40.h>

#include <stdio.h>
#include <stdlib.h>

#undef __STRICT_ANSI__   /* To get strdup in string.h */
#include <string.h>

/*
 * Include the EMBX API, shared memory transport and 
 * hardware mailbox headers.
 */
#include "embx.h"
#include "embxshm.h"
#include "embxmailbox.h"

#if defined BOARD_MB411 || defined BOARD_MB442

/* Use the Audio LX mailbox */
#define INTERRUPT_MBOX_LX	OS21_INTERRUPT_MB_LX_AUDIO

#define MAILBOX_AUDIO		(void *) 0xb9212000	/* Audio LX mailbox */
#define MAILBOX_VIDEO		(void *) 0xb9211000	/* Video LX mailbox */

#elif defined BOARD_MB618

/* Use the Audio LX mailbox */
#define INTERRUPT_MBOX_LX	OS21_INTERRUPT_MB_LX_AUDIO

#define MAILBOX_AUDIO		(void *) 0xfe212000	/* Audio LX mailbox */
#define MAILBOX_VIDEO		(void *) 0xfe211000	/* Video LX mailbox */

#elif defined BOARD_MB519

/* Use the Audio LX mailbox */
#define INTERRUPT_MBOX_LX	OS21_INTERRUPT_MBOX_SH4_AUD0

#define MAILBOX_AUDIO		(void *) 0xfd800000	/* Audio0 LX mailbox */
#define MAILBOX_VIDEO		(void *) 0xfd801000	/* Video0 LX mailbox */

#else

#error "Example only supports PLATFORM=[mb411|mb442|mb519|mb618]"

#endif

extern interrupt_name_t INTERRUPT_MBOX_LX;

/*
 * Shared memory transport factory configuration structure
 */
EMBXSHM_MailboxConfig_t config = {
	"shm",                   	/* Local transport name                      */
	0,                       	/* ST40/master is always CPU 0               */
	{ 1, 1, 1, 0, 0, 0, 0, 0 },	/* CPUs 0,1,2 will be participating          */
	0x60000000,              	/* Standard pointer warp for ST40 P2 address */
	0,			 	/* maxPorts (0 = dynamic)                    */
	16,			 	/* maxObjects                                */
	16,			 	/* freeListSize                              */
	NULL,				/* sharedAddr (NULL = dynamic allocation)    */
	4 * 1024 * 1024,		/* sharedSize : Heap size = 4MB              */
	NULL, 0,			/* No PRIMARY Warp Range                     */
	NULL, 0			 	/* No SECONDARY Warp Range                   */
};


int init_mailboxes();
int init_comms(EMBX_FACTORY *,EMBX_TRANSPORT *,EMBX_PORT *);
int do_work(EMBX_TRANSPORT, EMBX_PORT);


int main()
{
EMBX_FACTORY hFact;
EMBX_TRANSPORT hTrans;
EMBX_PORT masterPort;
int status = 0;

    /*
     * Start OS21
     */
    kernel_initialize(NULL);
    kernel_start();
    kernel_timeslice(OS21_TRUE);

    printf("three_cpu_hello started\n");

    if(init_mailboxes() < 0)
    { 
        status = 1;
	goto exit;
    }

    if(init_comms(&hFact,&hTrans,&masterPort) < 0)
    {
        status = 1;
        goto exit;
    }

    if(do_work(hTrans, masterPort) < 0)
        status = 1;

exit:
    /*
     * Close everything down. In this case it doesn't matter
     * if it fails as there is nothing we can do anyway.
     */
    EMBX_ClosePort(masterPort);
    EMBX_CloseTransport(hTrans);
    EMBX_Deinit();
    EMBX_UnregisterTransport(hFact);

    return status;
}


int init_mailboxes(void)
{
    EMBX_ERROR err;
    
    if((err = EMBX_Mailbox_Init()) != EMBX_SUCCESS) {
	printf("Failed to intialise mailbox library err = %d\n",err);
	return -1;
    }

    /*
     * Register the Audio LX hardware mailbox as the
     * one the ST40 will be interrupted by. This will use the 
     * second half of the register set when the mailbox library allocates
     * mailbox registers to us. The interrupt level parameter is
     * ignored when using OS21.
     */
    err = EMBX_Mailbox_Register(MAILBOX_AUDIO,
				INTERRUPT_MBOX_LX,
				0,
				EMBX_MAILBOX_FLAGS_SET2);
    
    if(err != EMBX_SUCCESS) {
	printf("Failed to register mbox 0 err = %d\n",err);
	return -1;
    }

    /*
     * Register the Video LX as the second hardware mailbox. This
     * will be used to signal the other CPU
     */
    if((err = EMBX_Mailbox_Register(MAILBOX_VIDEO, -1, -1, 0)) != EMBX_SUCCESS) {
	printf("Failed to register mbox 1 err = %d\n",err);
	return -1;
    }

    printf("Initialized mailbox library\n");

    return 0;
}


int init_comms(EMBX_FACTORY *pFact,EMBX_TRANSPORT *pTrans,EMBX_PORT *pPort)
{
EMBX_ERROR err;
EMBX_TPINFO tpinfo;


    *pFact  = 0;
    *pTrans = 0;
    *pPort  = 0;

    /*
     * Register the transport factory for the shared memory
     * transport configuration we require. 
     */
    err = EMBX_RegisterTransport(EMBXSHM_mailbox_factory,
                                 &config, sizeof(config), pFact);

    if(err != EMBX_SUCCESS)
    {
        printf("Factory registration failed err = %d\n",err);
        return -1;
    }

    /*
     * Initialise the EMBX library, this will attempt to create
     * a transport based on the factory we just registered.
     */
    if((err = EMBX_Init()) != EMBX_SUCCESS)
    {
        printf("EMBX initialisation failed err = %d\n",err);
        return -1;
    }

    /*
     * Try and get the information structure for the transport
     * we hope has just been created. If this returns an error
     * no transports were created during EMBX_Init, which means
     * something went wrong in the transport factory we registered.
     */
    if((err = EMBX_GetFirstTransport(&tpinfo)) != EMBX_SUCCESS)
    {
        printf("Querying transport failed err = %d\n",err);
        return -1;
    }

    /*
     * Open a transport handle for the transport we just queried.
     * As we only registered a single transport we know this is
     * the one we want to use.
     */

    printf("Opening transport, waiting for ST200s to join in\n");

    if((err = EMBX_OpenTransport(tpinfo.name, pTrans)) != EMBX_SUCCESS)
    {
        printf("Failed to open transport '%s' err = %d\n",tpinfo.name,err);
        return -1;
    }

    printf("Successfully registered and opened transport '%s'\n",tpinfo.name);


    /*
     * Create a port with a well known name to which the other CPUs
     * will connect to.
     */
    if((err = EMBX_CreatePort(*pTrans,"master",pPort)) != EMBX_SUCCESS)
    {
        printf("Failed to create master port err = %d\n",err);
        return -1;
    }

    printf("Successfully created master port\n");

    return 0;
}


int do_work(EMBX_TRANSPORT hTrans, EMBX_PORT masterPort)
{
EMBX_ERROR err;

    while(1)
    {
    EMBX_PORT remotePort;
    EMBX_RECEIVE_EVENT ev;
    char *cpuname = 0;

        /*
         * Wait for either of the other two CPUs to send us a message
         */
        if((err = EMBX_ReceiveBlock(masterPort, &ev)) != EMBX_SUCCESS)
        {
            if(err == EMBX_PORT_INVALIDATED)
            {
                /*
                 * When one of the other CPUs calls EMBX_Deinit the
                 * transport and hence all the ports also become invalid.
                 */
                printf("Master port is no longer valid.\n");
                return 0; 
            }
            else
            {
                printf("Failed to receive on master port, err = %d\n",err);
                return -1;
            }
        }

        /*
         * We are expecting a string containing the name of the senders
         * reply port.
         */
        cpuname = strdup(ev.data);

        printf("%s said hello\n",cpuname);

        /*
         * Try to connect to the sender, reply to it and then
         * disconnect. We expect the sender to already have created
         * the reply port.
         */
        if((err = EMBX_Connect(hTrans, cpuname, &remotePort)) != EMBX_SUCCESS)
        {
            /* We have the responsibility to free the message we just
             * recieved.
             */
            EMBX_Free(ev.data);
            free(cpuname);
            printf("Failed to connect to remote port err = %d\n",err);
            return -1;
        }

        /*
         * The reply is just resending the message we got in the first place.
         * Once we have done this the message buffer is no longer available
         * for use by us and we no longer have responsibility for freeing it.
         */
        if((err = EMBX_SendMessage(remotePort, ev.data, ev.size)) != EMBX_SUCCESS)
        {
            EMBX_ClosePort(remotePort);
            EMBX_Free(ev.data);
            free(cpuname);
            printf("Failed to send reply err = %d\n",err);
            return -1;
        }

        if((err = EMBX_ClosePort(remotePort)) != EMBX_SUCCESS)
        {
            free(cpuname);
            printf("Failed to close remote connection err = %d\n",err);
            return -1;
        }

        /*
         * We cannot use ev.data here because that message buffer
         * has been sent to another CPU and is no longer available to
         * us, that is why we made a copy of the contents.
         */
        printf("Finished handling %s\n",cpuname);

        free(cpuname);
    }
}

/*
 * Local Variables:
 *  tab-width: 8
 *  c-indent-level: 4
 *  c-basic-offset: 4
 * End:
 */
