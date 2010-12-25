#include <lib/gdi/glcddc.h>
#include <lib/gdi/lcd.h>
#include <lib/base/init.h>
#include <lib/base/init_num.h>

gLCDDC *gLCDDC::instance;

gLCDDC::gLCDDC()
{
	lcd = new eDBoxLCD();
	instance=this;

	update=1;

	surface.x=lcd->size().width();
	surface.y=lcd->size().height();
#ifdef QBOXHD
	surface.bpp=32;
	surface.bypp=4;
#else
	surface.bpp=8;
	surface.bypp=1;
#endif // QBOXHD
	surface.stride=lcd->stride();
	surface.data=lcd->buffer();

	surface.clut.colors=256;
	surface.clut.data=0;
#ifdef QBOXHD
	if(!surface.clut.data)
	{
		surface.clut.colors = 256;
		surface.clut.data = new gRGB[surface.clut.colors];
		memset(surface.clut.data,0,sizeof(*surface.clut.data)*surface.clut.colors);
	}
#endif // QBOXHD
	m_pixmap = new gPixmap(&surface);
}

gLCDDC::~gLCDDC()
{
	delete lcd;
	instance=0;
}

#ifdef QBOXHD
void gLCDDC::forceRefresh()
{
	lcd->update();
}
#endif // QBOXHD

void gLCDDC::exec(gOpcode *o)
{
	switch (o->opcode)
	{
	case gOpcode::flush:
		if (update)
			lcd->update();
	default:
		gDC::exec(o);
		break;
	}
}

void gLCDDC::setUpdate(int u)
{
	update=u;
}

eAutoInitPtr<gLCDDC> init_gLCDDC(eAutoInitNumbers::graphic-1, "gLCDDC");
