/*
 * companion.c
 *
 * Copyright (C) STMicroelectronics Limited 2004. All rights reserved.
 *
 * Remote component of the getstart example.
 */

#include <os21.h>
#include <os21/st200.h>

#include <assert.h>

#include <string.h>	/* strstr() */

#include <embx.h>
#include <embxshm.h>
#include <embxmailbox.h>
#include <mme.h>
#include "mixer.h"


#if defined BOARD_MB411 || defined BOARD_MB442

#define INTERRUPT_MBOX_SH	OS21_INTERRUPT_MBOX_SH4

#define MAILBOX_AUDIO		(void *) 0x19212000	/* Audio LX mailbox */
#define MAILBOX_VIDEO		(void *) 0x19211000	/* Video LX mailbox */

#elif defined BOARD_MB618

#define INTERRUPT_MBOX_SH	OS21_INTERRUPT_MBOX_SH4

#define MAILBOX_AUDIO		(void *) 0xfe212000	/* Audio LX mailbox */
#define MAILBOX_VIDEO		(void *) 0xfe211000	/* Video LX mailbox */

#elif defined BOARD_MB519

#define INTERRUPT_MBOX_SH	OS21_INTERRUPT_MBOX

#define MAILBOX_AUDIO		(void *) 0xfd800000	/* Audio0 LX mailbox */
#define MAILBOX_VIDEO		(void *) 0xfd801000	/* Video0 LX mailbox */

#else

#error "Example only supports PLATFORM=[mb411|mb442|mb519|mb618]"

#endif

extern interrupt_name_t INTERRUPT_MBOX_SH;

EMBXSHM_MailboxConfig_t config = {
   "shm",             		/* Local transport name                                */
   0,                 		/* CPU id, determined at runtime                       */
   { 1, 1, 1, 0, 0, 0, 0, 0 },	/* The participants map must be the same on all CPUs   */
   0x00000000         		/* No pointer warp necessary on companions */
};


char *cpu_names[] = { "st40", "audio", "video" };

#define CPU_NAME (cpu_names[config.cpuID])

int init_mailboxes(void)
{
	EMBX_ERROR err;

	if((err = EMBX_Mailbox_Init()) != EMBX_SUCCESS)	{
		printf("Failed to intialise mailbox library err = %d\n", err);
		return -1;
	}

	/*
	 * Register the local hardware mailbox on the SoC. The LX will own the
	 * first half of the register set and the ST40 will own the second half of the register set.
	 *
	 * The interrupt level parameteris ignored when using OS21.
	 */ 
	err = EMBX_Mailbox_Register((config.cpuID == 1 ? MAILBOX_AUDIO : MAILBOX_VIDEO),
				    INTERRUPT_MBOX_SH,
				    0,
				    EMBX_MAILBOX_FLAGS_SET1);

	if(err != EMBX_SUCCESS)	{
		printf("Failed to register mbox 1 err = %d\n",err);
		return -1;
	}

	/*
	 * Register the remote hardware mailbox on the SoC. This will be
	 * used to signal the other CPU
	 */
	if((err = EMBX_Mailbox_Register((config.cpuID == 1 ? MAILBOX_VIDEO : MAILBOX_AUDIO),
					-1, -1, 0)) != EMBX_SUCCESS) {
	  printf("Failed to register mbox 0 err = %d\n",err);
	  return -1;
	}
	
	printf("Initialized mailbox library\n");

	return 0;
}



int init_comms(EMBX_FACTORY *pFact, EMBX_TRANSPORT *pTrans, EMBX_PORT *pPort)
{
	EMBX_ERROR err;
	EMBX_TPINFO tpinfo;

	/*
	 * Register the transport factory for the shared memory
	 * transport configuration we require.
	 */
	err = EMBX_RegisterTransport(EMBXSHM_mailbox_factory,
				     &config, sizeof(config), pFact);

	if(err != EMBX_SUCCESS) {
		printf("Factory registration failed err = %d\n",err);
		return -1;
	}

	/*
	 * Initialise the EMBX library, this will attempt to create
	 * a transport based on the factory we just registered.
	 */
	if((err = EMBX_Init()) != EMBX_SUCCESS)	{
		printf("EMBX initialisation failed err = %d\n",err);
		return -1;
	}

	/*
	 * Try and get the information structure for the transport
	 * we hope has just been created. If this returns an error
	 * no transports were created during EMBX_Init, which means
	 * something went wrong in the transport factory we registered.
	 */
	if((err = EMBX_GetFirstTransport(&tpinfo)) != EMBX_SUCCESS) {
		printf("Transport Factory failed err = %d\n",err);
		return -1;
	}

	return 0;
}

static int setupEMBXAndOS() 
{
	EMBX_FACTORY   hFact;
	EMBX_TRANSPORT hTrans;
	EMBX_PORT      myPort;
	int status = 0;

	const char *cpu;
	
	/*
	 * Start OS21
	 */
	kernel_initialize(NULL);
	kernel_start();
	kernel_timeslice(OS21_TRUE);

	/* Query the CPU name from the BSP */
	cpu = kernel_cpu();

	/*
	 * Determine which ST200 [audio/video] we are running on
	 */
	if ((strstr(cpu, "audio") != NULL) || (strstr(cpu, "AUDIO") != NULL))
	{
	  printf("On the audio CPU\n");
	  config.cpuID = 1;	/* Audio LX is CPU #1 */
	}
	else if ((strstr(cpu, "video") != NULL) || (strstr(cpu, "VIDEO") != NULL))
	{
	  printf("On the video CPU\n");
	  config.cpuID = 2;	/* Video LX is CPU #2 */
	}
	else
	{
	  printf("ERROR: Cannot deteremine companion cpu\n");
	  status = 1;
	  goto exit;
	}

	if(init_mailboxes() < 0) {
		status = 1;
		goto exit;
	}

	if(init_comms(&hFact, &hTrans, &myPort) < 0) {
		status = 1;
		goto exit;
	}
 exit:
	return status;
}

#define TRANSFORMER_BASENAME "mme.8bit_mixer_"

int main(int argc, char* argv[])
{
	MME_ERROR err;
	char transformerName[MME_MAX_TRANSFORMER_NAME] = "mme.8bit_mixer_";

	/* initialize the underlying EMBX transport */
	setupEMBXAndOS();

	/* initialize the MME API */
	err = MME_Init();
	if (MME_SUCCESS != err) {
		printf("%s: ERROR: cannot initialize the MME API (%d)\n", CPU_NAME, err);
	}
	printf("%s: About to register transport shm\n", CPU_NAME);
	err = MME_RegisterTransport("shm");
	if (MME_SUCCESS != err) {
		printf("%s: ERROR: cannot register transport to external processors (%d)\n", CPU_NAME, err);
	}

	sprintf(transformerName, TRANSFORMER_BASENAME "%d", config.cpuID );

	printf("%s: About to register transformer %s\n", CPU_NAME, transformerName);

	err = Mixer_RegisterTransformer(transformerName);

	if (MME_SUCCESS != err) {
		printf("%s: ERROR: cannot register %s (%d)\n", CPU_NAME, transformerName, err);	}

        /* enter the main MME execution loop */
	err = MME_Run();
	if (MME_SUCCESS != err) {
		printf("%s: ERROR: MME execution loop reported an error (%d)\n", CPU_NAME, err);
	}

	err = MME_Term();
	if (MME_SUCCESS != err && MME_DRIVER_NOT_INITIALIZED != err) {
		printf("%s: ERROR: cannot terminate the MME API (%d)\n", CPU_NAME, err);
	}

	(void) EMBX_Deinit();

	return 0;
}
