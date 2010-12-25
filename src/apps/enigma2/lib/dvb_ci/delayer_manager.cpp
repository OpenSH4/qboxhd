#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <list>
#include <lib/dvb_ci/delayer_manager.h>
#include <errno.h>

#ifdef QBOXHD_MINI

eDelayerManager* eDelayerManager::instance = NULL;

eDelayerAdapter::eDelayerAdapter(int adapter, unsigned int delay_time)
{
		m_adapter = adapter;
		m_delay_time = delay_time;
}

eDelayerAdapter::~eDelayerAdapter()
{ }


int eDelayerAdapter::setDelayTime(unsigned int value)
{
	int fd;
	int res;
	DelayerEn delayer;

	/* Open the device for delayer */
	if ((fd = open(DELAYER_DEVICE_NAME, O_RDWR | O_NOCTTY| O_NONBLOCK)) < 0) {
		eDebug("open(): %s\n", strerror(errno));
		return -1;
	}

	delayer.enable     = 1;
	delayer.dvbAdapter = m_adapter;
	delayer.delay      = value;
	res=ioctl(fd, IOCTL_DELAYER_ACTIVATE, &delayer);
	if(res<0)
		eDebug("IOCTL_DELAYER_ACTIVATE ... res = %d (%m)\n",res);
	else {
		eDebug("IOCTL_DELAYER_ACTIVATE ... res = %d\n",res);
		m_delay_time = value;

		if ( !isActive() )
			eDelayerManager::getInstance()->delayerStateChanged(m_adapter, eDelayerManager::evDelayNoActive);
		else
			eDelayerManager::getInstance()->delayerStateChanged(m_adapter, eDelayerManager::evDelayActive);
	}

	close(fd);

	return res;
}



eDelayerManager::eDelayerManager()
{ 
}


eDelayerManager::~eDelayerManager()
{
	while (m_delayer_adapter_list.size())
	{
		delete m_delayer_adapter_list.back();
		m_delayer_adapter_list.pop_back();
	}
}


eDelayerManager* eDelayerManager::getInstance()
{
	if (instance == NULL)
		instance = new eDelayerManager;
	return instance;
}


void eDelayerManager::addDelayerAdapter( int adapter, unsigned int delay_time )
{
	eDelayerAdapter* m_delayer_adapter = NULL;

	m_delayer_adapter = new eDelayerAdapter( adapter, delay_time );
	
	m_delayer_adapter_list.push_back(m_delayer_adapter);
}


eDelayerAdapter* eDelayerManager::getDelayerAdapter( int adapter )
{
	if ( m_delayer_adapter_list.size() == 0 )
		return 0;

	for (ePtrList<eDelayerAdapter>::iterator i(m_delayer_adapter_list); i != m_delayer_adapter_list.end(); ++i)
		if ( i->getAdapter() == adapter)
			return *i;

	return 0;
}

#endif /* QBOXHD_MINI */

