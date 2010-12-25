#include <cstring>
#include <lib/driver/misc_options.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <lib/base/init.h>
#include <lib/base/init_num.h>
#include <lib/base/eerror.h>

Misc_Options *Misc_Options::instance = 0;

Misc_Options::Misc_Options()
	:m_12V_output_state(-1)
{
	ASSERT(!instance);
	instance = this;
}

int Misc_Options::set_12V_output(int state)
{
	if (state == m_12V_output_state)
		return 0;
#ifdef QBOXHD
	int fd = open("/etc/enigma2/stb/misc/12V_output", O_WRONLY);
	if (fd < 0)
	{
		eDebug("couldn't open /etc/enigma2/stb/misc/12V_output");
		return -1;
	}
#else
    int fd = open("/proc/stb/misc/12V_output", O_WRONLY);
    if (fd < 0)
    {
        eDebug("couldn't open /proc/stb/misc/12V_output");
        return -1;
    }
#endif // QBOXHD
	const char *str=0;
	if (state == 0)
		str = "off";
	else if (state == 1)
		str = "on";
	if (str)
		write(fd, str, strlen(str));
	m_12V_output_state = state;
	close(fd);
	return 0;
}

bool Misc_Options::detected_12V_output()
{
#ifdef QBOXHD
	int fd = open("/etc/enigma2/stb/misc/12V_output", O_WRONLY);
	if (fd < 0)
	{
		eDebug("couldn't open /etc/enigma2/stb/misc/12V_output");
		return false;
	}
#else
    int fd = open("/etc/enigma2/stb/misc/12V_output", O_WRONLY);
    if (fd < 0)
    {
        eDebug("couldn't open /etc/enigma2/stb/misc/12V_output");
        return false;
    }
#endif // QBOXHD
	close(fd);
	return true;
}

Misc_Options *Misc_Options::getInstance()
{
	return instance;
}

//FIXME: correct "run/startlevel"
eAutoInitP0<Misc_Options> init_misc_options(eAutoInitNumbers::rc, "misc options");
