#ifndef __dvb_demux_h
#define __dvb_demux_h

#include <lib/dvb/idvb.h>
#include <lib/dvb/idemux.h>
#ifndef QBOXHD
#include <lib/dvb/feeder.h>
#endif // QBOXHD

class eDVBDemux: public iDVBDemux
{
	DECLARE_REF(eDVBDemux);
public:
	enum {
		evtFlush
	};
	eDVBDemux(int adapter, int demux);
	virtual ~eDVBDemux();

	RESULT setSourceFrontend(int fenum);
	int getSource() { return source; }
#ifdef QBOXHD
	int getAdapter() { return adapter; }
	int getDemux()   { return demux; }
#endif // QBOXHD
#ifndef QBOXHD
	int isDVRBusy() { return m_dvr_busy; }
#endif // QBOXHD
	RESULT setSourcePVR(int pvrnum);

	RESULT createSectionReader(eMainloop *context, ePtr<iDVBSectionReader> &reader);
	RESULT createPESReader(eMainloop *context, ePtr<iDVBPESReader> &reader);
	RESULT createTSRecorder(ePtr<iDVBTSRecorder> &recorder);
	RESULT getMPEGDecoder(ePtr<iTSMPEGDecoder> &reader, int primary);
	RESULT getSTC(pts_t &pts, int num);
	RESULT getCADemuxID(uint8_t &id) { id = demux; return 0; }
	RESULT flush();
	RESULT connectEvent(const Slot1<void,int> &event, ePtr<eConnection> &conn);

	int getRefCount() { return ref; }
private:
	int adapter, demux, source;
	int m_dvr_busy;

	friend class eDVBSectionReader;
	friend class eDVBPESReader;
	friend class eDVBAudio;
	friend class eDVBVideo;
	friend class eDVBPCR;
	friend class eDVBTText;
	friend class eDVBTSRecorder;
	friend class eDVBCAService;
	Signal1<void, int> m_event;

	int openDemux(void);
};

class eDVBSectionReader: public iDVBSectionReader, public Object
{
	DECLARE_REF(eDVBSectionReader);
	int fd;
	Signal1<void, const __u8*> read;
	ePtr<eDVBDemux> demux;
	int active;
	int checkcrc;
	void data(int);
	ePtr<eSocketNotifier> notifier;
public:

	eDVBSectionReader(eDVBDemux *demux, eMainloop *context, RESULT &res);
	virtual ~eDVBSectionReader();
	RESULT start(const eDVBSectionFilterMask &mask);
	RESULT stop();
	RESULT connectRead(const Slot1<void,const __u8*> &read, ePtr<eConnection> &conn);
};

class eDVBPESReader: public iDVBPESReader, public Object
{
	DECLARE_REF(eDVBPESReader);
	int m_fd;
	Signal2<void, const __u8*, int> m_read;
	ePtr<eDVBDemux> m_demux;
	int m_active;
	void data(int);
	ePtr<eSocketNotifier> m_notifier;
public:
	eDVBPESReader(eDVBDemux *demux, eMainloop *context, RESULT &res);
	virtual ~eDVBPESReader();
	RESULT start(int pid);
	RESULT stop();
	RESULT connectRead(const Slot2<void,const __u8*, int> &read, ePtr<eConnection> &conn);
};

class eDVBRecordFileThread;

class eDVBTSRecorder: public iDVBTSRecorder, public Object
{
	DECLARE_REF(eDVBTSRecorder);
public:
	eDVBTSRecorder(eDVBDemux *demux);
	~eDVBTSRecorder();

	RESULT start();
	RESULT addPID(int pid);
	RESULT removePID(int pid);

	RESULT setTimingPID(int pid, int type);

	RESULT setTargetFD(int fd);
	RESULT setTargetFilename(const char *filename);
	RESULT setBoundary(off_t max);

	RESULT stop();

	RESULT getCurrentPCR(pts_t &pcr);

	RESULT connectEvent(const Slot1<void,int> &event, ePtr<eConnection> &conn);

private:
	RESULT startPID(int pid);
	void stopPID(int pid);

	eDVBRecordFileThread *m_thread;
	void filepushEvent(int event);

	std::map<int,int> m_pids;
	Signal1<void,int> m_event;

	ePtr<eDVBDemux> m_demux;

	int m_running, m_target_fd, m_source_fd;
	std::string m_target_filename;
#ifndef QBOXHD
    int m_timeshift_enabled;
    std::string m_timeshift_filename;
#endif // QBOXHD
};

#endif
