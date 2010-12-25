#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>

#include <lib/base/init.h>
#include <lib/base/init_num.h>
#include <lib/base/eerror.h>
#include <lib/base/ebase.h>
#include <lib/driver/avswitch.h>

#ifdef QBOXHD
#include <linux/dvb/video.h>

#include "qboxhd_generic.h"
#ifndef QBOXHD_MINI
	#include "stv6414_i2c.h"
#else
	#include "scart_mini.h"
#endif
#endif // QBOXHD

eAVSwitch *eAVSwitch::instance = 0;

eAVSwitch::eAVSwitch()
{
	ASSERT(!instance);
	instance = this;
	m_video_mode = 0;
	m_fp_fd = open("/dev/dbox/fp0", O_RDONLY|O_NONBLOCK);
	if (m_fp_fd == -1)
	{
		eDebug("couldnt open /dev/dbox/fp0 to monitor vcr scart slow blanking changed!");
		m_fp_notifier=0;
	}
	else
	{
		m_fp_notifier = eSocketNotifier::create(eApp, m_fp_fd, eSocketNotifier::Read|POLLERR);
		CONNECT(m_fp_notifier->activated, eAVSwitch::fp_event);
	}
}

#ifndef FP_IOCTL_GET_EVENT
#define FP_IOCTL_GET_EVENT 20
#endif

#ifndef FP_IOCTL_GET_VCR
#define FP_IOCTL_GET_VCR 7
#endif

#ifndef FP_EVENT_VCR_SB_CHANGED
#define FP_EVENT_VCR_SB_CHANGED 1
#endif

int eAVSwitch::getVCRSlowBlanking()
{
	int val=0;
	if (m_fp_fd >= 0)
	{
		FILE *f = fopen("/proc/stb/fp/vcr_fns", "r");
		if (f)
		{
			if (fscanf(f, "%d", &val) != 1)
				eDebug("read /proc/stb/fp/vcr_fns failed!! (%m)");
			fclose(f);
		}
		else if (ioctl(m_fp_fd, FP_IOCTL_GET_VCR, &val) < 0)
			eDebug("FP_GET_VCR failed (%m)");
	}
	return val;
}

void eAVSwitch::fp_event(int what)
{
	if (what & POLLERR) // driver not ready for fp polling
	{
		eDebug("fp driver not read for polling.. so disable polling");
		m_fp_notifier->stop();
	}
	else
	{
		FILE *f = fopen("/proc/stb/fp/events", "r");
		if (f)
		{
			int events;
			if (fscanf(f, "%d", &events) != 1)
				eDebug("read /proc/stb/fp/events failed!! (%m)");
			else if (events & FP_EVENT_VCR_SB_CHANGED)
				/* emit */ vcr_sb_notifier(getVCRSlowBlanking());
			fclose(f);
		}
		else
		{
			int val = FP_EVENT_VCR_SB_CHANGED;  // ask only for this event
			if (ioctl(m_fp_fd, FP_IOCTL_GET_EVENT, &val) < 0)
				eDebug("FP_IOCTL_GET_EVENT failed (%m)");
			else if (val & FP_EVENT_VCR_SB_CHANGED)
				/* emit */ vcr_sb_notifier(getVCRSlowBlanking());
		}
	}
}

eAVSwitch::~eAVSwitch()
{
	if ( m_fp_fd >= 0 )
		close(m_fp_fd);
}

eAVSwitch *eAVSwitch::getInstance()
{
	return instance;
}

bool eAVSwitch::haveScartSwitch()
{
	char tmp[255];
    int fd = open("/proc/stb/avs/0/input_choices", O_RDONLY);
    if(fd < 0) {
        eDebug("cannot open /proc/stb/avs/0/input_choices");
        return false;
    }
	read(fd, tmp, 255);
	close(fd);
	return !!strstr(tmp, "scart");
}

#ifdef QBOXHD
/* For disable/enable tuners and CI modules where the decoder enters/goes out in standby mode */
extern void disable_video_output(int cmd);
unsigned char in_standby_status=0;

void eAVSwitch::disable_scart_hdmi(int val)
{
	int fd_tmp;
	int tmp;

	FILE * file=NULL;
	char str[64];
	memset(str,0,64);
	file=fopen("/etc/fb.modes.supp","r");
	if(file!=NULL)
	{
		fgets(str,63,file);
		fclose(file);
	}
	else
		eDebug("There isn't the file of resolution\n");


	eDebug("Disable...: %d",val);

	if( (val!=ACT_RST) && (val!=DEACT_RST) )
	{
		eDebug("EROR!! -> UNKNOWN STANDBY MODE\n");
		return;
	}
	in_standby_status=val;
	/* Reset the tuners ...*/
	/* Reset of CI module in dvbci.cpp (eDVBCIInterfaces::rst_all_modules9 */
// 	if((fd_tmp = open("/dev/rst_0", O_WRONLY)) >= 0)
// 	{
// 		if(val==ACT_RST)	/* Enter in standby */
// 		{
// 			ioctl(fd_tmp,IOCTL_ACTIVATION_RST,&val);
// 			tmp=STANDBY_ON;
// 		}
// 		else if(val==DEACT_RST)	/* Exit in standby */
// 		{
// 			ioctl(fd_tmp,IOCTL_ACTIVATION_RST,&val);
// 			tmp=STANDBY_OFF;
// 		}
// 
// 		close(fd_tmp);
// 	}
// 	else
// 	{
// 		eDebug("cannot open /dev/rst_0");
// 		if(val==ACT_RST)	/* Enter in standby */
// 			tmp=STANDBY_ON;
// 		else if(val==DEACT_RST)	/* Exit in standby */
// 			tmp=STANDBY_OFF;
// 	}

	/* Set the correct value of scart standby parameter */
	if(val==ACT_RST)	/* Enter in standby */
		tmp=STANDBY_ON;
	else if(val==DEACT_RST)	/* Exit in standby */
		tmp=STANDBY_OFF;

	/* Disable video output ... */
	disable_video_output(val);

#ifndef QBOXHD_MINI
	/* Bypass of scart ... */
	if((fd_tmp = open("/dev/stv6414_0", O_WRONLY)) >= 0)
	{
		ioctl(fd_tmp,IOCTL_STANDBY_ON_OFF,&tmp);

		if(tmp==STANDBY_OFF)
		{
			if(strstr(str,"Scart")==NULL)	/* It isn't set the scart */
			{
				tmp=1;
				ioctl(fd_tmp,IOCTL_DISABLE_TV_OUTPUT,&tmp);
			}

			if( (strstr(str,"YPbPr")!=NULL) || (strstr(str,"HDMI")!=NULL) ) /* It is set the YPbPr */
			{
				system("stfbcontrol cy hr he");
			}
		}
		close(fd_tmp);
	}
	else
	{
		eDebug("Cannot open /dev/stv6414_0 ... ");
		eDebug("May be there isn't the driver because HDMI is connected");
		return;
	}
#else ///QBOXHD_MINI
	if((fd_tmp = open("/dev/scart_0", O_WRONLY)) >= 0)
	{
		if(tmp==STANDBY_ON)
		{
			tmp=0;
			ioctl(fd_tmp, IOCTL_ENABLE_SCART,&tmp);
		}
		else if(tmp==STANDBY_OFF)
		{
			if(strstr(str,"Scart")!=NULL)	/* It isn't set the scart */
			{
				tmp=1;
				ioctl(fd_tmp,IOCTL_ENABLE_SCART,&tmp);
			}
			else if( (strstr(str,"YPbPr")!=NULL) || (strstr(str,"HDMI")!=NULL) ) /* It is set the YPbPr */
			{
				system("stfbcontrol cy hr he");
			}
		}
		close(fd_tmp);
	}
	else
	{
		eDebug("Cannot open /dev/scart_0 ... ");
		eDebug("May be there isn't the driver because HDMI is connected");
		return;
	}
#endif
}

#else  /// !QBOXHD
extern unsigned char in_stand_by;
void eAVSwitch::standbyStatus(int val)
{
	in_stand_by=val;
}
void eAVSwitch::disable_scart_hdmi(int val)
{
	eDebug("Disable...: %d: DUMMY!",val);
}
#endif ///QBOXHD

void eAVSwitch::setInput(int val)
{
	/*
	0-encoder
	1-scart
	2-aux
	*/

	const char *input[] = {"encoder", "scart", "aux"};

	int fd;
    if((fd = open("/proc/stb/avs/0/input", O_WRONLY)) < 0) {
        eDebug("cannot open /proc/stb/avs/0/input");
        return;
    }
	write(fd, input[val], strlen(input[val]));
	close(fd);
}

void eAVSwitch::setFastBlank(int val)
{
	int fd;
	const char *fb[] = {"low", "high", "vcr"};

	if((fd = open("/proc/stb/avs/0/fb", O_WRONLY)) < 0) {
		eDebug("cannot open /proc/stb/avs/0/fb");
		return;
	}

	write(fd, fb[val], strlen(fb[0]));
	close(fd);
}

void eAVSwitch::setColorFormat(int format)
{
	/*
	0-CVBS
	1-RGB
	2-S-Video
	*/
	const char *cvbs="cvbs";
	const char *rgb="rgb";
	const char *svideo="svideo";
	const char *yuv="yuv";
	int fd;
    if((fd = open("/proc/stb/avs/0/colorformat", O_WRONLY)) < 0) {
        printf("cannot open /proc/stb/avs/0/colorformat\n");
        return;
    }
	switch(format) {
		case 0:
			write(fd, cvbs, strlen(cvbs));
			break;
		case 1:
			write(fd, rgb, strlen(rgb));
			break;
		case 2:
			write(fd, svideo, strlen(svideo));
			break;
		case 3:
			write(fd, yuv, strlen(yuv));
			break;
	}
	close(fd);
}

void eAVSwitch::setAspectRatio(int ratio)
{
	/*
	0-4:3 Letterbox
	1-4:3 PanScan
	2-16:9
	3-16:9 forced ("panscan")
	4-16:10 Letterbox
	5-16:10 PanScan
	6-16:9 forced ("letterbox")
	*/
	const char *aspect[] = {"4:3", "4:3", "any", "16:9", "16:10", "16:10", "16:9", "16:9"};
	const char *policy[] = {"letterbox", "panscan", "bestfit", "panscan", "letterbox", "panscan", "letterbox"};

	int fd;
	if((fd = open("/proc/stb/video/aspect", O_WRONLY)) < 0) {
		eDebug("cannot open /proc/stb/video/aspect");
		return;
	}
//	eDebug("set aspect to %s", aspect[ratio]);
	write(fd, aspect[ratio], strlen(aspect[ratio]));
	close(fd);

	if((fd = open("/proc/stb/video/policy", O_WRONLY)) < 0) {
		eDebug("cannot open /proc/stb/video/policy");
		return;
	}
//	eDebug("set ratio to %s", policy[ratio]);
	write(fd, policy[ratio], strlen(policy[ratio]));

	close(fd);
}

void eAVSwitch::setVideomode(int mode)
{
	const char *pal="pal";
	const char *ntsc="ntsc";

	if (mode == m_video_mode)
		return;

	if (mode == 2)
	{
        int fd1 = open("/proc/stb/video/videomode_50hz", O_WRONLY);
        if(fd1 < 0) {
            eDebug("cannot open /proc/stb/video/videomode_50hz");
            return;
        }
        int fd2 = open("/proc/stb/video/videomode_60hz", O_WRONLY);
        if(fd2 < 0) {
            eDebug("cannot open /proc/stb/video/videomode_60hz");
            close(fd1);
            return;
        }
		write(fd1, pal, strlen(pal));
		write(fd2, ntsc, strlen(ntsc));
		close(fd1);
		close(fd2);
	}
	else
	{
        int fd = open("/proc/stb/video/videomode", O_WRONLY);
        if(fd < 0) {
            eDebug("cannot open /proc/stb/video/videomode");
            return;
        }
		switch(mode) {
			case 0:
				write(fd, pal, strlen(pal));
				break;
			case 1:
				write(fd, ntsc, strlen(ntsc));
				break;
			default:
				eDebug("unknown videomode %d", mode);
		}
		close(fd);
	}

	m_video_mode = mode;
}

void eAVSwitch::setWSS(int val) // 0 = auto, 1 = auto(4:3_off)
{
	int fd;
    if((fd = open("/proc/stb/denc/0/wss", O_WRONLY)) < 0) {
        eDebug("cannot open /proc/stb/denc/0/wss");
        return;
    }
	const char *wss[] = {
		"off", "auto", "auto(4:3_off)", "4:3_full_format", "16:9_full_format",
		"14:9_letterbox_center", "14:9_letterbox_top", "16:9_letterbox_center",
		"16:9_letterbox_top", ">16:9_letterbox_center", "14:9_full_format"
	};
	write(fd, wss[val], strlen(wss[val]));
//	eDebug("set wss to %s", wss[val]);
	close(fd);
}


#ifdef QBOXHD
void set_scart_aspect(unsigned char d)
{
	int fd_tmp=0;
	int tmp=0;

#ifdef QBOXHD_MINI
	if((fd_tmp = open("/dev/scart_0", O_WRONLY)) >= 0)
	{
		if(d==169)
		{
			tmp=AR_16_9;
			ioctl(fd_tmp, IOCTL_SET_ASPECT_RATIO,&tmp);
		}
		else if(d==43)
		{
			tmp=AR_4_3;
			ioctl(fd_tmp,IOCTL_SET_ASPECT_RATIO,&tmp);
		}
		else
			eDebug("Unknown aspect ratio : %d",d);

		close(fd_tmp);
	}
	else
	{
		eDebug("Cannot open /dev/scart_0 ... ");
		eDebug("May be there isn't the driver because HDMI is connected");
		return;
	}
#else
	if((fd_tmp = open("/dev/stv6414_0", O_WRONLY)) >= 0)
	{
		if(d==169)
		{
			tmp=TV_16_9;
			ioctl(fd_tmp,IOCTL_SET_169_43,&tmp);
		}
		else if(d==43)
		{
			tmp=TV_4_3;
			ioctl(fd_tmp,IOCTL_SET_169_43,&tmp);
		}
		else
			eDebug("Unknown aspect ratio : %d",d);

		close(fd_tmp);
	}
	else
	{
		eDebug("Cannot open /dev/stv6414_0 ... ");
		eDebug("May be there isn't the driver because HDMI is connected");
		return;
	}

#endif
}
#endif

//FIXME: correct "run/startlevel"
eAutoInitP0<eAVSwitch> init_avswitch(eAutoInitNumbers::rc, "AVSwitch Driver");
