#ifdef QBOXHD

#ifndef __add_func_h
#define __add_func_h
#include "dvbci.h"


typedef struct
{
//	unsigned short NgBuf;
//	unsigned char linkID;
//	unsigned char SessionProfile;
//	unsigned char InitCom;
	int fd_ci;
}CI_Info_t;

typedef enum
{
	CAM_BYPASS_ON=0,
	CAM_BYPASS_OFF
}Enum_cam_tuner;


typedef struct
{
    unsigned char channels_quant;
    unsigned char ch_tuner[3];
    unsigned char ch_cam[3];                //0: none   1: cam A    2: cam B    3: cam A + B
    unsigned char ch_phys[3];
}bb_channels;

unsigned char BlackBoxGetLastChannels(bb_channels *channels);
unsigned char BlackBoxSwitch(bb_channels *channels);


/* Switching TS */
void set_tuner_to_cam(int tuner_no, data_source source);
unsigned char IsInserted(unsigned char id);
void switch_tuner_to_ts(Enum_cam_tuner p);

#endif //__add_func_h

#endif // QBOXHD




