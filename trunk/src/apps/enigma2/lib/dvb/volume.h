#ifndef __volume_h
#define __volume_h

#include <lib/base/ebase.h>

#ifdef QBOXHD
#define HDMI_PCM    0x01
#define HDMI_SPDIF  0x02

enum amixer_control {
	AMIXER_PRIMARY,
 AMIXER_GAIN_OVERRIDE,
    AMIXER_SPDIF_ENCODING,
    AMIXER_SPDIF_BYPASS
};
#endif // QBOXHD

class eDVBVolumecontrol
{
private:
#ifndef QBOXHD
	static eDVBVolumecontrol *instance;
#endif //QBOXHD
	eDVBVolumecontrol();
#ifdef SWIG
	~eDVBVolumecontrol();
#endif
	int openMixer();
	void closeMixer(int fd);

	bool muted;
	int leftVol, rightVol;

	int checkVolume(int vol);
public:
#ifdef QBOXHD
    static eDVBVolumecontrol *instance;
#endif //QBOXHD
    static eDVBVolumecontrol* getInstance();

	void volumeUp(int left = 5, int right = 5);
	void volumeDown(int left = 5, int right = 5);

	void setVolume(int left, int right);

	void volumeMute();
	void volumeUnMute();
	void volumeToggleMute();

	int getVolume();
	bool isMuted();
};

#ifdef QBOXHD
#define	MPEG_TYPE_VOLUME	0x00
#define	AC3_TYPE_VOLUME		0x01
#define	RADIO_TYPE_VOLUME	0xFF

void amixer_control_set(enum amixer_control, int);
//void hdmi_control(unsigned int);
void setVolumeAudioType(int);
#endif // QBOXHD

#endif //__volume_h
