#include <lib/driver/rcinput.h>
#include <lib/driver/rcdfbinput.h>

#include <lib/base/eerror.h>

#include <sys/ioctl.h>
#include <linux/input.h>
#include <sys/stat.h>

#include <lib/base/ebase.h>
#include <lib/base/init.h>
#include <lib/base/init_num.h>
#include <lib/driver/input_fake.h>

#ifdef QBOXHD
#include "rcqbox2.h"


#ifdef QBOXHD_MINI
#include "fpqboxmini.h"	
#else
#include "swqbox2.h"
#endif


#endif // QBOXHD

void eRCDeviceInputDev::handleCode(long rccode)
{
	struct input_event *ev = (struct input_event *)rccode;
	if (ev->type!=EV_KEY)
		return;

// 	eDebug("%x %x %x", ev->value, ev->code, ev->type);

	if (ev->type!=EV_KEY)
		return;

	int km = iskeyboard ? input->getKeyboardMode() : eRCInput::kmNone;

 	eDebug("keyboard mode %d", km);

	if (km == eRCInput::kmAll)
		return;

	if (km == eRCInput::kmAscii)
	{
// 		eDebug("filtering.. %d", ev->code);
		bool filtered = ( ev->code > 0 && ev->code < 61 );
		switch (ev->code)
		{
			case KEY_RESERVED:
			case KEY_ESC:
			case KEY_TAB:
			case KEY_BACKSPACE:
			case KEY_ENTER:
			case KEY_LEFTCTRL:
			case KEY_RIGHTSHIFT:
			case KEY_LEFTALT:
			case KEY_CAPSLOCK:
			case KEY_INSERT:
			case KEY_DELETE:
			case KEY_MUTE:
				filtered=false;
			default:
				break;
		}
		if (filtered)
			return;
//		eDebug("passed!");
	}

	switch (ev->value)
	{
	case 0:
		/*emit*/ input->keyPressed(eRCKey(this, ev->code, eRCKey::flagBreak));
		break;
	case 1:
		/*emit*/ input->keyPressed(eRCKey(this, ev->code, 0));
		break;
	case 2:
		/*emit*/ input->keyPressed(eRCKey(this, ev->code, eRCKey::flagRepeat));
		break;
	}
}

eRCDeviceInputDev::eRCDeviceInputDev(eRCInputEventDriver *driver)
	:eRCDevice(driver->getDeviceName(), driver), iskeyboard(false)
{
	int len=id.length();
	int idx=0;
	while(idx <= len-8)
	{
		if (!strncasecmp(&id[idx++], "KEYBOARD", 8))
		{
			iskeyboard=true;
			break;
		}
	}
	eDebug("Input device \"%s\" is %sa keyboard.", id.c_str(), iskeyboard ? "" : "not ");

}

const char *eRCDeviceInputDev::getDescription() const
{
	return id.c_str();
}

class eInputDeviceInit
{
#ifdef QBOXHD
//     ePtrList<eRCShortDriver> m_drivers;
    ePtrList<eRCDriver> m_drivers;
    ePtrList<eRCDevice> m_devices;
#else
	ePtrList<eRCInputEventDriver> m_drivers;
	ePtrList<eRCDeviceInputDev> m_devices;
#endif

public:
	eInputDeviceInit()
	{
#ifdef QBOXHD
        struct stat s;
		/* DirectFB Inputs Control */
		eDirectFBInputEventDriver *p;

		m_drivers.push_back(p = new eDirectFBInputEventDriver());
		m_devices.push_back(new eDirectFBInputDev(p));

		/********************************************************/
#ifndef QBOXHD_MINI
        if (!stat("/dev/sw0", &s))
        {
            eSWQBoxDriver2 *p;
            eSWDeviceQBox2 *d;
            m_drivers.push_back(p = new eSWQBoxDriver2("/dev/sw0"));
            m_devices.push_back(d= new eSWDeviceQBox2(p));
            eDebug("Found 1 Sensewheel devices!");
        }
#else
 
       if (!stat("/dev/fpanel_0", &s))
        {
            eFButtonDriver *p;
            eFButtonDevice *d;
            m_drivers.push_back(p = new eFButtonDriver("/dev/fpanel_0"));
            m_devices.push_back(d= new eFButtonDevice(p));
            eDebug("Found 1 FrontButton panel devices!");
        }

#endif
#if 0
        if (!stat("/dev/lirc", &s))
        {
            eRCQBoxDriver2 *p;
            eRCDeviceQBox2 *d;
            m_drivers.push_back(p = new eRCQBoxDriver2("/dev/lirc"));
            m_devices.push_back(d= new eRCDeviceQBox2(p));
            eDebug("Found 1 input devices!");
        }
#endif

#else
		int i = 0;
		while (1)
		{
			struct stat s;
			char filename[128];
			sprintf(filename, "/dev/input/event%d", i);
			if (stat(filename, &s))
				break;
			eRCInputEventDriver *p;
			m_drivers.push_back(p = new eRCInputEventDriver(filename));
			m_devices.push_back(new eRCDeviceInputDev(p));
			++i;
		}
        eDebug("Found %d input devices!", i);
#endif // QBOXHD
	}

	~eInputDeviceInit()
	{
		while (m_drivers.size())
		{
			delete m_devices.back();
			m_devices.pop_back();
			delete m_drivers.back();
			m_drivers.pop_back();
		}
	}
};

eAutoInitP0<eInputDeviceInit> init_rcinputdev(eAutoInitNumbers::rc+1, "input device driver");
