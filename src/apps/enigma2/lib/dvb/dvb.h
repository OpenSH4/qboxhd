#ifndef __dvb_dvb_h
#define __dvb_dvb_h

#ifndef SWIG

#include <lib/base/ebase.h>
#include <lib/base/filepush.h>
#include <lib/base/elock.h>
#include <lib/dvb/idvb.h>
#include <lib/dvb/demux.h>
#include <lib/dvb/frontend.h>
#include <lib/dvb/tstools.h>
#include <lib/dvb/esection.h>
#include <connection.h>

#ifndef QBOXHD
#include <lib/dvb/feeder.h>
#endif // QBOXHD

#include <dvbsi++/service_description_section.h>

class eDVBChannel;

	/* we do NOT handle resource conflicts here. instead, the allocateChannel
	   fails, and the application has to see why the channel is allocated
	   (and how to deallocate it). */
class iDVBAdapter;

class eDVBRegisteredFrontend: public iObject, public Object
{
	DECLARE_REF(eDVBRegisteredFrontend);
	ePtr<eTimer> disable;
	void closeFrontend()
	{
		if (!m_inuse && m_frontend->closeFrontend()) // frontend busy
			disable->start(60000, true);  // retry close in 60secs
	}
public:
	Signal0<void> stateChanged;
	eDVBRegisteredFrontend(eDVBFrontend *fe, iDVBAdapter *adap)
		:disable(eTimer::create(eApp)), m_adapter(adap), m_frontend(fe), m_inuse(0)
	{
		CONNECT(disable->timeout, eDVBRegisteredFrontend::closeFrontend);
	}
	void dec_use()
	{
		if (!--m_inuse)
		{
			/* emit */ stateChanged();
			disable->start(3000, true);
		}
	}
	void inc_use()
	{
		if (++m_inuse == 1)
		{
			m_frontend->openFrontend();
			/* emit */ stateChanged();
		}
	}
	iDVBAdapter *m_adapter;
	ePtr<eDVBFrontend> m_frontend;
	int m_inuse;
};

struct eDVBRegisteredDemux
{
	DECLARE_REF(eDVBRegisteredDemux);
public:
	iDVBAdapter *m_adapter;
	ePtr<eDVBDemux> m_demux;
	int m_inuse;
	eDVBRegisteredDemux(eDVBDemux *demux, iDVBAdapter *adap): m_adapter(adap), m_demux(demux), m_inuse(0) { }
};

class eDVBAllocatedFrontend
{
	DECLARE_REF(eDVBAllocatedFrontend);
public:

	eDVBAllocatedFrontend(eDVBRegisteredFrontend *fe);
	~eDVBAllocatedFrontend();
	eDVBFrontend &get() { return *m_fe->m_frontend; }
	operator eDVBRegisteredFrontend*() { return m_fe; }
	operator eDVBFrontend*() { return m_fe->m_frontend; }

private:
	eDVBRegisteredFrontend *m_fe;
};

class eDVBAllocatedDemux
{
	DECLARE_REF(eDVBAllocatedDemux);
public:

	eDVBAllocatedDemux(eDVBRegisteredDemux *demux);
	~eDVBAllocatedDemux();
	eDVBDemux &get() { return *m_demux->m_demux; }
	operator eDVBRegisteredDemux*() { return m_demux; }
	operator eDVBDemux*() { return m_demux->m_demux; }

private:
	eDVBRegisteredDemux *m_demux;
};

class iDVBAdapter: public iObject
{
public:
	virtual int getNumDemux() = 0;
	virtual RESULT getDemux(ePtr<eDVBDemux> &demux, int nr) = 0;

	virtual int getNumFrontends() = 0;
	virtual RESULT getFrontend(ePtr<eDVBFrontend> &fe, int nr, bool simulate=false) = 0;
};

class eDVBAdapterLinux: public iDVBAdapter
{
	DECLARE_REF(eDVBAdapterLinux);
public:
	eDVBAdapterLinux(int nr);

	int getNumDemux();
	RESULT getDemux(ePtr<eDVBDemux> &demux, int nr);

	int getNumFrontends();
	RESULT getFrontend(ePtr<eDVBFrontend> &fe, int nr, bool simulate=false);

	static int exist(int nr);
private:
	int m_nr;
	eSmartPtrList<eDVBFrontend> m_frontend, m_simulate_frontend;
	eSmartPtrList<eDVBDemux>    m_demux;
};
#endif // SWIG

SWIG_IGNORE(eDVBResourceManager);
class eDVBResourceManager: public iObject, public Object
{
	DECLARE_REF(eDVBResourceManager);
	int avail, busy;

	enum { DM7025, DM800, DM500HD, DM8000 };

	int m_boxtype;

	eSmartPtrList<iDVBAdapter> m_adapter;
	eSmartPtrList<eDVBRegisteredDemux> m_demux;
	eSmartPtrList<eDVBRegisteredFrontend> m_frontend, m_simulate_frontend;
	void addAdapter(iDVBAdapter *adapter);

	struct active_channel
	{
		eDVBChannelID m_channel_id;
			/* we don't hold a reference here. */
		eDVBChannel *m_channel;

		active_channel(const eDVBChannelID &chid, eDVBChannel *ch) : m_channel_id(chid), m_channel(ch) { }
	};

	std::list<active_channel> m_active_channels, m_active_simulate_channels;

	ePtr<iDVBChannelList> m_list;
	ePtr<iDVBSatelliteEquipmentControl> m_sec;
	static eDVBResourceManager *instance;

	friend class eDVBChannel;
	RESULT addChannel(const eDVBChannelID &chid, eDVBChannel *ch);
	RESULT removeChannel(eDVBChannel *ch);

	Signal1<void,eDVBChannel*> m_channelAdded;

	eUsePtr<iDVBChannel> m_cached_channel;
	Connection m_cached_channel_state_changed_conn;
	ePtr<eTimer> m_releaseCachedChannelTimer;
	void DVBChannelStateChanged(iDVBChannel*);
	void feStateChanged();
#ifndef SWIG
public:
#endif
	void releaseCachedChannel();
	eDVBResourceManager();
	virtual ~eDVBResourceManager();

	RESULT setChannelList(iDVBChannelList *list);
	RESULT getChannelList(ePtr<iDVBChannelList> &list);

	enum {
			/* errNoFrontend = -1 replaced by more spcific messages */
		errNoDemux    = -2,
		errChidNotFound = -3,
		errNoChannelList = -4,
		errChannelNotInList = -5,
		errAllSourcesBusy = -6,
		errNoSourceFound = -7,
	};

	RESULT connectChannelAdded(const Slot1<void,eDVBChannel*> &channelAdded, ePtr<eConnection> &connection);
	int canAllocateChannel(const eDVBChannelID &channelid, const eDVBChannelID &ignore, bool simulate=false);

		/* allocate channel... */
	RESULT allocateChannel(const eDVBChannelID &channelid, eUsePtr<iDVBChannel> &channel, bool simulate=false);
	RESULT allocatePVRChannel(eUsePtr<iDVBPVRChannel> &channel);
	static RESULT getInstance(ePtr<eDVBResourceManager> &);

			/* allocates a frontend able to tune to frontend paramters 'feperm'.
			   the frontend must be tuned lateron. there is no guarante
			   that tuning will succeed - it just means that if this frontend
			   can't tune, no other frontend could do it.

			   there might be a priority given to certain frontend/chid
			   combinations. this will be evaluated here. */
	RESULT allocateFrontend(ePtr<eDVBAllocatedFrontend> &fe, ePtr<iDVBFrontendParameters> &feparm, bool simulate=false);

	RESULT allocateFrontendByIndex(ePtr<eDVBAllocatedFrontend> &fe, int slot_index);

	/* allocate a demux able to filter on the selected frontend. */
	RESULT allocateDemux(eDVBRegisteredFrontend *fe, ePtr<eDVBAllocatedDemux> &demux, int &cap);


#ifdef SWIG
public:
#endif
	int canAllocateFrontend(ePtr<iDVBFrontendParameters> &feparm, bool simulate=false);
	bool canMeasureFrontendInputPower();
	PSignal1<void,int> frontendUseMaskChanged;
	SWIG_VOID(RESULT) allocateRawChannel(eUsePtr<iDVBChannel> &SWIG_OUTPUT, int slot_index);
	PyObject *setFrontendSlotInformations(SWIG_PYOBJECT(ePyObject) list);
};
SWIG_TEMPLATE_TYPEDEF(ePtr<eDVBResourceManager>, eDVBResourceManager);
SWIG_EXTEND(ePtr<eDVBResourceManager>,
	static ePtr<eDVBResourceManager> getInstance()
	{
		extern ePtr<eDVBResourceManager> NewResourceManagerPtr(void);
		return NewResourceManagerPtr();
	}
);

#ifndef SWIG

class eDVBChannelFilePush;

	/* iDVBPVRChannel includes iDVBChannel. don't panic. */
class eDVBChannel: public iDVBPVRChannel, public iFilePushScatterGather, public Object
{
	DECLARE_REF(eDVBChannel);
	friend class eDVBResourceManager;
public:
	eDVBChannel(eDVBResourceManager *mgr, eDVBAllocatedFrontend *frontend);
	virtual ~eDVBChannel();

		/* only for managed channels - effectively tunes to the channelid. should not be used... */
		/* cannot be used for PVR channels. */
	RESULT setChannel(const eDVBChannelID &id, ePtr<iDVBFrontendParameters> &feparam);
	eDVBChannelID getChannelID() { return m_channel_id; }
#if defined(__sh__)
    int getSkipMode() { return m_skipmode_m; }
#endif

	RESULT connectStateChange(const Slot1<void,iDVBChannel*> &stateChange, ePtr<eConnection> &connection);
	RESULT connectEvent(const Slot2<void,iDVBChannel*,int> &eventChange, ePtr<eConnection> &connection);

	RESULT getState(int &state);

	RESULT setCIRouting(const eDVBCIRouting &routing);
	RESULT getDemux(ePtr<iDVBDemux> &demux, int cap);


	RESULT getFrontend(ePtr<iDVBFrontend> &frontend);
	RESULT getCurrentFrontendParameters(ePtr<iDVBFrontendParameters> &param);

		/* iDVBPVRChannel */
	RESULT playFile(const char *file);
	void stopFile();

	void setCueSheet(eCueSheet *cuesheet);

	RESULT getLength(pts_t &len);
	RESULT getCurrentPosition(iDVBDemux *decoding_demux, pts_t &pos, int mode);

	int getUseCount() { return m_use_count; }

	RESULT requestTsidOnid(ePyObject callback);
private:
	ePtr<eDVBAllocatedFrontend> m_frontend;
	ePtr<eDVBAllocatedDemux> m_demux, m_decoder_demux;

	ePtr<iDVBFrontendParameters> m_current_frontend_parameters;
	eDVBChannelID m_channel_id;
	Signal1<void,iDVBChannel*> m_stateChanged;
	Signal2<void,iDVBChannel*,int> m_event;
	int m_state;

			/* for channel list */
	ePtr<eDVBResourceManager> m_mgr;

	void frontendStateChanged(iDVBFrontend*fe);
	ePtr<eConnection> m_conn_frontendStateChanged;

		/* for PVR playback */
	eDVBChannelFilePush *m_pvr_thread;
	void pvrEvent(int event);

	int m_pvr_fd_dst;
	eDVBTSTools m_tstools;

	ePtr<eCueSheet> m_cue;

	void cueSheetEvent(int event);
	ePtr<eConnection> m_conn_cueSheetEvent;
	int m_skipmode_m, m_skipmode_n, m_skipmode_frames, m_skipmode_frames_remainder;

	std::list<std::pair<off_t, off_t> > m_source_span;
	void getNextSourceSpan(off_t current_offset, size_t bytes_read, off_t &start, size_t &size);
	void flushPVR(iDVBDemux *decoding_demux=0);

	eSingleLock m_cuesheet_lock;

	friend class eUsePtr<eDVBChannel>;
		/* use count */
	oRefCount m_use_count;
	void AddUse();
	void ReleaseUse();
#ifndef QBOXHD
	struct feeder_cmd_st m_feeder;
    int fd_video, fd_audio, fd_ademux0, fd_ademux1, fd_vdemux0, fd_vdemux1;
//     enum { MPEG2, MPEG4_H264 };
//     enum { aMPEG, aAC3, aDTS, aAAC };
#endif // QBOXHD


		/* for tsid/onid read */
	ePyObject m_tsid_onid_callback;
	ePtr<iDVBDemux> m_tsid_onid_demux;
	ePtr<eTable<ServiceDescriptionSection> > m_SDT;
	void SDTready(int err);
};
#endif // SWIG

#endif
