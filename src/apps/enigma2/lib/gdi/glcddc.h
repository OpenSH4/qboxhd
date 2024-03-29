#ifndef __glcddc_h
#define __glcddc_h

#include "grc.h"
#include <lib/gdi/lcd.h>

class gLCDDC: public gDC
{
	eLCD *lcd;
	static gLCDDC *instance;
	int update;
	void exec(gOpcode *opcode);
	gSurface surface;
public:
	gLCDDC();
	~gLCDDC();
	void setUpdate(int update);
#ifdef QBOXHD
	void forceRefresh();
#endif
	static int getInstance(ePtr<gLCDDC> &ptr) { if (!instance) return -1; ptr = instance; return 0; }
	int islocked() { return lcd->islocked(); }
};

#endif
