#include <lib/service/servicedvbrecord.h>
#include <lib/base/eerror.h>
#include <lib/dvb/epgcache.h>
#include <lib/dvb/metaparser.h>
#include <fcntl.h>

#ifdef QBOXHD
/* For disable/enable tuners and CI modules when the decoder is in standby and there is a Timer */
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include "../driver/qboxhd_generic.h"
#include <lib/dvb/feeder.h>
extern unsigned char in_standby_status;
#ifndef QBOXHD_MINI
extern void reset_the_modules(int v);
#endif
#endif /// QBOXHD

	/* for cutlist */
#include <byteswap.h>
#include <netinet/in.h>

#ifndef BYTE_ORDER
#error no byte order defined!
#endif

#if defined(__sh__)
#include <sys/vfs.h>
//this is not available for stlinux :-(
//#include <linux/magic.h>

/* and these i dont get included :-(
#include <linux/usbdevice_fs.h>
#include <linux/smb.h>
#include <linux/nfs__fs.h>
#include <linux/ext3_fs.h>

so hack ;-)
*/
#define USBDEVICE_SUPER_MAGIC 0x9fa2
#define EXT2_SUPER_MAGIC      0xEF53
#define EXT3_SUPER_MAGIC      0xEF53
#define SMB_SUPER_MAGIC       0x517B
#define NFS_SUPER_MAGIC       0x6969
#define MSDOS_SUPER_MAGIC     0x4d44            /* MD */
#endif

DEFINE_REF(eDVBServiceRecord);

eDVBServiceRecord::eDVBServiceRecord(const eServiceReferenceDVB &ref): m_ref(ref)
{
	CONNECT(m_service_handler.serviceEvent, eDVBServiceRecord::serviceEvent);
	CONNECT(m_event_handler.m_eit_changed, eDVBServiceRecord::gotNewEvent);
	m_state = stateIdle;
	m_want_record = 0;
	m_tuned = 0;
	m_target_fd = -1;
	m_error = 0;
	m_streaming = 0;
	m_simulate = false;
	m_last_event_id = -1;
}

void eDVBServiceRecord::serviceEvent(int event)
{
	eDebug("eDVBServiceRecord::serviceEvent() - RECORD service event %d", event);
	switch (event)
	{
		case eDVBServicePMTHandler::eventTuned:
		{
			eDebug("tuned..");
			m_tuned = 1;

				/* start feeding EIT updates */
			ePtr<iDVBDemux> m_demux;
			if (!m_service_handler.getDataDemux(m_demux))
			{
				eServiceReferenceDVB &ref = (eServiceReferenceDVB&) m_ref;
				int sid = ref.getParentServiceID().get();
				if (!sid)
					sid = ref.getServiceID().get();
				if ( ref.getParentTransportStreamID().get() &&
					ref.getParentTransportStreamID() != ref.getTransportStreamID() )
					m_event_handler.startOther(m_demux, sid);
				else
					m_event_handler.start(m_demux, sid);
			}

			if (m_state == stateRecording && m_want_record)
				doRecord();
			m_event((iRecordableService*)this, evTunedIn);
			break;
		}
		case eDVBServicePMTHandler::eventTuneFailed:
		{
	// 		eDebug("record failed to tune");
			eDebug("eDVBServiceRecord::serviceEvent(eDVBServicePMTHandler::eventTuneFailed)\n\n");
			m_event((iRecordableService*)this, evTuneFailed);
			break;
		}
		case eDVBServicePMTHandler::eventNewProgramInfo:
		{
			eDebug("eDVBServiceRecord::serviceEvent(eDVBServicePMTHandler::eventNewProgramInfo)\n\n");
			if (m_state == stateIdle)
				doPrepare();
			else if (m_want_record) /* doRecord can be called from Prepared and Recording state */
				doRecord();
			m_event((iRecordableService*)this, evNewProgramInfo);
			break;
		}
		case eDVBServicePMTHandler::eventMisconfiguration:
		{
			eDebug("eDVBServiceRecord::serviceEvent(eDVBServicePMTHandler::eventMisconfiguration)\n\n");
			m_error = errMisconfiguration;
			m_event((iRecordableService*)this, evTuneFailed);
		}
			break;
		case eDVBServicePMTHandler::eventNoResources:
		{
			eDebug("eDVBServiceRecord::serviceEvent(eDVBServicePMTHandler::eventNoResources)\n\n");
			m_error = errNoResources;
			m_event((iRecordableService*)this, evTuneFailed);
		}
			break;
	}
//     eDebug("eDVBServiceRecord::serviceEvent() - DONE");
}

RESULT eDVBServiceRecord::prepare(const char *filename, time_t begTime, time_t endTime, int eit_event_id, const char *name, const char *descr, const char *tags)
{
	m_filename = filename;
	m_streaming = 0;

	if (m_state == stateIdle)
	{
		int ret = doPrepare();
		if (!ret)
		{
			eServiceReferenceDVB ref = m_ref.getParentServiceReference();
			ePtr<eDVBResourceManager> res_mgr;
			eDVBMetaParser meta;
			std::string service_data;
			if (!ref.valid())
				ref = m_ref;
			if (!eDVBResourceManager::getInstance(res_mgr))
			{
				ePtr<iDVBChannelList> db;
				if (!res_mgr->getChannelList(db))
				{
					ePtr<eDVBService> service;
					if (!db->getService(ref, service))
					{
						char tmp[255];
						sprintf(tmp, "f:%x", service->m_flags);
						service_data += tmp;
						// cached pids
						for (int x=0; x < eDVBService::cacheMax; ++x)
						{
							int entry = service->getCacheEntry((eDVBService::cacheID)x);
							if (entry != -1)
							{
								sprintf(tmp, ",c:%02d%04x", x, entry);
								service_data += tmp;
							}
						}
					}
				}
			}
			meta.m_time_create = begTime;
			meta.m_ref = m_ref;
			meta.m_data_ok = 1;
			meta.m_service_data = service_data;
			if (name)
				meta.m_name = name;
			if (descr)
				meta.m_description = descr;
			if (tags)
				meta.m_tags = tags;
			ret = meta.updateMeta(filename) ? -255 : 0;
			if (!ret)
			{
				const eit_event_struct *event = 0;
				eEPGCache::getInstance()->Lock();
				if ( eit_event_id != -1 )
				{
					eDebug("query epg event id %d", eit_event_id);
					eEPGCache::getInstance()->lookupEventId(ref, eit_event_id, event);
				}
				if ( !event && (begTime != -1 && endTime != -1) )
				{
					time_t queryTime = begTime + ((endTime-begTime)/2);
					tm beg, end, query;
					localtime_r(&begTime, &beg);
					localtime_r(&endTime, &end);
					localtime_r(&queryTime, &query);
					eDebug("query stime %d:%d:%d, etime %d:%d:%d, qtime %d:%d:%d",
						beg.tm_hour, beg.tm_min, beg.tm_sec,
						end.tm_hour, end.tm_min, end.tm_sec,
						query.tm_hour, query.tm_min, query.tm_sec);
					eEPGCache::getInstance()->lookupEventTime(ref, queryTime, event);
				}
				if ( event )
				{
					eDebug("found event.. store to disc");
					std::string fname = filename;
					fname.erase(fname.length()-2, 2);
					fname+="eit";
					int fd = open(fname.c_str(), O_CREAT|O_WRONLY, 0777);
					if (fd>-1)
					{
						int evLen=HILO(event->descriptors_loop_length)+12/*EIT_LOOP_SIZE*/;
						int wr = ::write( fd, (unsigned char*)event, evLen );
						if ( wr != evLen )
							eDebug("eit write error (%m)");
						::close(fd);
					}
				}
				eEPGCache::getInstance()->Unlock();
			}
		}
		return ret;
	}
	return -1;
}

RESULT eDVBServiceRecord::prepareStreaming()
{
	m_filename = "";
	m_streaming = 1;
	if (m_state == stateIdle)
		return doPrepare();
	return -1;
}

RESULT eDVBServiceRecord::start(bool simulate)
{
	#ifdef  QBOXHD
	#ifndef QBOXHD_MINI
		if(in_standby_status!=0)
			reset_the_modules(0);
	#endif
	#endif

	m_simulate = simulate;
	m_want_record = 1;
		/* when tune wasn't yet successfully, doRecord stays in "prepared"-state which is fine. */
	m_event((iRecordableService*)this, evStart);
	eDebug("eDVBServiceRecord::start: calling doRecord().\n");
	return doRecord();
}

RESULT eDVBServiceRecord::stop()
{
eDebug("eDVBServiceRecord::stop() ---------------- m_target_fd=%d\n",m_target_fd);

	#ifdef QBOXHD
	#ifndef QBOXHD_MINI
		if(in_standby_status!=0)
			reset_the_modules(1);
	#endif
	#endif //QBOXHD

	if (!m_simulate)
		eDebug("stop recording!");
	if (m_state == stateRecording)
	{
		if (m_record)
			m_record->stop();
		if (m_target_fd >= 0)
		{
			::close(m_target_fd);
			m_target_fd = -1;
		}

		saveCutlist();

		m_state = statePrepared;
	} else if (!m_simulate)
		eDebug("(was not recording)");
	if (m_state == statePrepared)
	{
		m_record = 0;
		m_state = stateIdle;
	}
	m_event((iRecordableService*)this, evRecordStopped);
	return 0;
}

int eDVBServiceRecord::doPrepare()
{
	eDebug("eDVBServiceRecord::doPrepare() called in m_state== '%d'...", m_state);
		/* allocate a ts recorder if we don't already have one. */
	if (m_state == stateIdle)
	{
		eDebug("eDVBServiceRecord::doPrepare() called to record to '%s'...", m_filename.c_str());
		m_pids_active.clear();
		m_state = statePrepared;
		return m_service_handler.tune(m_ref, 0, 0, m_simulate);
	}
	return 0;
}


#ifdef QBOXHD
/*
 *please do not remove the following functions
int Connect()
{
	struct sockaddr_un m_servaddr;
	int m_sock=-1, m_clilen;

	memset(&m_servaddr, 0, sizeof(struct sockaddr_un));
	m_servaddr.sun_family = AF_UNIX;
	strcpy(m_servaddr.sun_path, "/tmp/camd.socket");
	m_clilen = sizeof(m_servaddr.sun_family) + strlen(m_servaddr.sun_path);
	m_sock = socket(PF_UNIX, SOCK_STREAM, 0);

	if (m_sock != -1)
	{
		if (!connect(m_sock, (struct sockaddr *) &m_servaddr, m_clilen))
		{
			eDebug("[eDVBServiceRecord] Connect ...");
			int val=1;
			fcntl(m_sock, F_SETFL, O_NONBLOCK);
			setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, &val, 4);
		}
	}
	else
		eDebug("[eDVBServiceRecord] create socket failed %m");

	return m_sock;
}

unsigned char ascii2char(char *hexVal)
{
	unsigned char val=0;
	unsigned char valH=0;
	unsigned char valL=0;
	if(hexVal[0]>='0'&& hexVal[0]<='9')
		valH=(hexVal[0]-'0');
	else
	{
		if(hexVal[0]>='A'&& hexVal[0]<='F')
			valH=(hexVal[0]-'A')+10;
		else if(hexVal[0]>='a'&& hexVal[0]<='f')
			valH=(hexVal[0]-'a')+10;
	}

	if(hexVal[1]>='0'&& hexVal[1]<='9')
		valL=(hexVal[1]-'0');
	else
	{
		if(hexVal[1]>='A'&& hexVal[1]<='F')
			valL=(hexVal[1]-'A')+10;
		else if(hexVal[1]>='a'&& hexVal[1]<='f')
			valL=(hexVal[1]-'a')+10;
	}
	val= (valH<<4)|valL;

// 	eDebug("[eDVBServiceRecord] ascii2char(%c%c)=%d",hexVal[0],hexVal[1],val);
	return val;
}

void sendLastCAPMT()
{
	FILE *pmtfd=0;
	unsigned char pmt_hex[2048];
	char hexVal[3];
	const char filePmt[] = "/tmp/pmt.tmp";
	int filesize=0;

	///get file size
	struct stat sb;
	stat(filePmt,&sb);
// 	filesize = (long long) sb.st_size;
	filesize = (int) sb.st_size;

	pmtfd = fopen(filePmt, "r");
	if(pmtfd)
	{
		int read = 0;
		int i = 0;
		int m_sock=-1;

		eDebug("[eDVBServiceRecord] filesize=%d ------------------------------------------ ",filesize);

		///read file
		memset( pmt_hex, 0, sizeof(pmt_hex));
		for(i=0;i<filesize;i++)
		{
			read = fread(hexVal, 2*sizeof(char), 1, pmtfd);
			if(read)
				pmt_hex[i]=ascii2char(hexVal);
		}
		fclose( pmtfd );


		eDebug("[eDVBServiceRecord] 1. ------------------------------------------ ");
		///sendCAPMT
		unsigned char *m_capmt = pmt_hex;
		int wp=0;
		if ( m_capmt[3] & 0x80 )
		{
			int i=0;
			int lenbytes = m_capmt[3] & ~0x80;
			while(i < lenbytes)
				wp = (wp << 8) | m_capmt[4 + i++];
			wp+=4;
			wp+=lenbytes;
		}
		else
		{
			wp = m_capmt[3];
			wp+=4;
		}

		eDebug("[eDVBServiceRecord] wp=%d. ------------------------------------------ ",wp);

		m_sock=Connect();
		eDebug("[eDVBServiceRecord] m_sock=%d. ------------------------------------------ ",m_sock);

		if(m_sock>=0)
		{
			eDebug("[eDVBServiceRecord] sending %d bytes .... ++++++++++++++++++++++++++++++++++++++++++++",wp);

			if ( write(m_sock, m_capmt, wp) == wp )
			{
				///print data sent on socket !!!!!!!!!!!!!
				int i=0;
				char tmp[500];
				tmp[0]='\0';
				for(i=0;i<wp && (3*i)<500 ;i++)
					sprintf(tmp+(3*i),"%02x ",m_capmt[i]);

				eDebug("[eDVBServiceRecord] sent %d bytes:\n %s \n+++++++++++++++++++++++++++++++++++",wp,tmp);
			}
			else
				eDebug("[eDVBServiceRecord] WRITE to socket failed %m");
		}
	}
	else
		eDebug("[eDVBCAService] Open /tmp/pmt.tmp error");
}
*/
#endif

int eDVBServiceRecord::doRecord()
{
  eDebug("eDVBServiceRecord::doRecord() called to record to '%s'...", m_filename.c_str());

	int err = doPrepare();
	if (err)
	{
		m_error = errTuneFailed;
		m_event((iRecordableService*)this, evRecordFailed);
		return err;
	}

	if (!m_tuned)
		return 0; /* try it again when we are tuned in */

	if (!m_record && m_tuned && !m_streaming && !m_simulate)
	{
#if defined(__sh__)
		int flags = O_WRONLY|O_CREAT|O_LARGEFILE;
		struct statfs sbuf;
#endif
		eDebug("eDVBServiceRecord::doRecord() - Recording to %s...", m_filename.c_str());
		::remove(m_filename.c_str());
#if defined(__sh__)
		//nit2005, we must creat a file for statfs
 		int fd = ::open(m_filename.c_str(), O_WRONLY|O_CREAT|O_LARGEFILE, 0644);
		::close(fd);
		if (statfs(m_filename.c_str(), &sbuf) < 0)
		{
			eDebug("eDVBServiceRecord - can't get fs type assuming none NFS!");
		}
		else
		{
			if (sbuf.f_type == EXT3_SUPER_MAGIC)
				eDebug("eDVBServiceRecord - Ext2/3/4 Filesystem\n");
			else
			if (sbuf.f_type == NFS_SUPER_MAGIC)
			{
				eDebug("eDVBServiceRecord - NFS Filesystem; add O_DIRECT to flags\n");
				flags |= O_DIRECT;
			}
			else
			if (sbuf.f_type == USBDEVICE_SUPER_MAGIC)
				eDebug("eDVBServiceRecord - USB Device\n");
			else
			if (sbuf.f_type == SMB_SUPER_MAGIC)
				eDebug("eDVBServiceRecord - SMBs Device\n");
			else
			if (sbuf.f_type == MSDOS_SUPER_MAGIC)
				eDebug("eDVBServiceRecord - MSDOS Device\n");
		}

		fd = ::open(m_filename.c_str(), flags, 0644);
#else
		int fd = ::open(m_filename.c_str(), O_WRONLY|O_CREAT|O_LARGEFILE, 0644);
#endif  ///__sh__
		if (fd == -1)
		{
			eDebug("eDVBServiceRecord - can't open recording file!");
			m_error = errOpenRecordFile;
			m_event((iRecordableService*)this, evRecordFailed);
			return errOpenRecordFile;
		}

			/* turn off kernel caching strategies */
		posix_fadvise(fd, 0, 0, POSIX_FADV_RANDOM);

		ePtr<iDVBDemux> demux;
		if (m_service_handler.getDataDemux(demux))
		{
			eDebug("eDVBServiceRecord - NO DEMUX available!");
			m_error = errNoDemuxAvailable;
			m_event((iRecordableService*)this, evRecordFailed);
			return errNoDemuxAvailable;
		}
		demux->createTSRecorder(m_record);
		if (!m_record)
		{
			eDebug("eDVBServiceRecord - no ts recorder available.");
			m_error = errNoTsRecorderAvailable;
			m_event((iRecordableService*)this, evRecordFailed);
			return errNoTsRecorderAvailable;
		}
		m_record->setTargetFD(fd);
		m_record->setTargetFilename(m_filename.c_str());
		m_record->connectEvent(slot(*this, &eDVBServiceRecord::recordEvent), m_con_record_event);

		m_target_fd = fd;
	}

	if (m_streaming)
	{

#ifndef QBOXHD
		ePtr<iDVBDemux> demux;
		if (m_service_handler.getDataDemux(demux))
		{
			eDebug("eDVBServiceRecord - NO DEMUX available!");
			m_error = errNoDemuxAvailable;
			m_event((iRecordableService*)this, evRecordFailed);
			return errNoDemuxAvailable;
		}

		if ( demux->isDVRBusy() ) {
			eDebug("eDVBServiceRecord - DEMUX already used for recording!");
			m_error = errNoDemuxAvailable;
			m_event((iRecordableService*)this, evRecordFailed);
			return errNoDemuxAvailable;
		}
#endif

		m_state = stateRecording;
		eDebug("start streaming...");
	}
	else
	{
		eDebug("eDVBServiceRecord::doRecord() - start recording...");

		eDVBServicePMTHandler::program program;
		if (m_service_handler.getProgramInfo(program))
			eDebug("getting program info failed.");
		else
		{
			std::set<int> pids_to_record;

			pids_to_record.insert(0); // PAT

			if (program.pmtPid != -1)
				pids_to_record.insert(program.pmtPid); // PMT

			int timing_pid = -1, timing_pid_type = -1;

			eDebugNoNewLine("RECORD: have %d video stream(s)", program.videoStreams.size());
			if (!program.videoStreams.empty())
			{
				eDebugNoNewLine(" (");
				for (std::vector<eDVBServicePMTHandler::videoStream>::const_iterator
					i(program.videoStreams.begin());
					i != program.videoStreams.end(); ++i)
				{
					pids_to_record.insert(i->pid);

					if (timing_pid == -1)
					{
						timing_pid = i->pid;
						timing_pid_type = i->type;
					}

					if (i != program.videoStreams.begin())
							eDebugNoNewLine(", ");
					eDebugNoNewLine("%04x", i->pid);
				}
				eDebugNoNewLine(")");
			}
			eDebugNoNewLine(", and %d audio stream(s)", program.audioStreams.size());
			if (!program.audioStreams.empty())
			{
				eDebugNoNewLine(" (");
				for (std::vector<eDVBServicePMTHandler::audioStream>::const_iterator
					i(program.audioStreams.begin());
					i != program.audioStreams.end(); ++i)
				{
					pids_to_record.insert(i->pid);

					if (timing_pid == -1)
					{
						timing_pid = i->pid;
						timing_pid_type = -1;
					}

					if (i != program.audioStreams.begin())
						eDebugNoNewLine(", ");
					eDebugNoNewLine("%04x", i->pid);
				}
				eDebugNoNewLine(")");
			}
			if (!program.subtitleStreams.empty())
			{
				eDebugNoNewLine(", Subtitles(");
				for (std::vector<eDVBServicePMTHandler::subtitleStream>::const_iterator
					i(program.subtitleStreams.begin());
					i != program.subtitleStreams.end(); ++i)
				{
					pids_to_record.insert(i->pid);

					if (i != program.subtitleStreams.begin())
						eDebugNoNewLine(", ");
					eDebugNoNewLine("%04x", i->pid);
				}
				eDebugNoNewLine(")");
			}
			eDebugNoNewLine(", and the pcr pid is %04x", program.pcrPid);
			if (program.pcrPid != 0x1fff)
				pids_to_record.insert(program.pcrPid);
			eDebug(", and the text pid is %04x", program.textPid);
			if (program.textPid != -1)
				pids_to_record.insert(program.textPid); // Videotext

				/* find out which pids are NEW and which pids are obsolete.. */
			std::set<int> new_pids, obsolete_pids;

			std::set_difference(pids_to_record.begin(), pids_to_record.end(),
					m_pids_active.begin(), m_pids_active.end(),
					std::inserter(new_pids, new_pids.begin()));

			std::set_difference(
					m_pids_active.begin(), m_pids_active.end(),
					pids_to_record.begin(), pids_to_record.end(),
					std::inserter(obsolete_pids, obsolete_pids.begin())
					);


			for (std::set<int>::iterator i(new_pids.begin()); i != new_pids.end(); ++i)
			{
				eDebug("ADD PID: %04x", *i);
				m_record->addPID(*i);
			}

			for (std::set<int>::iterator i(obsolete_pids.begin()); i != obsolete_pids.end(); ++i)
			{
				eDebug("REMOVED PID: %04x", *i);
				m_record->removePID(*i);
			}

#ifdef QBOXHD
// 			sendLastCAPMT();
#endif

			if (timing_pid != -1)
				m_record->setTimingPID(timing_pid, timing_pid_type);

			m_pids_active = pids_to_record;

			if (m_state != stateRecording)
			{
				eDebug("eDVBServiceRecord::doRecord() - m_record->start() state=%s moving to stateRecording",
					   (m_state==stateIdle)?"stateIdle":"statePrepared");
				m_record->start();
				m_state = stateRecording;
			}

		}
	}
	m_error = 0;
	m_event((iRecordableService*)this, evRecordRunning);

	eDebug("eDVBServiceRecord::doRecord() - DONE.");
	return 0;
}

RESULT eDVBServiceRecord::frontendInfo(ePtr<iFrontendInformation> &ptr)
{
	ptr = this;
	return 0;
}

RESULT eDVBServiceRecord::connectEvent(const Slot2<void,iRecordableService*,int> &event, ePtr<eConnection> &connection)
{
	connection = new eConnection((iRecordableService*)this, m_event.connect(event));
	return 0;
}

RESULT eDVBServiceRecord::stream(ePtr<iStreamableService> &ptr)
{
	ptr = this;
	return 0;
}

extern void PutToDict(ePyObject &dict, const char*key, long val);  // defined in dvb/frontend.cpp

PyObject *eDVBServiceRecord::getStreamingData()
{
	eDVBServicePMTHandler::program program;
	if (!m_tuned || m_service_handler.getProgramInfo(program))
	{
		Py_RETURN_NONE;
	}

	ePyObject r = program.createPythonObject();
	ePtr<iDVBDemux> demux;
	if (!m_service_handler.getDataDemux(demux))
	{
		uint8_t demux_id;
		if (!demux->getCADemuxID(demux_id))
			PutToDict(r, "demux", demux_id);

#ifdef QBOXHD
		PutToDict(r, "adapter", demux->getAdapter());
#endif

	}

	return r;
}

void eDVBServiceRecord::recordEvent(int event)
{
	switch (event)
	{
	case iDVBTSRecorder::eventWriteError:
		eWarning("[eDVBServiceRecord] record write error");
		stop();
		m_event((iRecordableService*)this, evRecordWriteError);
		return;
	default:
		eDebug("unhandled record event %d", event);
	}
}

void eDVBServiceRecord::gotNewEvent()
{
	ePtr<eServiceEvent> event_now;
	m_event_handler.getEvent(event_now, 0);

	if (!event_now)
		return;

	int event_id = event_now->getEventId();

	pts_t p;

	if (m_record)
	{
		if (m_record->getCurrentPCR(p))
			eDebug("getting PCR failed!");
		else
		{
			m_event_timestamps[event_id] = p;
			eDebug("pcr of eit change: %llx", p);
		}
	}

	if (event_id != m_last_event_id)
		eDebug("[eDVBServiceRecord] now running: %s (%d seconds)", event_now->getEventName().c_str(), event_now->getDuration());

	m_last_event_id = event_id;

	m_event((iRecordableService*)this, evNewEventInfo);
}

void eDVBServiceRecord::saveCutlist()
{
			/* XXX: dupe of eDVBServicePlay::saveCuesheet, refactor plz */
	std::string filename = m_filename + ".cuts";

	eDVBTSTools tstools;

	if (tstools.openFile(m_filename.c_str()))
	{
		eDebug("[eDVBServiceRecord] saving cutlist failed because tstools failed");
		return;
	}

	FILE *f = fopen(filename.c_str(), "wb");

	if (f)
	{
		unsigned long long where;
		int what;

		for (std::map<int,pts_t>::iterator i(m_event_timestamps.begin()); i != m_event_timestamps.end(); ++i)
		{
			pts_t p = i->second;
			off_t offset = 0; // fixme, we need to note down both
			if (tstools.fixupPTS(offset, p))
			{
				eDebug("[eDVBServiceRecord] fixing up PTS failed, not saving");
				continue;
			}
			eDebug("fixed up %llx to %llx (offset %llx)", i->second, p, offset);
#if BYTE_ORDER == BIG_ENDIAN
			where = p;
#else
			where = bswap_64(p);
#endif
			what = htonl(2); /* mark */
			fwrite(&where, sizeof(where), 1, f);
			fwrite(&what, sizeof(what), 1, f);
		}
		fclose(f);
	}

}

RESULT eDVBServiceRecord::subServices(ePtr<iSubserviceList> &ptr)
{
	ptr = this;
	return 0;
}

int eDVBServiceRecord::getNumberOfSubservices()
{
	ePtr<eServiceEvent> evt;
	if (!m_event_handler.getEvent(evt, 0))
		return evt->getNumOfLinkageServices();
	return 0;
}

RESULT eDVBServiceRecord::getSubservice(eServiceReference &sub, unsigned int n)
{
	ePtr<eServiceEvent> evt;
	if (!m_event_handler.getEvent(evt, 0))
	{
		if (!evt->getLinkageService(sub, m_ref, n))
			return 0;
	}
	sub.type=eServiceReference::idInvalid;
	return -1;
}
