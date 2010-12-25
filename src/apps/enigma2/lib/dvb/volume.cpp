#include <lib/base/eerror.h>
#include <lib/dvb/volume.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#ifdef QBOXHD
/* For asound */
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>
#include <assert.h>
#include <alsa/asoundlib.h>
#include <alsa/control.h>
#include <sys/poll.h>
#include "stmhdmi.h"
#endif //QBOXHD

#if HAVE_DVB_API_VERSION < 3
#define VIDEO_DEV "/dev/dvb/card0/video0"
#define AUDIO_DEV "/dev/dvb/card0/audio0"
#include <ost/audio.h>
#include <ost/video.h>
#else
#define VIDEO_DEV "/dev/dvb/adapter0/video0"
#define AUDIO_DEV "/dev/dvb/adapter0/audio0"
#include <linux/dvb/audio.h>
#include <linux/dvb/video.h>
#endif


#ifdef QBOXHD
int prev_atype;

#define	MAX_VALUE_AC3		32256
#define	MAX_VALUE_MPEG		11600	/* ~36% of AC3 */
#define	MAX_VALUE_RADIO		4050	/* ~12.5% of AC3 */

static unsigned int max_value;
///TODO: may be it isn't necessary
/*
void hdmi_control(unsigned int val)
{
	int fd;

	if ((fd = open("/dev/hdmi0.0", O_RDWR)) < 0) {
		eDebug("Unable to open hdmi device");
		return;
	}

	if(val & HDMI_PCM) {
		eDebug("\nPCM\n");
		if (ioctl(fd, STMHDMIIO_SET_AUDIO_SOURCE, STMHDMIIO_AUDIO_SOURCE_PCM) < 0) {
			perror("Unable to change audio source");
			close(fd);
			return;
		}
	}
	else if (val & HDMI_SPDIF) {
		eDebug("\nSPDIF\n");
		if (ioctl(fd, STMHDMIIO_SET_AUDIO_SOURCE, STMHDMIIO_AUDIO_SOURCE_SPDIF) < 0) {
			perror("Unable to change audio source");
			close(fd);
			return;
		}
	}
	close(fd);
}
*/

void amixer_control_set(enum amixer_control mixer_control, int value)
{
	int err = 0;
	snd_mixer_t *handle;
	snd_mixer_elem_t *elem = NULL;
	snd_mixer_selem_id_t *sid;

	const char * card_name="hw:MIXER0";

	/* Allocate simple id*/
	snd_mixer_selem_id_alloca (&sid);

	/* Sets simple-mixer index and name*/
	snd_mixer_selem_id_set_index (sid, 0);

	switch (mixer_control) {
		case AMIXER_PRIMARY:
// 			eDebug("Open mixer handle with 'Primary'");
			snd_mixer_selem_id_set_name(sid, "Primary");
			break;
		case AMIXER_GAIN_OVERRIDE:
// 			eDebug("Open mixer handle with 'Gain Override'");
			snd_mixer_selem_id_set_name(sid, "Gain Override");
			break;
		case AMIXER_SPDIF_ENCODING:
// 			eDebug("Open mixer handle with 'SPDIF Encoding'");
			snd_mixer_selem_id_set_name(sid, "SPDIF Encoding");
			break;
		case AMIXER_SPDIF_BYPASS:
// 			eDebug("Open mixer handle with 'SPDIF Bypass'");
			snd_mixer_selem_id_set_name(sid, "SPDIF Bypass");
			break;
		default:
			eDebug("Invalid ALSA mixer control '%d'\n", mixer_control);
			return;
	}

	if ((err = snd_mixer_open(&handle, 0)) < 0) {
		eDebug("Could not open the mixer");
		return;
	}

	if ((err = snd_mixer_attach(handle, card_name)) < 0) {
		eDebug("Could not attach mixer with '%s'", card_name);
		return;
	}
	if ((err = snd_mixer_selem_register(handle, NULL, NULL)) < 0) {
		eDebug("Could not register the mixer");
		return;
	}
	if ((err = snd_mixer_load(handle)) < 0) {
		eDebug("Could not load the mixer");
		return;
	}

	if ((elem = snd_mixer_find_selem(handle, sid)) == NULL) {
		eDebug("Could not find the element id");
		return;
	}

	switch (mixer_control) {
		case AMIXER_PRIMARY:
			eDebug("Setting volume at %d", value);
			snd_mixer_selem_set_playback_volume_all(elem, value);
			break;
		case AMIXER_GAIN_OVERRIDE:
			eDebug("Switch on the channel %d", value);
			snd_mixer_selem_set_playback_switch_all(elem, value);
			break;
		case AMIXER_SPDIF_ENCODING:
			eDebug("Setting SPDIF encoding to %d", value);
			snd_mixer_selem_set_enum_item(elem, (snd_mixer_selem_channel_id_t)0, value);
			break;
		case AMIXER_SPDIF_BYPASS:
			eDebug("Setting SPDIF by-pass to %d", value);
			snd_mixer_selem_set_playback_switch_all(elem, value);
			break;
		default:
			break;
	}

	snd_mixer_close(handle);
}

void audio_amixer_primary(int value)
{
	int err = 0;
	snd_mixer_t *handle;
	snd_mixer_elem_t *elem=NULL;
	snd_mixer_selem_id_t *sid;

	const char *play_mix_name = "Primary";
	const char * card_name="hw:MIXER0";

	/* Allocate simple id*/
	snd_mixer_selem_id_alloca (&sid);

	/* Sets simple-mixer index and name*/
	snd_mixer_selem_id_set_index (sid, 0);
	snd_mixer_selem_id_set_name (sid, play_mix_name);

	eDebug("Open mixer handle with 'Primary'");
	if ((err = snd_mixer_open (&handle, 0)) < 0)
	{
		eDebug("Can't open the mixer");
		return;
	}
	do {
		if ((err = snd_mixer_attach (handle, card_name)) < 0)
		{
			eDebug("Can't attact mixer with 'hw:MIXER0'");
			break ;
		}
		if ((err = snd_mixer_selem_register (handle, NULL, NULL)) < 0)
		{
			eDebug("Can't register the mixer");
			break ;
		}
		err = snd_mixer_load(handle);
		if (err < 0)
		{
			eDebug("Can't load the mixer");
			break ;
		}

		elem = snd_mixer_find_selem (handle, sid);

		if (elem==NULL)
		{
			eDebug("Can't find the elemnt id of 'Primary'");
			break;
		}

		eDebug("Setting volume at %d",value);
		snd_mixer_selem_set_playback_volume_all (elem, value);
	} while(0);

	eDebug("Close mixer handle with 'Primary'");
	snd_mixer_close(handle);
}

void audio_amixer_gain_override(int value)
{
	int err = 0;
	snd_mixer_t *handle;
	snd_mixer_elem_t *elem=NULL;
	snd_mixer_selem_id_t *sid;

	const char *play_mix_name = "Gain Override";
	const char * card_name="hw:MIXER0";

	/* Allocate simple id*/
	snd_mixer_selem_id_alloca (&sid);

	/* Sets simple-mixer index and name*/
	snd_mixer_selem_id_set_index (sid, 0);
	snd_mixer_selem_id_set_name (sid, play_mix_name);

	 eDebug("Open mixer handle with 'Gain Override'");
	if ((err = snd_mixer_open (&handle, 0)) < 0)
	{
		eDebug("Can't open the mixer");
		return;
	}
	do{
		if ((err = snd_mixer_attach (handle, card_name)) < 0)
		{
			eDebug("Can't attact mixer with 'hw:MIXER0'");
			break ;
		}
		if ((err = snd_mixer_selem_register (handle, NULL, NULL)) < 0)
		{
			eDebug("Can't register the mixer");
			break ;
		}
		err = snd_mixer_load(handle);
		if (err < 0)
		{
			eDebug("Can't load the mixer");
			break ;
		}

		elem = snd_mixer_find_selem (handle, sid);

		if (elem==NULL)
		{
			eDebug("Can't find the elemnt id of 'Primary'");
			break;
		}

		eDebug("Switch on the channel");
		snd_mixer_selem_set_playback_switch_all(elem,value);
	}while(0);

	eDebug("Close mixer handle with 'Gain Override'");
	snd_mixer_close(handle);
}
#endif // QBOXHD

eDVBVolumecontrol* eDVBVolumecontrol::instance = NULL;

eDVBVolumecontrol* eDVBVolumecontrol::getInstance()
{
	if (instance == NULL)
		instance = new eDVBVolumecontrol;
	return instance;
}

eDVBVolumecontrol::eDVBVolumecontrol()
{
#ifdef QBOXHD
	amixer_control_set(AMIXER_GAIN_OVERRIDE, 100);
	max_value=MAX_VALUE_MPEG; /* the default is MPEG */
    volumeToggleMute();
    prev_atype = -1;
#endif // QBOXHD
    volumeUnMute();
    setVolume(25, 25);
}

int eDVBVolumecontrol::openMixer()
{
	return open( AUDIO_DEV, O_RDWR );
}

void eDVBVolumecontrol::closeMixer(int fd)
{
	close(fd);
}

void eDVBVolumecontrol::volumeUp(int left, int right)
{
	setVolume(leftVol + left, rightVol + right);
}

void eDVBVolumecontrol::volumeDown(int left, int right)
{
	setVolume(leftVol - left, rightVol - right);
}

int eDVBVolumecontrol::checkVolume(int vol)
{
	if (vol < 0)
		vol = 0;
	else if (vol > 100)
		vol = 100;
	return vol;
}

void eDVBVolumecontrol::setVolume(int left, int right)
{
	/* left, right is 0..100 */
	leftVol = checkVolume(left);
	rightVol = checkVolume(right);

	/* convert to -1dB steps */
	left = 63 - leftVol * 63 / 100;
	right = 63 - rightVol * 63 / 100;
		/* now range is 63..0, where 0 is loudest */

#if HAVE_DVB_API_VERSION < 3
	audioMixer_t mixer;
#else
	audio_mixer_t mixer;
#endif

#if HAVE_DVB_API_VERSION < 3
		/* convert to linear scale. 0 = loudest, ..63 */
	mixer.volume_left = 63.0-pow(1.068241, 63-left);
	mixer.volume_right = 63.0-pow(1.068241, 63-right);
#else
	mixer.volume_left = left;
	mixer.volume_right = right;
#endif

	eDebug("Setvolume: %d %d (raw)", leftVol, rightVol);
// 	eDebug("Setvolume: %d %d (-1db)", left, right);
#if HAVE_DVB_API_VERSION < 3
	eDebug("Setvolume: %d %d (lin)", mixer.volume_left, mixer.volume_right);
#endif

    //HACK?
    FILE *f;
    if((f = fopen("/proc/stb/avs/0/volume", "wb")) == NULL) {
        eDebug("cannot open /proc/stb/avs/0/volume(%m)");
        return;
    }

    fprintf(f, "%d", left); /* in -1dB */

    fclose(f);
}

int eDVBVolumecontrol::getVolume()
{
	return leftVol;
}

bool eDVBVolumecontrol::isMuted()
{
	return muted;
}


void eDVBVolumecontrol::volumeMute()
{
#ifndef QBOXHD
	int fd = openMixer();
#ifdef HAVE_DVB_API_VERSION
	ioctl(fd, AUDIO_SET_MUTE, true);
#endif
	closeMixer(fd);
#endif // QBOXHD
    muted = true;

	amixer_control_set(AMIXER_PRIMARY, 0);

    //HACK?
    FILE *f;
    if((f = fopen("/proc/stb/audio/j1_mute", "wb")) == NULL) {
        eDebug("cannot open /proc/stb/audio/j1_mute(%m)");
        return;
    }

    fprintf(f, "%d", 1);

    fclose(f);
}

void eDVBVolumecontrol::volumeUnMute()
{
#ifndef QBOXHD
	int fd = openMixer();
#ifdef HAVE_DVB_API_VERSION
	ioctl(fd, AUDIO_SET_MUTE, false);
#endif
	closeMixer(fd);
#endif // QBOXHD
	muted = false;

	amixer_control_set(AMIXER_PRIMARY, max_value);

    //HACK?
    FILE *f;
    if((f = fopen("/proc/stb/audio/j1_mute", "wb")) == NULL) {
        eDebug("cannot open /proc/stb/audio/j1_mute(%m)");
        return;
    }

    fprintf(f, "%d", 0);

    fclose(f);
}

void eDVBVolumecontrol::volumeToggleMute()
{
	if (isMuted())
		volumeUnMute();
	else
		volumeMute();
}

#ifdef QBOXHD
/*
 * Workaround for volume difference between MPEG and AC3
 * MPEG audio is ~36%   higher than AC3 in video channels
 * MPEG audio is ~12.5% higher than RADIO channels
 */
void setVolumeAudioType(int cur_atype)
{
    if (prev_atype == cur_atype)
        return;

    if (!eDVBVolumecontrol::instance)
        return;

    int vol = eDVBVolumecontrol::instance->getVolume();

	switch(cur_atype)
	{
		case MPEG_TYPE_VOLUME:
			max_value=MAX_VALUE_MPEG;
			eDebug("Set max level in MPEG level: %d",max_value);
			break;
		case AC3_TYPE_VOLUME:
			max_value=MAX_VALUE_AC3;
			eDebug("Set max level in AC3 level: %d",max_value);
			break;
		case RADIO_TYPE_VOLUME:
			max_value=MAX_VALUE_RADIO;
			eDebug("Set max level in RADIO level: %d",max_value);
			break;
		default:
			max_value=MAX_VALUE_MPEG;
			eDebug("Set max level default level (MPEG): %d",max_value);
			break;
	}


    prev_atype = cur_atype;

	/* If there is the mute, it doesn't apply now the new volume max value */
	if (eDVBVolumecontrol::instance->isMuted())
		return;
	amixer_control_set(AMIXER_PRIMARY, max_value);
    eDVBVolumecontrol::instance->setVolume(vol, vol);
}
#endif // QBOXHD
