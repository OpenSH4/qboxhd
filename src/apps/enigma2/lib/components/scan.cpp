#include <lib/dvb/dvb.h>
#include <lib/dvb/db.h>
#include <lib/components/scan.h>
#include <lib/base/eerror.h>
#include <lib/dvb/scan.h>

#ifdef QBOXHD
#ifndef QBOXHD_MINI
	#include "../dvb_ci/add_func.h"
#endif
#endif // QBOXHD

DEFINE_REF(eComponentScan);

void eComponentScan::scanEvent(int evt)
{
	switch(evt)
	{
		case eDVBScan::evtFinish:
		{
			m_done = 1;
			ePtr<iDVBChannelList> db;
			ePtr<eDVBResourceManager> res;
			
			int err;
			if ((err = eDVBResourceManager::getInstance(res)) != 0)
			{
				eDebug("no resource manager");
				m_failed = 2;
			} else if ((err = res->getChannelList(db)) != 0)
			{
				m_failed = 3;
				eDebug("no channel list");
			} else
			{
				m_scan->insertInto(db);
				db->flush();
				eDebug("scan done!");
			}
			break;
		}
		case eDVBScan::evtNewService:
			newService();
			return;
		case eDVBScan::evtFail:
			eDebug("scan failed.");
			m_failed = 1;
			m_done = 1;
			break;
		case eDVBScan::evtUpdate:
			break;
	}
	statusChanged();
}

eComponentScan::eComponentScan(): m_done(-1), m_failed(0)
{
}

eComponentScan::~eComponentScan()
{
}

void eComponentScan::clear()
{
	m_initial.clear();
}

void eComponentScan::addInitial(const eDVBFrontendParametersSatellite &p)
{
	ePtr<eDVBFrontendParameters> parm = new eDVBFrontendParameters();
	parm->setDVBS(p);
	m_initial.push_back(parm);
}

void eComponentScan::addInitial(const eDVBFrontendParametersCable &p)
{
	ePtr<eDVBFrontendParameters> parm = new eDVBFrontendParameters();
	parm->setDVBC(p);
	m_initial.push_back(parm);
}

void eComponentScan::addInitial(const eDVBFrontendParametersTerrestrial &p)
{
	ePtr<eDVBFrontendParameters> parm = new eDVBFrontendParameters();
	parm->setDVBT(p);
	m_initial.push_back(parm);
}


int eComponentScan::start(int feid, int flags)
{
	if (m_initial.empty())
		return -2;

	if (m_done != -1)
		return -1;
        
/*
#ifdef QBOXHD
#ifndef QBOXHD_MINI
	// Re-init connection tuner //
	switch_tuner_to_ts(CAM_BYPASS_ON);
#endif
#endif // QBOXHD
*/

	m_done = 0;
	ePtr<eDVBResourceManager> mgr;
	
	eDVBResourceManager::getInstance(mgr);

	eUsePtr<iDVBChannel> channel;

	if (mgr->allocateRawChannel(channel, feid))
	{
		eDebug("scan: allocating raw channel (on frontend %d) failed!", feid);
		return -1;
	}

	std::list<ePtr<iDVBFrontendParameters> > list;
	m_scan = new eDVBScan(channel);
	m_scan->connectEvent(slot(*this, &eComponentScan::scanEvent), m_scan_event_connection);

#ifdef QBOXHD
    #ifndef QBOXHD_MINI
        unsigned int i;
        bb_channels channels, *ch;
        ePtr<iDVBFrontend> fe_tmp;
        int slotid_tmp=0;
        m_scan->getFrontend(fe_tmp);
        eDVBFrontend *fe2 = (eDVBFrontend*) &(*fe_tmp);
        slotid_tmp = fe2->getSlotID();
        ePtr<iDVBDemux> m_d;
        channel->getDemux(m_d);
        eDVBDemux *d2=(eDVBDemux*)&(*m_d);

	eDVBCIInterfaces::getInstance()->recheckPMTHandlers();
        
        BlackBoxGetLastChannels(&channels);
        ch = &channels;

        if (ch->channels_quant<2)
        {
            ch->ch_tuner[ch->channels_quant] = d2->getAdapter();
            ch->ch_phys[ch->channels_quant] = slotid_tmp;
            ch->ch_cam[ch->channels_quant] = 0;
            ch->channels_quant++;
            //eDebug("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ BlackBoxSwitch Start");
            BlackBoxSwitch(ch);
            //eDebug("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ BlackBoxSwitch End");
        }
        else
        {
            eDebug("Scan Failed! No stream paths available for frontend %d ", feid);
            return -1;
        }
    #endif //QBOXHD_MINI
#endif // QBOXHD
        
	if (!(flags & scanRemoveServices))
	{
		ePtr<iDVBChannelList> db;
		ePtr<eDVBResourceManager> res;
		int err;
		if ((err = eDVBResourceManager::getInstance(res)) != 0)
			eDebug("no resource manager");
		else if ((err = res->getChannelList(db)) != 0)
			eDebug("no channel list");
		else
		{
			if (m_initial.size() > 1)
			{
				ePtr<iDVBFrontendParameters> tp = m_initial.first();
				int type;
				if (tp && !tp->getSystem(type))
				{
					switch(type)
					{
						case iDVBFrontend::feSatellite:
						{
							eDVBFrontendParametersSatellite parm;
							tp->getDVBS(parm);
							db->removeFlags(eDVBService::dxNewFound, -1, -1, -1, parm.orbital_position);
							break;
						}
						case iDVBFrontend::feCable:
							db->removeFlags(eDVBService::dxNewFound, 0xFFFF0000, -1, -1, -1);
							break;
						case iDVBFrontend::feTerrestrial:
							db->removeFlags(eDVBService::dxNewFound, 0xEEEE0000, -1, -1, -1);
							break;
					}
				}
			}
		}
	}
	m_scan->start(m_initial, flags);

	return 0;
}

int eComponentScan::getProgress()
{
	if (!m_scan)
		return 0;
	int done, total, services;
	m_scan->getStats(done, total, services);
	if (!total)
		return 0;
	return done * 100 / total;
}

int eComponentScan::getNumServices()
{
	if (!m_scan)
		return 0;
	int done, total, services;
	m_scan->getStats(done, total, services);

#ifdef QBOXHD
    eDebug("Number of services: %d",services);
#endif // QBOXHD
	return services;
}

int eComponentScan::isDone()
{
	return m_done;
}

int eComponentScan::getError()
{
	return m_failed;
}

void eComponentScan::getLastServiceName(std::string &string)
{
	if (!m_scan)
		return;
	m_scan->getLastServiceName(string);
}

RESULT eComponentScan::getFrontend(ePtr<iDVBFrontend> &fe)
{
	if (m_scan)
		return m_scan->getFrontend(fe);
	fe = 0;
	return -1;
}

RESULT eComponentScan::getCurrentTransponder(ePtr<iDVBFrontendParameters> &tp)
{
	if (m_scan)
		return m_scan->getCurrentTransponder(tp);
	tp = 0;
	return -1;
}

