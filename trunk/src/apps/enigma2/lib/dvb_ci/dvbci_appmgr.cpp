/* DVB CI Application Manager */

#include <lib/base/eerror.h>
#include <lib/dvb_ci/dvbci_appmgr.h>
#include <lib/dvb_ci/dvbci_ui.h>

#ifdef QBOXHD
#include "add_func.h"

extern int first_tuner_found;

#if 0
typedef struct
{
	int nim;
	unsigned char type[16];
}Tuner_info_t;

//Duolabs
static int where_dvt(void)
{
	FILE * file;
	file=fopen("/proc/bus/nim_socket","r");
	if(file<0)
	{
		eDebug("Error when open /proc/bus/nim_socket\n");
		return (-1);
	}
	
	int byte=0,len=0;
	unsigned char str_file[64];
	Tuner_info_t info;
	byte=fgetc(file);
	do{
		str_file[len]=byte;
		len++;
		byte=fgetc(file);
	}while (byte!=':');	/* Read Nim socket */
	//TODO
	sscanf(str_file,"%d",&info.nim);
	len=0;
	byte=fgetc(file);
	do{
		str_file[len]=byte;
		len++;
		byte=fgetc(file);
	}while (byte!=':'); /* Read Type */
	len=0;
	byte=fgetc(file);
	do{
		str_file[len]=byte;
		len++;
		byte=fgetc(file);
	}while (byte!=':'); /* Read the value of type */
	if(strstr(str_file,"DVB-T")!=NULL) /* Find DVB-T position!! */
		return info.nim;
}
#endif
#endif // QBOXHD

eDVBCIApplicationManagerSession::eDVBCIApplicationManagerSession(eDVBCISlot *tslot)
{
	slot = tslot;
	slot->setAppManager(this);
}

eDVBCIApplicationManagerSession::~eDVBCIApplicationManagerSession()
{
	slot->setAppManager(NULL);
}

int eDVBCIApplicationManagerSession::receivedAPDU(const unsigned char *tag,const void *data, int len)
{
	eDebugNoNewLine("SESSION(%d)/APP %02x %02x %02x: ", session_nb, tag[0], tag[1], tag[2]);
	for (int i=0; i<len; i++)
		eDebugNoNewLine("%02x ", ((const unsigned char*)data)[i]);
	eDebug("");

	if ((tag[0]==0x9f) && (tag[1]==0x80))
	{
		switch (tag[2])
		{
		case 0x21:
		{
			int dl;
			eDebug("application info:");
			eDebug("  len: %d", len);
			eDebug("  application_type: %d", ((unsigned char*)data)[0]);
			eDebug("  application_manufacturer: %02x %02x", ((unsigned char*)data)[2], ((unsigned char*)data)[1]);
			eDebug("  manufacturer_code: %02x %02x", ((unsigned char*)data)[4],((unsigned char*)data)[3]);
			eDebugNoNewLine("  menu string: ");
			dl=((unsigned char*)data)[5];
			if ((dl + 6) > len)
			{
				eDebug("warning, invalid length (%d vs %d)", dl+6, len);
				dl=len-6;
			}
			char str[dl + 1];
			memcpy(str, ((char*)data) + 6, dl);
			str[dl] = '\0';
			for (int i = 0; i < dl; ++i)
				eDebugNoNewLine("%c", ((unsigned char*)data)[i+6]);
			eDebug("");
#ifdef QBOXHD
            // Workaround for some terrestrial CAMs
			if(strcmp(str,"SmarCAM Italia")==0)
			{
                int     cnt,
                        dvbt_nim;
                char    *ptr = NULL,
                        line[32];
                FILE    *nim_fd;
                unsigned char adapter=0;

                if ((nim_fd = fopen("/proc/bus/nim_sockets", "r")) == NULL)
                    eDebug("%s(): open(): %m\n", __func__);

                while (fgets(line, sizeof(char) * 31, nim_fd)) {
//                     printf("-----> line: '%s'\n", line);
                    ptr = strstr(line, ":");
                    if (!ptr)
                        continue;
                    
                    dvbt_nim = *(ptr - 1) - 48;
//                     printf("-----> dvbt nim: '%d'\n", dvbt_nim);
                    if (dvbt_nim < 0 || dvbt_nim > 2) {
                        ptr = NULL;
                        continue;
                    }
    
                    if (fgets(line, sizeof(char) * 31, nim_fd) == NULL) {
                        eDebug("%s(): read(): %m\n", __func__);
                        ptr = NULL;
                        continue;
                    }
    
//                     printf("-----> 2 line: '%s'\n", line);
                    ptr = strstr(line, "DVB-T");
                    if (ptr)
                        break;

                    adapter++;
                }

                fclose(nim_fd);


                bb_channels channels, *ch;
                unsigned char i;

                BlackBoxGetLastChannels(&channels);
                ch = &channels;

                for(i=0;i<ch->channels_quant;i++)
                {
                    if (ch->ch_tuner[i]==adapter && ch->ch_phys[i]==dvbt_nim) break;
                }

                if (i<2)
                {
                    ch->ch_tuner[i] = adapter;
                    ch->ch_phys[i] = dvbt_nim;
                    ch->ch_cam[i] = (ch->ch_cam[i]<3 && ch->ch_cam[i]!=(slot->getSlotID()+1)) ? ch->ch_cam[i]+slot->getSlotID()+1 : ch->ch_cam[i];
                    i++;
                    ch->channels_quant = (i > ch->channels_quant) ? i : ch->channels_quant;
                    //eDebug("================================================================== BlackBoxSwitch Start");
                    BlackBoxSwitch(ch);
                    //eDebug("================================================================== BlackBoxSwitch End");
                }
                else
                {
                    eDebug("DVB-T stream failed! No stream paths available for frontend %d ", dvbt_nim);
                    return -1;
                }
                //printf("_______________________________----------------------__________________-----------------> dvbt nim: '%d'  adapter: %d\n", dvbt_nim, adapter);

/*
                if (ptr) {
                    if (slot->getSlotID() == 0)
                        set_tuner_to_cam(dvbt_nim, CI_A);
                    else if (slot->getSlotID() == 1)
                        set_tuner_to_cam(dvbt_nim, CI_B);
                }
*/
			}
#endif	
			eDVBCI_UI::getInstance()->setAppName(slot->getSlotID(), str);

			eDVBCI_UI::getInstance()->setState(slot->getSlotID(), 2);
			break;
		}
		default:
			eDebug("unknown APDU tag 9F 80 %02x", tag[2]);
			break;
		}
	}
	return 0;
}

int eDVBCIApplicationManagerSession::doAction()
{
  switch (state)
  {
  case stateStarted:
  {
    const unsigned char tag[3]={0x9F, 0x80, 0x20}; // application manager info e    sendAPDU(tag);
		sendAPDU(tag);
    state=stateFinal;
    return 1;
  }
  case stateFinal:
    eDebug("in final state.");
		wantmenu = 0;
    if (wantmenu)
    {
      eDebug("wantmenu: sending Tenter_menu");
      const unsigned char tag[3]={0x9F, 0x80, 0x22};  // Tenter_menu
      sendAPDU(tag);
      wantmenu=0;
      return 0;
    } else
      return 0;
  default:
    return 0;
  }
}

int eDVBCIApplicationManagerSession::startMMI()
{
	eDebug("in appmanager -> startmmi()");
	const unsigned char tag[3]={0x9F, 0x80, 0x22};  // Tenter_menu
	sendAPDU(tag);
	return 0;
}

