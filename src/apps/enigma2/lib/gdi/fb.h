#ifndef __FB_H
#define __FB_H

#include <lib/base/eerror.h>
#include <linux/fb.h>

#ifdef QBOXHD
#include <dfb++.h>
#endif

class fbClass
{
	int fd;
	unsigned int xRes, yRes, stride, bpp;
	int available;
	struct fb_var_screeninfo screeninfo, oldscreen;
	fb_cmap cmap;
	__u16 red[256], green[256], blue[256], trans[256];
	static fbClass *instance;
	int locked;

	int m_manual_blit;
	int m_number_of_pages;
#ifndef QBOXHD
	int m_phys_mem;
#endif
#ifdef SWIG
	fbClass(const char *fb="/dev/fb/0");
	~fbClass();
public:
#else
public:
	unsigned char *lfb;
	void enableManualBlit();
	void disableManualBlit();
	int showConsole(int state);
	int SetMode(unsigned int xRes, unsigned int yRes, unsigned int bpp);
	int Available() { return available; }

	int getNumPages() { return m_number_of_pages; }
#ifndef QBOXHD
	unsigned long getPhysAddr() { return m_phys_mem; }
#endif
	int setOffset(int off);
	int waitVSync();
	void blit();
	unsigned int Stride() { return stride; }
	fb_cmap *CMAP() { return &cmap; }

	fbClass(const char *fb="/dev/fb0");
	~fbClass();

			// low level gfx stuff
	int PutCMAP();
#endif
	static fbClass *getInstance();

	int lock();
#ifdef QBOXHD
	void unlock_parm(unsigned char sm);
#endif
	void unlock();
	int islocked() { return locked; }
	int SetStretchMode(char *fbmode);
// 	int SetStretchMode(unsigned int xRes, unsigned int yRes, unsigned int bpp, unsigned int pixclock, unsigned int left_margin, unsigned int right_margin, unsigned int upper_margin, unsigned int lower_margin, unsigned int hsync_len, unsigned int vsync_len);

#ifdef QBOXHD

	void dfb_grab(char * dir, char * name);

	/* IVAN 2009_09_18 */
	int getfbResolution(unsigned int *xRes, unsigned int *yRes, unsigned int *bpp);
	IDirectFB              *dfb;
	IDirectFBDisplayLayer  *layer;
	IDirectFBSurface       *primary,
	                       *surf2;
	IDirectFBPalette       *palette;
    DFBRectangle           src_rect;
    DFBRectangle           dst_rect;
    DFBException           *ex;
	int w;
	int h;
#endif // QBOXHD
};

#endif
