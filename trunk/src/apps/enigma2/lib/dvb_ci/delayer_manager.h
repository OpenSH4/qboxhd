/**
 * @file      delayer_manager.h
 * @author    Massimo
 *
 * Copyright (c) 2010-2011 Duolabs S.p.a. <br/>
 *
 * Changelog: <br/>
 * Date           Author      Comments <br/>
 * ------------------------------------------------------------------------ <br/>
 * 01.10.2010    Massimo      First version <br/>
 */

#ifndef __DL_DELAYER_MANAGER_H__
#define __DL_DELAYER_MANAGER_H__

#ifdef QBOXHD_MINI

#include <lib/base/ebase.h>
#include <lib/base/init.h>
#include <lib/base/init_num.h>
#include <lib/base/eerror.h>
#include <lib/base/estring.h>
#include <lib/base/eptrlist.h>
#include <lib/base/object.h>

#include <lib/python/python.h>
#include <lib/python/connections.h>

#define DELAYER_DEVICE_NAME            "/dev/delayer"
#define DELAYER_MAJOR_NUM               179
#define DELAYER_MINOR_START               0

typedef struct DelayerEn_s
{
	unsigned int   enable;
	unsigned int   dvbAdapter;
	unsigned int   delay;
}DelayerEn;

/* Ioctrl */
#define IOCTL_DELAYER_MAGIC             'f'

#define IOCTL_DELAYER_RESET             _IOW(IOCTL_DELAYER_MAGIC, 1, int)
#define IOCTL_DELAYER_ACTIVATE        _IOW(IOCTL_DELAYER_MAGIC, 2, DelayerEn)


class eDelayerAdapter
{
	private:
		int m_adapter;
		unsigned int m_delay_time;

	public:
		eDelayerAdapter(int adapter, unsigned int delay_time);
		~eDelayerAdapter();

		int getAdapter(void) { return m_adapter; }
		int getDelayTime(void) { return m_delay_time; }
		int setDelayTime(unsigned int value);
		bool isActive(void) { return ( m_delay_time > 0 ); }
};


class eDelayerManager: public Object
{
	private:
		ePtrList<eDelayerAdapter> m_delayer_adapter_list;
		static eDelayerManager *instance;

		void stateChanged(int adapter, int val) { delayerStateChanged(adapter, val); }
	public:
		eDelayerManager();
		~eDelayerManager();

		enum { evDelayNoActive, evDelayActive	};
		PSignal2<void,int,int> delayerStateChanged;

		static eDelayerManager* getInstance();

		void addDelayerAdapter( int adapter, unsigned int delay_time );
		eDelayerAdapter* getDelayerAdapter(int adapter);
};

#endif /* QBOXHD_MINI */

#endif /// __DL_DELAYER_MANAGER_H___
