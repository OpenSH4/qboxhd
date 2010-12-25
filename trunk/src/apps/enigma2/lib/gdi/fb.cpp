#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <memory.h>
#include <linux/kd.h>

#include <lib/gdi/fb.h>

#ifdef QBOXHD
#include <errno.h>
#include <string.h>
#include <linux/stmfb.h>
#ifndef QBOXHD_MINI
	#include "../driver/stv6414_i2c.h"
#else
	#include "../driver/scart_mini.h"
#endif ///QBOXHD_MINI
#endif ///QBOXHD

#ifndef FBIO_WAITFORVSYNC
#define FBIO_WAITFORVSYNC _IOW('F', 0x20, __u32)
#endif

#ifndef FBIO_BLIT
#define FBIO_SET_MANUAL_BLIT _IOW('F', 0x21, __u8)
#define FBIO_BLIT 0x22
#endif

#ifdef QBOXHD
#define	FILE_NOT_FOUND	3

/* Global variable */
static int fd_support;

/* Prototype */
void disable_video_output(int cmd);

#endif ///QBOXHD

fbClass *fbClass::instance;

#ifdef QBOXHD
int read_fb_mode_supp(int *res, char *port)
{
	FILE * file=NULL;
	char *tmp1, *tmp2,str[64];
	int freq;
	memset(str,0,64);
	file=fopen("/etc/fb.modes.supp","r");
	if(file==NULL)
	{
		eDebug("There isn't the file for video resolution\n");
		return (-FILE_NOT_FOUND);
	}
	fgets(str,63,file);
	//eDebug("------->string:%s",str);
	tmp1=strchr(str,'@');
	tmp1++;
	tmp2=strchr(tmp1,'@');
	tmp2++;

	freq=atoi(tmp1);
	//eDebug("--------------->freq: %d",freq);

	memcpy(port,tmp2,strlen(tmp2));
	eDebug("PORT: %s",tmp2);

	*res=0;

	if (!strncmp(port, "Scart", 5))
	{
		if (!strncmp(str, "PAL", 3))
		{
			*res=576;
			eDebug("Set Scart in PAL mode\n");
			system("rm -fr /etc/fb.modes");
			system("ln -s /etc/fb_50i.modes /etc/fb.modes");
		}
		else if (!strncmp(str, "NTSC", 4))
		{
			*res=480;
			eDebug("Set Scart in NTSC mode\n");
			system("rm -fr /etc/fb.modes");
			system("ln -s /etc/fb_60i.modes /etc/fb.modes");
		}
		else
		{
			eDebug("Unknown Scart mode\n");
		}
	}
	else
	{
		if( (*(tmp1-2)=='i') && (freq==50) )
		{
			eDebug("INTERLACED %dHz",freq);
			system("rm -fr /etc/fb.modes");
			system("ln -s /etc/fb_50i.modes /etc/fb.modes");
		}
		else if( (*(tmp1-2)=='p') && (freq==50) )
		{
			eDebug("PROGRESSIVE %dHz",freq);
			system("rm -fr /etc/fb.modes");
			system("ln -s /etc/fb_50p.modes /etc/fb.modes");
		}
		else if( (*(tmp1-2)=='i') && (freq==60) )
		{
			eDebug("INTERLACED %dHz",freq);
			system("rm -fr /etc/fb.modes");
			system("ln -s /etc/fb_60i.modes /etc/fb.modes");
		}
		else if( (*(tmp1-2)=='p') && (freq==60) )
		{
			eDebug("PROGRESSIVE %dHz\n",freq);
			system("rm -fr /etc/fb.modes");
			system("ln -s /etc/fb_60p.modes /etc/fb.modes");
		}
		else
			eDebug("Unknown '%c' '%d'",*(tmp1-2),freq);
	}
	if (*res==0)
	{
		*res=atoi(str);
		eDebug("RESOLUTION: %d\n",*res);
	}

	if(file!=NULL)
		fclose(file);
}
#endif ///QBOXHD

fbClass *fbClass::getInstance()
{
	return instance;
}

fbClass::fbClass(const char *fb)
{
#ifdef QBOXHD
	primary = NULL;
	surf2 = NULL;
#endif ///QBOXHD
	m_manual_blit=-1;
	instance=this;
	locked=0;
	available=0;
	cmap.start=0;
	cmap.len=256;
	cmap.red=red;
	cmap.green=green;
	cmap.blue=blue;
	cmap.transp=trans;

#ifdef QBOXHD
	fd_support=fd=open(fb, O_RDWR);
#else
	fd=open(fb, O_RDWR);
#endif ///QBOXHD
	if (fd<0)
	{
		perror(fb);
		goto nolfb;
	}

	if (ioctl(fd, FBIOGET_VSCREENINFO, &screeninfo)<0)
	{
		perror("FBIOGET_VSCREENINFO");
		goto nolfb;
	}

	memcpy(&oldscreen, &screeninfo, sizeof(screeninfo));

	fb_fix_screeninfo fix;
	if (ioctl(fd, FBIOGET_FSCREENINFO, &fix)<0)
	{
		perror("FBIOGET_FSCREENINFO");
		goto nolfb;
	}

	available=fix.smem_len;
#ifndef QBOXHD
	m_phys_mem = fix.smem_start;
#endif  ///QBOXHD
	eDebug("%dk video mem", available/1024);
#ifndef QBOXHD
	lfb=(unsigned char*)mmap(0, available, PROT_WRITE|PROT_READ, MAP_SHARED, fd, 0);
	if (!lfb)
	{
		perror("mmap");
		goto nolfb;
	}
#endif // QBOXHD
	showConsole(0);

	enableManualBlit();

#ifdef QBOXHD
	try {
        int                     argc = 1,
                                surface_pitch;
        char                    **argv = NULL;
        void                    *surface_data;
        DFBSurfaceDescription   dsc;

		char port[16];
		int res_w_h;
		int ret_value;
//         DFBDisplayLayerDescription layer_dsc;

		memset(port,0,16);
    	ret_value=read_fb_mode_supp(&res_w_h,port);
		if(ret_value==(-FILE_NOT_FOUND))
		{
			eDebug("There isn't the resolution file. Using the FB resolution\n");
			struct fb_var_screeninfo tmpscreeninfo;
			if (ioctl(fd, FBIOGET_VSCREENINFO, &tmpscreeninfo)<0)
			{
				perror("FBIOGET_VSCREENINFO");
				goto nolfb;
			}
			res_w_h=tmpscreeninfo.yres;
		}


        DirectFB::Init( &argc, &argv );
        dfb = DirectFB::Create();
        dfb->SetCooperativeLevel(DFSCL_FULLSCREEN);

//      DFBDisplayLayerConfig       layer_config;
//      layer_config.flags = DLCONF_SURFACE_CAPS;
//      layer_config.surface_caps = DSCAPS_PREMULTIPLIED;
//      layer->SetConfiguration(layer_config);


        layer = dfb->GetDisplayLayer(DLID_PRIMARY);
//      layer->SetOpacity(0);
//      layer->SetDstColorKey(0x0, 0x0, 0x0);

		/* Primary surface*/
		dsc.flags = (DFBSurfaceDescriptionFlags)(DSDESC_CAPS | DSDESC_WIDTH | DSDESC_HEIGHT);
        dsc.caps = (DFBSurfaceCapabilities)(DSCAPS_PRIMARY);

        if(res_w_h==480)
		{
			dsc.width = 720;
    	    dsc.height = 480;
		}
		else if(res_w_h==576)
		{
			dsc.width = 720;
    	    dsc.height = 576;
		}
		else if(res_w_h==720)
		{
			dsc.width = 1280;
    	    dsc.height = 720;
		}
		else if(res_w_h==1080)
		{
			dsc.width = 1920;
    	    dsc.height = 1080;
		}

        primary = dfb->CreateSurface(dsc);
        primary->GetSize(&w, &h);
        eDebug("----> primary surface: w: %d, h: %d", w, h);

		dst_rect.w = w;
    	dst_rect.h = h;

		/* Second surface */
        dsc.flags = (DFBSurfaceDescriptionFlags)(DSDESC_CAPS | DSDESC_WIDTH | DSDESC_HEIGHT);
        dsc.caps = (DFBSurfaceCapabilities)(DLCAPS_ALPHACHANNEL);
/*
        dsc.width = 720;
        dsc.height = 576;
*/
        //dsc.width = screeninfo.xres;
        //dsc.height = screeninfo.yres;

        surf2 = dfb->CreateSurface(dsc);

        surf2->SetDstColorKey(0x0, 0x0, 0x0);
        surf2->Lock(DSLF_WRITE, &surface_data, &surface_pitch);
        lfb = (unsigned char *)surface_data;

        src_rect.x = 0;
        src_rect.y = 0;
        //src_rect.w = screeninfo.xres;
        //src_rect.h = screeninfo.yres;
		src_rect.w = w;
    	src_rect.h = h;
/*
        src_rect.w = 720;
        src_rect.h = 576;
*/
        dst_rect.x = 0;
        dst_rect.y = 0;

		if (!strncmp(port, "HDMI", 4))
		{
			eDebug("Enable HDMI and YPbPr\n");
			system("stfbcontrol cy hr he");
		}
		else if (!strncmp(port, "Scart", 5))
		{
			eDebug("Enable Scart \n");
			system("stfbcontrol cc cr hr he");
#ifndef QBOXHD_MINI
			int fd_tmp,tmp_stv=0;
			/* Bypass of scart ... */
			if((fd_tmp = open("/dev/stv6414_0", O_WRONLY)) >= 0)
			{
				ioctl(fd_tmp,IOCTL_DISABLE_TV_OUTPUT,&tmp_stv);//enable tv_out
				close(fd_tmp);
			}
#else
			int fd_tmp,tmp_stv=1;
			/* Bypass of scart ... */
			if((fd_tmp = open("/dev/scart_0", O_WRONLY)) >= 0)
			{
				ioctl(fd_tmp,IOCTL_ENABLE_SCART,&tmp_stv);//enable tv_out
				close(fd_tmp);
			}
#endif
		}
		else if (!strncmp(port, "YPbPr", 5))
		{
			eDebug("Enable HDMI and YPbPr\n");
			system("stfbcontrol cy hr he");
		}
		else
		{
			eDebug("Unknown port %s \n", port);
		}
    }
    catch (DFBException * ex) {
        eDebug("DirectFB exception: %s\n", ex->GetAction());
    }
#endif // QBOXHD

	return;
nolfb:
	lfb=0;
	printf("framebuffer not available.\n");
	return;
}

int fbClass::showConsole(int state)
{
#if defined(__sh__)
    int fd=open("/dev/ttyAS0", O_RDWR);
#else
	int fd=open("/dev/vc/0", O_RDWR);
	if(fd>=0)
	{
		if(ioctl(fd, KDSETMODE, state?KD_TEXT:KD_GRAPHICS)<0)
		{
			eDebug("setting /dev/vc/0 status failed.");
		}
		close(fd);
	}
#endif
	return 0;
}

#ifdef QBOXHD
bool changingResolution = false;

/* IVAN 2009_09_18 */
int fbClass::getfbResolution( unsigned int *xRes, unsigned int *yRes, unsigned int *bpp)
{
	struct fb_var_screeninfo tmpscreeninfo;

	if (ioctl(fd, FBIOGET_VSCREENINFO, &tmpscreeninfo)<0)
	{
		perror("FBIOGET_VSCREENINFO");
		return -1;
	}
	*xRes = tmpscreeninfo.xres;
	*yRes = tmpscreeninfo.yres;
	*bpp = tmpscreeninfo.bits_per_pixel;

	return 0;
}

/*
 * Change the display _and_ e2 resolution.
 * fbmode istaken from /etc/enigma2/stb/videomode_choices
 * The values of struct screeninfo are taken from fb.modes
 */
#if 0
int fbClass::SetStretchMode(char *fbmode)
{
    if (system("stfbset -n") < 0)
        eDebug("stfbset failed");

    return 0;
    // FIXME: OSD (graphics plane) may change resolution, but not video planes
    // thus, there's no need for fb's ioctl's
    eDebug("-----> %s(): Forcing fbmode: '%s'", __FUNCTION__, fbmode);
    DFBSurfaceDescription   dsc;
    if (ioctl(fd, FBIOGET_VSCREENINFO, &screeninfo) < 0){
		perror("FBIOGET_FSCREENINFO");
		printf("fb failed\n");
		return -1;
	}

    memset(&screeninfo, 0, sizeof(struct fb_var_screeninfo));

    screeninfo.nonstd = 0;
    screeninfo.activate = 0;
    screeninfo.height = 0;
    screeninfo.width = 0;
    screeninfo.accel_flags = 0;

    // 480p: 720x480-60
    if (!strncmp(fbmode, "480p", 4)) {
        screeninfo.xres_virtual = screeninfo.xres = 720;
        screeninfo.yres_virtual = screeninfo.yres = 480;
        screeninfo.xoffset = screeninfo.yoffset = 0;
        screeninfo.bits_per_pixel= 16;

        screeninfo.red.offset = 11;
        screeninfo.red.length = 5;
        screeninfo.green.offset = 5;
        screeninfo.green.length = 6;
        screeninfo.blue.offset = 0;
        screeninfo.blue.length = 5;
        screeninfo.transp.offset = 0;
        screeninfo.transp.length = 0;

        screeninfo.pixclock = 37000;
        screeninfo.left_margin = 60;
        screeninfo.right_margin = 16;
        screeninfo.upper_margin = 30;
        screeninfo.lower_margin = 9;
        screeninfo.hsync_len = 62;
        screeninfo.vsync_len = 6;
        screeninfo.sync = 3;
    }
    // pal: 720x576-50i
    else if (!strncmp(fbmode, "pal", 3)) {
        screeninfo.xres_virtual = screeninfo.xres = 720;
        screeninfo.yres_virtual = screeninfo.yres = 576;
        screeninfo.xoffset = screeninfo.yoffset = 0;
        screeninfo.bits_per_pixel= 16;

        screeninfo.red.offset = 11;
        screeninfo.red.length = 5;
        screeninfo.green.offset = 5;
        screeninfo.green.length = 6;
        screeninfo.blue.offset = 0;
        screeninfo.blue.length = 5;
        screeninfo.transp.offset = 0;
        screeninfo.transp.length = 0;

        screeninfo.pixclock = 37037;
        screeninfo.left_margin = 68;
        screeninfo.right_margin = 12;
        screeninfo.upper_margin = 39;
        screeninfo.lower_margin = 5;
        screeninfo.hsync_len = 64;
        screeninfo.vsync_len = 5;
        screeninfo.sync = 3;
        screeninfo.vmode = 0;
    }
    // 576p: 720x576-50
    else if (!strncmp(fbmode, "576p", 4)) {
        screeninfo.xres_virtual = screeninfo.xres = 720;
        screeninfo.yres_virtual = screeninfo.yres = 576;
        screeninfo.xoffset = screeninfo.yoffset = 0;
        screeninfo.bits_per_pixel= 16;

        screeninfo.red.offset = 11;
        screeninfo.red.length = 5;
        screeninfo.green.offset = 5;
        screeninfo.green.length = 6;
        screeninfo.blue.offset = 0;
        screeninfo.blue.length = 5;
        screeninfo.transp.offset = 0;
        screeninfo.transp.length = 0;

        screeninfo.pixclock = 37037;
        screeninfo.left_margin = 68;
        screeninfo.right_margin = 12;
        screeninfo.upper_margin = 39;
        screeninfo.lower_margin = 5;
        screeninfo.hsync_len = 64;
        screeninfo.vsync_len = 5;
    }
    // 720p50: 1280x720-50
    else if (!strncmp(fbmode, "720p50", 11)) {
        screeninfo.xres_virtual = screeninfo.xres = 1280;
        screeninfo.yres_virtual = screeninfo.yres = 720;
        screeninfo.xoffset = screeninfo.yoffset = 0;
        screeninfo.bits_per_pixel= 16;

        screeninfo.red.offset = 11;
        screeninfo.red.length = 5;
        screeninfo.green.offset = 5;
        screeninfo.green.length = 6;
        screeninfo.blue.offset = 0;
        screeninfo.blue.length = 5;
        screeninfo.transp.offset = 0;
        screeninfo.transp.length = 0;

        screeninfo.pixclock = 13468;
        screeninfo.left_margin = 220;
        screeninfo.right_margin = 440;
        screeninfo.upper_margin = 20;
        screeninfo.lower_margin = 5;
        screeninfo.hsync_len = 40;
        screeninfo.vsync_len = 5;
        screeninfo.sync = 3;
        screeninfo.vmode = 0;
    }
    // 1920x1080-50i
    else if (!strncmp(fbmode, "1080i", 5)) {
        screeninfo.xres_virtual = screeninfo.xres = 1920;
        screeninfo.yres_virtual = screeninfo.yres = 1080;
        screeninfo.xoffset = screeninfo.yoffset = 0;
        screeninfo.bits_per_pixel= 32;

        screeninfo.red.offset = 16;
        screeninfo.red.length = 8;
        screeninfo.green.offset = 8;
        screeninfo.green.length = 8;
        screeninfo.blue.offset = 0;
        screeninfo.blue.length = 8;
        screeninfo.transp.offset = 24;
        screeninfo.transp.length = 8;

        screeninfo.pixclock = 13468;
        screeninfo.left_margin = 148;
        screeninfo.right_margin = 528;
        screeninfo.upper_margin = 35;
        screeninfo.lower_margin = 5;
        screeninfo.hsync_len = 44;
        screeninfo.vsync_len = 5;
        screeninfo.sync = 3;
        screeninfo.vmode = 1;
    }
    else {
        printf("Unsupported mode '%s'", fbmode);
        return -1;
    }

    if (ioctl(fd, FBIOPUT_VSCREENINFO, &screeninfo) < 0 ) {
        perror("FBIOPUT_VSCREENINFO");
        printf("fb failed\n");
        return -1;
    }

    primary->Release();
    primary = NULL;

    dsc.flags = (DFBSurfaceDescriptionFlags)(DSDESC_CAPS | DSDESC_WIDTH | DSDESC_HEIGHT);
    dsc.caps = (DFBSurfaceCapabilities)(DSCAPS_PRIMARY);
    dsc.width = screeninfo.xres;
    dsc.height = screeninfo.yres;
    primary = dfb->CreateSurface(dsc);
    primary->GetSize(&w, &h);

    dst_rect.w = w;
    dst_rect.h = h;

    if (system("stfbset -n") < 0)
        eDebug("stfbset failed");

    return 0;
}
#endif

#ifdef QBOXHD_MINI
extern void suspend_display_dfb(int par);
#endif
char c=0;
int fbClass::SetStretchMode(char *fbmode)
{
    if (system("stfbset -n") < 0)
        eDebug("stfbset failed");

    // FIXME: OSD (graphics plane) may change resolution, but not video planes
    // thus, there's no need for fb's ioctl's

	if(c<1)
	{
		c++;
		return 0;
	}

	int wl,hl;

    eDebug("-----> %s(): Forcing fbmode: '%s'", __FUNCTION__, fbmode);
    DFBSurfaceDescription   dsc;

	char *tmp=NULL;
	char mode[16];
	char port[16];

	memset(mode,0,16);
	memset(port,0,16);

	tmp=strchr(fbmode,'@');
//	eDebug("\n");
//	eDebug("---->tmp: %s\n",tmp);

	memcpy(mode,fbmode,strlen(fbmode)-strlen(tmp));
	memcpy(port,tmp+1,strlen(tmp)-1);

//	eDebug("---->mode: %s\n",mode);
//	eDebug("---->port: %s\n",port);
//	eDebug("\n");
#ifdef QBOXHD
	changingResolution = true;
#endif

#ifdef QBOXHD_MINI
	suspend_display_dfb(1);
#endif
    disable_video_output(1);

	primary->Release();
    primary = NULL;

	surf2->GetSize(&wl, &hl);

	surf2->Unlock();
	surf2->Release();
	surf2 = NULL;
	dfb->Release();

	void                    *surface_data;
	int                     argc = 1,
                            surface_pitch;
	char                    **argv = NULL;

	dfb = NULL;

    // pal: 720x576-50i
    if (!strncmp(mode, "pal", 3))
	{
		screeninfo.xres_virtual = screeninfo.xres = 720;
        screeninfo.yres_virtual = screeninfo.yres = 576;
		system("rm -fr /etc/fb.modes");
		system("ln -s /etc/fb_50i.modes /etc/fb.modes");
    }
    else if (!strncmp(mode, "ntsc", 4))
	{
        screeninfo.xres_virtual = screeninfo.xres = 720;
        screeninfo.yres_virtual = screeninfo.yres = 480;
		system("rm -fr /etc/fb.modes");
		system("ln -s /etc/fb_60i.modes /etc/fb.modes");
    }
    // 480p: 720x480-60
    else if (!strncmp(mode, "480p", 4))
	{
        screeninfo.xres_virtual = screeninfo.xres = 720;
        screeninfo.yres_virtual = screeninfo.yres = 480;
		system("rm -fr /etc/fb.modes");
		system("ln -s /etc/fb_60p.modes /etc/fb.modes");
    }
    // 576p50: 720x576-50
    else if (!strncmp(mode, "576p50", 6))
	{
        screeninfo.xres_virtual = screeninfo.xres = 720;
        screeninfo.yres_virtual = screeninfo.yres = 576;
		system("rm -fr /etc/fb.modes");
		system("ln -s /etc/fb_50p.modes /etc/fb.modes");
    }
    // 576p: 720x576-60
    else if (!strncmp(mode, "576p", 4))
	{
        screeninfo.xres_virtual = screeninfo.xres = 720;
        screeninfo.yres_virtual = screeninfo.yres = 576;
		system("rm -fr /etc/fb.modes");
		system("ln -s /etc/fb_50p.modes /etc/fb.modes");
    }
    // 720p50: 1280x720-50
    else if (!strncmp(mode, "720p50", 6))
	{
        screeninfo.xres_virtual = screeninfo.xres = 1280;
        screeninfo.yres_virtual = screeninfo.yres = 720;
		system("rm -fr /etc/fb.modes");
		system("ln -s /etc/fb_50p.modes /etc/fb.modes");
    }
	// 720p: 1280x720-60
	else if (!strncmp(mode, "720p", 4))
	{
        screeninfo.xres_virtual = screeninfo.xres = 1280;
        screeninfo.yres_virtual = screeninfo.yres = 720;
		system("rm -fr /etc/fb.modes");
		system("ln -s /etc/fb_60p.modes /etc/fb.modes");
    }
    // 1920x1080-50i
    else if (!strncmp(mode, "1080i50", 7))
	{
        screeninfo.xres_virtual = screeninfo.xres = 1920;
        screeninfo.yres_virtual = screeninfo.yres = 1080;
		system("rm -fr /etc/fb.modes");
		system("ln -s /etc/fb_50i.modes /etc/fb.modes");
    }
	// 1920x1080-60i
	else if (!strncmp(mode, "1080i", 5))
	{
        screeninfo.xres_virtual = screeninfo.xres = 1920;
        screeninfo.yres_virtual = screeninfo.yres = 1080;
		system("rm -fr /etc/fb.modes");
		system("ln -s /etc/fb_60i.modes /etc/fb.modes");
    }
    else
	{
        eDebug("Unsupported mode '%s'", fbmode);
		disable_video_output(0);
#ifdef QBOXHD_MINI
		suspend_display_dfb(0);
#endif

#ifdef QBOXHD
		changingResolution = false;
#endif
        return -1;
    }

	/* No 'fdatasync' because it is for logical link and no files */
	system("sync");

	/* Set DirectFB resolutiom */
/*
    surf2->Release();
	primary->Release();
	dfb->Release();
    primary = NULL;

	void                    *surface_data;
	int                     argc = 1,
                            surface_pitch;
	char                    **argv = NULL;
	*/

	DirectFB::Init( &argc, &argv );
    dfb = DirectFB::Create();
    dfb->SetCooperativeLevel(DFSCL_FULLSCREEN);

    dsc.flags = (DFBSurfaceDescriptionFlags)(DSDESC_CAPS | DSDESC_WIDTH | DSDESC_HEIGHT);
    dsc.caps = (DFBSurfaceCapabilities)(DSCAPS_PRIMARY);
//	eDebug("\n");
//	eDebug("-------->screeninfo.xres: %d\n",screeninfo.xres);
//	eDebug("-------->screeninfo.yres: %d\n",screeninfo.yres);
//	eDebug("\n");

	dsc.width = screeninfo.xres;
    dsc.height = screeninfo.yres;
    primary = dfb->CreateSurface(dsc);
    primary->GetSize(&w, &h);

    dst_rect.x = 0;
    dst_rect.y = 0;

    dst_rect.w = w;
    dst_rect.h = h;


	dsc.width = wl;
    dsc.height = hl;
	dsc.flags = (DFBSurfaceDescriptionFlags)(DSDESC_CAPS | DSDESC_WIDTH | DSDESC_HEIGHT);
	dsc.caps = (DFBSurfaceCapabilities)(DLCAPS_ALPHACHANNEL);
    surf2 = dfb->CreateSurface(dsc);

    surf2->SetDstColorKey(0x0, 0x0, 0x0);
    surf2->Lock(DSLF_WRITE, &surface_data, &surface_pitch);
    lfb = (unsigned char *)surface_data;

    src_rect.x = 0;
    src_rect.y = 0;
    src_rect.w = wl;
    src_rect.h = hl;


	disable_video_output(0);
#ifdef QBOXHD_MINI
	suspend_display_dfb(0);
#endif
	/* Disbale tv_out */
#ifndef QBOXHD_MINI
	int fd_tmp,tmp_stv=1;
	if((fd_tmp = open("/dev/stv6414_0", O_WRONLY)) >= 0)
	{
		ioctl(fd_tmp,IOCTL_DISABLE_TV_OUTPUT,&tmp_stv);
		close(fd_tmp);
	}
#else
	int fd_tmp,tmp_stv=0;
	if((fd_tmp = open("/dev/scart_0", O_WRONLY)) >= 0)
	{
		ioctl(fd_tmp,IOCTL_ENABLE_SCART,&tmp_stv); // disable scart */
		close(fd_tmp);
	}
#endif


	if (!strncmp(port, "HDMI", 4))
		system("stfbcontrol cy hr he");
	else if (!strncmp(port, "Scart", 5))
	{
		system("stfbcontrol cc cr hr he");
#ifndef QBOXHD_MINI
		tmp_stv=0;
		if((fd_tmp = open("/dev/stv6414_0", O_WRONLY)) >= 0)
		{
			ioctl(fd_tmp,IOCTL_DISABLE_TV_OUTPUT,&tmp_stv);//enable tv_out
			close(fd_tmp);
		}
#else
		tmp_stv=1;
		if((fd_tmp = open("/dev/scart_0", O_WRONLY)) >= 0)
		{
			ioctl(fd_tmp,IOCTL_ENABLE_SCART,&tmp_stv);//enable tv_out
			close(fd_tmp);
		}
#endif
	}
	else if (!strncmp(port, "YPbPr", 5))
		system("stfbcontrol cy hr he");
	else
		eDebug("Unknown port %s \n", port);

	if (system("stfbset -n") < 0)
        eDebug("stfbset failed");

#ifdef QBOXHD
	changingResolution = false;
#endif
    return 0;
}
#endif // QBOXHD

int fbClass::SetMode(unsigned int nxRes, unsigned int nyRes, unsigned int nbpp)
{
#ifdef QBOXHD
	m_number_of_pages = 2; // FIXME: should be 1, no double buffering

    xRes = nxRes;
    yRes = nyRes;
    bpp = nbpp;

    stride = xRes * nbpp / 8;

    if (ioctl(fd, FBIOGET_VSCREENINFO, &screeninfo) < 0){
		perror("FBIOGET_VSCREENINFO");
		printf("fb failed\n");
		return -1;
	}

    printf ( "[FBIOGET_VSCREENINFO] FrameBuffer ScreenInfo w:%d h:%d\n", screeninfo.xres, screeninfo.yres );

/* IVAN 2009_09_18 */
#if 0
    dst_rect.x = 0;
    dst_rect.y = 0;
    dst_rect.w = screeninfo.xres;
    dst_rect.h = screeninfo.yres;

    if (dst_rect.w == 1920)
        dst_rect.w -= 10;

    memset(lfb, 0, stride*yRes);
#endif /// 0

/* IVAN 2009_09_18 */
	DFBSurfaceDescription   	dsc;
	int                         surface_pitch;
	void  	                *surface_data;

	primary->Release();
	primary = NULL;

	dsc.flags = (DFBSurfaceDescriptionFlags)(DSDESC_CAPS | DSDESC_WIDTH | DSDESC_HEIGHT);
	dsc.caps = (DFBSurfaceCapabilities)(DSCAPS_PRIMARY);

	/* Create primary surface and setting FrameBuffer resolution */
	dsc.width = screeninfo.xres;
	dsc.height = screeninfo.yres;

	primary = dfb->CreateSurface(dsc);
	primary->GetSize(&w, &h);

	printf("Setting Primary surface resolution %dx%d\n", w, h);

    dst_rect.x = 0;
    dst_rect.y = 0;

	dst_rect.w = screeninfo.xres;
	dst_rect.h = screeninfo.yres;

	surf2->Unlock();
	surf2->Release();
	surf2 = NULL;

	dsc.flags = (DFBSurfaceDescriptionFlags)(DSDESC_CAPS | DSDESC_WIDTH | DSDESC_HEIGHT | DVCAPS_SCALE);
	dsc.width = xRes;
	dsc.height = yRes;
	dsc.caps = (DFBSurfaceCapabilities)(DLCAPS_ALPHACHANNEL);

	surf2 = dfb->CreateSurface(dsc);
	surf2->SetDstColorKey(0x0, 0x0, 0x0);
	surf2->Lock(DSLF_WRITE, &surface_data, &surface_pitch);
	lfb = (unsigned char *)surface_data;

	surf2->GetSize(&w, &h);

	printf("Setting OSD surface resolution %dx%d\n", w, h);

    src_rect.x = 0;
    src_rect.y = 0;

    src_rect.w = w;
    src_rect.h = h;

#else  /// !QBOXHD
	screeninfo.xres_virtual=screeninfo.xres=nxRes;
	screeninfo.yres_virtual=(screeninfo.yres=nyRes)*2;
	screeninfo.height=0;
	screeninfo.width=0;
	screeninfo.xoffset=screeninfo.yoffset=0;
	screeninfo.bits_per_pixel=nbpp;

	switch (nbpp) {
	case 16:
		// ARGB 1555
		screeninfo.transp.offset = 15;
		screeninfo.transp.length = 1;
		screeninfo.red.offset = 10;
		screeninfo.red.length = 5;
		screeninfo.green.offset = 5;
		screeninfo.green.length = 5;
		screeninfo.blue.offset = 0;
		screeninfo.blue.length = 5;
		break;
	case 32:
		// ARGB 8888
		screeninfo.transp.offset = 24;
		screeninfo.transp.length = 8;
		screeninfo.red.offset = 16;
		screeninfo.red.length = 8;
		screeninfo.green.offset = 8;
		screeninfo.green.length = 8;
		screeninfo.blue.offset = 0;
		screeninfo.blue.length = 8;
		break;
	}

	if (ioctl(fd, FBIOPUT_VSCREENINFO, &screeninfo)<0)
	{
		// try single buffering
		screeninfo.yres_virtual=screeninfo.yres=nyRes;

		if (ioctl(fd, FBIOPUT_VSCREENINFO, &screeninfo)<0)
		{
			perror("FBIOPUT_VSCREENINFO");
			printf("fb failed\n");
			return -1;
		}
		eDebug(" - double buffering not available.");
	} else
		eDebug(" - double buffering available!");

	m_number_of_pages = screeninfo.yres_virtual / nyRes;

	ioctl(fd, FBIOGET_VSCREENINFO, &screeninfo);

	if ((screeninfo.xres!=nxRes) && (screeninfo.yres!=nyRes) && (screeninfo.bits_per_pixel!=nbpp))
	{
		eDebug("SetMode failed: wanted: %dx%dx%d, got %dx%dx%d",
			nxRes, nyRes, nbpp,
			screeninfo.xres, screeninfo.yres, screeninfo.bits_per_pixel);
	}
	xRes=screeninfo.xres;
	yRes=screeninfo.yres;
	bpp=screeninfo.bits_per_pixel;
	fb_fix_screeninfo fix;
	if (ioctl(fd, FBIOGET_FSCREENINFO, &fix)<0)
	{
		perror("FBIOGET_FSCREENINFO");
		printf("fb failed\n");
	}
	stride=fix.line_length;
	memset(lfb, 0, stride*yRes);
#endif // QBOXHD
	return 0;
}

int fbClass::setOffset(int off)
{
	screeninfo.xoffset = 0;
	screeninfo.yoffset = off;
	return ioctl(fd, FBIOPAN_DISPLAY, &screeninfo);
}

int fbClass::waitVSync()
{
	int c = 0;
	return ioctl(fd, FBIO_WAITFORVSYNC, &c);
}

void fbClass::blit()
{
#ifdef QBOXHD
//     primary->Blit(surf2, NULL, 0, 0);
     if( (primary) && (surf2) )
	{
        primary->StretchBlit(surf2, NULL, &dst_rect);
//         primary->Flip();
    }
#else
	if (m_manual_blit == 1) {
		if (ioctl(fd, FBIO_BLIT) < 0)
			perror("FBIO_BLIT");
	}
#endif
}

#ifdef QBOXHD
void fbClass::dfb_grab(char * dir, char * name)
{
	surf2->Dump(dir,name);
}
#endif


fbClass::~fbClass()
{
/*
#FIXME: Sometimes primary release blocked
#ifdef QBOXHD
    surf2->Unlock();
    surf2->Release();
    primary->Release();
    dfb->Release();
#endif
*/

	if (available)
		ioctl(fd, FBIOPUT_VSCREENINFO, &oldscreen);
	if (lfb)
		munmap(lfb, available);
	showConsole(1);
	disableManualBlit();
}

int fbClass::PutCMAP()
{
#if QBOXHD
    return 1;
#else
	return ioctl(fd, FBIOPUTCMAP, &cmap);
#endif
}

int fbClass::lock()
{
	if (locked)
		return -1;
	if (m_manual_blit == 1)
	{
		locked = 2;
		disableManualBlit();
	}
	else
		locked = 1;
	return fd;
}
#ifndef QBOXHD
void fbClass::unlock()
{
	if (!locked)
		return;
	if (locked == 2)  // re-enable manualBlit
		enableManualBlit();
	locked=0;
	SetMode(xRes, yRes, bpp);
	PutCMAP();
}
#else
void fbClass::unlock_parm(unsigned char sm)
{
	if (!locked)
		return;
	if (locked == 2)  // re-enable manualBlit
		enableManualBlit();
	locked=0;
	if(sm==1)
		SetMode(xRes, yRes, bpp);
	PutCMAP();
	system("stfbcontrol n");

}
void fbClass::unlock()
{
	unlock_parm(1);
}
#endif

void fbClass::enableManualBlit()
{
#ifndef QBOXHD
	unsigned char tmp = 1;
	if (ioctl(fd,FBIO_SET_MANUAL_BLIT, &tmp)<0)
		perror("FBIO_SET_MANUAL_BLIT");
	else
#endif
		m_manual_blit = 1;
}

void fbClass::disableManualBlit()
{
#ifndef QBOXHD
	unsigned char tmp = 0;
	if (ioctl(fd,FBIO_SET_MANUAL_BLIT, &tmp)<0)
		perror("FBIO_SET_MANUAL_BLIT");
	else
#endif
		m_manual_blit = 0;
}

#ifdef QBOXHD
/* This function permits to disable/enable the video output and the audio output */
void disable_video_output(int cmd)
{
	struct stmfbio_output_configuration outputConfig = {0};

	outputConfig.outputid = 1;
	if(ioctl(fd_support, STMFBIO_GET_OUTPUT_CONFIG, &outputConfig)<0)
		eDebug("Getting current output configuration failed");

	outputConfig.caps = 0;
	outputConfig.activate = STMFBIO_ACTIVATE_IMMEDIATE;

	eDebug("Disable_video_output... : %d",cmd);

	if(cmd==1)	/* disable video */
	{
		/* disable OUT_CVBS */
		outputConfig.caps |= STMFBIO_OUTPUT_CAPS_ANALOGUE_CONFIG;
		outputConfig.analogue_config &= ~STMFBIO_OUTPUT_ANALOGUE_CVBS;

		/* disable OUT_SVIDEO */
		outputConfig.caps |= STMFBIO_OUTPUT_CAPS_ANALOGUE_CONFIG;
		outputConfig.analogue_config &= ~STMFBIO_OUTPUT_ANALOGUE_YC;

		/* disable OUT_YUV */
		outputConfig.caps |= STMFBIO_OUTPUT_CAPS_ANALOGUE_CONFIG;
		outputConfig.analogue_config &= ~STMFBIO_OUTPUT_ANALOGUE_YPrPb;

		/* disable OUT_RGB */
		outputConfig.caps |= STMFBIO_OUTPUT_CAPS_ANALOGUE_CONFIG;
		outputConfig.analogue_config &= ~STMFBIO_OUTPUT_ANALOGUE_RGB;

		/* disable OUT_HDMI */
		outputConfig.caps |= STMFBIO_OUTPUT_CAPS_HDMI_CONFIG;
		outputConfig.hdmi_config |= STMFBIO_OUTPUT_HDMI_DISABLED;

		/* disable OUT_DVO */
		outputConfig.caps |= STMFBIO_OUTPUT_CAPS_DVO_CONFIG;
		outputConfig.hdmi_config |= STMFBIO_OUTPUT_DVO_DISABLED;
		eDebug("OutputConfig.analogue_config: 0x%04X",outputConfig.analogue_config);
		eDebug("OutputConfig.hdmi_config: 0x%04X",outputConfig.hdmi_config);

		if(ioctl(fd_support, STMFBIO_SET_OUTPUT_CONFIG, &outputConfig)<0)
	    	eDebug("Setting output configuration failed");

	}
	else if(cmd==0)	/* enable video */
	{
		/* enable OUT_CVBS */
		outputConfig.caps |= STMFBIO_OUTPUT_CAPS_ANALOGUE_CONFIG;
		outputConfig.analogue_config |= STMFBIO_OUTPUT_ANALOGUE_CVBS;
		eDebug("CVBS OutputConfig.analogue_config: 0x%04X",outputConfig.analogue_config);
		if(ioctl(fd_support, STMFBIO_SET_OUTPUT_CONFIG, &outputConfig)<0)
    		eDebug("Setting output configuration failed");

		/* enable OUT_SVIDEO */
		outputConfig.caps |= STMFBIO_OUTPUT_CAPS_ANALOGUE_CONFIG;
		outputConfig.analogue_config |= STMFBIO_OUTPUT_ANALOGUE_YC;
		eDebug("SVIDEO OutputConfig.analogue_config: 0x%04X",outputConfig.analogue_config);
		if(ioctl(fd_support, STMFBIO_SET_OUTPUT_CONFIG, &outputConfig)<0)
    		eDebug("Setting output configuration failed");

		/* enable OUT_YUV */
// 		outputConfig.caps |= STMFBIO_OUTPUT_CAPS_ANALOGUE_CONFIG;
// 		outputConfig.analogue_config |= STMFBIO_OUTPUT_ANALOGUE_YPrPb;
// 		eDebug("YPrPb OutputConfig.analogue_config: 0x%04X",outputConfig.analogue_config);
// 		if(ioctl(fd_support, STMFBIO_SET_OUTPUT_CONFIG, &outputConfig)<0)
//     		eDebug("Setting output configuration failed");

		/* enable OUT_RGB */
		outputConfig.caps |= STMFBIO_OUTPUT_CAPS_ANALOGUE_CONFIG;
		outputConfig.analogue_config |= STMFBIO_OUTPUT_ANALOGUE_RGB;
		eDebug("RGB OutputConfig.analogue_config: 0x%04X",outputConfig.analogue_config);
		if(ioctl(fd_support, STMFBIO_SET_OUTPUT_CONFIG, &outputConfig)<0)
    		eDebug("Setting output configuration failed");

		/* enable OUT_HDMI */
		outputConfig.caps |= STMFBIO_OUTPUT_CAPS_HDMI_CONFIG;
		outputConfig.hdmi_config &= ~STMFBIO_OUTPUT_HDMI_DISABLED;
		eDebug("HDMI OutputConfig.hdmi_config: 0x%04X",outputConfig.hdmi_config);
		if(ioctl(fd_support, STMFBIO_SET_OUTPUT_CONFIG, &outputConfig)<0)
    		eDebug("Setting output configuration failed");

		/* enable OUT_DVO */
		outputConfig.caps |= STMFBIO_OUTPUT_CAPS_DVO_CONFIG;
		outputConfig.hdmi_config &= ~STMFBIO_OUTPUT_DVO_DISABLED;
		eDebug("DVO OutputConfig.hdmi_config: 0x%04X",outputConfig.hdmi_config);
		if(ioctl(fd_support, STMFBIO_SET_OUTPUT_CONFIG, &outputConfig)<0)
    		eDebug("Setting output configuration failed");
	}
}
#endif /// QBOXHD

