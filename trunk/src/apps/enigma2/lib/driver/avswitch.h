#ifndef __avswitch_h
#define __avswitch_h

#include <lib/base/object.h>
#include <lib/python/connections.h>

#ifdef QBOXHD
#define	LETTERBOX	0
#define	NONLINEAR	1
#define	PANeSCAN	2
#define	BESTFIT		3

#define	V_4_3	0
#define	V_16_9	1

typedef struct {
	unsigned char aspect_ratio;
	unsigned char policy;
} Format_video_t;

void set_scart_aspect(unsigned char d);

#endif // QBOXHD

class eSocketNotifier;

class eAVSwitch: public Object
{
	static eAVSwitch *instance;
	int m_video_mode;
	ePtr<eSocketNotifier> m_fp_notifier;
	void fp_event(int what);
	int m_fp_fd;
#ifdef SWIG
	eAVSwitch();
	~eAVSwitch();
#endif
protected:
public:
#ifndef SWIG
	eAVSwitch();
	~eAVSwitch();
#endif
	static eAVSwitch *getInstance();
	bool haveScartSwitch();
	int getVCRSlowBlanking();
	void setColorFormat(int format);
	void setAspectRatio(int ratio);
	void setVideomode(int mode);

// 	void standbyStatus(int val);
#ifdef QBOXHD
	void setFastBlank(int val);
//	void setSlowBlank(int val);
	void disable_scart_hdmi(int val);
#endif // QBOXHD

	void setInput(int val);
	void setWSS(int val);
	PSignal1<void, int> vcr_sb_notifier;
};
#endif
