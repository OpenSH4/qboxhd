#ifndef __rcdfbdbox_h
#define __rcdfbdbox_h


#include <lib/driver/rc.h>
#include <dfb++.h>

class eDirectFBInputDev: public eRCDevice
{
public:

	void handleCode(long code);
	int convertKeySymbol(DFBInputEvent *ev);
	eDirectFBInputDev(eDirectFBInputEventDriver *driver);
	const char *getDescription() const;
};

#endif
