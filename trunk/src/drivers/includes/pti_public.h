#ifndef pti_public_
#define pti_public_

/* public pti header */

#define NEW_DESCRAMBLER

#ifdef NEW_DESCRAMBLER

#if defined(UFS910) || defined(CONFIG_SH_QBOXHD_1_0) || defined(CONFIG_SH_QBOXHD_MINI_1_0)
	/*ufs910 has a memory problem causing artefacts while watching HDTV channels
	so we need to reduce the number of descramblers,
	maybe other boxes have the same problems*/
#define NUMBER_OF_DESCRAMBLERS 4
#else
	/*quack: 6 is the max for HDBOX maybe other values have to be chosen for other boxes*/
#define NUMBER_OF_DESCRAMBLERS 6
#endif

typedef struct DescrPid_
{
	short int pid;
	short int descrIdx;
}DescrPid;

typedef struct DescrMap_
{
	int       adapter;
	int       device;
	int       descrambler;
	short int descramblerIndx;
}DescrMap;

typedef enum
{
  VID_BUFFER = 222,
  AUD_BUFFER,
  MISC_BUFFER,
  OTHER_BUFFER
} BUFFER_TYPE;

struct PtiSession
{
	short int       pidtable[32];
	short int       descramblerForPid[8192];

	short int       references[32];

	short int       type[32];
	short int       pes_type[32];
	short int       num_pids;

	int             slots[32];

	int             session;
	int             descrambler;
	int             descramblers[NUMBER_OF_DESCRAMBLERS];
	int             descramblerindex[32];

	int             source;

	BUFFER_TYPE     buf_type[32];  //save current buffer's pool type
};
#else
struct PtiSession
{
  short int 		pidtable[32];
  short int 		references[32];

  short int 		type[32];
  short int 		pes_type[32];
  short int 		num_pids;

  int 			slots[32];

  int 			session;
  int 			descrambler;
  int 			source;

};

typedef enum
{
  VID_BUFFER = 222,
  AUD_BUFFER,
  MISC_BUFFER,
  OTHER_BUFFER
} BUFFER_TYPE;

#endif ///NEW_DESCRAMBLER

/* source */
typedef enum
{
  TSIN0 = 0,
  TSIN1,
  TSIN2,
  SWTS0,
  TS_NOTAGS = 0x80
} tInputSource;

struct stpti
{
  size_t 		InterruptDMABufferSize;
  dma_addr_t    	InterruptDMABufferInfo;
  void          	*InterruptBufferStart_p;

  /* spinlock for the interrupt handler */
  spinlock_t 		irq_lock;

  /* gemappter speicherbereich; TCDevice_t* */
  unsigned int 		pti_io;

  struct dvb_device*    ca_device;

  /* work queue for polling the DMA (if configured) */
  wait_queue_head_t queue;
};



#endif
