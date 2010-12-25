/************************************************************************
Source file name : dvb_demux.c
Author :           Julian

Implementation of linux dvb demux hooks

Date        Modification                                    Name
----        ------------                                    --------
01-Nov-06   Created                                         Julian

************************************************************************/

#include <linux/module.h>
#include <linux/dvb/dmx.h>
#include <linux/dvb/audio.h>
#include <linux/dvb/video.h>
#ifdef __TDT__
#include <linux/dvb/ca.h>
#include "dvb_ca_en50221.h"
#endif
#include "dvb_demux.h"          /* provides kernel demux types */

#include "dvb_module.h"
#include "dvb_audio.h"
#include "dvb_video.h"
#include "dvb_dmux.h"
#include "backend.h"
#include <asm/io.h>
#include <linux/delay.h>
#include <asm/cacheflush.h>

//__TDT__: many modifications in this file

extern int AudioIoctlSetAvSync (struct DeviceContext_s* Context, unsigned int State);
extern int AudioIoctlStop (struct DeviceContext_s* Context);

extern int stpti_start_feed (struct dvb_demux_feed *dvbdmxfeed,
			     struct DeviceContext_s *DeviceContext);
int stpti_stop_feed (struct dvb_demux_feed *dvbdmxfeed,
		     struct DeviceContext_s *pContext);

#if defined(CONFIG_SH_QBOXHD_1_0) || defined(CONFIG_SH_QBOXHD_MINI_1_0)
#define DVB_DEMUX_DEBUG
#ifdef DVB_DEMUX_DEBUG
		int stdemux_debug=0;
		module_param(stdemux_debug, int, 0644);
		MODULE_PARM_DESC(stdemux_debug, "Modify DVB_DEMUX debugging level (default:0=OFF)");

		static int testPid = -1;
		module_param(testPid, int, 0644);
		MODULE_PARM_DESC(testPid, "testPid: pid to looking for into filtering stream");
		static int testPid_freq = 0;
		module_param(testPid_freq, int, 0644);
		MODULE_PARM_DESC(testPid_freq, "testPid_freq: number of packets with PID=testPid.\n0=print never, 1=print each packet, 1000=print each 1000 packets");
		static int injectCount = 0;
		static int demuxCount = 0;
#define TAGDEBUG "[DVB_DEMUX] "
#define dprintk(level, x...) do { if (stdemux_debug && (level <= stdemux_debug)) printk(TAGDEBUG x); } while (0)
#else
#define dprintk(x...)
#endif
#endif

/********************************************************************************
*  This file contains the hook functions which allow the player to use the built-in
*  kernel demux device so that in-mux non audio/video streams can be read out of
*  the demux device.
********************************************************************************/

/*{{{  COMMENT DmxWrite*/
#if 0
/********************************************************************************
*  \brief      Write user data into player and kernel filter engine
*              DmxWrite is called by the dvr device write function.  It allows us
*              to intercept data writes from the user and de blue ray them.
*              Data is injected into the kernel first to preserve user context.
********************************************************************************/
int
DmxWrite (struct dmx_demux *Demux, const char *Buffer, size_t Count)
{
  size_t DataLeft = Count;
  int Result = 0;
  unsigned int Offset = 0;
  unsigned int Written = 0;
  struct dvb_demux *DvbDemux = (struct dvb_demux *) Demux->priv;
  struct DeviceContext_s *Context = (struct DeviceContext_s *) DvbDemux->priv;

  if (((Count % TRANSPORT_PACKET_SIZE) == 0)
      || ((Count % BLUERAY_PACKET_SIZE) != 0))
    Context->DmxWrite (Demux, Buffer, Count);
  else
    {
      Offset = sizeof (unsigned int);
      while (DataLeft > 0)
	{
	  Result =
	    Context->DmxWrite (Demux, Buffer + Offset, TRANSPORT_PACKET_SIZE);
	  Offset += BLUERAY_PACKET_SIZE;
	  DataLeft -= BLUERAY_PACKET_SIZE;
	  if (Result < 0)
	    return Result;
	  else if (Result != TRANSPORT_PACKET_SIZE)
	    return Written + Result;
	  else
	    Written += BLUERAY_PACKET_SIZE;
	}
    }

  return DemuxInjectFromUser (Context->DemuxStream, Buffer, Count);	/* Pass data to player before putting into the demux */

}
#endif
/*}}}  */
#if 0
/*{{{  StartFeed*/
/********************************************************************************
*  \brief      Set up player to receive transport stream
*              StartFeed is called by the demux device immediately before starting
*              to demux data.
********************************************************************************/
static const unsigned int AudioId[DVB_MAX_DEVICES_PER_ADAPTER] =
  { DMX_TS_PES_AUDIO0, DMX_TS_PES_AUDIO1, DMX_TS_PES_AUDIO2,
DMX_TS_PES_AUDIO3 };
static const unsigned int VideoId[DVB_MAX_DEVICES_PER_ADAPTER] =
  { DMX_TS_PES_VIDEO0, DMX_TS_PES_VIDEO1, DMX_TS_PES_VIDEO2,
DMX_TS_PES_VIDEO3 };
int
StartFeed (struct dvb_demux_feed *Feed)
{
  struct dvb_demux *DvbDemux = Feed->demux;
  struct DeviceContext_s *Context = (struct DeviceContext_s *) DvbDemux->priv;
  struct DvbContext_s *DvbContext = Context->DvbContext;
  int Result = 0;
  int i;
  unsigned int Video = false;
  unsigned int Audio = false;

  DVB_DEBUG (">\n");

  if (Feed->pes_type > DMX_TS_PES_OTHER)
    return -EINVAL;

  switch (Feed->type)
    {
    case DMX_TYPE_TS:
      for (i = 0; i < DVB_MAX_DEVICES_PER_ADAPTER; i++)
	{
	  if (Feed->pes_type == AudioId[i])
	    {
	      Audio = true;
	      break;
	    }
	  if (Feed->pes_type == VideoId[i])
	    {
	      Video = true;
	      break;
	    }
	}
      if (!Audio && !Video)
	return 0;

      mutex_lock (&(DvbContext->Lock));
      if ((Context->Playback == NULL)
	  && (Context->DemuxContext->Playback == NULL))
	{
	  Result = PlaybackCreate (&Context->Playback);
	  if (Result < 0)
	    return Result;
	  if (Context != Context->DemuxContext)
	    Context->DemuxContext->Playback = Context->Playback;
	}
      if ((Context->DemuxStream == NULL)
	  && (Context->DemuxContext->DemuxStream == NULL))
	{
	  Result =
	    PlaybackAddDemux (Context->Playback, Context->DemuxContext->Id,
			      &Context->DemuxStream);
	  if (Result < 0)
	    {
	      mutex_unlock (&(DvbContext->Lock));
	      return Result;
	    }
	  if (Context != Context->DemuxContext)
	    Context->DemuxContext->DemuxStream = Context->DemuxStream;
	}

      if (Video)
	{
	  struct DeviceContext_s *VideoContext =
	    &Context->DvbContext->DeviceContext[i];

	  VideoContext->DemuxContext = Context;
	  VideoIoctlSetId (VideoContext, Feed->pid);
	  VideoIoctlPlay (VideoContext);
	}
      else
	{
	  struct DeviceContext_s *AudioContext =
	    &Context->DvbContext->DeviceContext[i];

	  AudioContext->DemuxContext = Context;
	  AudioIoctlSetId (AudioContext, Feed->pid);
	  AudioIoctlPlay (AudioContext);
	}
      mutex_unlock (&(DvbContext->Lock));

      break;
    case DMX_TYPE_SEC:
      break;
    default:
      return -EINVAL;
    }

  DVB_DEBUG ("<\n");
  return 0;
}

/*}}}  */
#else
/*{{{  StartFeed*/
/********************************************************************************
*  \brief      Set up player to receive transport stream
*              StartFeed is called by the demux device immediately before starting
*              to demux data.
********************************************************************************/
static const unsigned int AudioId[DVB_MAX_DEVICES_PER_ADAPTER] =
  { DMX_TS_PES_AUDIO0, DMX_TS_PES_AUDIO1 };

static const unsigned int VideoId[DVB_MAX_DEVICES_PER_ADAPTER] =
  { DMX_TS_PES_VIDEO0, DMX_TS_PES_VIDEO1 };

int StartFeed (struct dvb_demux_feed *Feed)
{
  struct dvb_demux *DvbDemux = Feed->demux;
  struct DeviceContext_s *Context = (struct DeviceContext_s *) DvbDemux->priv;
  struct DeviceContext_s *AvContext = NULL;
  struct DvbContext_s *DvbContext = Context->DvbContext;
  int Result = 0;
  int i;
  unsigned int Video = false;
  unsigned int Audio = false;

  dprintk(2,"%s:> Feed->pid = 0x%04x (%d)\n",__func__, Feed->pid, Feed->pid);

#ifdef no_subtitles
	if ((Feed->type == DMX_TYPE_TS) && (Feed->pes_type > DMX_TS_PES_OTHER))
	{
		dprintk(1,"%s: pes_type %d > %d (OTHER)>\n",__func__, Feed->pes_type, DMX_TS_PES_OTHER);
		return -EINVAL;
	}
#endif

	switch (Feed->type)
	{
		case DMX_TYPE_SEC:
		{
			dprintk(4,"%s: Feed->type = SEC\n",__func__);

			mutex_lock (&(DvbContext->Lock));
			stpti_start_feed (Feed, Context);
			mutex_unlock (&(DvbContext->Lock));

			break;
		}
		case DMX_TYPE_TS:
		{
			dprintk(2,"%s: Feed->type = TS\n",__func__);

			for (i = 0; i < DVB_MAX_DEVICES_PER_ADAPTER; i++)
			{
				dprintk(3,"%s: \tTS i=%d\n",__func__,i);

				if (Feed->pes_type == AudioId[i])
				{
					Audio = true;
					break;
				}

				if (Feed->pes_type == VideoId[i])
				{
					Video = true;
					break;
				}

				//videotext & subtitles (other)
				if ((Feed->pes_type == DMX_TS_PES_TELETEXT) ||
					(Feed->pes_type == DMX_TS_PES_OTHER))
				{
					mutex_lock (&(DvbContext->Lock));
					dprintk(3,"%s: \tFeed->ts_type=%d Feed->pes_type =%s \n",__func__,
							Feed->ts_type,
							Feed->pes_type==DMX_TS_PES_OTHER?"DMX_TS_PES_OTHER":"DMX_TS_PES_TELETEXT");
					stpti_start_feed (Feed, Context);
					mutex_unlock (&(DvbContext->Lock));
					break;
				}
			}

			dprintk(3,"%s: TS Audio=%d, Video=%d\n",__func__,Audio,Video);

			if (!Audio && !Video)
			{
				dprintk(2,"%s: WARNING - pes_type = %d DONE.\n\n",__func__, Feed->pes_type);
				return 0;
			}

			mutex_lock (&(DvbContext->Lock));

			AvContext = &Context->DvbContext->DeviceContext[i];

			if ((AvContext->Playback == NULL)
			&& (AvContext->SyncContext->Playback == NULL))
			{
				Result = PlaybackCreate (&AvContext->Playback);

				if (Result < 0)
				{
					mutex_unlock (&(DvbContext->Lock));
					return Result;
				}

				AvContext->SyncContext->Playback = AvContext->Playback;
				if (AvContext->PlaySpeed != DVB_SPEED_NORMAL_PLAY)
				{
					Result = VideoIoctlSetSpeed (AvContext, AvContext->PlaySpeed);
					if (Result < 0)
					{
						mutex_unlock (&(DvbContext->Lock));
						return Result;
					}
				}
				if ((AvContext->PlayInterval.start != DVB_TIME_NOT_BOUNDED) ||
					(AvContext->PlayInterval.end != DVB_TIME_NOT_BOUNDED))
				{
					Result =
					VideoIoctlSetPlayInterval (AvContext, &AvContext->PlayInterval);
					if (Result < 0)
					{
						mutex_unlock (&(DvbContext->Lock));
						return Result;
					}
				}
			}
			else if (AvContext->Playback == NULL)
			{
				AvContext->Playback = AvContext->SyncContext->Playback;
			}
			else if (AvContext->SyncContext->Playback == NULL)
			{
				AvContext->SyncContext->Playback = AvContext->Playback;
			}
			else if (AvContext->Playback != AvContext->SyncContext->Playback)
				dprintk(1,"%s: ERROR - Context playback not equal to sync context playback\n",__func__);

			if (AvContext->DemuxStream == NULL)
			{
				Result =
					PlaybackAddDemux (AvContext->Playback, AvContext->DemuxContext->Id,
							&AvContext->DemuxStream);

				if (Result < 0)
				{
					mutex_unlock (&(DvbContext->Lock));
					return Result;
				}
			}

			if (Video)
			{
				dprintk(2,"%s: VIDEO ==> Feed->type=DMX_TYPE_TS Feed->ts_type=%d Feed->pes_type =%d\n", __func__, Feed->ts_type,Feed->pes_type);
				stpti_start_feed (Feed, Context);

				if(Feed->ts_type & TS_DECODER)
					VideoIoctlSetId (AvContext, Feed->pid);
			}
			else if (Audio)
			{
				dprintk(2,"%s: AUDIO ==> Feed->type=DMX_TYPE_TS Feed->ts_type=%d Feed->pes_type =%d\n", __func__, Feed->ts_type,Feed->pes_type);
				stpti_start_feed (Feed, Context);

				if(Feed->ts_type & TS_DECODER)
					AudioIoctlSetId (AvContext, Feed->pid);
			}
			mutex_unlock (&(DvbContext->Lock));
		}
		break;
		default:
			dprintk(1,"%s: < (type = %d unknown)\n", __func__, Feed->type);
			return -EINVAL;
	}

	dprintk(2,"%s: <\n", __func__);
  return 0;
}

/*}}}  */
#endif
/*{{{  StopFeed*/
/********************************************************************************
*  \brief      Shut down this feed
*              StopFeed is called by the demux device immediately after finishing
*              demuxing data.
********************************************************************************/
int StopFeed (struct dvb_demux_feed *Feed)
{
    struct dvb_demux *DvbDemux = Feed->demux;
    struct DeviceContext_s* Context = (struct DeviceContext_s*)DvbDemux->priv;
    struct DeviceContext_s* AvContext = NULL;
    struct DvbContext_s* DvbContext      = Context->DvbContext;
    int i;
	int Result = 0;
	unsigned int Video = false;
	unsigned int Audio = false;

//    DVB_DEBUG(">\n");

    switch (Feed->type)
    {
        case DMX_TYPE_SEC:
			DVB_DEBUG ("feed type = SEC\n");
			mutex_lock (&(DvbContext->Lock));
			stpti_stop_feed(Feed, Context);
			mutex_unlock (&(DvbContext->Lock));

		break;
		case DMX_TYPE_TS:
			DVB_DEBUG ("feed type = TS\n");
            for (i = 0; i < DVB_MAX_DEVICES_PER_ADAPTER; i++)
            {
                if (Feed->pes_type == AudioId[i])
                {
            	    mutex_lock (&(DvbContext->Lock));

					AvContext = &Context->DvbContext->DeviceContext[i];

		    /*if(Feed->ts_type & TS_DECODER)
					{
						AudioIoctlSetAvSync (AvContext, 0);
						AudioIoctlStop (AvContext);
		    }*/

					stpti_stop_feed(Feed, Context);

					mutex_unlock (&(DvbContext->Lock));

					Audio = true;
					break;
				}
				if (Feed->pes_type == VideoId[i])
				{
					mutex_lock (&(DvbContext->Lock));

					AvContext = &Context->DvbContext->DeviceContext[i];

		    /*if(Feed->ts_type & TS_DECODER)
		      VideoIoctlStop(AvContext, AvContext->VideoState.video_blank);*/

					stpti_stop_feed(Feed, Context);

					mutex_unlock (&(DvbContext->Lock));

					Video = true;
					break;
				}
				//videotext & subtitles (other)
				// FIXME: TTX1, TTX2, TTX3, PCR1 etc.
				if ((Feed->pes_type == DMX_TS_PES_TELETEXT) ||
					(Feed->pes_type == DMX_TS_PES_OTHER))
				{
					mutex_lock (&(DvbContext->Lock));
					stpti_stop_feed(Feed, Context);

		    		mutex_unlock (&(DvbContext->Lock));
					break;
				}
				else if (Feed->pes_type == DMX_TS_PES_PCR)
					break;
			}

			if (Audio) {
				if (AvContext) {
					if (AvContext->DemuxStream != NULL && AvContext->VideoStream == NULL) {
						mutex_lock (&(DvbContext->Lock));
						Result = PlaybackRemoveDemux(AvContext->Playback, AvContext->DemuxStream);
						AvContext->DemuxStream = NULL;
						//AvContext->Playback = NULL;
						//AvContext->SyncContext->Playback = NULL;
						mutex_unlock (&(DvbContext->Lock));
						if (Result < 0)
							return Result;
					}
				}
			}

            if (i >= DVB_MAX_DEVICES_PER_ADAPTER)
            {
				printk("%s(): INVALID PES TYPE (%d, %d)\n", __func__,
					   Feed->pid, Feed->pes_type);
                return -EINVAL;
            }

            break;
        default:
	    	printk("%s(): INVALID FEED TYPE (%d)\n", __func__, Feed->type);
            return -EINVAL;
    }

//     DVB_DEBUG("<\n");
    return 0;
}
/*}}}  */

/* Uncomment the define to enable player decoupling from the DVB API.
   With this workaround packets sent to the player do not block the DVB API
   and do not cause the scheduling bug (waiting on buffers during spin_lock).
   However, there is a side effect - playback may disturb recordings. */
#define DECOUPLE_PLAYER_FROM_DVBAPI
#ifndef DECOUPLE_PLAYER_FROM_DVBAPI

/*{{{  WriteToDecoder*/
int WriteToDecoder (struct dvb_demux_feed *Feed, const u8 *buf, size_t count)
{
	struct dvb_demux* demux = Feed->demux;
	struct DeviceContext_s* Context = (struct DeviceContext_s*)demux->priv;
	int j = 0;
	int audio = 0;

	if(Feed->type != DMX_TYPE_TS)
		return 0;

	/* select the context */
	/* no more than two output devices supported */
	switch (Feed->pes_type)
	{
		case DMX_PES_AUDIO0:
			audio = 1;
		case DMX_PES_VIDEO0:
			Context = &Context->DvbContext->DeviceContext[0];
		break;
		case DMX_PES_AUDIO1:
			audio = 1;
		case DMX_PES_VIDEO1:
			Context = &Context->DvbContext->DeviceContext[1];
		break;
		default:
			return 0;
	}

	/* injecting scrambled data crashes the player */
	while (j < count)
	{
		if ((buf[j+3] & 0xc0) > 0)
			return count;
		j+=188;
	}

	/* don't inject if playback is stopped */
	if (audio == 1)
	{
		if (Context->AudioState.play_state == AUDIO_STOPPED)
			return count;
	}
	else if (Context->VideoState.play_state == VIDEO_STOPPED)
			return count;

	return StreamInject(Context->DemuxContext->DemuxStream, buf, count);
}

void demultiplexDvbPackets(struct dvb_demux* demux, const u8 *buf, int count)
{
	dvb_dmx_swfilter_packets(demux, buf, count);
}

#else

/*{{{  WriteToDecoder*/
int WriteToDecoder (struct dvb_demux_feed *Feed, const u8 *buf, size_t count)
{
  struct dvb_demux* demux = Feed->demux;
  struct DeviceContext_s* Context = (struct DeviceContext_s*)demux->priv;

	/* The decoder needs only the video and audio PES.
		For whatever reason the demux provides the video packets twice
		(once as PES_VIDEO and then as PES_PCR). Therefore it is IMPORTANT
		not to overwrite the flag or the PES type. */
	if((Feed->type == DMX_TYPE_TS) &&
		((Feed->pes_type == DMX_PES_AUDIO0) ||
		(Feed->pes_type == DMX_PES_VIDEO0) ||
		(Feed->pes_type == DMX_PES_AUDIO1) ||
		(Feed->pes_type == DMX_PES_VIDEO1)))
	{
		Context->provideToDecoder = 1;
		Context->feedPesType = Feed->pes_type;
	}

	return 0;
}
/*}}}  */


int writeToDecoder (struct dvb_demux *demux, int pes_type, const u8 *buf, size_t count)
{
	struct DeviceContext_s* Context = (struct DeviceContext_s*)demux->priv;
	int j = 3;

	/* select the context */
	/* no more than two output devices supported */
  /* don't inject if playback is stopped */
	switch (pes_type)
	{
		case DMX_PES_AUDIO0:
			Context = &Context->DvbContext->DeviceContext[0];
			if (Context->AudioState.play_state == AUDIO_STOPPED)
				return count;
		break;
		case DMX_PES_VIDEO0:
			Context = &Context->DvbContext->DeviceContext[0];
			if (Context->VideoState.play_state == VIDEO_STOPPED)
				return count;
		break;
		case DMX_PES_AUDIO1:
			Context = &Context->DvbContext->DeviceContext[1];
			if (Context->AudioState.play_state == AUDIO_STOPPED)
				return count;
		break;
		case DMX_PES_VIDEO1:
			Context = &Context->DvbContext->DeviceContext[1];
			if (Context->VideoState.play_state == VIDEO_STOPPED)
				return count;
		break;
		default:
			return 0;
	}

	/* injecting scrambled data crashes the player */
	while (j < count)
	{
		if ((buf[j] & 0xc0) > 0)
		{
			dprintk(5,"%s: dropping scrambled data\n",__func__);
			return count;
		}
		j+=188;
	}

#if defined(CONFIG_SH_QBOXHD_1_0) || defined(CONFIG_SH_QBOXHD_MINI_1_0)
	if((((buf[1] & 0x1f) << 8) + buf[2]) == testPid)
		if(testPid_freq>0 && testPid_freq==injectCount++)
		{
			injectCount=0;
			dprintk(4,"%s: injecting packet: [0]%02x [1]%02x [2]%02x [3]%02x >\n",
				__func__,buf[0],buf[1],buf[2],buf[3]);
		}
#endif
  return StreamInject(Context->DemuxContext->DemuxStream, buf, count);
}


static inline u16 ts_pid(const u8 *buf)
{
        return ((buf[1] & 0x1f) << 8) + buf[2];
}

void demultiplexDvbPackets(struct dvb_demux* demux, const u8 *buf, int count)
{
	int first = 0;
	int next = 0;
	int cnt = 0;
	u16 pid, firstPid;
	struct DeviceContext_s* Context = (struct DeviceContext_s*)demux->priv;

	/* Group the packets by the PIDs and feed them into the kernel demuxer.
		If there is data for the decoder we will be informed via the callback.
		After the demuxer finished its work on the packet block that block is
		fed into the decoder if required.
		This workaround eliminates the scheduling bug caused by waiting while
		the demux spin is locked. */
	while (count > 0)
	{
		int wrBytes=0;
		first = next;
		cnt = 0;
		firstPid = ts_pid(&buf[first]);

		while(count > 0)
		{
			count--;
			next += 188;
			cnt++;
			pid = ts_pid(&buf[next]);

#if defined(CONFIG_SH_QBOXHD_1_0) || defined(CONFIG_SH_QBOXHD_MINI_1_0)
			//if scrambled data
			if ( buf[next]==0x47 && (buf[next+3]&0xC0) && (buf[next+3]&0x0F)==0x0F )
				dprintk(4,"%s: 0x%04x =>buf[0]=0x%02x buf[3]=0x%02x [%01x|%01x]\n",
						__func__,pid,buf[next],buf[next+3],(buf[next+3]&0xC0)>>4, (buf[next+3]&0x0F));

			if(pid == testPid)
			{
				if(testPid_freq>0 && testPid_freq==demuxCount++)
				{
					demuxCount=0;
					dprintk(5,"%s: pck: [0]%02x [1]%02x [2]%02x [3]%02x\n",
							__func__,buf[next+0],buf[next+1],buf[next+2],buf[next+3]);
				}
			}
			else
				dprintk(6,"%s: buf:[0]%02x [1]%02x [2]%02x [3]%02x\n",
						__func__,buf[next+0],buf[next+1],buf[next+2],buf[next+3]);
#endif
			if((pid != firstPid) || (cnt > 8))
				break;
		}

		wrBytes=(next - first);

		//if((next - first) > 0)
		if(wrBytes > 0)
		{
			mutex_lock_interruptible(&Context->injectMutex);

			/* reset the flag (to be set by the callback */
			Context->provideToDecoder = 0;

			dvb_dmx_swfilter_packets(demux, buf + first, cnt);
			if(Context->provideToDecoder)
			{
				/* the demuxer indicated that the packets are for the decoder */
				// writeToDecoder(demux, Context->feedPesType, buf + first, next - first);
				writeToDecoder(demux, Context->feedPesType, buf + first, wrBytes);
			}
			mutex_unlock(&Context->injectMutex);
		}
	}
}
#endif
