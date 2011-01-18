/*
 * 	Copyright (C) 2010 Duolabs Srl
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

/*****************************
 * INCLUDES
 *****************************/

#include <sys/types.h> // undefines __USE_POSIX
#include <sys/poll.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/fb.h>

#include "duoutils.h"

/*****************************
 * MACROS
 *****************************/

#define lengthof(a) (sizeof(a) / sizeof((a)[0]))

#define NAME_SIZE 	256
#define FBDEV0		"/dev/fb0"
#define DISPLAY 	"/sys/class/stmcoredisplay/display%d/"
#define HDMI 		"hdmi%d.%d/"

#define USING_SCART	0
#define USING_HDMI 	1

#undef DEBUGD

#ifdef DEBUGD
#define fdebug(args...) fprintf(stderr, ##args)
#else
#define fdebug(args...)
#endif

/*****************************
 * DATA TYPES
 *****************************/

char progname[NAME_SIZE];

/* Status tell us how interface is connected, or we are using,
   status = USING_SCART --> we are using SCART
   satus  = USING_HDMI  --> we are usinf HDMI
*/
unsigned char status=0;


struct videomode {
	char standard;
	int x;
	int y;
	char scan;
	int refresh;
};

enum {
	ATTR_HDMI_NAME,
	ATTR_HDMI_CEA861,
	ATTR_HDMI_DISPLAY,
	ATTR_HDMI_HOTPLUG,
	ATTR_HDMI_MODES,
	ATTR_MAX
};

const char *attributes[ATTR_MAX] = {
	[ATTR_HDMI_NAME]    = DISPLAY HDMI "name",
	[ATTR_HDMI_CEA861]  = DISPLAY HDMI "cea861_codes",
	[ATTR_HDMI_DISPLAY] = DISPLAY HDMI "type",
	[ATTR_HDMI_HOTPLUG] = DISPLAY HDMI "hotplug",
	[ATTR_HDMI_MODES]   = DISPLAY HDMI "modes"
};

static int device_number = 0;
static int subdevice_number = 0;

/*****************************
 * FUNCTIONS IMPLEMENTATION
 *****************************/

int flush_video_buffer()
{
	int fbfd, fbsize, i;
	unsigned char *fbbuf;
	struct fb_var_screeninfo vinfo;

	fdebug("\n\nFLUSH VIDEO BUFFER \n\n");
	/* Open video memory */
	if ((fbfd = open("/dev/fb0", O_RDWR)) < 0) {
		fdebug("FLUSH ERROR !! \n");
		return -1;
	}

	/* Get variable display parameters */
	if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo)) {
		LOG_DUO(L_ERR, "Bad vscreeninfo ioctl\n");
		return -2;
	}

	/* Size of frame buffer =
	 (X-resolution * Y-resolution * bytes per pixel) */
	fbsize = vinfo.xres*vinfo.yres*(vinfo.bits_per_pixel/8);

	/* Map video memory */
	if ((fbbuf = mmap(0, fbsize, PROT_READ|PROT_WRITE,MAP_SHARED, fbfd, 0)) == (void *) -1)
	{
		fdebug("FLUSH ERROR \n");
		return -3;
	}

	/* Clear the screen */
	for (i=0; i<fbsize; i++) 
		*(fbbuf+i) = 110;


	munmap(fbbuf, fbsize);
	close(fbfd);
	return 0;
}

// 1920x1080-50i
static short int set_1920x1080i_50(int fd)
{
	struct fb_var_screeninfo screeninfo;

	if (ioctl(fd, FBIOGET_VSCREENINFO, &screeninfo) < 0 ) {
        fdebug("%s: FBIOGET_VSCREENINFO: %s\n", progname, strerror(errno));
        return -1;
    }

	memset(&screeninfo, 0, sizeof(struct fb_var_screeninfo));

	screeninfo.xres = screeninfo.xres_virtual = 1920;
	screeninfo.yres = screeninfo.yres_virtual = 1080;
	screeninfo.xoffset = 0;
	
	screeninfo.bits_per_pixel = 32;
	
	screeninfo.red.offset = 16;
	screeninfo.red.length = 8;
	screeninfo.green.offset = 8;
	screeninfo.green.length = 8;
	screeninfo.blue.offset = 0;
	screeninfo.blue.length = 8;
	screeninfo.transp.offset = 24;
	screeninfo.transp.length = 8;

	screeninfo.height = -1;
	screeninfo.width = -1;

	screeninfo.pixclock = 13468;
	screeninfo.left_margin = 148;
	screeninfo.right_margin = 528;
	screeninfo.upper_margin = 35;
	screeninfo.lower_margin = 5;
	screeninfo.hsync_len = 44;
	screeninfo.vsync_len = 5;
	screeninfo.sync = 3;
	screeninfo.vmode = 1;

	if (ioctl(fd, FBIOPUT_VSCREENINFO, &screeninfo) < 0 ) {
	        fdebug("%s: FBIOPUT_VSCREENINFO: %s\n", progname, strerror(errno));
	        return -1;
    }

	return 0;
}

// 1280x720p-50: 720p50 
static short int set_1280x720p_50(int fd)
{
	struct fb_var_screeninfo screeninfo;

	if (ioctl(fd, FBIOGET_VSCREENINFO, &screeninfo) < 0 ) 
	{
		fdebug("%s: FBIOGET_VSCREENINFO: %s\n", progname, strerror(errno));
		return -1;
        }

	memset(&screeninfo, 0, sizeof(struct fb_var_screeninfo));

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

	if (ioctl(fd, FBIOPUT_VSCREENINFO, &screeninfo) < 0 ) 
	{
		fdebug("%s: FBIOPUT_VSCREENINFO: %s\n", progname, strerror(errno));
		return -1;
        }

	return 0;
}

// 720x576-50: 576p50
static short int set_720x576p_50(int fd)
{
	struct fb_var_screeninfo screeninfo;

	if (ioctl(fd, FBIOGET_VSCREENINFO, &screeninfo) < 0 )
	{
		fdebug("%s: FBIOGET_VSCREENINFO: %s\n", progname, strerror(errno));
		return -1;
   	}

	memset(&screeninfo, 0, sizeof(struct fb_var_screeninfo));

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

	if (ioctl(fd, FBIOPUT_VSCREENINFO, &screeninfo) < 0 ) 
	{
		fdebug("%s: FBIOPUT_VSCREENINFO: %s\n", progname, strerror(errno));
		return -1;
        }

	return 0;
}

// 720x576i-50: 576i50: pal
static short int set_720x576i_50(int fd)
{
	struct fb_var_screeninfo screeninfo;

	memset(&screeninfo, 0, sizeof(struct fb_var_screeninfo));

	screeninfo.xres_virtual = screeninfo.xres = 720;
	screeninfo.yres_virtual = screeninfo.yres = 576;
	screeninfo.xoffset = screeninfo.yoffset = 0;
	screeninfo.xoffset = screeninfo.xoffset = 0;
	screeninfo.bits_per_pixel= 16;
	screeninfo.grayscale=0;
	
	screeninfo.red.offset = 11 ;
	screeninfo.red.length = 5 ;
	screeninfo.red.msb_right = 0 ;

	screeninfo.green.offset = 5 ;
	screeninfo.green.length = 6 ;
	screeninfo.green.msb_right = 0 ;

	screeninfo.blue.offset = 0 ;
	screeninfo.blue.length = 5 ;
	screeninfo.blue.msb_right = 0 ;

	screeninfo.transp.offset = 0 ;
	screeninfo.transp.length = 0 ;
	screeninfo.transp.msb_right = 0 ;

	screeninfo.nonstd = 0 ;
	screeninfo.activate = 0 ;
	screeninfo.height = -1 ;
	screeninfo.width = -1 ;
	screeninfo.accel_flags = 0 ;

	screeninfo.pixclock = 74074 ;

	screeninfo.left_margin = 69 ;
	screeninfo.right_margin= 12 ;
	screeninfo.upper_margin = 41 ;
	screeninfo.lower_margin = 5 ;

	screeninfo.hsync_len = 63; 
	screeninfo.vsync_len = 3 ;
	screeninfo.sync = 0 ;
	screeninfo.vmode = 1 ;

	if (ioctl(fd, FBIOPUT_VSCREENINFO, &screeninfo) < 0 ) {
		fdebug("%s: FBIOPUT_VSCREENINFO: %s\n", progname, strerror(errno));
		return -1;
   	}

	return 0;
}


char *get_attribute(int attr)
{
	char attrname[256];

	snprintf(attrname, 256, attributes[attr], device_number, device_number, subdevice_number);

	int fd = open(attrname, O_RDONLY);
	if (fd < 0)
		return NULL;

	long page_size = sysconf(_SC_PAGESIZE);

	char *p = malloc(page_size + 1);
	if (!p) {
		return NULL;
	}

	ssize_t sz = read(fd, p, page_size);
	close(fd);
	if (sz < 0 || sz > page_size) {
		free(p);
		return NULL;
	}

	// ensure string functions can safely operate on the attribute
	p[sz] = '\0';
	
	return p;
}

int get_display_name(char *name, int len)
{
	char *attr = get_attribute(ATTR_HDMI_NAME);
	if (!attr)
		return -1;

	int res = -1;
	int attr_len = strlen(attr);
	if (attr_len <= len && attr[attr_len - 1] == '\n') {
		// copy but swallow the trailing '\n'
		memcpy(name, attr, attr_len - 1);
		name[attr_len - 1] = '\0';

		res = 0;
	}

	free(attr);
	return res;
}

bool show_display_name(void)
{
	char name[32];
	int res = get_display_name(name, sizeof(name));
	if (res < 0) {
		fdebug("ERROR: Cannot determine display name\n");
		return false;
	}

	if (0 == strcmp("UNKNOWN", name)) {
		fdebug("Display device is not responding (turned off?) or gives no name\n");
		return false;
	}

	printf("Display name: %s\n", name);
	return true;
}

int hdmi_connected(void)
{
	int res = -1;
	char *attr = get_attribute(ATTR_HDMI_HOTPLUG);

	if (!attr) {
		fdebug("%s: Cannot determine HDMI connection status of display device\n",
			progname);
		fdebug("%s: Setting SCART mode\n", progname);
		return res;
	}

	if (!strcmp(attr, "y\n")) {
		res = 0;
	} 
	else if (!strcmp(attr, "n\n")) {
		fdebug("%s: HDMI display device is NOT connected. Setting SCART mode\n",progname);
	}

	free(attr);
	return res;
}

int get_modes(struct videomode *modes, int len)
{
	char *attr = get_attribute(ATTR_HDMI_MODES);
	if (!attr)
		return -1;

	char *mode, *r;
	int num_modes = 0;
	for (mode = strtok_r(attr, "\n", &r); mode; mode = strtok_r(NULL, "\n", &r)) {
		if (num_modes >= len) {
			free(attr);
			return -1;
		}

		if (5 != sscanf(mode, "%c:%dx%d%c-%d",
		                &modes[num_modes].standard, &modes[num_modes].x, &modes[num_modes].y,
		                &modes[num_modes].scan, &modes[num_modes].refresh)) {
			free(attr);
			return -1;
		}

		num_modes++;
	}

	free(attr);
	return num_modes;
}


/* function to update configuration file "/etc/directfbrc" that is used by user space apps to startup */
/* Mode map: 
	0 	1920x1080-50i
	1 	1280x720-50p
	2 	1280x720-50i	
	3 	720x576-50p
	4	720x576-50i
	5	720x576i_50  --> SCART !!! 

*/
#if 0
int configuration_file_update(unsigned char mode)
{
	char dfb_mode[NAME_SIZE / 8]; // 32 bytes
	switch (mode) {
		case 0:
			sprintf(dfb_mode, "1920x1080-50i");
			break;
		case 1:
			sprintf(dfb_mode, "1280x720-50p");
			break;
		case 2:
			sprintf(dfb_mode, "1280x720-50i");
			break;
		case 3:
			sprintf(dfb_mode, "720x576-50p");
			break;
		case 4:
			sprintf(dfb_mode, "720x576-50i");
			break;	
		case 5:
		default:
			sprintf(dfb_mode, "720x576-50i");	
			break;
	}

	if (config_file_param_write("/etc/directfbrc", "mode", dfb_mode) < 0) {
		LOG_DUO(L_ERR, "Could not save new mode '%s'", mode);
		return -1;
	}
	return 0;
}
#endif

#define	FILE_NOT_FOUND	3
#define	NO_SAVED_RES	4

int read_fb_mode_supp(int *res_x, int *res_y, int *freq, unsigned char *mode, char *port)
{
	FILE * file=NULL;
	char *tmp1, *tmp2,str[64];
	int res;

	memset(str,0,64);

	file=fopen("/etc/fb.modes.supp","r");
	if(file==NULL)
	{
		printf("The video resolution file doesn't exist\n");
		return (-FILE_NOT_FOUND);
	}
	fgets(str,63,file);
	fclose(file);

	/* Check if this is a saved resolution */
	if (strstr(str, "SAVED")==NULL)
	{
		printf("No saved resolution\n");
		return (-NO_SAVED_RES);
	}

	tmp1=strchr(str,'@');	/* freq */
	tmp1++;
	tmp2=strchr(tmp1,'@');	/* port (+ 'saved') */
	tmp2++;
	
	*freq=atoi(tmp1);
	res=0;

	memcpy(port,tmp2,strlen(tmp2));

	if (!strncmp(port, "Scart", 5))
	{
		if (!strncmp(str, "PAL", 3))
		{
			res=576;
			*res_x=720;
			*res_y=576;
			*mode='i';
			printf("Set Scart in PAL mode\n");
			system("rm -fr /etc/fb.modes");
			system("ln -s /etc/fb_50i.modes /etc/fb.modes");
		}
		else if (!strncmp(str, "NTSC", 4))
		{
			res=480;
			*res_x=720;
			*res_y=576;
			*mode='i';
			printf("Set Scart in NTSC mode\n");
			system("rm -fr /etc/fb.modes");
			system("ln -s /etc/fb_60i.modes /etc/fb.modes");
		}
		else
		{
			printf("Unknown Scart mode\n");
		}
	}
	else
	{
		if( (*(tmp1-2)=='i') && (*freq==50) )
		{
			*mode='i';
			printf("INTERLACED %dHz\n",*freq);
			system("rm -fr /etc/fb.modes");
			system("ln -s /etc/fb_50i.modes /etc/fb.modes");
		}
		else if( (*(tmp1-2)=='p') && (*freq==50) )
		{
			*mode='p';
			printf("PROGRESSIVE %dHz\n",*freq);
			system("rm -fr /etc/fb.modes");
			system("ln -s /etc/fb_50p.modes /etc/fb.modes");
		}
		else if( (*(tmp1-2)=='i') && (*freq==60) )
		{
			*mode='i';

			printf("INTERLACED %dHz\n",*freq);
			system("rm -fr /etc/fb.modes");
			system("ln -s /etc/fb_60i.modes /etc/fb.modes");
		}
		else if( (*(tmp1-2)=='p') && (*freq==60) )
		{
			*mode='p';
			printf("PROGRESSIVE %dHz\n",*freq);
			system("rm -fr /etc/fb.modes");
			system("ln -s /etc/fb_60p.modes /etc/fb.modes");
		}
		else	
			printf("Unknown '%c' '%d'",*(tmp1-2),*freq);
	}

	if (res==0)
	{
		res=atoi(str);
		printf("RESOLUTION: %d\n",res);
	
		if(res==480)
		{
			*res_x = 720;
    	    *res_y = 480;
		}
		else if(res==576)
		{
			*res_x = 720;
    	    *res_y = 576;
		}
		else if(res==720)
		{
			*res_x = 1280;
    	    *res_y = 720;
		}
		else if(res==1080)
		{
			*res_x = 1920;
    	    *res_y = 1080;
		}
	}
	
	system("sync");

	return 0;
}

int only_read_fb_supp(char * str)
{
	FILE * file=NULL;

	memset(str,0,64);

	file=fopen("/etc/fb.modes.supp","r");
	if(file==NULL)
	{
		printf("The video resolution file doesn't exist\n");
		return (-FILE_NOT_FOUND);
	}
	fgets(str,63,file);
	fclose(file);

	return 0;
}

/* function to update configuration file "/etc/directfbrc" that is used by user space apps to startup */
/* Mode map: 
	0 	1920x1080-50i
	1 	1280x720-50p
	2 	1280x720-50i	
	3 	720x576-50p
	4	720x576-50i
	5	720x576i_50  --> SCART !!! 

*/

void set_res(unsigned char mode)
{
	char dfb_mode[32];
	memset(dfb_mode,0,32);	

	switch (mode) {
		case 0:
			sprintf(dfb_mode, "fbset '1920x1080-50i'");
			break;
		case 1:
			sprintf(dfb_mode, "fbset '1280x720-50'");
			break;
		case 2:
			sprintf(dfb_mode, "fbset '1280x720-50i'");
			break;
		case 3:
			sprintf(dfb_mode, "fbset '720x576-50'");
			break;
		case 4:
			sprintf(dfb_mode, "fbset '720x576-50i'");
			break;	
		case 5:
			sprintf(dfb_mode, "fbset '720x576-50i'");	
			break;

		case 10:	//1080i60
#ifndef INITRAMFS
			system("rm -fr /etc/fb.modes");
			system("ln -s /etc/fb_60i.modes /etc/fb.modes");
#endif
			sprintf(dfb_mode, "fbset '1920x1080-60i'");	
			break;
		case 11:	//720p60
#ifndef INITRAMFS
			system("rm -fr /etc/fb.modes");
			system("ln -s /etc/fb_60p.modes /etc/fb.modes");
#endif
			sprintf(dfb_mode, "fbset '1280x720-60'");	
			break;
		case 12:	//480p60
#ifndef INITRAMFS
			system("rm -fr /etc/fb.modes");
			system("ln -s /etc/fb_60p.modes /etc/fb.modes");
#endif
			sprintf(dfb_mode, "fbset '720x480-60'");	
			break;

		default:
			sprintf(dfb_mode, "fbset '720x576-50i'");	
			break;
	}

	if (mode>=10) 
		system("sync");
	system(dfb_mode);
}

int configuration_file_update(unsigned char mode, char * p)
{
#ifndef INITRAMFS
	char dfb_mode[32];
	FILE * file=NULL;

	memset(dfb_mode,0,32);	

	switch (mode) {
		case 0:
			//sprintf(dfb_mode, "1920x1080-50i");
			sprintf(dfb_mode, "1080i@50Hz@HDMI\n");
			break;
		case 1:
			//sprintf(dfb_mode, "1280x720-50p");
			sprintf(dfb_mode, "720p@50Hz@HDMI\n");
			break;
		case 2:
			//sprintf(dfb_mode, "1280x720-50i");
			sprintf(dfb_mode, "720i@50Hz@HDMI\n");
			break;
		case 3:
			//sprintf(dfb_mode, "720x576-50p");
			sprintf(dfb_mode, "576p@50Hz@HDMI\n");
			break;
		case 4:
			//sprintf(dfb_mode, "720x576-50i");
			sprintf(dfb_mode, "PAL@50Hz@Scart\n");
			break;	
		case 5:
			//sprintf(dfb_mode, "720x576-50i");	
			sprintf(dfb_mode, "PAL@50Hz@Scart\n");
			break;	
		case 10:
			sprintf(dfb_mode, "1080i@60Hz@HDMI\n");
			break;	
		case 11:
			sprintf(dfb_mode, "720p@60Hz@HDMI\n");
			break;	
		case 12:
			sprintf(dfb_mode, "480p@60Hz@HDMI\n");
			break;	
		default:
			//sprintf(dfb_mode, "720x576-50i");	
			sprintf(dfb_mode, "PAL@50Hz@Scart\n");
			break;
	}

	if(strcmp(dfb_mode,p)==0)	/* have the same resolution...*/
	{
		printf("Resolution '%s' was already present, discarding\n",p);
		set_res(mode);
		return 0;
	}

	system("rm -fr /etc/fb.modes.supp");
	system("sync");

	file=fopen("/etc/fb.modes.supp","w");
	fputs(dfb_mode,file);
	fclose(file);
	system("sync");
#endif

	set_res(mode);

	return 0;
}

bool set_hdmi_modes(char * port)
{
	int i, fd;
	struct videomode modes[64];
	int res = get_modes(modes, lengthof(modes));

	if (res < 0) {
		fdebug("%s: Cannot determine available display modes\n", progname);
		return -1;
	}
	
	if ((fd = open(FBDEV0, O_RDWR)) < 0) {
		fdebug("%s: Could not open frame buffer device\n", progname);
		return -1;
	}

	// Search the best TV LCD mode that we support and set it
	// 1920x1080-50i
	for (i = 0; i < res; i++) {
		if (modes[i].y == 1080 && modes[i].scan == 'i' && modes[i].refresh == 50) {
			printf("Setting mode to %dx%d%c-%d\n", 
				modes[i].x, modes[i].y, modes[i].scan, modes[i].refresh);
			if (set_1920x1080i_50(fd) < 0) {
				fdebug("%s: Cannot set 1920x1080-50i mode\n", progname);
				return -1;
			}
			else {
#ifndef INITRAMFS
				configuration_file_update(0,port);
#endif
				return 0;

			}
		}
	}

	// 1280x720p-50: 720p50 
	for (i = 0; i < res; i++) {
		if (modes[i].y == 720 && modes[i].scan == 'p' && modes[i].refresh == 50) {
			printf("Setting mode to %dx%d%c-%d\n", 
				modes[i].x, modes[i].y, modes[i].scan, modes[i].refresh);
			if (set_1280x720p_50(fd) < 0) {
				fdebug("%s: Cannot set 1280x720p-50 mode\n", progname);
				return -1;
			}
			else {
#ifndef INITRAMFS
				configuration_file_update(1,port);	
#endif
				return 0;
			}
		}
	}

	// 720x576-50: 576p50
	for (i = 0; i < res; i++) {
		if (modes[i].y == 576 && modes[i].scan == 'p' && modes[i].refresh == 50) {
			printf("Setting mode to %dx%d%c-%d\n", 
				modes[i].x, modes[i].y, modes[i].scan, modes[i].refresh);
			if (set_720x576p_50(fd) < 0) {
				fdebug("%s: Cannot set 720x576p-50 mode\n", progname);
				return -1;
			}
			else {
#ifndef INITRAMFS
				configuration_file_update(3,port);	
#endif
				return 0;
			}
		}
	}

	// 720x576i-50: 576i50: pal
	for (i = 0; i < res; i++) {
		if (modes[i].y == 576 && modes[i].scan == 'i' && modes[i].refresh == 50) {
			printf("Setting mode to %dx%d%c-%d\n", 
				modes[i].x, modes[i].y, modes[i].scan, modes[i].refresh);
			if (set_720x576i_50(fd) < 0) {
				fdebug("%s: Cannot set 720x576i-50 mode\n", progname);
				return -1;
			}
			else {	
#ifndef INITRAMFS
				configuration_file_update(4,port);	
#endif
				return 0;
			}
		}
	}

	/* Case for 60 Hz */
	// 1920x1080i-60: 1080i60 
	for (i = 0; i < res; i++) 
	{
		if (modes[i].y == 1080 && modes[i].scan == 'i' && modes[i].refresh == 60) 
		{
			printf("Setting mode to %dx%d%c-%d\n", 
				modes[i].x, modes[i].y, modes[i].scan, modes[i].refresh);
			configuration_file_update(10,port);
			return 0;
		}
	}
	// 1280x720p-60: 720p60 
	for (i = 0; i < res; i++) {
		if (modes[i].y == 720 && modes[i].scan == 'p' && modes[i].refresh == 60) {
			printf("Setting mode to %dx%d%c-%d\n", 
				modes[i].x, modes[i].y, modes[i].scan, modes[i].refresh);
			configuration_file_update(11,port);	
			return 0;
		}
	}

	// 720x480p-60: 480p60
	for (i = 0; i < res; i++) {
		if (modes[i].y == 480 && modes[i].scan == 'p' && modes[i].refresh == 60) {
			printf("Setting mode to %dx%d%c-%d\n", 
				modes[i].x, modes[i].y, modes[i].scan, modes[i].refresh);
			configuration_file_update(12,port);	
			return 0;
		}
	}

	close(fd);
	
	return 0;
}


bool set_scart_mode(char * port)
{
	int fd;

	if ((fd = open(FBDEV0, O_RDWR)) < 0) {
		fdebug("%s: Could not open frame buffer device\n", progname);
		return -1;
	}
	fdebug("Open device ok ! \n");

	if (set_720x576i_50(fd) < 0) {
		fdebug("%s: Cannot set 720x576i-50 mode\n", progname);
		return -1;
	}
	else
        configuration_file_update(5,port);	

	// TODO: save settings to file /etc/directfbrc

	return 0;
}

int main(int argc, char *argv[])
{
#ifndef INITRAMFS
	int 			ret = 0, res_x = 0,res_y = 0,freq = 0;
	char 			port[64];
	char 			*t1 = NULL;
	unsigned char 	mode = 'i';
	FILE 			*tmp_f = NULL;

	memset(port,0,64);

	printf("Version of dvo_control: 0.0.1\n");

	if ((argc == 2) && (strcmp("EMG", argv[1]) == 0) ) {
		only_read_fb_supp(port);
		t1=strstr(port,"SAVED");
		if (t1 != NULL) {
			port[strlen(port)-strlen(t1)-1]='\n';
			port[strlen(port)-strlen(t1)]='\0';

			system("rm -fr /etc/fb.modes.supp");
			system("sync");
	
			tmp_f=fopen("/etc/fb.modes.supp","w");
			fputs(port,tmp_f);
			fclose(tmp_f);
			system("sync");	
		}
		return 0;
	}

	ret = read_fb_mode_supp(&res_x, &res_y, &freq, &mode, port);
	if (ret == 0) {
		/* Enable port */
		if (!strncmp(port, "HDMI", 4)) {
			printf("Enable HDMI and YPbPr\n");
			system("stfbcontrol cy hr he");
		}
		else if (!strncmp(port, "Scart", 5)) {
			printf("Enable Scart \n");
			system("stfbcontrol cc cr hr he");
		}
		else if (!strncmp(port, "YPbPr", 5)) {
			printf("Enable HDMI and YPbPr\n");
			system("stfbcontrol cy hr he");
		}
		else
			printf("Unknown port %s \n", port);

		/* Set configuration */
		if(mode=='i')
			sprintf(port, "fbset '%dx%d-%d%c'",res_x,res_y,freq,mode);
		else
			sprintf(port, "fbset '%dx%d-%d'",res_x,res_y,freq);	/* Not explicit 'p' for progressive */
		system(port);
		
		return 0;
	}	
	else if(ret==(-NO_SAVED_RES)) {
		printf("Read all resolution of file\n");		
		only_read_fb_supp(port);	
	}

	reset_buff(progname, NAME_SIZE);
	strncpy(progname, argv[0], NAME_SIZE);
#endif

	// If we have the HDMI cable plugged in
	if (hdmi_connected() == 0) {
		// Find the modes that the LCD TV supports and set the best one for us
#ifdef INITRAMFS
		set_hdmi_modes(0);
#else
		set_hdmi_modes(port);
#endif
		flush_video_buffer();
		status = USING_HDMI;
		fdebug("\n %s :: USING_HDMI\n", argv[0]);
	}
	// else assume the SCART is connected and set a mode for it
#ifndef INITRAMFS
	else {
		set_scart_mode(port);
		flush_video_buffer();
		status = USING_SCART;
		fdebug("\n %s :: USING_SCART\n", argv[0]);
	}
#endif

	return 0;
}

