#include <lib/driver/rc.h>

#include <asm/types.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>

#include <lib/base/init.h>
#include <lib/base/init_num.h>
#include <lib/base/eerror.h>

#include "input_devices.h"

#ifdef QBOXHD_MINI
#include "fpqboxmini.h"
#endif

/*****************/
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <directfb.h>
#include <directfb_keynames.h>

#include <direct/mem.h>
#include <direct/messages.h>
#include <direct/thread.h>
#include <direct/util.h>
/*****************/
/*
 *  note on the enigma input layer:
 *  the enigma input layer (rc*) supports n different devices which
 *  all have completely different interfaces, mapped down to 32bit +
 *  make/break/release codes mapped down (via xml files) to "actions".
 *  this was necessary to support multiple remote controls with proprietary
 *  interfaces. now everybody is using input devices, and thus adding
 *  another input layer seems to be a bit overkill. BUT:
 *  image a remote control with two hundred buttons. each and every function
 *  in enigma can be bound to a button. no need to use them twice.
 *  for example, you would have KEY_MENU assigned to a menu for setup etc.,
 *  but no audio and video settings, since you have special keys for that,
 *  and you don't want to display a big menu with entries that are available
 *  with another single key.
 *  then image a remote control with ten buttons. do you really want to waste
 *  KEY_MENU for a simple menu? you need the audio/video settings there too.
 *  take this just as a (bad) example. another (better) example might be front-
 *  button-keys. usually you have KEY_UP, KEY_DOWN, KEY_POWER. you don't want
 *  them to behave like the remote-control-KEY_UP, KEY_DOWN and KEY_POWER,
 *  don't you?
 *  so here we can map same keys of different input devices to different
 *  actions. have fun.
 */
//int fd_fake;
/* For lircd socket */
int fd_sock;

eRCDevice::eRCDevice(std::string id, eRCDriver *driver): driver(driver), id(id)
{
	input=driver->getInput();
	driver->addCodeListener(this);
	eRCInput::getInstance()->addDevice(id, this);
#ifdef QBOXHD
	rc_id="";
#endif
}

eRCDevice::~eRCDevice()
{
	driver->removeCodeListener(this);
	eRCInput::getInstance()->removeDevice(id.c_str());
}

#ifdef QBOXHD
void eRCDevice::setRCIdentifier(std::string rc_id)
{
	this->rc_id = rc_id;
}
#endif

eRCDriver::eRCDriver(eRCInput *input): input(input), enabled(1)
{
}

eRCDriver::~eRCDriver()
{
	for (std::list<eRCDevice*>::iterator i=listeners.begin(); i!=listeners.end(); ++i)
		delete *i;
}

#ifdef QBOXHD
void eRCShortDriver::keyPressed(int)
{
	//__u16 rccode;
	unsigned int rccode;
	unsigned int buttoncode=0;
	unsigned int rcid;
	int res=0;

#ifdef QBOXHD_MINI

	res=read(handle, &rccode, 4);
	if (res!=4) return;

	if (enabled && !input->islocked()) {
		for (std::list<eRCDevice*>::iterator i(listeners.begin()); i!=listeners.end(); ++i)
		{
			(*i)->handleCode(rccode); // Key_POWER
		}
	}	
#else
	while (1)
	{
		res=read(handle, &rccode, 4);
		if (res!=4)
			break;
		
		rcid = rccode & 0x00FF0000;
		buttoncode = rccode & 0x0000FFFF;

		//printf("rccode 0x%08X RC ID: 0x%08x ButtonCode: 0x%04x\n", rccode, rcid, buttoncode );
		if (enabled && !input->islocked())
			for (std::list<eRCDevice*>::iterator i(listeners.begin()); i!=listeners.end(); ++i)
			{
				/* Work around for stand-by */
				if(buttoncode == 0x000F)
				{
					(*i)->handleCode(rcid | 0x000F); // Key_POWER
					usleep(250000);
					(*i)->handleCode(0x00FF);	 // Simulate key release
				}
				else
					(*i)->handleCode(rccode);
			}
	}
#endif
}
#else
void eRCShortDriver::keyPressed(int)
{
    __u16 rccode;
    while (1)
    {
        if (read(handle, &rccode, 2)!=2)
            break;
        if (enabled && !input->islocked())
            for (std::list<eRCDevice*>::iterator i(listeners.begin()); i!=listeners.end(); ++i)
                (*i)->handleCode(rccode);
    }
}
#endif // QBOXHD

eRCShortDriver::eRCShortDriver(const char *filename): eRCDriver(eRCInput::getInstance())
{
	handle=open(filename, O_RDONLY|O_NONBLOCK);
	if (handle<0)
	{
		eDebug("failed to open %s", filename);
		sn=0;
	} else
	{
		sn=eSocketNotifier::create(eApp, handle, eSocketNotifier::Read);
		CONNECT(sn->activated, eRCShortDriver::keyPressed);
		eRCInput::getInstance()->setFile(handle);
	}
}

eRCShortDriver::~eRCShortDriver()
{
	if (handle>=0)
		close(handle);
}

void eRCInputEventDriver::keyPressed(int)
{
	struct input_event ev;
	while (1)
	{
		if (read(handle, &ev, sizeof(struct input_event))!=sizeof(struct input_event))
			break;
		if (enabled && !input->islocked())
			for (std::list<eRCDevice*>::iterator i(listeners.begin()); i!=listeners.end(); ++i)
                (*i)->handleCode((long)&ev);
	}
}

eRCInputEventDriver::eRCInputEventDriver(const char *filename): eRCDriver(eRCInput::getInstance())
{
	handle=open(filename, O_RDONLY|O_NONBLOCK);
	if (handle<0)
	{
		eDebug("failed to open %s", filename);
		sn=0;
	} else
	{
		sn=eSocketNotifier::create(eApp, handle, eSocketNotifier::Read);
		CONNECT(sn->activated, eRCInputEventDriver::keyPressed);
		eRCInput::getInstance()->setFile(handle);
	}
}

std::string eRCInputEventDriver::getDeviceName()
{
	char name[128]="";
	if (handle >= 0)
		::ioctl(handle, EVIOCGNAME(128), name);
	return name;
}

eRCInputEventDriver::~eRCInputEventDriver()
{
	if (handle>=0)
		close(handle);
}


// ******************************************************************************************
// DirectFB input devicefd_fakes
// ******************************************************************************************
/* In this function there are 3 versions of key_press function
	It depends wich metho we are using to detect the press button
	Now we are using the lircd daemon */
extern Input_Devices_t buttons[MAX_BUTTONS];
void eDirectFBInputEventDriver::keyPressed(int)
{
//	DFBInputEvent      *inputevent;
//	DFBEvent event;
#if 0
	while (1)
	{
		if (read(handle, &event, sizeof(DFBEvent)) != sizeof(DFBEvent) )
			break;

		 inputevent=(DFBInputEvent *)(&event);
#ifdef MAX_LOG
eDebug("-----> eDirectFBInputEventDriver::keyPressed():[Time: %s] Class: %d. Type: %d, Key code: %d, Key id: %d, Key symbol: %d",
       get_timestamp(ubuf),inputevent->clazz, inputevent->type, inputevent->key_code, inputevent->key_id, inputevent->key_symbol);
#else
        	eDebug("-----> %s(): Class: %d. Type: %d, Key code: %d, Key id: %d, Key symbol: %d", __FUNCTION__, inputevent->clazz, inputevent->type, inputevent->key_code, inputevent->key_id, inputevent->key_symbol);
#endif ///MAXLOG
		if (enabled && !input->islocked()) {
			for (std::list<eRCDevice*>::iterator i(listeners.begin()); i!=listeners.end(); ++i)
				(*i)->handleCode((long)inputevent);
		}
	}
#endif

	int rp;
	unsigned char c=0;
/* It isn't necessary because this function is called when there is something in lirc device */
//	while (1)
	{
		rp=read_lircd();
		if(rp<0)
			return;
	
		if (enabled && !input->islocked())
		{
			for(c=0;c<rp;c++)
			{
				for (std::list<eRCDevice*>::iterator i(listeners.begin()); i!=listeners.end(); ++i)
					(*i)->handleCode((long)&buttons[c]);
			}
		}
	}

#if 0
	DFBInputEvent      inputevent;
	key_event_read(&inputevent);
	if(inputevent.key_symbol!=DIKS_NULL)
	{
		if (enabled && !input->islocked())
		{
			for (std::list<eRCDevice*>::iterator i(listeners.begin()); i!=listeners.end(); ++i)
				(*i)->handleCode((long)&inputevent);
		}
	}
#endif
}


extern ePtr<gFBDC> NewgFBDCPtr(void);

extern DFBEnumerationResult DirectFBShowInputDevices( DFBInputDeviceID    device_id,
                   				      DFBInputDeviceDescription  desc,
                   				      void                      *data );


/* In this function there are 3 method to detect the press button
	1: using DirectFB event
	2: lircd daemon
	3. using a fake fd (a file)
Now we are using the lircd daemon */
eDirectFBInputEventDriver::eDirectFBInputEventDriver(): eRCDriver(eRCInput::getInstance())
{
	ePtr<gFBDC> ptr;
	ptr = NewgFBDCPtr();

	if (!ptr) {
	    eDebug("Could not get DirectFB ptr");
	    return;
	}
	else {
	    dfb = (IDirectFB *)ptr->fb->dfb;
	}

	dfb->EnumInputDevices( DirectFBShowInputDevices, NULL );

        try {
            DFBInputDeviceCapabilities input_caps = DICAPS_KEYS;
/*
            event_buff = eDirectFBInputEventDriver::dfb->CreateInputEventBuffer(input_caps, DFB_TRUE);
	    handle = event_buff->CreateFileDescriptor();

	    fcntl(handle, F_SETFL, O_NONBLOCK);
            sn = eSocketNotifier::create(eApp, handle, eSocketNotifier::Read);
            CONNECT(sn->activated, eDirectFBInputEventDriver::keyPressed);
            eRCInput::getInstance()->setFile(handle);
*/
			/* Open the socket to lircd daemon ... */
			struct sockaddr_un  sa;
			sa.sun_family = AF_UNIX;
			direct_snputs( sa.sun_path, "/dev/lircd", sizeof(sa.sun_path) );
			fd_sock = socket( PF_UNIX, SOCK_STREAM, 0 );

			int val = 1;
			if (setsockopt(fd_sock, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) == -1)
				eDebug("[eDirectFBInputEventDriver] SO_REUSEADDR (%m)");
			else if ((val = fcntl(fd_sock, F_GETFL)) == -1)
				eDebug("[eDirectFBInputEventDriver] F_GETFL (%m)");
			else if (fcntl(fd_sock, F_SETFL, val | O_NONBLOCK) == -1)
				eDebug("[eDirectFBInputEventDriver] F_SETFL (%m)");
			else if (connect(fd_sock, (struct sockaddr*)&sa, sizeof(sa)) == -1)
				eDebug("[eDirectFBInputEventDriver] connect (%m)");
			else {
				sn = eSocketNotifier::create( eApp, fd_sock, eSocketNotifier::Read );
				sn->start();
				CONNECT(sn->activated, eDirectFBInputEventDriver::keyPressed );
				eRCInput::getInstance()->setFile(fd_sock);
				eDebug("[eDirectFBInputEventDriver] created successfully");
				return;
			}

			close(fd_sock);
			fd_sock = -1;

#if 0
			if (connect( fd_sock, (struct sockaddr*)&sa, sizeof(sa) ) < 0)
			{
				close( fd_sock );
				return;
			}

			/* Connect the socket to the eSocketNotifier ... */
			sn = eSocketNotifier::create(eApp, fd_sock, eSocketNotifier::Read);
			sn->start();
			CONNECT(sn->activated, eDirectFBInputEventDriver::keyPressed);
			eRCInput::getInstance()->setFile(fd_sock);
			
#endif
/*
			fd_fake = open("/tmp/lirc", O_CREAT | O_RDONLY);
			sn = eSocketNotifier::create(eApp, fd_fake, eSocketNotifier::Read);
            CONNECT(sn->activated, eDirectFBInputEventDriver::keyPressed);
            eRCInput::getInstance()->setFile(fd_fake);
			init_input_device();
*/
#ifdef MAX_LOG
eDebug("-----> eDirectFBInputEventDriver::eDirectFBInputEventDriver - Connected eSocketNotifier to eApp\n\n\n");
eDebug("-----> eDirectFBInputEventDriver::eDirectFBInputEventDriver - Connected eSocketNotifier=<%p> to eApp ",&sn);
#endif

        }
        catch (DFBException * ex) {
            eFatal("Caught exception: %s\n", ex->GetAction());
        }
}

std::string eDirectFBInputEventDriver::getDeviceName()
{
	char name[128]="";
	if (handle >= 0)
		::ioctl(handle, EVIOCGNAME(128), name);
	return name;
}

eDirectFBInputEventDriver::~eDirectFBInputEventDriver()
{
	if (handle>=0)
		close(handle);
}

// ******************************************************************************************

eRCConfig::eRCConfig()
{
	reload();
}

eRCConfig::~eRCConfig()
{
	save();
}

void eRCConfig::set( int delay, int repeat )
{
	rdelay = delay;
	rrate = repeat;
}

void eRCConfig::reload()
{
	rdelay=500;
	rrate=100;
}

void eRCConfig::save()
{
}

eRCInput *eRCInput::instance;

eRCInput::eRCInput()
{
	ASSERT( !instance);
	instance=this;
	handle = -1;
	locked = 0;
	keyboardMode = kmNone;
}

eRCInput::~eRCInput()
{
}

void eRCInput::close()
{
}

bool eRCInput::open()
{
	return false;
}

int eRCInput::lock()
{
	locked=1;
	return handle;
}

void eRCInput::unlock()
{
	if (locked)
		locked=0;
}

void eRCInput::setFile(int newh)
{
	handle=newh;
}

void eRCInput::addDevice(const std::string &id, eRCDevice *dev)
{
	devices.insert(std::pair<std::string,eRCDevice*>(id, dev));
}

void eRCInput::removeDevice(const std::string &id)
{
	devices.erase(id);
}

eRCDevice *eRCInput::getDevice(const std::string &id)
{
	std::map<std::string,eRCDevice*>::iterator i=devices.find(id);
	if (i == devices.end())
	{
		eDebug("failed, possible choices are:");
		for (std::map<std::string,eRCDevice*>::iterator i=devices.begin(); i != devices.end(); ++i)
			eDebug("%s", i->first.c_str());
		return 0;
	}
	return i->second;
}

std::map<std::string,eRCDevice*,eRCInput::lstr> &eRCInput::getDevices()
{
	return devices;
}

eAutoInitP0<eRCInput> init_rcinput(eAutoInitNumbers::rc, "RC Input layer");
