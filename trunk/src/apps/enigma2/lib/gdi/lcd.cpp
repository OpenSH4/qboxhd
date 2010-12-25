#include <lib/gdi/lcd.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#ifndef QBOXHD
#include <dbox/fp.h>
#include <dbox/lcd-ks0713.h>
#endif // QBOXHD

#include <lib/gdi/esize.h>
#include <lib/base/init.h>
#include <lib/base/init_num.h>
#include <lib/gdi/glcddc.h>

#ifdef QBOXHD
#include <sys/mman.h>
#endif /// QBOXHD

eDBoxLCD *eDBoxLCD::instance;

eLCD::eLCD(eSize size): res(size)
{
	lcdfd = -1;
	locked=0;
#ifdef QBOXHD
	sizelcd = res.height()*res.width()*(32/8);
	_buffer=new unsigned char[sizelcd];
	memset(_buffer, 0, sizelcd);
	_stride=res.width()*4;

#else
	_buffer=new unsigned char[res.height()*res.width()];
	memset(_buffer, 0, res.height()*res.width());
	_stride=res.width();
#endif /// QBOXHD
}

eLCD::~eLCD()
{
	delete [] _buffer;
}

#ifdef QBOXHD
void eLCD::draw( unsigned char *buff )
{
	memcpy( _buffer, buff, sizelcd );
}
#endif /// QBOXHD

int eLCD::lock()
{
	if (locked)
		return -1;

	locked=1;
	return lcdfd;
}

void eLCD::unlock()
{
	locked=0;
}

#ifdef QBOXHD_MINI

/* This is a function that permits to release and re-init the DFB_Stretch struct when
	it will be changed the resolution */
void suspend_display_dfb(int par)
{
	eDBoxLCD *inst=inst->getInstance();
	inst->deinit_DFB_for_display(par);
}

void eDBoxLCD::init_DFB_for_display(void)
{
	int argc=1;
	char **argv=NULL;
	DirectFB::Init( &argc, &argv );
	strc.dfb = DirectFB::Create();
	strc.dfb->SetCooperativeLevel(DFSCL_FULLSCREEN);
	strc.layer = strc.dfb->GetDisplayLayer(DLID_PRIMARY);

	/* Create surfaces */
#if 0
	/* Primary surface (dummy) */
	strc.dsc.flags = (DFBSurfaceDescriptionFlags)(DSDESC_CAPS);
    strc.dsc.caps = (DFBSurfaceCapabilities)(DSCAPS_PRIMARY);
	strc.dsc.width = 720;
    strc.dsc.height = 576;
    strc.primary.surf = strc.dfb->CreateSurface(strc.dsc);
#endif
	/* Second surface (old display) */
    strc.dsc.flags = (DFBSurfaceDescriptionFlags)(DSDESC_CAPS | DSDESC_WIDTH | DSDESC_HEIGHT| DSDESC_PIXELFORMAT|DVCAPS_SCALE);
    strc.dsc.caps = (DFBSurfaceCapabilities)(DLCAPS_ALPHACHANNEL);

	strc.dsc.width = DISPLAY_WIDTH;
    strc.dsc.height = DISPLAY_HEIGHT;
	strc.dsc.pixelformat=DSPF_RGB16;
    strc.surf2.surf = strc.dfb->CreateSurface(strc.dsc);

    strc.surf2.surf->SetDstColorKey(0x0, 0x0, 0x0);
    strc.surf2.surf->Lock((DFBSurfaceLockFlags)(DSLF_WRITE|DSLF_READ),
							&strc.surf2.surface_data, &strc.surf2.surface_pitch);
    strc.surf2.pBUFF = (unsigned short *)strc.surf2.surface_data;
    strc.src_rect.x = 0;
    strc.src_rect.y = 0;
	strc.src_rect.w = DISPLAY_WIDTH;
   	strc.src_rect.h = DISPLAY_HEIGHT;

	/* Third surface (new display) */
    strc.dsc.flags = (DFBSurfaceDescriptionFlags)(DSDESC_CAPS | DSDESC_WIDTH | DSDESC_HEIGHT| DSDESC_PIXELFORMAT | DVCAPS_SCALE);
    strc.dsc.caps = (DFBSurfaceCapabilities)(DLCAPS_ALPHACHANNEL);

	strc.dsc.width = DISPLAY_WIDTH_MINI;
    strc.dsc.height = DISPLAY_HEIGHT_MINI;
	strc.dsc.pixelformat=DSPF_RGB16;
    strc.surf3.surf = strc.dfb->CreateSurface(strc.dsc);

    strc.surf3.surf->SetDstColorKey(0x0, 0x0, 0x0);
    strc.surf3.surf->Lock((DFBSurfaceLockFlags)(DSLF_WRITE|DSLF_READ),
							&strc.surf3.surface_data, &strc.surf3.surface_pitch);
    strc.surf3.pBUFF = (unsigned short *)strc.surf3.surface_data;
    strc.dst_rect.x = 0;
    strc.dst_rect.y = 0;
	strc.dst_rect.w = DISPLAY_WIDTH_MINI;
   	strc.dst_rect.h = DISPLAY_HEIGHT_MINI;
}

void eDBoxLCD::deinit_DFB_for_display(int par)
{
	if(par==1)
	{
		if(strc.init_struct==INIT)
		{
			strc.init_struct=SUSPEND;
			if(strc.surf3.surf!=NULL)
			{
				strc.surf3.surf->Release();
				strc.surf3.surf=NULL;
			}
			if(strc.surf2.surf!=NULL)
			{
				strc.surf2.surf->Unlock();
				strc.surf2.surf->Release();	
				strc.surf2.surf=NULL;
			}
			if(strc.primary.surf!=NULL)
			{
				strc.primary.surf->Release();
				strc.primary.surf=NULL;
			}
			if(strc.layer!=NULL)
			{
				strc.layer->Release();
				strc.layer=NULL;
			}
			if(strc.dfb!=NULL)
			{
				strc.dfb->Release();
				strc.dfb=NULL;
			}
		}
	}
	else if(par==0)
	{
		strc.init_struct=NO_INIT;
	}
	else
		eDebug("ERROR: %d wrong parameter!\n",par);	

}

#endif


#ifndef QBOXHD //DLB_LCD_RGB_565
eDBoxLCD::eDBoxLCD(): eLCD(eSize(132, 64))
#else
eDBoxLCD::eDBoxLCD(): eLCD(eSize(DISPLAY_WIDTH, DISPLAY_HEIGHT))
#endif
{
#ifdef QBOXHD
	unsigned int resHeight=res.height();
	unsigned int resWidth=res.width();
#endif /// QBOXHD
	
#ifndef NO_LCD

	lcdfd = open("/dev/oled0", O_RDWR);
	if (lcdfd < 0)
	{
		lcdfd = open("/dev/lcd", O_RDWR);
		is_oled = 0;
	} else
	{
		eDebug("found OLED display!");
		is_oled = 1;
	}
#else
	lcdfd = -1;
#endif
	instance=this;

	if (lcdfd<0)
		eDebug("couldn't open LCD - load lcd.o!");
	else
	{
#ifndef QBOXHD
		int i=LCD_MODE_BIN;
		ioctl(lcdfd, LCD_IOCTL_ASC_MODE, &i);
#endif // QBOXHD
		inverted=0;
	}

#ifdef QBOXHD
	/* For QboxHD when using the mmap function */
	buff_mmap=NULL;

	buff_mmap=(unsigned short *)mmap(0, resWidth*resHeight*2, PROT_WRITE, MAP_SHARED, lcdfd,0);
	if(buff_mmap<0)
	{
		eDebug("Error to write in display\n");
		return;
	}
///
///FIXME: THEN THE MINI MUST WILL USE THE 'mmap' FUNCTION
///

#ifdef QBOXHD_MINI
	strc.init_struct=NO_INIT;
	strc.dfb=NULL;
	strc.layer=NULL;
	strc.primary.surf=NULL;
	strc.surf2.surf=NULL;
	strc.surf3.surf=NULL;
#endif

#endif /// QBOXHD
}

#ifndef QBOXHD //DLB_LCD_RGB_565

void eDBoxLCD::setInverted(unsigned char inv)
{
	inverted=inv;
	update();
}

int eDBoxLCD::setLCDContrast(int contrast)
{

	int fp;
	if((fp=open("/dev/fp0", O_RDWR))<=0)
	{
		eDebug("[LCD] can't open /dev/fp0");
		return(-1);
	}
/*
	if(ioctl(lcdfd, LCD_IOCTL_SRV, &contrast))
	{
		eDebug("[LCD] can't set lcd contrast");
	}

	close(fp);
*/
	return(0);
}
#endif ///QBOXHD

int eDBoxLCD::setLCDBrightness(int brightness)
{
eDebug("setLCDBrightness %d", brightness);
#ifdef QBOXHD

	if ( lcdfd >= 0 )
	{
		if(ioctl(lcdfd, LCD_IOCTL_REG_BRIGHTNESS, brightness))
		{
			eDebug("[LCD] can't set lcd brightness");
		}
	}
#else
    FILE *f=fopen("/proc/stb/fp/oled_brightness", "w");
    if (f)
    {
        if (fprintf(f, "%d", brightness) == 0)
            eDebug("write /proc/stb/fp/oled_brightness failed!! (%m)");
        fclose(f);
    }
    else
    {
        int fp;
        if((fp=open("/dev/dbox/fp0", O_RDWR))<=0)
        {
            eDebug("[LCD] can't open /dev/dbox/fp0");
            return(-1);
        }

        if(ioctl(fp, FP_IOCTL_LCD_DIMM, &brightness)<=0)
            eDebug("[LCD] can't set lcd brightness (%m)");
        close(fp);
    }
#endif // QBOXHD
	return(0);
}

eDBoxLCD::~eDBoxLCD()
{
#ifdef QBOXHD
	unsigned int resHeight=res.height();
	unsigned int resWidth=res.width();
#endif // QBOXHD
	if (lcdfd>=0)
	{
		close(lcdfd);
		lcdfd=-1;
	}
#ifdef QBOXHD
	if(buff_mmap>0)
		munmap(buff_mmap, resWidth*resHeight*2);

///
///FIXME: THEN THE MINI MUST WILL USE THE 'mmap' FUNCTION
///
#ifdef QBOXHD_MINI

	if(strc.surf3.surf!=NULL)
	{
		strc.surf3.surf->Release();
		strc.surf3.surf=NULL;
	}
	if(strc.surf2.surf!=NULL)
	{
		strc.surf2.surf->Unlock();
		strc.surf2.surf->Release();	
		strc.surf2.surf=NULL;
	}
	if(strc.primary.surf!=NULL)
	{
		strc.primary.surf->Release();
		strc.primary.surf=NULL;
	}
	if(strc.layer!=NULL)
	{
		strc.layer->Release();
		strc.layer=NULL;
	}
	if(strc.dfb!=NULL)
	{
		strc.dfb->Release();
		strc.dfb=NULL;
	}
	if(strc.init_struct==INIT)
		strc.init_struct=NO_INIT;

#endif // QBOXHD_MINI
#endif // QBOXHD
}

eDBoxLCD *eDBoxLCD::getInstance()
{
	return instance;
}


#ifdef QBOXHD //DLB_LCD_RGB_565

void eLCD::enable_update()
{
	ePtr<gLCDDC> t_lcd;
	gLCDDC::getInstance(t_lcd);
	t_lcd->setUpdate(1);	
	t_lcd->forceRefresh();
}

void eDBoxLCD::DrawRGB565()
{
	unsigned char r,g,b;
	unsigned int x, y;
	unsigned short pixel;
	unsigned char value;
	unsigned int resHeight=res.height();
	unsigned int resWidth=res.width();
	unsigned char * Index = _buffer;
// 	unsigned short buff[resWidth*resHeight];
// 	unsigned short * Pbuff = buff;

///
///FIXME: THEN THE MINI MUST WILL USE THE 'mmap' FUNCTION
///
#ifndef QBOXHD_MINI 
	/* Using the mmap function */

	/* Stop the current write */
	ioctl(lcdfd,LCD_IOCTL_MMAP_WRITE,LCD_STOP);

	/* Copy the new data in 'kernel buffer' of the lcd drver */
	for (y = 0; y < resHeight; y++)        
	{
		for (x = 0; x < resWidth; x++)
		{
			/* pixel 1 */
			value = *Index++;
			b = value >> 3;
			value = *Index++;
			g = value >> 2;
			value = *Index++;
			r = value >> 3;
			value = *Index++;

			pixel = (r << 11) | (g << 5) | (b);
			//*Pbuff++ = pixel;
			buff_mmap[x+(320*y)]=pixel;
			buff_mmap[x+1+(320*y)]=pixel>>8;

		}
	}
// 	write(lcdfd, &buff, sizeof(buff));
	/* Start thw write to display wirh the new data */
	ioctl(lcdfd,LCD_IOCTL_MMAP_WRITE,LCD_START);
#else
	///UGLY WORK AROUD
	if(strc.init_struct==NO_INIT)
	{
		init_DFB_for_display();
		strc.init_struct=INIT;
	}
	else if(strc.init_struct==SUSPEND)///no write in the display because the DFB struct are released
	{
		return;
	}

	unsigned short * Pbuff;
	Pbuff=strc.surf2.pBUFF;

	for (y = 0; y < resHeight; y++)
	{
		for (x = 0; x < resWidth; x++)
		{
			/* pixel 1 */
			value = *Index++;
			b = value >> 3;
			value = *Index++;
			g = value >> 2;
			value = *Index++;
			r = value >> 3;
			value = *Index++;

			pixel = (r << 11) | (g << 5) | (b);

			*Pbuff++ = pixel;
		}
	}

	strc.surf3.surf->Unlock();
	strc.surf3.surf->StretchBlit(strc.surf2.surf, NULL, &strc.dst_rect);
	write(lcdfd, strc.surf3.pBUFF, DISPLAY_WIDTH_MINI*DISPLAY_HEIGHT_MINI*2);
#endif
}


#if 0
void eDBoxLCD::DrawRGB565()
{
	unsigned char r,g,b;
	unsigned int x, y;
	unsigned short pixel;
	unsigned char value;
	unsigned int resHeight=res.height();
	unsigned int resWidth=res.width();
	unsigned char * Index = _buffer;
	unsigned short buff[resWidth*resHeight];
	unsigned short * Pbuff = buff;

	for (y = 0; y < resHeight; y++)        
	{
		for (x = 0; x < resWidth; x++)
		{
			/* pixel 1 */
			value = *Index++;

			b = value >> 3;
			g = value >> 2;
			r = value >> 3;

			pixel = (r << 11) | (g << 5) | (b);

			*Pbuff++ = pixel;
		}
	}

	write(lcdfd, &buff, sizeof(buff));
}
#endif
#endif

void eDBoxLCD::update()
{
	if (!is_oled)
	{

#ifndef QBOXHD //DLB_LCD_RGB_565

		unsigned char raw[132*8];
		int x, y, yy;
		for (y=0; y<8; y++)
		{
			unsigned int aab = 0;

			for (x=0; x<132; x++)
			{
				if ( x < 40 ){
					int pix=0,aa=0;
					for (yy=0; yy<8; yy++)
					{
						pix|=(_buffer[(y*8+yy)*132+x]>=108)<<yy;

					}
					//printf("%02X", pix);
					aa=pix;
					for (yy=0; yy<8; yy++)
					{
						if (aa & 0x80) printf("O"); else printf (" ");
						aa<<=1;

					}
					aab++;
					raw[y*132+x]=(pix^inverted);
				}
			}
			printf("\n");
		}

		if (lcdfd >= 0)
			write(lcdfd, raw, 132*8);
#else  /// !QBOXHD

		DrawRGB565();

#endif ///QBOXHD
	}
	else
	{
		unsigned char raw[64*64];
		int x, y;
		memset(raw, 0, 64*64);
		for (y=0; y<64; y++)
		{
#ifdef QBOXHD
			for (x=0; x<128 / 2; x++)
				raw[y*64+x] = (_buffer[y*132 + x * 2 + 2] & 0xF0) |(_buffer[y*132 + x * 2 + 1 + 2] >> 4);
#else ///! QBOXHD
			int pix=0;
			for (x=0; x<128 / 2; x++)
			{
				pix = (_buffer[y*132 + x * 2 + 2] & 0xF0) |(_buffer[y*132 + x * 2 + 1 + 2] >> 4);
				if (inverted)
					pix = 0xFF - pix;
				raw[y*64+x] = pix;
			}
#endif ///QBOXHD
		}
		if (lcdfd >= 0)
			write(lcdfd, raw, 64*64);
	}
}

