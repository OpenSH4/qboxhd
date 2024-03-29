#include <lib/gdi/gfbdc.h>

#include <lib/base/init.h>
#include <lib/base/init_num.h>
#ifndef  QBOXHD
#include <lib/gdi/accel.h>
#endif

#include <time.h>

gFBDC *gFBDC::instance;

ePtr<gFBDC> NewgFBDCPtr(void)
{
	ePtr<gFBDC> ptr;
	gFBDC::getInstance(ptr);
	return ptr;
}

gFBDC::gFBDC()
{
	instance=this;
	fb=new fbClass;

	if (!fb->Available())
		eFatal("no framebuffer available");

	surface.clut.data = 0;
#ifdef QBOXHD
	/* IVAN 2009_09_18 */
	{
		unsigned int xres, yres, bpp;

		/* Read from FrameBuffer the resolution */
		if (fb->getfbResolution( &xres, &yres, &bpp) < 0)
			eFatal("Framebuffer Error");
	
		printf("[gFBDC] FrameBuffer Resolution read : %dx%dx\n", xres, yres );
		setResolution( xres, yres );
	}
#else
	setResolution(720, 576); // default res
#endif /// QBOXHD

	reloadSettings();
}

gFBDC::~gFBDC()
{
	delete fb;
	delete[] surface.clut.data;
	instance=0;
}

void gFBDC::calcRamp()
{
#if 0
	float fgamma=gamma ? gamma : 1;
	fgamma/=10.0;
	fgamma=1/log(fgamma);
	for (int i=0; i<256; i++)
	{
		float raw=i/255.0; // IIH, float.
		float corr=pow(raw, fgamma) * 256.0;

		int d=corr * (float)(256-brightness) / 256 + brightness;
		if (d < 0)
			d=0;
		if (d > 255)
			d=255;
		ramp[i]=d;

		rampalpha[i]=i*alpha/256;
	}
#endif
	for (int i=0; i<256; i++)
	{
		int d;
		d=i;
		d=(d-128)*(gamma+64)/(128+64)+128;
		d+=brightness-128; // brightness correction
		if (d<0)
			d=0;
		if (d>255)
			d=255;
		ramp[i]=d;

		rampalpha[i]=i*alpha/256;
	}

	rampalpha[255]=255; // transparent BLEIBT bitte so.
}

void gFBDC::setPalette()
{
#ifdef QBOXHD
    return;
#else
	if (!surface.clut.data)
		return;

	for (int i=0; i<256; ++i)
	{
		fb->CMAP()->red[i]=ramp[surface.clut.data[i].r]<<8;
		fb->CMAP()->green[i]=ramp[surface.clut.data[i].g]<<8;
		fb->CMAP()->blue[i]=ramp[surface.clut.data[i].b]<<8;
		fb->CMAP()->transp[i]=rampalpha[surface.clut.data[i].a]<<8;
	}
		fb->PutCMAP();
#endif // QBOXHD
}

void gFBDC::exec(gOpcode *o)
{
	switch (o->opcode)
	{
        case gOpcode::setPalette:
        {
            gDC::exec(o);
            setPalette();
            break;
        }
        case gOpcode::flip:
        {
            if (m_enable_double_buffering)
            {
                gSurface s(surface);
                surface = surface_back;
                surface_back = s;
#ifndef QBOXHD
    			fb->setOffset(surface_back.offset);
#endif /// QBOXHD
            }
            break;
        }
        case gOpcode::waitVSync:
        {
            static timeval l;
            static int t;
            timeval now;
    
            if (t == 1000)
            {
                gettimeofday(&now, 0);
    
                int diff = (now.tv_sec - l.tv_sec) * 1000 + (now.tv_usec - l.tv_usec) / 1000;
                eDebug("%d ms latency (%d fps)", diff, t * 1000 / (diff ? diff : 1));
                l = now;
                t = 0;
            }
    
            ++t;
#ifndef QBOXHD
    		fb->waitVSync();
#endif /// QBOXHD
            break;
        }
        case gOpcode::flush:
            fb->blit();
            break;
        default:
            gDC::exec(o);
            break;
	}
}

void gFBDC::setAlpha(int a)
{
	alpha=a;

	calcRamp();
	setPalette();
}

void gFBDC::setBrightness(int b)
{
	brightness=b;

	calcRamp();
	setPalette();
}

void gFBDC::setGamma(int g)
{
	gamma=g;

	calcRamp();
	setPalette();
}

void gFBDC::setResolution(int xres, int yres)
{
	if ((m_xres == xres) && (m_yres == yres))
		return;

	m_xres = xres; m_yres = yres;

	fb->SetMode(m_xres, m_yres, 32);

	for (int y=0; y<m_yres; y++)	// make whole screen transparent
		memset(fb->lfb+y*fb->Stride(), 0x00, fb->Stride());

	surface.type = 0;
	surface.x = m_xres;
	surface.y = m_yres;
	surface.bpp = 32;
	surface.bypp = 4;
	surface.stride = fb->Stride();
	surface.data = fb->lfb;
	surface.offset = 0;

#ifdef QBOXHD
    int fb_size = surface.stride * surface.y;
    m_enable_double_buffering = 0;
    eDebug("%dkB available for acceleration surfaces.", (fb->Available() - fb_size) / 1024);
#else
    //surface.data_phys = 50*1024*1024; // FIXME
	surface.data_phys = fb->getPhysAddr();

	int fb_size = surface.stride * surface.y;

	if (fb->getNumPages() > 1)
	{
		m_enable_double_buffering = 1;
		surface_back.type = 0;
		surface_back.x = m_xres;
		surface_back.y = m_yres;
		surface_back.bpp = 32;
		surface_back.bypp = 4;
		surface_back.stride = fb->Stride();
		surface_back.offset = surface.y;
		surface_back.data = fb->lfb + fb_size;
		surface_back.data_phys = surface.data_phys + fb_size;

		fb_size *= 2;
	} else
		m_enable_double_buffering = 0;

	eDebug("%dkB available for acceleration surfaces.", (fb->Available() - fb_size) / 1024);
	eDebug("resolution: %d x %d x %d (stride: %d)", surface.x, surface.y, surface.bpp, fb->Stride());

    if (gAccel::getInstance())
        gAccel::getInstance()->setAccelMemorySpace(fb->lfb + fb_size, surface.data_phys + fb_size, fb->Available() - fb_size);
#endif // QBOXHD

	if (!surface.clut.data)
	{
		surface.clut.colors = 256;
		surface.clut.data = new gRGB[surface.clut.colors];
		memset(surface.clut.data, 0, sizeof(*surface.clut.data)*surface.clut.colors);
	}

	surface_back.clut = surface.clut;

	m_pixmap = new gPixmap(&surface);
}

void gFBDC::saveSettings()
{
}

void gFBDC::reloadSettings()
{
	alpha=255;
	gamma=128;
	brightness=128;

	calcRamp();
	setPalette();
}

// eAutoInitPtr<gFBDC> init_gFBDC(eAutoInitNumbers::graphic-1, "GFBDC");
#ifndef WITH_SDL
eAutoInitPtr<gFBDC> init_gFBDC(eAutoInitNumbers::graphic-1, "GFBDC");
#endif
