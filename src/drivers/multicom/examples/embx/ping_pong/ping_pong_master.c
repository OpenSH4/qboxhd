/*******************************************************************/
/* Copyright 2005 STMicroelectronics R&D Ltd. All rights reserved. */ 
/*                                                                 */
/* File: ping_pong_master.c                                        */
/*                                                                 */
/* Description:                                                    */ 
/*    ST40 master for ping pong example                            */
/*                                                                 */
/*******************************************************************/

/*
 * Standard OS21, ST40 support headers
 */
#include <os21.h>
#include <os21/st40.h>

#include <stdio.h>
#include <stdlib.h>

#include "embx.h"
#include "embxmailbox.h"

#if defined BOARD_MB379 || defined BOARD_MB392

#define INTERRUPT_MBOX_LX	OS21_INTERRUPT_MBOX1_0

#define MAILBOX_ADDR		(void *) 0xb0200000	/* Audio LX mailbox */

#define MAILBOX_FLAGS		EMBX_MAILBOX_FLAGS_SET1

#elif defined BOARD_MB411 || defined BOARD_MB442

#define INTERRUPT_MBOX_LX	OS21_INTERRUPT_MB_LX_AUDIO

#define MAILBOX_ADDR		(void *) 0xb9212000	/* Audio LX mailbox */

#define MAILBOX_FLAGS		EMBX_MAILBOX_FLAGS_SET2

#elif defined BOARD_MB618

#define INTERRUPT_MBOX_LX	OS21_INTERRUPT_MB_LX_AUDIO

#define MAILBOX_ADDR		(void *) 0xfe212000	/* Audio LX mailbox */

#define MAILBOX_FLAGS		EMBX_MAILBOX_FLAGS_SET2

#elif defined BOARD_MB519

#define INTERRUPT_MBOX_LX	OS21_INTERRUPT_MBOX_SH4_AUD0

#define MAILBOX_ADDR		(void *) 0xfd800000	/* Audio0 LX mailbox */

#define MAILBOX_FLAGS		EMBX_MAILBOX_FLAGS_SET2

#endif

extern interrupt_name_t INTERRUPT_MBOX_LX;

EMBX_Mailbox_t *mbox_local, *mbox_remote;

/*
 * Mailbox callback
 */
static void mbox_callback (void *arg)
{
  semaphore_t *signal = (semaphore_t *) arg;

  /* Clear an interrupt for bit 0 */
  EMBX_Mailbox_InterruptClear (mbox_local, 0);

  semaphore_signal (signal);
}


/*
 * Mailbox test
 */
int main (void)
{
  EMBX_ERROR err;
  semaphore_t *mbox_signal;
  int i;

  /* Initialise and start OS21. Also enable timeslicing */
  kernel_initialize (NULL);
  kernel_start ();
  kernel_timeslice (OS21_TRUE);

  printf ("ping_pong started\n");

  /* Create the mailbox signal */
  if ((mbox_signal = semaphore_create_fifo (0)) == NULL)
  {
    printf ("Failed to create mailbox signal\n");
    return 1;
  }

  /* Initialise the mailbox library on the master CPU */
  if (EMBX_Mailbox_Init () != EMBX_SUCCESS)
  {
    printf ("Failed to initialise mailbox library\n");
    return 1;
  }

  printf ("Mailbox library initialised\n");

  /*
   * Register a mailbox where the first register set will generate interrupts
   * on the master CPU. The mailbox is also registered by the slave CPU such
   * that the second register set will generate interrupts for the slave CPU.
   */
  err = EMBX_Mailbox_Register (MAILBOX_ADDR,
			       INTERRUPT_MBOX_LX,
			       0,
			       MAILBOX_FLAGS);

  if (err != EMBX_SUCCESS)
  {
    printf ("Failed to register mbox err = %d\n",err);
    return 1;
  }

  printf ("Mailbox successfully registered\n");

  /* Allocate a matched status/enable pair for the master CPU */
  err = EMBX_Mailbox_Alloc (mbox_callback, (void *) mbox_signal, &mbox_local);
  if (err != EMBX_SUCCESS)
  {
    printf ("Failed to allocate mbox err = %d\n", err);
    return 1;
  }

  printf ("Mailbox successfully allocated\n");

  /* Synchronise with remote mailbox */
  err = EMBX_Mailbox_Synchronize (mbox_local, 0xb0dd1e, &mbox_remote);
  if (err != EMBX_SUCCESS)
  {
    printf ("Failed to synchronize mbox err = %d\n",err);
    return 1;
  }

  printf ("Mailbox successfully synchronised\n");

  /* Enable interrupt generation for bit 0 */
  EMBX_Mailbox_InterruptEnable (mbox_local, 0);

  for (i = 0; i < 5; ++i)
  {
    printf ("Waiting on mailbox interrupt %d from slave CPU\n", i);

    semaphore_wait (mbox_signal);

    printf ("Received interrupt %d from slave CPU\n", i);
    printf ("Raising mailbox interrupt %d for slave CPU\n", i);

    /* Raise an interrupt for bit 0 on slave CPU */
    EMBX_Mailbox_InterruptRaise (mbox_remote, 0);
  }

  /* Disable the interrupt generation for bit 0 */
  EMBX_Mailbox_InterruptDisable (mbox_local, 0);

  /* Free the matched status/enable pair for the master CPU */
  EMBX_Mailbox_Free (mbox_local);

  /* Deregister the master CPU mailbox */
  EMBX_Mailbox_Deregister (MAILBOX_ADDR);

  /* Delete the mailbox signal */
  semaphore_delete (mbox_signal);

  printf ("ping_pong finished\n");

  return 0;
}
