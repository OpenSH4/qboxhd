/************************************************************************
COPYRIGHT (C) STMicroelectronics 2005

Source file name : dvb_module.c
Author :           Julian

Implementation of the LinuxDVB interface to the DVB streamer

Date        Modification                                    Name
----        ------------                                    --------
24-Mar-05   Created                                         Julian

************************************************************************/

#include <linux/sched.h>
#include <linux/syscalls.h>

#if defined(CONFIG_SH_QBOXHD_1_0) || defined(CONFIG_SH_QBOXHD_MINI_1_0)
//__TDT__
#if defined (CONFIG_KERNELVERSION) /* ST Linux 2.3 */
#include <linux/bpa2.h>
#else
#include <linux/bigphysarea.h>
#endif
#include <linux/module.h>
#include <linux/file.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <asm/uaccess.h>

#include <linux/dvb/audio.h>
#include <linux/dvb/video.h>
#include <linux/dvb/version.h>

#include "dvb_demux.h"          /* provides kernel demux types */

#define USE_KERNEL_DEMUX

#include "dvb_module.h"
#include "dvb_audio.h"
#include "dvb_video.h"
#include "dvb_dmux.h"
#include "dvb_dvr.h"
#include "dvb_ca.h"
#include "backend.h"
#include "nim_tuner.h"

#if (DVB_API_VERSION > 3)
DVB_DEFINE_MOD_OPT_ADAPTER_NR(adapter_num);
#endif
extern int __init dvp_init(void);
extern void linuxdvb_v4l2_init(void);
#ifdef __TDT__
extern void init_e2_proc(struct DeviceContext_s* DC);
extern void cleanup_e2_proc(void);
extern void install_vmpeg_procs(struct DvbCurrDecoder_s* DvbCurrDecoder, unsigned char decoder);
extern void cleanup_vmpeg_proc(void);
extern void ptiInit ( struct DeviceContext_s *pContext, struct plat_frontend_config *nims_config);
#endif

/*static*/ int  __init      StmLoadModule (void);
static void __exit      StmUnloadModule (void);

int debug;

#ifdef __TDT__
extern int SetSource (struct dmx_demux* demux, const dmx_source_t *src);
#endif

module_init             (StmLoadModule);
module_exit             (StmUnloadModule);

MODULE_DESCRIPTION      ("Linux DVB video and audio driver for STM streaming architecture.");
MODULE_AUTHOR           ("Julian Wilson");
MODULE_LICENSE          ("GPL");

#define MODULE_NAME     "STM Streamer"

#ifdef __TDT__
int highSR = 0;
int swts = 0;

module_param(highSR, int, 0444);
MODULE_PARM_DESC(highSR, "Start Driver with support for Symbol Rates 30000.\nIf 1 then some CAMS will not work.");

// module_param(swts, int, 0444);
module_param(swts, int, 0644);
MODULE_PARM_DESC(swts, "Do not route injected data through the tsm/pti\n");
#endif


#ifdef CONFIG_SH_QBOXHD_1_0
#define MOD               "-HD"
#elif  CONFIG_SH_QBOXHD_MINI_1_0
#define MOD               "-Mini"
#else
#define MOD               ""
#endif

// int dvb_debug = 0;
dvb_debug = 0;
module_param(dvb_debug, int, 0644);
MODULE_PARM_DESC(dvb_debug, "Control for DVB_DEBUG\n");

#define STMDVB_VERSION       "0.0.8"MOD
MODULE_VERSION(STMDVB_VERSION);


#define DVB_CONTEXT_MAX		3
struct DvbContext_s*     DvbContextArray[DVB_CONTEXT_MAX];

#define DVB_DECODER_MAX	1
struct DvbCurrDecoder_s*     DvbCurrDecoderArray[DVB_DECODER_MAX];

long DvbGenericUnlockedIoctl(struct file *file, unsigned int foo, unsigned long bar)
{
    return dvb_generic_ioctl(NULL, file, foo, bar);
}

int __init StmLoadModule (void)
{
    int                         Result;
    int                         i;
	int							tuners_found;
	short						nr_adapters, nr_decoders;
	struct plat_frontend_config nims_config;

	memset(DvbContextArray, 0, sizeof(struct DvbContext_s*) * DVB_CONTEXT_MAX);
	memset(DvbCurrDecoderArray, 0, sizeof(struct DvbContext_s*) * DVB_DECODER_MAX);

	get_nims_num_found(&tuners_found);
	get_nims_config(&nims_config);

	DVB_DEBUG("NIMs found: %d\n", tuners_found);

	for (nr_decoders = 0; nr_decoders < DVB_DECODER_MAX; nr_decoders++)
	{
		DvbCurrDecoderArray[nr_decoders] = kmalloc (sizeof (struct DvbCurrDecoder_s),  GFP_KERNEL);
		if (DvbCurrDecoderArray[nr_decoders] == NULL)
		{
			DVB_ERROR("Unable to allocate device memory\n");
			return -ENOMEM;
		}

		memset(DvbCurrDecoderArray[nr_decoders], 0, sizeof(struct DvbCurrDecoder_s));
	}

	for (nr_adapters = 0; nr_adapters < tuners_found; nr_adapters++) {
		struct DvbContext_s*     	DvbContext;

    	DvbContextArray[nr_adapters] = kmalloc (sizeof (struct DvbContext_s),  GFP_KERNEL);
    	if (DvbContextArray[nr_adapters] == NULL)
    	{
        	DVB_ERROR("Unable to allocate device memory\n");
        	return -ENOMEM;
    	}

    	memset(DvbContextArray[nr_adapters], 0, sizeof(struct DvbContext_s));
		DvbContext = DvbContextArray[nr_adapters];
		DvbContext->DvbContextArray  = DvbContextArray;
		DvbContext->DvbCurrDecoderArray  = DvbCurrDecoderArray;

		//printk("%s:DvbContext=<%p> nr_adapters=%d\n",__func__, DvbContext,nr_adapters);

#if 0
		DvbContext  = kmalloc (sizeof (struct DvbContext_s),  GFP_KERNEL);
    	if (DvbContext == NULL)
    	{
        	DVB_ERROR("Unable to allocate device memory\n");
        	return -ENOMEM;
    	}

#ifdef __TDT__
		/*memset(DvbContext, 0, sizeof*DvbContext);*/
    	memset(DvbContext, 0, sizeof(struct DvbContext_s));
#endif

		DvbContextArray[nr_adapters] = DvbContext;
		DVB_DEBUG("-------------------> Creating adapter%d\n", nr_adapters);
#endif

#ifdef __TDT__
    	if (swts)
      		printk("swts ->routing streams from dvr0 to tsm to pti to player\n");
    	else
      		printk("no swts ->routing streams from dvr0 direct to the player\n");
#endif

#if defined (CONFIG_KERNELVERSION)
#if (DVB_API_VERSION > 3)
		Result      = dvb_register_adapter (&DvbContext->DvbAdapter, MODULE_NAME, THIS_MODULE, NULL, &nr_adapters);
#else
		Result      = dvb_register_adapter (&DvbContext->DvbAdapter, MODULE_NAME, THIS_MODULE,NULL);
#endif
#else /* STLinux 2.2 kernel */
#ifdef __TDT__
    	Result      = dvb_register_adapter (&DvbContext->DvbAdapter, MODULE_NAME, THIS_MODULE,NULL);
#else
    	Result      = dvb_register_adapter (&DvbContext->DvbAdapter, MODULE_NAME, THIS_MODULE);
#endif
#endif /* CONFIG_KERNELVERSION */
		if (Result < 0)
		{
			DVB_ERROR("Failed to register adapter (%d)\n", Result);
			kfree(DvbContextArray[nr_adapters]);
			DvbContextArray[nr_adapters] = NULL;

			kfree(DvbCurrDecoderArray[nr_decoders]);
			DvbCurrDecoderArray[nr_decoders] = NULL;

			/*kfree(DvbContext);*/
			/*DvbContext      = NULL;*/
			return -ENOMEM;
		}

		mutex_init (&(DvbContext->Lock));
		mutex_lock (&(DvbContext->Lock));
		/**  Register devices*/
		for (i = 0; i < DVB_MAX_DEVICES_PER_ADAPTER; i++)
		{
			struct DeviceContext_s* DeviceContext   = &DvbContext->DeviceContext[i];
			struct dvb_demux*       DvbDemux        = &DeviceContext->DvbDemux;
			struct dmxdev*          DmxDevice       = &DeviceContext->DmxDevice;
			struct dvb_device*      DvrDevice;

			//sylvester: wenn der stream vom user kommt soll WriteToDecoder nix
			//tun, da das ja hier schon passiert. keine ahnung wie man das ansonsten
			//verhindern soll;-)
			DeviceContext->dvr_write = 0;

			/*DeviceContext->DvbContext               = DvbContext;*/
			DeviceContext->DvbContext               = DvbContextArray[nr_adapters];
#if defined (USE_KERNEL_DEMUX)
        	memset (DvbDemux, 0, sizeof (struct dvb_demux));
#ifdef __TDT__
        	DvbDemux->dmx.capabilities              = DMX_TS_FILTERING | DMX_SECTION_FILTERING | DMX_MEMORY_BASED_FILTERING | DMX_TS_DESCRAMBLING;
        	/* currently only dummy to avoid EINVAL error. Later we need it for second frontend ?! */
        	DvbDemux->dmx.set_source                = SetSource;
#else
        	DvbDemux->dmx.capabilities              = DMX_TS_FILTERING | DMX_SECTION_FILTERING | DMX_MEMORY_BASED_FILTERING;
#endif

			DvbDemux->priv                          = DeviceContext;
			DvbDemux->filternum                     = 32;
			DvbDemux->feednum                       = 32;
			DvbDemux->start_feed                    = StartFeed;
			DvbDemux->stop_feed                     = StopFeed;
			DvbDemux->write_to_decoder              = WriteToDecoder;
			Result                                  = dvb_dmx_init (DvbDemux);
			if (Result < 0)
			{
				DVB_ERROR ("dvb_dmx_init failed (errno = %d)\n", Result);
				mutex_unlock (&(DvbContext->Lock));
				return Result;
			}

			memset (DmxDevice, 0, sizeof (struct dmxdev));
			DmxDevice->filternum                    = DvbDemux->filternum;
			DmxDevice->demux                        = &DvbDemux->dmx;
			DmxDevice->capabilities                 = 0;
			Result                                  = dvb_dmxdev_init (DmxDevice, &DvbContext->DvbAdapter);
			if (Result < 0)
			{
				DVB_ERROR("dvb_dmxdev_init failed (errno = %d)\n", Result);
				dvb_dmx_release (DvbDemux);
				mutex_unlock (&(DvbContext->Lock));
				return Result;
			}
			DvrDevice                               = DvrInit (DmxDevice->dvr_dvbdev->fops);
			/* Unregister the built-in dvr device and replace it with our own version */
#ifdef __TDT__
			printk("%s: %d: DeviceContext %p, DvbDemux %p, DmxDevice %p\n", __func__, i, DeviceContext, DvbDemux, DmxDevice);
#endif
			dvb_unregister_device  (DmxDevice->dvr_dvbdev);
			dvb_register_device (&DvbContext->DvbAdapter,
								 &DmxDevice->dvr_dvbdev,
								  DvrDevice,
								  DmxDevice,
								  DVB_DEVICE_DVR);


			DeviceContext->MemoryFrontend.source    = DMX_MEMORY_FE;
			Result                                  = DvbDemux->dmx.add_frontend (&DvbDemux->dmx, &DeviceContext->MemoryFrontend);
			if (Result < 0)
			{
				DVB_ERROR ("add_frontend failed (errno = %d)\n", Result);
				dvb_dmxdev_release (DmxDevice);
				dvb_dmx_release    (DvbDemux);
				mutex_unlock (&(DvbContext->Lock));
				return Result;
			}
#else
			dvb_register_device (&DvbContext->DvbAdapter,
								&DeviceContext->DemuxDevice,
								DemuxInit (DeviceContext),
								DeviceContext,
								DVB_DEVICE_DEMUX);

			dvb_register_device (&DvbContext->DvbAdapter,
								&DeviceContext->DvrDevice,
								DvrInit (DeviceContext),
								DeviceContext,
								DVB_DEVICE_DVR);
#endif

        	dvb_register_device (&DvbContext->DvbAdapter,
								 &DeviceContext->AudioDevice,
								  AudioInit (DeviceContext),
								  DeviceContext,
								  DVB_DEVICE_AUDIO);

        	/* register the CA device (e.g. CIMAX) */
#ifdef __TDT__
        	if(i < 2)
#endif
				dvb_register_device (&DvbContext->DvbAdapter,
									 &DeviceContext->CaDevice,
									 CaInit (DeviceContext),
									 DeviceContext,
									 DVB_DEVICE_CA);

			dvb_register_device (&DvbContext->DvbAdapter,
								 &DeviceContext->VideoDevice,
								  VideoInit (DeviceContext),
								  DeviceContext,
								  DVB_DEVICE_VIDEO);

			DeviceContext->Id                       = i;
			DeviceContext->DemuxContext             = DeviceContext;        /* wire directly to own demux by default */
			DeviceContext->SyncContext              = DeviceContext;        /* we are our own sync group by default */
			DeviceContext->Playback                 = NULL;
			DeviceContext->StreamType               = STREAM_TYPE_TRANSPORT;
			//DeviceContext->DvbContext               = DvbContextArray[nr_adapters];
			DeviceContext->DemuxStream              = NULL;
			DeviceContext->VideoStream              = NULL;
			DeviceContext->AudioStream              = NULL;
			DeviceContext->PlaySpeed                = DVB_SPEED_NORMAL_PLAY;
			DeviceContext->PlayInterval.start       = DVB_TIME_NOT_BOUNDED;
			DeviceContext->PlayInterval.end         = DVB_TIME_NOT_BOUNDED;
			DeviceContext->dvr_in                   = kmalloc(65536,GFP_KERNEL); // 128Kbytes is quite a lot per device.
			DeviceContext->dvr_out                  = kmalloc(65536,GFP_KERNEL); // However allocating on each write is expensive.
			DeviceContext->EncryptionOn             = 0;

#if defined CONFIG_SH_QBOXHD_1_0 || defined CONFIG_SH_QBOXHD_MINI_1_0
			DeviceContext->VideoOutputWindow.X                = 0;
			DeviceContext->VideoOutputWindow.Y                = 0;
			DeviceContext->VideoOutputWindow.Width            = 0;
			DeviceContext->VideoOutputWindow.Height           = 0;

			DeviceContext->VideoInputWindow.X                 = 0;
			DeviceContext->VideoInputWindow.Y                 = 0;
			DeviceContext->VideoInputWindow.Width             = 0;
			DeviceContext->VideoInputWindow.Height            = 0;
#endif

#ifdef __TDT__
			DeviceContext->VideoPlaySpeed           = DVB_SPEED_NORMAL_PLAY;
			DeviceContext->provideToDecoder = 0;
			DeviceContext->feedPesType = 0;
			mutex_init(&DeviceContext->injectMutex);

			//printk("%s: calling ptiInit() - i=%d nr_adapters=%d\n", __func__, i, nr_adapters);
			if(i < 1)
				ptiInit(DeviceContext, &nims_config);
			else
				DeviceContext->pPtiSession = NULL; ///DvbContext->DeviceContext[0].pPtiSession;

			if(i < 1 && !nr_adapters) {
				init_e2_proc(DeviceContext);
			}

#endif
    	}
		mutex_unlock (&(DvbContext->Lock));

		nim_tuner_attach_frontend(&DvbContext->DvbAdapter, nr_adapters);
	}

	for (nr_decoders = 0; nr_decoders < DVB_DECODER_MAX; nr_decoders++) {
		DvbCurrDecoderArray[nr_decoders]->DvbContext = DvbContextArray[0];
		install_vmpeg_procs(DvbCurrDecoderArray[nr_decoders], nr_decoders);
	}

	BackendInit ();

#ifndef __TDT__
    dvp_init();
#endif

    linuxdvb_v4l2_init();

    DVB_DEBUG("STM stream device loaded\n");

    return 0;
}

static void __exit StmUnloadModule (void)
{
    int i;
    int j;

    BackendDelete ();

	cleanup_e2_proc();

	cleanup_vmpeg_proc();

	for (j = 0; j < 3; j++)
	{
		struct DvbContext_s*     DvbContext;

		if (!DvbContextArray[j])
			continue;
		DvbContext = DvbContextArray[j];

		for (i = 0; i < DVB_MAX_DEVICES_PER_ADAPTER; i++)
		{
			struct DeviceContext_s* DeviceContext   = &DvbContext->DeviceContext[i];
			struct dvb_demux*       DvbDemux        = &DeviceContext->DvbDemux;
			struct dmxdev*          DmxDevice       = &DeviceContext->DmxDevice;

#if defined (USE_KERNEL_DEMUX)
			if (DmxDevice != NULL)
			{
				/* We don't need to unregister DmxDevice->dvr_dvbdev as this will be done by dvb_dmxdev_release */
				dvb_dmxdev_release (DmxDevice);
			}
			if (DvbDemux != NULL)
			{
				DvbDemux->dmx.remove_frontend (&DvbDemux->dmx, &DeviceContext->MemoryFrontend);
				dvb_dmx_release    (DvbDemux);
			}
#else
			dvb_unregister_device  (DeviceContext->DemuxDevice);
			dvb_unregister_device  (DeviceContext->DvrDevice);
#endif
			if (DeviceContext->AudioDevice != NULL)
				dvb_unregister_device  (DeviceContext->AudioDevice);
			if (DeviceContext->VideoDevice != NULL)
				dvb_unregister_device  (DeviceContext->VideoDevice);

			PlaybackDelete (DeviceContext->Playback);
			DeviceContext->AudioStream              = NULL;
			DeviceContext->VideoStream              = NULL;
			DeviceContext->Playback                 = NULL;
		kfree(DeviceContext->dvr_in);
		kfree(DeviceContext->dvr_out);
		}


		if (DvbContext != NULL)
		{
			dvb_unregister_adapter (&DvbContext->DvbAdapter);
			kfree (DvbContext);
		}
		DvbContext  = NULL;

	}

    DVB_DEBUG("STM stream device unloaded\n");

    return;
}

#endif
