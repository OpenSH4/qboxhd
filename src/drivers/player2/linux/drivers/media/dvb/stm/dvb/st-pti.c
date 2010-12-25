
/*
 * ST-PTI DVB driver
 *
 * indent: indent -bl -bli0 -cdb -sc -bap -bad -pcs -prs -bls -lp -npsl -bbb st-pti.c
 */
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/module.h>
#include <linux/usb.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/errno.h>
#include <asm/semaphore.h>
#include <linux/platform_device.h>
#include <linux/mutex.h>

#include <asm/io.h>

#if defined (CONFIG_KERNELVERSION) /* ST Linux 2.3 */
#include <linux/bpa2.h>
#else
#include <linux/bigphysarea.h>
#endif

#include "dvb_frontend.h"
#include "dmxdev.h"
#include "dvb_demux.h"
#include "dvb_net.h"

#include "backend.h"


#include <linux/dvb/frontend.h>
#include <linux/dvb/dmx.h>
#include <linux/dvb/ca.h>

#include "st-merger.h"
#if defined(CONFIG_SH_QBOXHD_1_0) || defined(CONFIG_SH_QBOXHD_MINI_1_0)
#include "pti.h"
#include "pti_hal.h"
#include "nim_tuner.h"

#if defined(CONFIG_SH_QBOXHD_MINI_1_0)
#define DELAYER

#ifdef DELAYER
#include "delayer.h"
#endif ///DELAYER
#endif

#else
#include "st-common.h"
#endif

//__TDT__: many modifications in this file
#if defined(CONFIG_SH_QBOXHD_1_0) || defined(CONFIG_SH_QBOXHD_MINI_1_0)
	#define PTI_DEBUG
	#ifdef PTI_DEBUG
// 		#define dprintk(x...) do { printk("[ST-PTI] " x); } while(0)
// 		extern int debug;
		int stpti_debug=0;
		module_param(stpti_debug, int, 0644);
		MODULE_PARM_DESC(stpti_debug, "Modify ST-PTI debugging level (default:0=OFF)");
		#define TAGDEBUG "[ST-PTI] "
		#define dprintk(level, x...) do { if (stpti_debug && (level <= stpti_debug)) printk(TAGDEBUG x); } while (0)
	#else
		#define dprintk(x...)
	#endif
#endif

#if defined(CONFIG_SH_QBOXHD_1_0) || defined(CONFIG_SH_QBOXHD_MINI_1_0)
//void pti_hal_init(struct stpti *, void (*_demultiplexDvbPackets)(struct dvb_demux*, const u8 *, int), int);
void pti_hal_init(struct stpti *, struct dvb_demux *, void (*_demultiplexDvbPackets)(struct dvb_demux*, const u8 *, int), int);
#endif

#ifdef UFS922
extern void cx21143_register_frontend(struct dvb_adapter *dvb_adap);
#elif defined(FORTIS_HDBOX)
extern void stv090x_register_frontend(struct dvb_adapter *dvb_adap);
#elif defined(HL101)
extern void fe_core_register_frontend(struct dvb_adapter *dvb_adap);
#else
extern void cx24116_register_frontend(struct dvb_adapter *dvb_adap);
#endif

extern void demultiplexDvbPackets(struct dvb_demux* demux, const u8 *buf, int count);

extern int swts;


static void dumpArray(int *buffer,int len,const char *msg)
{
	int count=len;
	char line[500]="";
	char val[5]="";

	while (count--)
	{
		sprintf(val, "%d,", *buffer);
		strcat(line,val);
		buffer++;
	};

	dprintk(3,"%s: %s %s\n",__func__,msg,line);
}

static void dumpDescramblerIndex(struct PtiSession *pSession)
{

	if(pSession)
	{
		dprintk(3,"%s: pSession=<%p>\n",__func__,pSession);
		dumpArray(pSession->descramblerindex,32,"DescramblerIndex=");
	}
}

int stpti_start_feed ( struct dvb_demux_feed *dvbdmxfeed,
						struct DeviceContext_s *DeviceContext )
{
	struct dvb_demux *demux = dvbdmxfeed->demux;
	int vLoop, my_pes_type;
	struct PtiSession *pSession = DeviceContext->pPtiSession;
	BUFFER_TYPE bufType = MISC_BUFFER;

#ifdef DELAYER
	struct DvbContext_s *dvbContext = DeviceContext->DvbContext;
	struct dvb_adapter *adapter = &(dvbContext->DvbAdapter);
	int adapterId = adapter->num;
#endif
	dprintk(2,"%s: start demux:%p, pSession:%p (session handle:%d), pid:0x%04x, type: %d, pes_type:%d\n", __func__,
			demux, pSession,(pSession?pSession->session:-1), dvbdmxfeed->pid, dvbdmxfeed->type, dvbdmxfeed->pes_type);

	if ( pSession == NULL )
	{
		dprintk(1,"%s: pSession == NULL\n", __func__);
		return -1;
	}

	/* PTI is only started if the source is one of two frontends or
		if playback via SWTS is activated. Otherwise playback would
		unnecessarily waste a buffer (might lead to loss of a second
		recording). */
	if (!(((pSession->source >= DMX_SOURCE_FRONT0) &&
		   (pSession->source < DMX_SOURCE_FRONT2)) ||
		  ((pSession->source == DMX_SOURCE_DVR0) && swts)))
	{
		dprintk(1,"%s: invalid source pSession->source= %d\n", __func__,pSession->source);
		return -1;
	}

	switch ( dvbdmxfeed->type )
	{
		case DMX_TYPE_TS:
			break;
		case DMX_TYPE_SEC:
			bufType = MISC_BUFFER;
			break;
		case DMX_TYPE_PES:
		default:
			dprintk(1,"%s: feed type = %d (not supported) <\n",
					__func__, dvbdmxfeed->type );

			return -EINVAL;
	}

	if ( dvbdmxfeed->type == DMX_TYPE_TS )
	{
		switch ( dvbdmxfeed->pes_type )
		{
			case DMX_PES_VIDEO0:
			case DMX_PES_VIDEO1:
				bufType = VID_BUFFER;
			break;
			case DMX_PES_AUDIO0:
			case DMX_PES_AUDIO1:
				bufType = AUD_BUFFER;
			break;
			case DMX_TS_PES_OTHER:
				bufType = OTHER_BUFFER;
			break;
			case DMX_TS_PES_TELETEXT:
			case DMX_TS_PES_PCR:
			//case DMX_TS_PES_OTHER:
			break;
			default:
				dprintk(1,"%s: pes type = %d (not supported) <\n",
						__func__, dvbdmxfeed->pes_type );

			return -EINVAL;
		}
	}


#ifdef DELAYER
	if((dvbdmxfeed->type==DMX_TYPE_SEC) && hasDelayer(pSession))
	{
		int res = 0;
		/// set pid to delay
		dprintk(2,"%s: DELAYER => adapterId =%d \n",__func__, adapterId);
		res = setDelayerPid(dvbdmxfeed->pid,pSession);
		dprintk(3,"%s: setDelayerPid returned %d\n",__func__,res);
	}
#endif

	if (dvbdmxfeed->type == DMX_TYPE_SEC)
		my_pes_type = 99;
	else
		my_pes_type = dvbdmxfeed->pes_type;

	for ( vLoop = 0; vLoop < pSession->num_pids; vLoop++ )
	{
		if (( ( unsigned short ) pSession->pidtable[vLoop] == ( unsigned short ) dvbdmxfeed->pid ))
		{
#if defined(CONFIG_SH_QBOXHD_1_0) || defined(CONFIG_SH_QBOXHD_MINI_1_0)
			if(pSession->references[vLoop]==1 && pSession->buf_type[vLoop]==VID_BUFFER)
			{
				/// if current bufType == VID_BUFFER swap it with one of type OTHER_BUFFER
				// 1. release slot with VID_BUFFER buffer
				pti_hal_slot_unlink_buffer ( pSession->session, pSession->slots[vLoop]);
				pti_hal_slot_clear_pid ( pSession->session, pSession->slots[vLoop] );
				pti_hal_slot_free ( pSession->session, pSession->slots[vLoop] );

				// 2. get a new slot with OTHER_BUFFER buffer
				pSession->slots[vLoop] = pti_hal_get_new_slot_handle ( pSession->session, dvbdmxfeed->type, dvbdmxfeed->pes_type, demux , NULL, NULL);
				if(pti_hal_slot_link_buffer ( pSession->session, pSession->slots[vLoop], OTHER_BUFFER) == 0)
				{
					/* link audio/video slot to the descrambler */
					if ((dvbdmxfeed->pes_type == DMX_TS_PES_VIDEO) || (dvbdmxfeed->pes_type == DMX_TS_PES_AUDIO) || (dvbdmxfeed->pid>50)) //DMX_TS_PES_OTHER
					{
						int err;
						if(pSession->descramblerForPid[dvbdmxfeed->pid] != -1) /// a descrambler index already set for this PID by CA_SET_PID
						{
							pSession->descramblerindex[vLoop]= pSession->descramblerForPid[dvbdmxfeed->pid];

							if ((err = pti_hal_descrambler_link(pSession->session,pSession->descramblers[pSession->descramblerindex[vLoop]], pSession->slots[vLoop])) != 0)
								printk("%s: ERROR linking slot %d to descrambler %d, err = %d\n", __func__, pSession->slots[vLoop], pSession->descramblers[pSession->descramblerindex[vLoop]], err);
						}
					}
					pti_hal_slot_set_pid ( pSession->session, pSession->slots[vLoop], dvbdmxfeed->pid );

					// 2.1 update new assigned buffer type
					pSession->buf_type[vLoop]==OTHER_BUFFER;
					dprintk(2,"%s: CHANGED  VID_BUFFER =====> OTHER_BUFFER !!! <---------------",__func__);
				}
				else
					dprintk(2,"%s: 1. pti_hal_slot_link_buffer FAILED!!! <---------------\n",__func__);
			}
#endif

			pSession->references[vLoop]++;

			//ok we have a reference but maybe this one must be rechecked to a new
			//dma (video) and maybe we must attach the descrambler
			//normally we should all these things (referencing etc)
			//in the hal module. later ;-)

			dumpDescramblerIndex(pSession);
#if 0
			/* link audio/video slot to the descrambler */
			if ( dvbdmxfeed->type == DMX_TYPE_TS )
			{
				if ((dvbdmxfeed->pes_type == DMX_TS_PES_VIDEO) || (dvbdmxfeed->pes_type == DMX_TS_PES_AUDIO))
				{
					int err;
/*
					if ((err = pti_hal_descrambler_link(pSession->session, pSession->descramblers[pSession->descramblerindex[vLoop]], pSession->slots[vLoop])) != 0)
						dprintk(1,"%s, Error linking slot %d to descrambler %d, err = %d\n",
							   __func__, pSession->slots[vLoop], pSession->descramblers[pSession->descramblerindex[vLoop]], err);
					else
						dprintk(2,"%s: linked slot %d to descrambler %d, session = %d type = %d for PID=0x%04x (%d)\n",
								 __func__, pSession->slots[vLoop], pSession->descramblers[pSession->descramblerindex[vLoop]],
								pSession->session, dvbdmxfeed->pes_type,dvbdmxfeed->pid,dvbdmxfeed->pid);
*/
					dprintk(2,"\n\n\n +++++++++++++++++++++++++++++++++\n %s: NO DESCRAMBLER LINKED !!!!!!!!\n +++++++++++++++++++++++++++++++++\n\n\n\n",__func__);
				}
			}
#endif

//			dprintk(2,"%s: PID=0x%04x (%d) buf_type=%d                        <================\n",
//					__func__,dvbdmxfeed->pid,dvbdmxfeed->pid,pSession->buf_type[vLoop]);

			dprintk(2,"%s: PID=0x%04x (%d) already collecting => references=%d <---------------\n",
					__func__,dvbdmxfeed->pid,dvbdmxfeed->pid,pSession->references[vLoop]);
			return 0;
		}
	}

	dprintk(2,"%s: NEW PID=0x%04x (%d) \n", __func__, dvbdmxfeed->pid, dvbdmxfeed->pid);

	pSession->pidtable[pSession->num_pids]   = dvbdmxfeed->pid;
	pSession->type[pSession->num_pids]       = dvbdmxfeed->type;
	pSession->pes_type[pSession->num_pids]   = my_pes_type;

#if defined(CONFIG_SH_QBOXHD_1_0) || defined(CONFIG_SH_QBOXHD_MINI_1_0)
	/// save current buffer type
// 	if(bufType == VID_BUFFER)
// 		dprintk(2,"%s: saving bufType == VID_BUFFER for PID=0x%04x\n", __func__, dvbdmxfeed->pid);
	pSession->buf_type[pSession->num_pids]   = bufType;
#endif ///
	pSession->references[pSession->num_pids] = 1;
	pSession->slots[pSession->num_pids]      = pti_hal_get_new_slot_handle ( pSession->session,
																			dvbdmxfeed->type,
																			dvbdmxfeed->pes_type,
																			demux , NULL, NULL);

	if(pti_hal_slot_link_buffer ( pSession->session,
									pSession->slots[pSession->num_pids],
									bufType) != 0)
	{
		dprintk(2,"%s: pti_hal_slot_link_buffer FAILED!!! <---------------\n",__func__);
		// free slot
		pti_hal_slot_free(pSession->session, pSession->slots[pSession->num_pids]);

		return -1;
	}

	if ( dvbdmxfeed->type == DMX_TYPE_TS )
	{
		/* link audio/video slot to the descrambler */
		if ((dvbdmxfeed->pes_type == DMX_TS_PES_VIDEO) ||
			(dvbdmxfeed->pes_type == DMX_TS_PES_AUDIO)
			|| (dvbdmxfeed->pid>50)) //DMX_TS_PES_OTHER
		{
			int err;
			dprintk(2,"%s: pSession->descramblerForPid[dvbdmxfeed->pid=0x%04x]=%d\n",__func__,dvbdmxfeed->pid,pSession->descramblerForPid[dvbdmxfeed->pid]);

			if(pSession->descramblerForPid[dvbdmxfeed->pid] != -1) /// a descrambler index already set for this PID by CA_SET_PID
			{
				dprintk(2,"%s: LINK SLOT to DESCRAMBLER: descramblerIndex=%d\n",__func__,
						pSession->descramblerindex[pSession->num_pids]);

				pSession->descramblerindex[pSession->num_pids]= pSession->descramblerForPid[dvbdmxfeed->pid];
				dprintk(2,"%s: descramblerindex=%d, SlotHandle = %d\n",__func__,
						pSession->descramblerindex[pSession->num_pids],
						pSession->slots[pSession->num_pids] );

				if ((err = pti_hal_descrambler_link(pSession->session,
													pSession->descramblers[pSession->descramblerindex[pSession->num_pids]],
													pSession->slots[pSession->num_pids])) != 0)
					printk("%s: ERROR linking slot %d to descrambler %d, err = %d\n",
							__func__, pSession->slots[pSession->num_pids],
							pSession->descramblers[pSession->descramblerindex[pSession->num_pids]], err);
				else
					dprintk(2,"%s: linked slot %d to descrambler %d, session = %d type=%d\n\tpSession->descramblerindex[pSession->num_pids=%d]=%d\n",
							__func__,pSession->slots[pSession->num_pids],
							pSession->descramblers[pSession->descramblerindex[pSession->num_pids]],
							pSession->session, dvbdmxfeed->pes_type,
							pSession->num_pids, pSession->descramblerindex[pSession->num_pids]);
			}
			dprintk(2,"\n\t==============================\n\t%s: NO DESCRAMBLER LINKED !!!!!!!!\n\t==============================\n",__func__);
		}
	}

	pti_hal_slot_set_pid ( pSession->session, pSession->slots[pSession->num_pids],
							dvbdmxfeed->pid );

	pSession->num_pids++;

	dprintk(2,"%s, pid = %d, num_pids = %d \n", __func__, dvbdmxfeed->pid, pSession->num_pids );

	#if 0
	dprintk("#  pid t pt ref\n");
	for ( vLoop = 0; vLoop < ( pSession->num_pids ); vLoop++ )
	{
		dprintk("%d %4d %d %2d  %d\n", vLoop, pSession->pidtable[vLoop], pSession->type[vLoop], pSession->pes_type[vLoop],
			pSession->references[vLoop] );
	}
	#endif

	dprintk(2,"%s: <\n\n", __func__);
	return 0;
}

EXPORT_SYMBOL ( stpti_start_feed );


int stpti_stop_feed ( struct dvb_demux_feed *dvbdmxfeed,
		      struct DeviceContext_s *pContext )
{
	int n, vLoop, my_pes_type;
	int haveFound = 0;
	struct PtiSession *pSession = pContext->pPtiSession;

	if ( pSession == NULL )
	{
		dprintk(1,"%s: pSession == NULL\n", __func__);
		return -1;
	}

	/* PTI was only started if the source is one of two frontends or
		if playback via SWTS was activated. */
	if (!(((pSession->source >= DMX_SOURCE_FRONT0) &&
			(pSession->source < DMX_SOURCE_FRONT2)) ||
			((pSession->source == DMX_SOURCE_DVR0) && swts)))
		return -1;

	dprintk(2,"%s(): demux = %p, context = %p, session = %p, pid = 0x%04x (%d), type = %d, pes_type = %d >\n",
			__func__, dvbdmxfeed->demux, pContext, pSession, dvbdmxfeed->pid, dvbdmxfeed->pid,
			dvbdmxfeed->type, dvbdmxfeed->pes_type );

#ifdef DELAYER
	if((dvbdmxfeed->type==DMX_TYPE_SEC) && hasDelayer(pSession))
	{
		/// clear pid to delay
		dprintk(2,"%s: type = %d \n",__func__, dvbdmxfeed->type );
		clearDelayerPid(dvbdmxfeed->pid,pSession);
	}
#endif

	if (dvbdmxfeed->type == DMX_TYPE_SEC)
		my_pes_type = 99;
	else
		my_pes_type = dvbdmxfeed->pes_type;

	/*
	* Now reallocate the pids, and update id information
	*/
	for ( vLoop = 0; vLoop < ( pSession->num_pids ); vLoop++ )
	{
		if (( pSession->pidtable[vLoop] == dvbdmxfeed->pid ) )
		{
			pSession->references[vLoop]--;

			dprintk(3,"Reference = %d\n", pSession->references[vLoop]);

			haveFound = 1;

			if (pSession->references[vLoop] == 0)
			{

				pti_hal_slot_unlink_buffer ( pSession->session,
								pSession->slots[vLoop]);

				//pti_hal_buffer_disable ( pSession->session, pSession->buffers[0] );
				pti_hal_slot_clear_pid ( pSession->session, pSession->slots[vLoop] );
				pti_hal_slot_free ( pSession->session, pSession->slots[vLoop] );

				dumpDescramblerIndex(pSession);

				dprintk(2,"%s: found pid to stop: PID=0x%04x (index = %d/num_pids=%d ) type=%d, pes_type=%d\n", __func__, pSession->pidtable[vLoop],
						vLoop , pSession->num_pids, pSession->type[vLoop], pSession->pes_type[vLoop]);

				for ( n = vLoop; n < ( pSession->num_pids - 1 ); n++ )
				{
					dprintk(3,"n = %d, old pid = %d, %d, %d, new pid = %d\n",
						n, pSession->pidtable[n], pSession->type[n], pSession->pes_type[n], pSession->pidtable[n + 1] );

					pSession->pidtable[n]   = pSession->pidtable[n + 1];
					pSession->slots[n]      = pSession->slots[n + 1];
					pSession->type[n]       = pSession->type[n + 1];
					pSession->pes_type[n]   = pSession->pes_type[n + 1];
#if defined(CONFIG_SH_QBOXHD_1_0) || defined(CONFIG_SH_QBOXHD_MINI_1_0)
					pSession->buf_type[n]   = pSession->buf_type[n + 1];
#endif ///
					pSession->references[n] = pSession->references[n + 1];
#ifdef NEW_DESCRAMBLER
					pSession->descramblerindex[n]=pSession->descramblerindex[n + 1];
#endif
				}

				pSession->descramblerindex[n]=pSession->descramblerindex[n + 1];
				pSession->num_pids--;

				dumpDescramblerIndex(pSession);

			#if 0
				if(dvbdmxfeed->pes_type == DMX_TS_PES_VIDEO)
				{
					/* reset the DMA threshold to 1 to allow low rate TS
						to be signalled on time */
				/* FIXME: quick hack assuming that DMA 0 is always responsible for
					the video */
				setDmaThreshold(0, 1);
				}
			#endif

				break;
			}
		}
	}

	if (!haveFound)
	{
		dprintk(1,"demux try to stop feed not captured by pti\n" );
	}

	return 0;
}

EXPORT_SYMBOL ( stpti_stop_feed );


static int convert_source ( const dmx_source_t source)
{
	int tag = TS_NOTAGS;
	dprintk(2,"%s(): source (%d)\n", __func__, source );

	switch ( source )
	{
		case DMX_SOURCE_FRONT0:
		#if !defined(TF7700) && !defined(UFS922) && !defined(FORTIS_HDBOX) \
	&& !defined(CONFIG_SH_QBOXHD_1_0) && !defined(CONFIG_SH_QBOXHD_MINI_1_0)
			/* in UFS910 the CIMAX output is connected to TSIN2 */
			tag = TSIN2;
			break;
		#else
			/* in TF7700 and QBoxHD/QBoxHD-mini the first tuner is connected to TSIN0 */
			tag = TSIN0;
			break;
		case DMX_SOURCE_FRONT1:
			/* in TF7700 and QBoxHD/QBoxHD-mini the second tuner is connected to TSIN1 */
			tag = TSIN1;
			break;
		#endif

		case DMX_SOURCE_DVR0:
			dprintk(2,"%s(): DMX_SOURCE_DVR0\n", __func__);
			tag = SWTS0;
			break;
		default:
			dprintk(1,"%s(): invalid frontend source (%d)\n", __func__, source );
	}

	return tag;
}

static int sessionCounter = 0;
static int ptiInitialized = 0;

static struct stpti pti;

/********************************************************/

#if defined(CONFIG_SH_QBOXHD_1_0) || defined(CONFIG_SH_QBOXHD_MINI_1_0)
void ptiInit ( struct DeviceContext_s *pContext, struct plat_frontend_config *nims_config)
#else
void ptiInit ( struct DeviceContext_s *pContext )
#endif
{
	unsigned long start = 0x19230000;
	struct PtiSession *pSession;
	int tag;
#ifdef NEW_DESCRAMBLER
	int i;
#endif

	dprintk(2,"%s context = %p, demux = %p\n",  __FUNCTION__,
			pContext, &pContext->DvbDemux);

	if ( pContext->pPtiSession != NULL )
	{
		dprintk(1,"PTI ERROR: attempted to initialize a device context with an existing session\n");
		return;
	}

	if(!ptiInitialized)
	{
		// the function is called for the first time
		// perform common PTI initialization

		/*
		* ioremap the pti address space
		*/
		pti.pti_io = (unsigned int)ioremap ( start, 0xFFFF );

		/*
		* Setup the transport stream merger based on the configuration
		*/
#if defined(CONFIG_SH_QBOXHD_1_0) || defined(CONFIG_SH_QBOXHD_MINI_1_0)
    	stm_tsm_init ( 0, nims_config );
#else
    	stm_tsm_init (  /*config */ 1 );
#endif

#if defined(TF7700) || defined(UFS922) || defined(FORTIS_HDBOX)
		pti_hal_init ( &pti, &pContext->DvbDemux, demultiplexDvbPackets, 2);
#elif defined(CONFIG_SH_QBOXHD_1_0) || defined(CONFIG_SH_QBOXHD_MINI_1_0)
		//pti_hal_init ( &pti, &pContext->DvbDemux, demultiplexDvbPackets, 2);
		pti_hal_init ( &pti, &pContext->DvbDemux, demultiplexDvbPackets, 1);
#else
		pti_hal_init ( &pti, &pContext->DvbDemux, demultiplexDvbPackets, 1);
#endif

#if defined(FORTIS_HDBOX)
		stv090x_register_frontend(&pContext->DvbContext->DvbAdapter);
#elif defined(CONFIG_SH_QBOXHD_1_0) || defined(CONFIG_SH_QBOXHD_MINI_1_0)
	/* Do nothing. We attach the tuners in nim_tuner_attach_frontend() */
#elif !defined(UFS922)
		cx24116_register_frontend( &pContext->DvbContext->DvbAdapter);
#else
		cx21143_register_frontend ( &pContext->DvbContext->DvbAdapter);
#endif
		ptiInitialized = 1;
	}

	/*
	* Allocate the session structure
	*/
	pSession = ( struct PtiSession * ) kmalloc ( sizeof ( struct PtiSession ), GFP_KERNEL );
	if ( !pSession )
		return;

	memset ( ( void * ) pSession, 0, sizeof ( *pSession ) );

	pSession->num_pids = 0;

	tag = convert_source(sessionCounter + DMX_SOURCE_FRONT0);
	pSession->session = pti_hal_get_new_session_handle(tag, &pContext->DvbDemux);

	dprintk(2,"%s: Session Handler = %d\n", __func__, pSession->session);

	// get new descrambler handle
	pSession->descrambler = pti_hal_get_new_descrambler(pSession->session);

#ifdef NEW_DESCRAMBLER
	pSession->descramblers[0] = pSession->descrambler;
	for(i=1;i<NUMBER_OF_DESCRAMBLERS;i++)
		pSession->descramblers[i] = pti_hal_get_new_descrambler(pSession->session);
	dprintk(2,"%s: Descrambler Handler = %d\n", __func__,pSession->descrambler);
	for(i=0;i<8192;i++)
		pSession->descramblerForPid[i] = -1; //0;

	for(i=0;i<32;i++)
		pSession->descramblerindex[i] = -1;

#else
	dprintk(2,"%s: Descrambler Handler = %d\n", __func__, pSession->descrambler);
#endif

	pContext->pPtiSession = pSession;

	sessionCounter++;

	return;
}

EXPORT_SYMBOL ( ptiInit );

#define MAX_DEMUX_MOD
int SetSource (struct dmx_demux* demux, const dmx_source_t *src)
{
	struct dvb_demux* pDvbDemux = (struct dvb_demux*)demux->priv;
	struct DeviceContext_s* pContext = (struct DeviceContext_s*)pDvbDemux->priv;
#ifdef MAX_DEMUX_MOD
	dprintk(2,"%s(): swts=%d\n", __func__, swts);

	if(swts)
	{
		if((pContext == NULL) || (pContext->pPtiSession == NULL) || (src == NULL))
		{
			dprintk(1,"%s(): invalid pointer (%p, %p, %p)\n",
				__func__, pContext, pContext->pPtiSession, src);
			return -EINVAL;
		}

		dprintk(2,"swts=%d SetSource(<0x%p>, %d)\n", swts, pDvbDemux, *src);

		pContext->pPtiSession->source = *src;

		if (((*src >= DMX_SOURCE_FRONT0) && (*src <= DMX_SOURCE_FRONT3)) || (*src == DMX_SOURCE_DVR0 && swts))
		{
			int res = pti_hal_set_source( pContext->pPtiSession->session, convert_source(*src) );
			if(res<0)
			{
				dprintk(1,"[ST-PTI] ERROR: swts=%d SetSource(%p, %d) FAILED\n", swts,pDvbDemux, *src);
				return -EINVAL;
			}
		}

		return 0;
	}


	if((pContext == NULL) || (src == NULL))
	{
		dprintk(1,"[ST-PTI] %s(): invalid pointer (%p, %p)\n", __func__, pContext, src);
		return -EINVAL;
	}

	dprintk(2,"\n\n %s():SetSource(<0x%p>, %d)        <---------------------- \n", __func__, pDvbDemux, *src);

	/**
	 *  PVR does not flow through TS-merger: we need to evaluate src
	 */
	if ((*src >= DMX_SOURCE_FRONT0) && (*src <= DMX_SOURCE_FRONT3))
	{
		char *SRC[]={"DMX_SOURCE_FRONT0","DMX_SOURCE_FRONT1","DMX_SOURCE_FRONT2","DMX_SOURCE_FRONT3"};
		int res = pti_hal_set_source( pContext->pPtiSession->session, convert_source(*src) );
		if(res<0)
		{
			dprintk(1,"%s(): ERROR: SetSource(%p, %d) FAILED\n",__func__, pDvbDemux, *src);
			return -EINVAL;
		}
		dprintk(2,"%s(): SetSource(%p, %s)\n",__func__, pDvbDemux, SRC[*src]);
	}


	if((*src >= DMX_SOURCE_DVR0) && (*src <= DMX_SOURCE_DVR3))
	{
		char *SRC[]={"DMX_SOURCE_DVR0","DMX_SOURCE_DVR1","DMX_SOURCE_DVR2","DMX_SOURCE_DVR3"};

		/**
		 *   In this case we need to handle Demux params
		 */
		if(pDvbDemux == NULL)
		{
			dprintk(1,"%s(): SetSource(%p, %d)\n", __func__, pDvbDemux, (*src));
			return -EINVAL;
		}
		dprintk(2,"%s(): SetSource(%p, %s)\n", __func__, pDvbDemux, SRC[((*src)-DMX_SOURCE_DVR0)]);
	}

    return 0;
#else
	if((pContext == NULL) || (pContext->pPtiSession == NULL) || (src == NULL))
	{
		dprintk(1,"%s(): invalid pointer (%p, %p, %p)\n",
			__func__, pContext, pContext->pPtiSession, src);
		return -EINVAL;
	}

	dprintk(3,"SetSource(%p, %d)\n", pDvbDemux, *src);

	pContext->pPtiSession->source = *src;

	if (((*src >= DMX_SOURCE_FRONT0) && (*src <= DMX_SOURCE_FRONT3)) || (*src == DMX_SOURCE_DVR0))
	{
		if (pti_hal_set_source(pContext->pPtiSession->session, convert_source(*src)) < 0)
			dprintk(1,"Could not set source '%d'", *src);
	}

	return 0;
#endif ///MAX_DEMUX_MOD

}

