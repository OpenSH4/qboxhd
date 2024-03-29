/************************************************************************
COPYRIGHT (C) STMicroelectronics 2005

Source file name : dvb_audio.c
Author :           Julian

Implementation of linux dvb audio device

Date        Modification                                    Name
----        ------------                                    --------
24-Mar-05   Created                                         Julian

************************************************************************/

#include <linux/module.h>
#include <linux/dvb/video.h>
#include <linux/dvb/audio.h>

#include "dvb_module.h"
#include "dvb_audio.h"
#include "dvb_video.h"
#include "backend.h"

#include "dvb_v4l2.h"

#ifdef CONFIG_STM_TKDMA
#include <linux/stm/tkdma.h>
#endif

#ifdef __TDT__
//Dagobert Hack
#include <sound/driver.h>
#include <sound/core.h>
#include <sound/control.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/rawmidi.h>
#include <sound/initval.h>

#include <stm_ioctls.h>

#include "e2_proc/e2_proc.h" 

#include "../../../../sound/pseudocard/pseudo_mixer.h"

#if defined(CONFIG_SH_QBOXHD_1_0) || defined(CONFIG_SH_QBOXHD_MINI_1_0)
extern unsigned int hdmi_audio_source_ply;
#endif

extern struct snd_kcontrol ** pseudoGetControls(int* numbers);
extern int snd_pseudo_switch_put(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol);
extern int snd_pseudo_integer_put(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol);

#define PSEUDO_ADDR(x) (offsetof(struct snd_pseudo_mixer_settings, x))
extern struct DeviceContext_s* DeviceContext;
#endif

/*{{{  prototypes*/
static int AudioOpen                    (struct inode           *Inode,
                                         struct file            *File);
static int AudioRelease                 (struct inode           *Inode,
                                         struct file            *File);
static int AudioIoctl                   (struct inode           *Inode,
                                         struct file            *File,
                                         unsigned int            IoctlCode,
                                         void                   *ParamAddress);
static ssize_t AudioWrite               (struct file            *File,
                                         const char __user      *Buffer,
                                         size_t                  Count,
                                         loff_t                 *ppos);
static unsigned int AudioPoll           (struct file            *File,
                                         poll_table             *Wait);
#ifdef __TDT__
int AudioIoctlSetAvSync (struct DeviceContext_s* Context, unsigned int State);
#else
static int AudioIoctlSetAvSync          (struct DeviceContext_s* Context,
                                         unsigned int            State);
#endif
static int AudioIoctlChannelSelect      (struct DeviceContext_s* Context,
                                         audio_channel_select_t  Channel);
static int AudioIoctlSetSpeed           (struct DeviceContext_s* Context,
                                         int                     Speed);
static int AudioIoctlSetPlayInterval    (struct DeviceContext_s* Context,
                                         dvb_play_interval_t*    PlayInterval);
/*}}}  */
/*{{{  static data*/
static char* AudioContent[]     =
{
    [AUDIO_ENCODING_AUTO]       = BACKEND_AUTO_ID,
    [AUDIO_ENCODING_LPCM]       = BACKEND_LPCM_ID,
    [AUDIO_ENCODING_LPCM]       = BACKEND_LPCM_ID,
    [AUDIO_ENCODING_MPEG1]      = BACKEND_MPEG1_ID,
    [AUDIO_ENCODING_MPEG2]      = BACKEND_MPEG2_ID,
    [AUDIO_ENCODING_MP3]        = BACKEND_MP3_ID,
    [AUDIO_ENCODING_AC3]        = BACKEND_AC3_ID,
    [AUDIO_ENCODING_DTS]        = BACKEND_DTS_ID,
    [AUDIO_ENCODING_AAC]        = BACKEND_AAC_ID,
    [AUDIO_ENCODING_WMA]        = BACKEND_WMA_ID,
    [AUDIO_ENCODING_RAW]        = BACKEND_RAW_ID,
    [AUDIO_ENCODING_LPCMA]      = BACKEND_LPCMA_ID,
    [AUDIO_ENCODING_LPCMH]      = BACKEND_LPCMH_ID,
    [AUDIO_ENCODING_LPCMB]      = BACKEND_LPCMB_ID,
    [AUDIO_ENCODING_SPDIF]      = BACKEND_SPDIFIN_ID,
    [AUDIO_ENCODING_DTS_LBR]    = BACKEND_DTS_LBR_ID,
    [AUDIO_ENCODING_MLP]        = BACKEND_MLP_ID,
    [AUDIO_ENCODING_RMA]        = BACKEND_RMA_ID,
    [AUDIO_ENCODING_AVS]        = BACKEND_AVS_ID,
    [AUDIO_ENCODING_NONE]       = BACKEND_NONE_ID
};


static struct file_operations AudioFops =
{
        owner:          THIS_MODULE,
        write:          AudioWrite,
        unlocked_ioctl: DvbGenericUnlockedIoctl,
        open:           AudioOpen,
        release:        AudioRelease,
        poll:           AudioPoll,
};

static struct dvb_device AudioDevice =
{
        priv:            NULL,
        users:           8,
        readers:         7,
        writers:         1,
        fops:            &AudioFops,
        kernel_ioctl:    AudioIoctl,
};
/*}}}  */
/*{{{  AudioInit*/
struct dvb_device* AudioInit (struct DeviceContext_s* Context)
{
    Context->AudioState.AV_sync_state           = 1;
    Context->AudioState.mute_state              = 0;
    Context->AudioState.play_state              = AUDIO_STOPPED;
    Context->AudioState.stream_source           = AUDIO_SOURCE_DEMUX;
    Context->AudioState.channel_select          = AUDIO_STEREO;
    Context->AudioState.bypass_mode             = false;        /*the sense is inverted so this really means off*/

    Context->AudioId                            = DEMUX_INVALID_ID;     /* CodeToInteger('a','u','d','s'); */
    Context->AudioEncoding                      = AUDIO_ENCODING_AUTO;
    Context->AudioStream                        = NULL;
    Context->AudioOpenWrite                     = 0;

    mutex_init (&(Context->AudioWriteLock));
    Context->ActiveAudioWriteLock               = &(Context->AudioWriteLock);

    return &AudioDevice;
}
/*}}}  */
/*{{{  Ioctls*/
/*{{{  AudioIoctlStop*/
#if defined(CONFIG_SH_QBOXHD_1_0) || defined(CONFIG_SH_QBOXHD_MINI_1_0)

#define STMHDMIIO_AUDIO_SOURCE_2CH_I2S (0)
#define STMHDMIIO_AUDIO_SOURCE_PCM     STMHDMIIO_AUDIO_SOURCE_2CH_I2S
#define STMHDMIIO_AUDIO_SOURCE_SPDIF   (1)
#define STMHDMIIO_AUDIO_SOURCE_8CH_I2S (2)
#define STMHDMIIO_AUDIO_SOURCE_NONE    (0xffffffff)

extern long stmhdmiio_set_audio_source(unsigned int arg);
extern long stmhdmiio_get_audio_source(unsigned int *arg);

#endif

int AudioIoctlStop (struct DeviceContext_s* Context)
{
    int Result  = 0;

    DVB_DEBUG("\n");

    /*if (Context->AudioState.play_state == AUDIO_PLAYING)*/
    if (Context->AudioStream != NULL)
    {
        struct mutex*   WriteLock       = Context->ActiveAudioWriteLock;
#if defined(CONFIG_SH_QBOXHD_1_0) || defined(CONFIG_SH_QBOXHD_MINI_1_0)
	if ( hdmi_audio_source_ply == STMHDMIIO_AUDIO_SOURCE_PCM )
		stmhdmiio_set_audio_source(STMHDMIIO_AUDIO_SOURCE_NONE);
#endif
        /* Discard previously injected data to free the lock. */
		StreamDrain (Context->AudioStream, true);

        /*mutex_lock (WriteLock);*/
        if (mutex_lock_interruptible (WriteLock) != 0)
            return -ERESTARTSYS;                /* Give up for now.  The stream will be removed later by the release */

        /*StreamEnable (Context->PlayerContext, STREAM_CONTENT_AUDIO, false);*/
        Result  = PlaybackRemoveStream (Context->Playback, Context->AudioStream);
        if (Result == 0)
        {
            Context->AudioStream                = NULL;
            Context->AudioState.play_state      = AUDIO_STOPPED;
            Context->ActiveAudioWriteLock       = &(Context->AudioWriteLock);
            /*AudioInit (Context);*/
        }
        mutex_unlock (WriteLock);
    }
    DVB_DEBUG("Play state = %d\n", Context->AudioState.play_state);

    return Result;
}
/*}}}  */
/*{{{  AudioIoctlPlay*/
int AudioIoctlPlay (struct DeviceContext_s* Context)
{
    int Result  = 0;
    sigset_t newsigs, oldsigs;

	//struct DvbContext_s * DvbContext = Context->DvbContext;
	//struct dvb_device * DvbDevice = &DvbContext->DvbDevice;
	//struct dvb_adapter * DvbAdapter = &DvbContext->DvbAdapter;

	//DVB_DEBUG("Device: %d, Adapter: %d\n", DvbDevice->id, DvbAdapter->num);

    DVB_DEBUG("\n");

    if ((Context->AudioState.play_state == AUDIO_STOPPED) || (Context->AudioState.play_state == AUDIO_INCOMPLETE))
    {
        if (Context->Playback == NULL)
        {
            /*
               Check to see if we are wired to a demux.  If so the demux should create the playback and we will get
                another play call.  Just exit in this case.  If we are playing from memory we need to create a playback.
            */
            if (Context->AudioState.stream_source == AUDIO_SOURCE_DEMUX)
            {
                if (Context->DemuxContext->Playback == NULL)
                    return 0;
                else
                    Context->Playback   = Context->DemuxContext->Playback;
            }
            else if (Context->SyncContext->Playback == NULL)
            {
                Result      = PlaybackCreate (&Context->Playback);
                if (Result < 0)
                    return Result;
                if (Context->PlaySpeed != DVB_SPEED_NORMAL_PLAY)
                {
                    Result      = AudioIoctlSetSpeed (Context, Context->PlaySpeed);
                    if (Result < 0)
                        return Result;
                }
                if ((Context->PlayInterval.start != DVB_TIME_NOT_BOUNDED) ||
                    (Context->PlayInterval.end   != DVB_TIME_NOT_BOUNDED))
                {
                    Result      = AudioIoctlSetPlayInterval (Context, &Context->PlayInterval);
                    if (Result < 0)
                        return Result;
                }
                Context->SyncContext->Playback  = Context->Playback;
            }
            else
                Context->Playback       = Context->SyncContext->Playback;
        }
        PlaybackInit (Context);

        if ((Context->AudioStream == NULL) || (Context->AudioState.play_state == AUDIO_INCOMPLETE))
        {
            unsigned int DemuxId = (Context->AudioState.stream_source == AUDIO_SOURCE_DEMUX) ? Context->DemuxContext->Id : DEMUX_INVALID_ID;
            /* a signal received in here can cause issues! Lets turn them off, just for this bit... */ 
            sigfillset(&newsigs);
            sigprocmask(SIG_BLOCK, &newsigs, &oldsigs);

			Result = PlaybackAddStream    (Context->Playback,
					BACKEND_AUDIO_ID,
					BACKEND_PES_ID,
					AudioContent[Context->AudioEncoding],
					DemuxId,
					Context->Id,
					&Context->AudioStream);

            sigprocmask(SIG_SETMASK, &oldsigs, NULL);

            if (Result == STREAM_INCOMPLETE)
            {
                Context->AudioState.play_state    = AUDIO_INCOMPLETE;
                return 0;
            }

			if (Result == 0)
				Result  = AudioIoctlSetId     (Context, Context->AudioId);
			if (Result == 0)
				Result  = AudioIoctlSetAvSync (Context, Context->AudioState.AV_sync_state);
			if (Result == 0)
				Result  = AudioIoctlChannelSelect (Context, Context->AudioState.channel_select);
            /*
            If we are connected to a demux we will want to use the VIDEO write lock of the demux device
            (which could be us).
            */
            if ((Result == 0) && (Context->AudioState.stream_source == AUDIO_SOURCE_DEMUX))
                Context->ActiveAudioWriteLock   = &(Context->DemuxContext->VideoWriteLock);
        }
    }
    else
    {
        /* Play is used implicitly to exit slow motion and fast forward states so
           set speed to times 1 if audio is playing or has been paused */
        Result  = AudioIoctlSetSpeed (Context, DVB_SPEED_NORMAL_PLAY);
    }

    if (Result == 0)
    {
	StreamEnable (Context->AudioStream, true);
        Context->AudioState.play_state    = AUDIO_PLAYING;
	
#if defined(CONFIG_SH_QBOXHD_1_0) || defined(CONFIG_SH_QBOXHD_MINI_1_0)
	if ( hdmi_audio_source_ply == STMHDMIIO_AUDIO_SOURCE_PCM ) {
		stmhdmiio_set_audio_source( hdmi_audio_source_ply );
		printk("HDMI audio source SET: %d\n",hdmi_audio_source_ply);
	}
#endif
    }
    DVB_DEBUG("State = %d\n", Context->AudioState.play_state);

    return Result;
}
/*}}}  */
/*{{{  AudioIoctlPause*/
#ifdef __TDT__
int AudioIoctlPause (struct DeviceContext_s* Context)
#else
static int AudioIoctlPause (struct DeviceContext_s* Context)
#endif
{
    int Result  = 0;

    DVB_DEBUG("\n");
    if (Context->AudioState.play_state == AUDIO_PLAYING)
    {
        Result  = AudioIoctlSetSpeed (Context, DVB_SPEED_STOPPED);
        if (Result < 0)
            return Result;
        Context->AudioState.play_state  = AUDIO_PAUSED;
    }

    return 0;
}
/*}}}  */
/*{{{  AudioIoctlContinue*/
#ifdef __TDT__
int AudioIoctlContinue (struct DeviceContext_s* Context)
#else
static int AudioIoctlContinue (struct DeviceContext_s* Context)
#endif
{
    int Result  = 0;

    DVB_DEBUG("\n");
    if ((Context->AudioState.play_state == AUDIO_PAUSED) || (Context->AudioState.play_state == AUDIO_PLAYING))
    {
        Result  = AudioIoctlSetSpeed (Context, DVB_SPEED_NORMAL_PLAY);
        if (Result < 0)
            return Result;
        Context->AudioState.play_state  = AUDIO_PLAYING;
    }

    return 0;
}
/*}}}  */
/*{{{  AudioIoctlSelectSource*/
static int AudioIoctlSelectSource (struct DeviceContext_s* Context, audio_stream_source_t Source)
{
    DVB_DEBUG("\n");
    Context->AudioState.stream_source = Source;
    if (Source == AUDIO_SOURCE_DEMUX)
        Context->StreamType = STREAM_TYPE_TRANSPORT;
    else
        Context->StreamType = STREAM_TYPE_PES;
    DVB_DEBUG("Source = %x\n", Context->AudioState.stream_source);

    return 0;
}
/*}}}  */
/*{{{  AudioIoctlSetMute*/
static int AudioIoctlSetMute (struct DeviceContext_s* Context, unsigned int State)
{
    int Result  = 0;

    DVB_DEBUG("Mute = %d (was %d)\n", State, Context->AudioState.mute_state);
    if (Context->AudioStream != NULL)
        Result  = StreamEnable (Context->AudioStream, !State);

    if (Result == 0)
        Context->AudioState.mute_state  = (int)State;


    return Result;
}
/*}}}  */
/*{{{  AudioIoctlSetAvSync*/
#ifdef __TDT__
int AudioIoctlSetAvSync (struct DeviceContext_s* Context, unsigned int State)
#else
static int AudioIoctlSetAvSync (struct DeviceContext_s* Context, unsigned int State)
#endif
{
    int Result  = 0;

    DVB_DEBUG("\n");
    /* implicitly un-mute audio when we re-enable AV sync */
    if (Context->AudioState.play_state == AUDIO_PLAYING)
        StreamEnable (Context->AudioStream, true);

    if (Context->AudioStream != NULL)
        Result  = StreamSetOption (Context->AudioStream, PLAY_OPTION_AV_SYNC, State ? PLAY_OPTION_VALUE_ENABLE : PLAY_OPTION_VALUE_DISABLE);

    if (Result != 0)
        return Result;

    Context->AudioState.AV_sync_state = (int)State;
    DVB_DEBUG("AV Sync = %d\n", Context->AudioState.AV_sync_state);

    return Result;
}
/*}}}  */
/*{{{  AudioIoctlSetBypassMode*/
#ifdef __TDT__
int AudioIoctlSetBypassMode (struct DeviceContext_s* Context, unsigned int Mode)
#else
static int AudioIoctlSetBypassMode (struct DeviceContext_s* Context, unsigned int Mode)
#endif
{
#ifdef __TDT__
    int vLoop;
    int number = 0;
    struct snd_kcontrol *single_control = NULL;
    struct snd_kcontrol ** kcontrol = pseudoGetControls(&number);

    //DVB_DEBUG("(not implemented)\n");
    //mpeg2=4, ac3=6
    //e2 : 1=mpegm 0=ac3, 2=dts, 8=aac, 9=aache, 6=lpcm
    //libdreamdvd : 5=dts 6=lpcm
    DVB_DEBUG("Set BypassMode to %d\n", Mode);
    
    if (Mode == 0)
			Context->AudioEncoding      = (audio_encoding_t) AUDIO_ENCODING_AC3;
    else if (Mode == 5 || Mode == 2)
      Context->AudioEncoding      = (audio_encoding_t) AUDIO_ENCODING_DTS;
    else if (Mode == 6)
      Context->AudioEncoding      = (audio_encoding_t) AUDIO_ENCODING_LPCM;
    else if (Mode == 8)
      Context->AudioEncoding      = (audio_encoding_t) AUDIO_ENCODING_AAC;
    else
      Context->AudioEncoding      = (audio_encoding_t) AUDIO_ENCODING_MPEG2;

//before we jump to any conclusions, does the user really want passtrough, its possible that he wants downmix!
//ask e2_proc_audio what the user wants

    for (vLoop = 0; vLoop < number; vLoop++) {
        if (kcontrol[vLoop]->private_value == PSEUDO_ADDR(spdif_bypass)) {
            single_control = kcontrol[vLoop];
            DVB_DEBUG("Find spdif_bypass control at %p\n", single_control);
            break;
        }		
    }	

    if ((kcontrol != NULL) && (single_control != NULL)) {
        struct snd_ctl_elem_value ucontrol;
        ucontrol.value.integer.value[0] = e2_proc_audio_getPassthrough();

        snd_pseudo_switch_put(single_control, &ucontrol);

    } else {
        printk("Pseudo Mixer does not deliver controls\n");
    }

#ifdef use_hdmi_bypass
/* Dagobert; set hdmi bypass, maybe we need another prco for this? */
    for (vLoop = 0; vLoop < number; vLoop++) {
        if (kcontrol[vLoop]->private_value == PSEUDO_ADDR(hdmi_bypass)) {
            single_control = kcontrol[vLoop];
            printk("Find hdmi_bypass control at %p\n", single_control);
            break;
        }		
    }

    if ((kcontrol != NULL) && (single_control != NULL)) {
        struct snd_ctl_elem_value ucontrol;
        ucontrol.value.integer.value[0] = e2_proc_audio_getPassthrough();

        snd_pseudo_switch_put(single_control, &ucontrol);

    } else {
        printk("Pseudo Mixer does not deliver controls\n");
    }
#endif

    return 0;
#else
    DVB_DEBUG("(not implemented)\n");
    return -EPERM;
#endif
}
/*}}}  */
/*{{{  AudioIoctlChannelSelect*/
static int AudioIoctlChannelSelect (struct DeviceContext_s* Context, audio_channel_select_t Channel)
{
    int                 Result  = 0;

    DVB_DEBUG("Channel = %x\n", Channel);
    if (Context->AudioStream != NULL)
        Result  = StreamChannelSelect (Context->AudioStream, (channel_select_t)Channel);

    if (Result != 0)
        return Result;

    Context->AudioState.channel_select  = Channel;

    return Result;
}
/*}}}  */
/*{{{  AudioIoctlGetStatus*/
static int AudioIoctlGetStatus (struct DeviceContext_s* Context, struct audio_status* Status)
{
    memcpy (Status, &Context->AudioState, sizeof(struct audio_status));
    DVB_DEBUG("\n");

    return 0;
}
/*}}}  */
/*{{{  AudioIoctlGetCapabilities*/
static int AudioIoctlGetCapabilities (struct DeviceContext_s* Context, int* Capabilities)
{
    *Capabilities = AUDIO_CAP_DTS | AUDIO_CAP_LPCM | AUDIO_CAP_MP1 | AUDIO_CAP_MP2 | AUDIO_CAP_MP3 | AUDIO_CAP_AAC | AUDIO_CAP_AC3;
    DVB_DEBUG("Capabilities returned = %x\n", *Capabilities);

    return 0;
}
/*}}}  */
/*{{{  AudioIoctlGetPts*/
static int AudioIoctlGetPts (struct DeviceContext_s* Context, unsigned long long int* Pts)
{
    int                 Result;
    struct play_info_s  PlayInfo;

    /*DVB_DEBUG("\n");*/
    Result      = StreamGetPlayInfo (Context->AudioStream, &PlayInfo);
    if (Result != 0)
        return Result;

    *Pts        = PlayInfo.pts;

    return Result;
}
/*}}}  */
/*{{{  AudioIoctlGetPlayTime*/
static int AudioIoctlGetPlayTime (struct DeviceContext_s* Context, audio_play_time_t* AudioPlayTime)
{
    struct play_info_s  PlayInfo;

    if (Context->AudioStream == NULL)
        return -EINVAL;

    StreamGetPlayInfo (Context->AudioStream, &PlayInfo);

    AudioPlayTime->system_time          = PlayInfo.system_time;
    AudioPlayTime->presentation_time    = PlayInfo.presentation_time;
    AudioPlayTime->pts                  = PlayInfo.pts;

    return 0;
}
/*}}}  */
/*{{{  AudioIoctlGetPlayInfo*/
static int AudioIoctlGetPlayInfo (struct DeviceContext_s* Context, audio_play_info_t* AudioPlayInfo)
{
    struct play_info_s  PlayInfo;

    if (Context->AudioStream == NULL)
        return -EINVAL;

    StreamGetPlayInfo (Context->AudioStream, &PlayInfo);

    AudioPlayInfo->system_time          = PlayInfo.system_time;
    AudioPlayInfo->presentation_time    = PlayInfo.presentation_time;
    AudioPlayInfo->pts                  = PlayInfo.pts;
    AudioPlayInfo->frame_count          = PlayInfo.frame_count;

    return 0;
}
/*}}}  */
/*{{{  AudioIoctlClearBuffer*/
#ifdef __TDT__
int AudioIoctlClearBuffer (struct DeviceContext_s* Context)
#else
static int AudioIoctlClearBuffer (struct DeviceContext_s* Context)
#endif
{
    int                 Result  = 0;
#ifdef __TDT__
    int prevSpeed = 0;
#endif
    dvb_discontinuity_t Discontinuity   = DVB_DISCONTINUITY_SKIP | DVB_DISCONTINUITY_SURPLUS_DATA;

    DVB_DEBUG("\n");

#ifdef __TDT__
    prevSpeed = Context->AudioState.play_state;
#endif

    /* Discard previously injected data to free the lock. */
    StreamDrain (Context->AudioStream, true);

    /*mutex_lock (Context->ActiveAudioWriteLock);*/
    if (mutex_lock_interruptible (Context->ActiveAudioWriteLock) != 0)
        return -ERESTARTSYS;

    StreamDrain (Context->AudioStream, true);
    if (Result == 0)
        Result  = StreamDiscontinuity (Context->AudioStream, Discontinuity);
    mutex_unlock (Context->ActiveAudioWriteLock);

#ifdef __TDT__
    //it seems like the player forgets our current status after clear buffer
    if(prevSpeed == AUDIO_PAUSED)
    {
      AudioIoctlSetSpeed (Context, DVB_SPEED_STOPPED);
    }
#endif
    return Result;
}
/*}}}  */
/*{{{  AudioIoctlSetId*/
int AudioIoctlSetId (struct DeviceContext_s* Context, int Id)
{
    int                 Result  = 0;
    unsigned int        DemuxId = (Context->AudioState.stream_source == AUDIO_SOURCE_DEMUX) ? Context->DemuxContext->Id : DEMUX_INVALID_ID;

    DVB_DEBUG("Setting Audio Id to %04x\n", Id);

	if (Context->AudioStream != NULL)
		Result  = StreamSetId (Context->AudioStream, DemuxId, Id);

    if (Result != 0)
        return Result;

    Context->AudioId    = Id;

    return Result;
}
/*}}}  */
/*{{{  AudioIoctlSetMixer*/
static int AudioIoctlSetMixer (struct DeviceContext_s* Context, audio_mixer_t* Mix)
{
    DVB_DEBUG("Volume left = %d, Volume right = %d (not yet implemented)\n", Mix->volume_left, Mix->volume_right);

#ifdef __TDT__
/* Dagobert: 04.10.2009: Do nothing here because we do this in
 * e2_proc_avs also.
 */
    return 0;
#else
    return -EPERM;
#endif
}
/*}}}  */
/*{{{  AudioIoctlSetStreamType*/
static int AudioIoctlSetStreamType(struct DeviceContext_s* Context, unsigned int Type)
{
    DVB_DEBUG("Set type to %x\n", Type);

    /*if ((Type < STREAM_TYPE_TRANSPORT) || (Type > STREAM_TYPE_PES))*/
    if ((Type < STREAM_TYPE_NONE) || (Type > STREAM_TYPE_RAW))
        return  -EINVAL;

    Context->StreamType         = (stream_type_t)Type;

    return 0;
}
/*}}}  */
/*{{{  AudioIoctlSetExtId*/
static int AudioIoctlSetExtId (struct DeviceContext_s* Context, int Id)
{
    DVB_ERROR("Not supported\n");
    return -EPERM;
}
/*}}}  */
/*{{{  AudioIoctlSetAttributes*/
static int AudioIoctlSetAttributes (struct DeviceContext_s* Context, audio_attributes_t Attributes)
{
    DVB_ERROR("Not supported\n");
    return -EPERM;
}
/*}}}  */
/*{{{  AudioIoctlSetKaraoke*/
static int AudioIoctlSetKaraoke (struct DeviceContext_s* Context, audio_karaoke_t* Karaoke)
{
    DVB_ERROR("Not supported\n");
    return -EPERM;
}
/*}}}  */
/*{{{  AudioIoctlSetEncoding*/
static int AudioIoctlSetEncoding (struct DeviceContext_s* Context, unsigned int Encoding)
{
    DVB_DEBUG("Set encoding to %d\n", Encoding);

    if ((Encoding < AUDIO_ENCODING_AUTO) || (Encoding > AUDIO_ENCODING_PRIVATE))
        return  -EINVAL;

    if ((Context->AudioState.play_state != AUDIO_STOPPED) && (Context->AudioState.play_state != AUDIO_INCOMPLETE))
    {
        DVB_ERROR("Cannot change encoding after play has started\n");
        return -EPERM;
    }

    Context->AudioEncoding      = (audio_encoding_t)Encoding;

    /* At this point we have received the missing piece of information which will allow the
     * stream to be fully populated so we can reissue the play. */
    if (Context->AudioState.play_state == AUDIO_INCOMPLETE)
        AudioIoctlPlay (Context);

    return 0;
}
/*}}}  */
/*{{{  AudioIoctlFlush*/
static int AudioIoctlFlush (struct DeviceContext_s* Context)
{
    int                         Result  = 0;
    struct DvbContext_s*        DvbContext      = Context->DvbContext;

    DVB_DEBUG("\n");
    /* If the stream is frozen it cannot be drained so an error is returned. */
    if ((Context->PlaySpeed == 0) || (Context->PlaySpeed == DVB_SPEED_REVERSE_STOPPED))
        return -EPERM;

    if (mutex_lock_interruptible (Context->ActiveAudioWriteLock) != 0)
        return -ERESTARTSYS;
    mutex_unlock (&(DvbContext->Lock));                 /* release lock so non-writing ioctls still work while draining */

    Result      = StreamDrain (Context->AudioStream, false);

    mutex_unlock (Context->ActiveAudioWriteLock);       /* release write lock so actions which have context lock can complete */
    mutex_lock (&(DvbContext->Lock));                   /* reclaim lock so can be released by outer function */

    return Result;
}
/*}}}  */
/*{{{  AudioIoctlSetSpdifSource*/
static int AudioIoctlSetSpdifSource (struct DeviceContext_s* Context, unsigned int Mode)
{
    int Result;
    int BypassCodedData = (Mode == AUDIO_SPDIF_SOURCE_ES);

    DVB_DEBUG("\n");
    if ((Mode != AUDIO_SPDIF_SOURCE_PP) && (Mode != AUDIO_SPDIF_SOURCE_ES))
        return -EINVAL;

    DVB_DEBUG("Bypass = %s\n", BypassCodedData ? "Enabled (ES)" : "Disabled (Post-proc LPCM)");

    if (Context->AudioState.bypass_mode == BypassCodedData)
        return 0;

    Result      = StreamSetOption (Context->AudioStream, PLAY_OPTION_AUDIO_SPDIF_SOURCE, (unsigned int)BypassCodedData);
    if (Result == 0)
        Context->AudioState.bypass_mode  = Mode;

    return Result;
}
/*}}}  */
/*{{{  AudioIoctlSetSpeed*/
static int AudioIoctlSetSpeed (struct DeviceContext_s* Context, int Speed)
{
#if 0
    int Result;

    DVB_DEBUG("\n");
    if (Context->Playback == NULL)
    {
        Context->PlaySpeed      = Speed;
        return 0;
    }

    Result      = PlaybackSetSpeed (Context->Playback, Speed);
    if (Result < 0)
        return Result;
    Result      = PlaybackGetSpeed (Context->Playback, &Context->PlaySpeed);
    if (Result < 0)
        return Result;

    DVB_DEBUG("Speed set to %d\n", Context->PlaySpeed);

    return Result;
#else
    int DirectionChange;
    int Result;

    DVB_DEBUG("\n");
    if (Context->Playback == NULL)
    {
        Context->PlaySpeed      = Speed;
        return 0;
    }

    /* If changing direction we require a write lock */
    DirectionChange     = ((Speed * Context->PlaySpeed) < 0);
    if (DirectionChange)
    {
        /* Discard previously injected data to free the lock. */
        StreamDrain (Context->AudioStream, true);
        if  (mutex_lock_interruptible (Context->ActiveAudioWriteLock) != 0)
            return -ERESTARTSYS;                /* Give up for now. */
    }

    Result      = PlaybackSetSpeed (Context->Playback, Speed);
    if (Result >= 0)
        Result  = PlaybackGetSpeed (Context->Playback, &Context->PlaySpeed);

    /* If changing direction release write lock*/
    if (DirectionChange)
        mutex_unlock( Context->ActiveAudioWriteLock );

    DVB_DEBUG("Speed set to %d\n", Context->PlaySpeed);

    return Result;
#endif
}
/*}}}  */
/*{{{  AudioIoctlDiscontinuity*/
static int AudioIoctlDiscontinuity (struct DeviceContext_s* Context, audio_discontinuity_t Discontinuity)
{
    int                 Result  = 0;

    DVB_DEBUG("%d\n", Discontinuity);

    /*
    If the stream is frozen a discontinuity cannot be injected.
    */
    if ((Context->AudioState.play_state == AUDIO_PAUSED) || (Context->PlaySpeed == 0) || (Context->PlaySpeed == DVB_SPEED_REVERSE_STOPPED))
        return -EINVAL;

    /*mutex_lock (Context->ActiveAudioWriteLock);*/
    if (mutex_lock_interruptible (Context->ActiveAudioWriteLock) != 0)
        return -ERESTARTSYS;

    Result      = StreamDiscontinuity (Context->AudioStream, (discontinuity_t)Discontinuity);
    mutex_unlock (Context->ActiveAudioWriteLock);

    return Result;
}
/*}}}  */
/*{{{  AudioIoctlSetPlayInterval*/
static int AudioIoctlSetPlayInterval (struct DeviceContext_s* Context, dvb_play_interval_t* PlayInterval)
{
    int Result;

    DVB_DEBUG("\n");

    memcpy (&Context->PlayInterval, PlayInterval, sizeof(dvb_play_interval_t));

    if (Context->Playback == NULL)
        return 0;

    Result      = PlaybackSetPlayInterval (Context->Playback, (play_interval_t*)PlayInterval);
    if (Result < 0)
        return Result;

    return Result;
}
/*}}}  */
/*{{{  AudioIoctlSetSyncGroup*/
static int AudioIoctlSetSyncGroup (struct DeviceContext_s* Context, unsigned int Group)
{
    int         Result  = 0;

    if (Context != Context->SyncContext)
    {
        DVB_ERROR("Sync group already set - cannot reset\n");
        return -EPERM;
    }

    if ((Group < 0) || (Group >= DVB_MAX_DEVICES_PER_ADAPTER))
    {
        DVB_ERROR("Invalid sync group - out of range.\n");
        return -EPERM;
    }

    if (Context->SyncContext->Playback)
    {
        DVB_ERROR("Sync group already set - cannot reset\n");
        return -EPERM;
    }

    Context->SyncContext        = &Context->DvbContext->DeviceContext[Group];

    DVB_DEBUG("Sync group set to device %d\n", Group);

    return Result;
}
/*}}}  */
/*}}}  */
/*{{{  AudioOpen*/
static int AudioOpen (struct inode*     Inode,
                      struct file*      File)
{
    struct dvb_device*          DvbDevice       = (struct dvb_device*)File->private_data;
    struct DeviceContext_s*     Context         = (struct DeviceContext_s*)DvbDevice->priv;
    int                         Error;

    Error       = dvb_generic_open (Inode, File);
    if (Error < 0)
        return Error;

    if ((File->f_flags & O_ACCMODE) != O_RDONLY)
    {
        Context->AudioState.play_state  = AUDIO_STOPPED;
        Context->AudioOpenWrite         = 1;
    }

    return 0;

}
/*}}}  */
/*{{{  AudioRelease*/
static int AudioRelease (struct inode*  Inode,
                         struct file*   File)
{
    struct dvb_device*          DvbDevice       = (struct dvb_device*)File->private_data;
    struct DeviceContext_s*     Context         = (struct DeviceContext_s*)DvbDevice->priv;
    struct DvbContext_s*        DvbContext      = Context->DvbContext;

    DVB_DEBUG("\n");

    if ((File->f_flags & O_ACCMODE) != O_RDONLY)
    {
        mutex_lock (&(DvbContext->Lock));

        if (Context->AudioStream != NULL)
        {
            unsigned int    MutexIsLocked   = true;
            /* Discard previously injected data to free the lock. */
            StreamDrain (Context->AudioStream, true);

            if (mutex_lock_interruptible (Context->ActiveAudioWriteLock) != 0)
                MutexIsLocked       = false;

            PlaybackRemoveStream (Context->Playback, Context->AudioStream);
			Context->AudioStream    = NULL;

            if (MutexIsLocked)
                mutex_unlock (Context->ActiveAudioWriteLock);
        }

        DisplayDelete (BACKEND_AUDIO_ID, Context->Id);

        /* Check to see if video and demux have also finished so we can release the playback */
		if ((Context->VideoStream == NULL) && (Context->DemuxStream == NULL) && (Context->Playback != NULL))
        {
            /* Check to see if our playback has already been deleted by the demux context */
            if (Context->DemuxContext->Playback != NULL)
            {
                /* Try and delete playback then set our demux to Null if succesful or not.  If we fail someone else
                   is still using it but we are done. */
                if (PlaybackDelete (Context->Playback) == 0)
                    DVB_DEBUG("Playback deleted successfully\n");
            }
            Context->Playback               = NULL;
            Context->StreamType             = STREAM_TYPE_TRANSPORT;
            Context->PlaySpeed              = DVB_SPEED_NORMAL_PLAY;
            Context->PlayInterval.start     = DVB_TIME_NOT_BOUNDED;
            Context->PlayInterval.end       = DVB_TIME_NOT_BOUNDED;
            Context->SyncContext            = Context;
        }

        AudioInit (Context);

        mutex_unlock (&(DvbContext->Lock));
    }
    return dvb_generic_release (Inode, File);
}
/*}}}  */

#ifdef ENABLE_DVB_DEBUG
static void	print_ioctl_str(int IoctlCode)
{
	char *strIoctl;

	switch(IoctlCode) {
		case AUDIO_STOP:				strIoctl="AUDIO_STOP";				break;
		case AUDIO_PLAY:				strIoctl="AUDIO_PLAY";				break;
		case AUDIO_PAUSE:				strIoctl="AUDIO_PAUSE";				break;
		case AUDIO_CONTINUE:			strIoctl="AUDIO_CONTINUE";			break;
		case AUDIO_SELECT_SOURCE:		strIoctl="AUDIO_SELECT_SOURCE";		break;
		case AUDIO_SET_MUTE:			strIoctl="AUDIO_SET_MUTE";			break;
		case AUDIO_SET_AV_SYNC:			strIoctl="AUDIO_SET_AV_SYNC";		break;
		case AUDIO_SET_BYPASS_MODE:		strIoctl="AUDIO_SET_BYPASS_MODE";	break;
		case AUDIO_CHANNEL_SELECT:		strIoctl="AUDIO_CHANNEL_SELECT";	break;
		case AUDIO_GET_STATUS:			strIoctl="AUDIO_GET_STATUS";		break;
		case AUDIO_GET_CAPABILITIES:	strIoctl="AUDIO_GET_CAPABILITIES";	break;
		case AUDIO_GET_PTS:				strIoctl="AUDIO_GET_PTS";			break;
		case AUDIO_CLEAR_BUFFER:		strIoctl="AUDIO_CLEAR_BUFFER";		break;
		case AUDIO_SET_ID:				strIoctl="AUDIO_SET_ID";			break;
		case AUDIO_SET_MIXER:			strIoctl="AUDIO_SET_MIXER";			break;
		case AUDIO_SET_STREAMTYPE:		strIoctl="AUDIO_SET_STREAMTYPE";	break;
		case AUDIO_SET_EXT_ID:			strIoctl="AUDIO_SET_EXT_ID";		break;
		case AUDIO_SET_ATTRIBUTES:		strIoctl="AUDIO_SET_ATTRIBUTES";	break;
		case AUDIO_SET_KARAOKE:			strIoctl="AUDIO_SET_KARAOKE";		break;
        case AUDIO_SET_ENCODING:        strIoctl="AUDIO_SET_ENCODING";		break;
        case AUDIO_FLUSH:               strIoctl="AUDIO_FLUSH";				break;
        case AUDIO_SET_SPDIF_SOURCE:    strIoctl="AUDIO_SET_SPDIF_SOURCE";	break;
        case AUDIO_SET_SPEED:           strIoctl="AUDIO_SET_SPEED";			break;
        case AUDIO_DISCONTINUITY:       strIoctl="AUDIO_DISCONTINUITY";		break;
        case AUDIO_SET_PLAY_INTERVAL:   strIoctl="AUDIO_SET_PLAY_INTERVAL";	break;
        case AUDIO_SET_SYNC_GROUP:      strIoctl="AUDIO_SET_SYNC_GROUP";	break;
        case AUDIO_GET_PLAY_TIME:       strIoctl="AUDIO_GET_PLAY_TIME";		break;
        case AUDIO_GET_PLAY_INFO:       strIoctl="AUDIO_GET_PLAY_INFO";		break;
		default:						strIoctl="UNKNOWN AUDIO IOCTL";		break;
	}
	printk("-----------\n%s\n-----------\n",strIoctl);
}
#endif

/*{{{  AudioIoctl*/
static int AudioIoctl (struct inode*    Inode,
                       struct file*     File,
                       unsigned int     IoctlCode,
                       void*            Parameter)
{
    struct dvb_device*          DvbDevice       = (struct dvb_device*)File->private_data;
    struct DeviceContext_s*     Context         = (struct DeviceContext_s*)DvbDevice->priv;
    struct DvbContext_s*        DvbContext      = Context->DvbContext;
    int                         Result          = 0;

    /*DVB_DEBUG("AudioIoctl : Ioctl %08x\n", IoctlCode); */

    if (((File->f_flags & O_ACCMODE) == O_RDONLY) && (IoctlCode != AUDIO_GET_STATUS))
        return -EPERM;

    if (!Context->AudioOpenWrite)       /* Check to see that somebody has the device open for write */
        return -EBADF;

#ifdef ENABLE_DVB_DEBUG
	print_ioctl_str(IoctlCode);
#endif
	mutex_lock (&(DvbContext->Lock));
    switch (IoctlCode)
    {
        case AUDIO_STOP:                Result = AudioIoctlStop            (Context);                                        break;
        case AUDIO_PLAY:                Result = AudioIoctlPlay            (Context);                                        break;
        case AUDIO_PAUSE:               Result = AudioIoctlPause           (Context);                                        break;
        case AUDIO_CONTINUE:            Result = AudioIoctlContinue        (Context);                                        break;
        case AUDIO_SELECT_SOURCE:       Result = AudioIoctlSelectSource    (Context, (audio_stream_source_t)Parameter);      break;
        case AUDIO_SET_MUTE:            Result = AudioIoctlSetMute         (Context, (unsigned int)Parameter);               break;
        case AUDIO_SET_AV_SYNC:         Result = AudioIoctlSetAvSync       (Context, (unsigned int)Parameter);               break;
        case AUDIO_SET_BYPASS_MODE:     Result = AudioIoctlSetBypassMode   (Context, (unsigned int)Parameter);               break;
        case AUDIO_CHANNEL_SELECT:      Result = AudioIoctlChannelSelect   (Context, (audio_channel_select_t)Parameter);     break;
        case AUDIO_GET_STATUS:          Result = AudioIoctlGetStatus       (Context, (struct audio_status*)Parameter);       break;
        case AUDIO_GET_CAPABILITIES:    Result = AudioIoctlGetCapabilities (Context, (int*)Parameter);                       break;
        case AUDIO_GET_PTS:             Result = AudioIoctlGetPts          (Context, (unsigned long long int*)Parameter);    break;
        case AUDIO_CLEAR_BUFFER:        Result = AudioIoctlClearBuffer     (Context);                                        break;
        case AUDIO_SET_ID:              Result = AudioIoctlSetId           (Context, (int)Parameter);                        break;
        case AUDIO_SET_MIXER:           Result = AudioIoctlSetMixer        (Context, (audio_mixer_t*)Parameter);             break;
        case AUDIO_SET_STREAMTYPE:      Result = AudioIoctlSetStreamType   (Context, (unsigned int)Parameter);               break;
        case AUDIO_SET_EXT_ID:          Result = AudioIoctlSetExtId        (Context, (int)Parameter);                        break;
        case AUDIO_SET_ATTRIBUTES:      Result = AudioIoctlSetAttributes   (Context, (audio_attributes_t) ((int)Parameter)); break;
        case AUDIO_SET_KARAOKE:         Result = AudioIoctlSetKaraoke      (Context, (audio_karaoke_t*)Parameter);           break;
        case AUDIO_SET_ENCODING:        Result = AudioIoctlSetEncoding     (Context, (unsigned int)Parameter);               break;
        case AUDIO_FLUSH:               Result = AudioIoctlFlush           (Context);                                        break;
        case AUDIO_SET_SPDIF_SOURCE:    Result = AudioIoctlSetSpdifSource  (Context, (unsigned int)Parameter);               break;
        case AUDIO_SET_SPEED:           Result = AudioIoctlSetSpeed        (Context, (int)Parameter);                        break;
        case AUDIO_DISCONTINUITY:       Result = AudioIoctlDiscontinuity   (Context, (audio_discontinuity_t)Parameter);      break;
        case AUDIO_SET_PLAY_INTERVAL:   Result = AudioIoctlSetPlayInterval (Context, (dvb_play_interval_t*)Parameter);       break;
        case AUDIO_SET_SYNC_GROUP:      Result = AudioIoctlSetSyncGroup    (Context, (unsigned int)Parameter);               break;
        case AUDIO_GET_PLAY_TIME:       Result = AudioIoctlGetPlayTime     (Context, (audio_play_time_t*)Parameter);         break;
        case AUDIO_GET_PLAY_INFO:       Result = AudioIoctlGetPlayInfo     (Context, (audio_play_info_t*)Parameter);         break;

        default:
            DVB_ERROR("Invalid ioctl %08x\n", IoctlCode);
            Result      = -ENOIOCTLCMD;
    }
	mutex_unlock (&(DvbContext->Lock));

    return Result;
}
/*}}}  */
/*{{{  AudioWrite*/
static ssize_t AudioWrite (struct file *File, const char __user* Buffer, size_t Count, loff_t* ppos)
{
    struct dvb_device*          DvbDevice       = (struct dvb_device*)File->private_data;
    struct DeviceContext_s*     Context         = (struct DeviceContext_s*)DvbDevice->priv;
    int                         Result          = 0;
#ifdef CONFIG_STM_TKDMA
    char*                       InBuffer        = &Context->dvr_in[16*2048];
    char*                       OutBuffer       = &Context->dvr_out[16*2048];
#endif
    void*                       PhysicalBuffer  = NULL;
    const unsigned char*        ClearData       = Buffer;

    if ((File->f_flags & O_ACCMODE) == O_RDONLY)
        return -EPERM;

    if (Context->AudioState.stream_source == AUDIO_SOURCE_DEMUX)
    {
        DVB_ERROR("Audio stream source not AUDIO_SOURCE_MEMORY - cannot use write\n");
        return -EPERM;          /* Not allowed to write to device if connected to demux */
    }

    mutex_lock (&(Context->AudioWriteLock));

    if (Context->AudioStream == NULL)
    {
        DVB_ERROR ("No audio stream exists to be written to (previous AUDIO_PLAY failed?)\n");
        mutex_unlock (&(Context->AudioWriteLock));
        return -ENODEV;
    }

    if (!StreamBufferFree (Context->AudioStream) && ((File->f_flags & O_NONBLOCK) != 0))
    {
        mutex_unlock (&(Context->AudioWriteLock));
        return -EWOULDBLOCK;
    }

    PhysicalBuffer      = stm_v4l2_findbuffer ((unsigned long)Buffer, Count, 0);

#ifdef CONFIG_STM_TKDMA
    if (Context->EncryptionOn) 
    {
        if ((Count % 2048) != 0)
        {
            DVB_ERROR ("Count size incorrect for decryption (%d)\n", Count);
            mutex_unlock (&(Context->AudioWriteLock));
            return -EINVAL;
        }

        if (PhysicalBuffer != NULL)
        {
            tkdma_hddvd_decrypt_data (OutBuffer, PhysicalBuffer, Count/2048, TKDMA_VIDEO_CHANNEL, 0); // Got to check blocking
        }
        else
        {
            /*
               If the incoming data is aligned to 2K/4K we would be able to avoid
               this copy from user, we just decrypt max two packets at a time, certainly scatter gather
               would be easier.
            */
            copy_from_user (InBuffer, Buffer, Count);
            tkdma_hddvd_decrypt_data (OutBuffer, InBuffer, Count/2048, TKDMA_VIDEO_CHANNEL, 0); // Got to check blocking
        }
        ClearData       = OutBuffer;
    }
    else
#endif	/* CONFIG_STM_TKDMA */
	    if (PhysicalBuffer)
	    	    ClearData   = PhysicalBuffer;

#define PACK_HEADER_START_CODE  ((ClearData[0]==0x00)&&(ClearData[1]==0x00)&&(ClearData[2]==0x01)&&(ClearData[3]==0xba))
#define PACK_HEADER_LENGTH      14
#define PES_LENGTH              ((ClearData[4]<<8)+ClearData[5]+6)      /* Extra 6 includes bytes up to length bytes */
    if (PACK_HEADER_START_CODE)         /* TODO Skip Pack header until handled by audio collator */
    {
          ClearData     = &ClearData[PACK_HEADER_LENGTH];
          Count         = PES_LENGTH;
    }

    if (Context->AudioState.play_state == AUDIO_INCOMPLETE)
    {
        if (Context->AudioEncoding == AUDIO_ENCODING_AUTO)
        {
            StreamGetFirstBuffer (Context->AudioStream, Buffer, Count);
            StreamIdentifyAudio  (Context->AudioStream, &Context->AudioEncoding);
        }
        else
            DVB_ERROR("Audio incomplete with encoding set (%d)\n", Context->AudioEncoding);

        AudioIoctlPlay       (Context);

        if (Context->AudioState.play_state == AUDIO_INCOMPLETE)
            Context->AudioState.play_state    = AUDIO_STOPPED;
    }

    if (Context->AudioState.play_state != AUDIO_PLAYING)
    {
        DVB_ERROR("Audio not playing - cannot write (state %x)\n", Context->AudioState.play_state);
        Result  = -EPERM;                                       /* Not allowed to write to device if paused */
    }
    else
        Result  = StreamInject (Context->AudioStream, ClearData, Count);
        /*Result  = StreamInjectFromUser (Context->AudioStream, Buffer, Count);*/

    mutex_unlock (&(Context->AudioWriteLock));

    return Result;
}
/*}}}  */
/*{{{  AudioPoll*/
static unsigned int AudioPoll (struct file* File, poll_table* Wait)
{
    struct dvb_device*          DvbDevice       = (struct dvb_device*)File->private_data;
    struct DeviceContext_s*     Context         = (struct DeviceContext_s*)DvbDevice->priv;
    unsigned int                Mask            = 0;

    if (((File->f_flags & O_ACCMODE) == O_RDONLY) || (Context->AudioStream == NULL))
        return 0;

#if defined (USE_INJECTION_THREAD)
    poll_wait (File, &Context->AudioStream->BufferEmpty, Wait);
#endif

    if ((Context->AudioState.play_state == AUDIO_PLAYING) || (Context->AudioState.play_state == AUDIO_INCOMPLETE))
    {
        if (StreamBufferFree (Context->AudioStream))
            Mask   |= (POLLOUT | POLLWRNORM);
    }
    return Mask;
}
/*}}}  */

