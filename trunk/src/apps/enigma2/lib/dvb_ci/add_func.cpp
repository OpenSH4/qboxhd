//#define QBOXHD

#ifdef QBOXHD
#include <stdlib.h>
#include <fcntl.h>
#include <linux/ioctl.h>
#include <asm/ioctl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "starci2win.h"
#include "add_func.h"

//#define PPDEBUG

#ifdef PPDEBUG
    #define ppDebug(...) eDebug(__VA_ARGS__)
#else
    #define ppDebug(...)
#endif

#define MAP_SIZE 	8192UL //4096UL
#define MAP_MASK (MAP_SIZE - 1)

CI_Info_t ci_info[2];
extern int first_tuner_found;

static unsigned char bb_phys_pos[3];
static unsigned char bb_streams[2] = {0xFF, 0xFF};          //0: tun0   1: tun1     2: tun2     0xFF: none
static unsigned char bb_cams[2] = {0, 0};                   //0: none   1: cam A    2: cam B    3: cam A & B
static bb_channels bb_last_channels;

/*
static unsigned char BB_Translate_tuner_num(bb_channels *channels, unsigned char channel_idx)
{
    unsigned char phys_num;

    if (channels->ch_phys[channel_idx])   return channels->ch_tuner[channel_idx];
    else
    {
        //TO DO physical translation
        return phys_num;
    }
}
*/

static void BB_Map_tuner_physnum(bb_channels *channels)
{
    unsigned char i;

    bb_phys_pos[0] = bb_phys_pos[1] = bb_phys_pos[2] = 0;

    for(i=0;i<channels->channels_quant;i++)                                           //Scan all current channels
    {
        bb_phys_pos[channels->ch_tuner[i]] = channels->ch_phys[i];
    }
}

static unsigned char BB_GetPhysNum(unsigned char tuner)
{
    return bb_phys_pos[tuner];
}

#if 0
static unsigned char BB_Set_Fpga(unsigned char out0, unsigned char out1, unsigned char out2)
{
    char string[30] = {"testreg 4 "};
    char little[2] = {" "};
    unsigned char flag = 0;

    if (out2)
    {
        flag = 1;
        little[0] = '0'+ out2;
        strcat(string, little);
    }
    if (out1 || flag)
    {
        little[0] = '0'+ out1;
        strcat(string, little);
    }

    little[0] = '0'+ out0;
    strcat(string, little);

    #ifndef PPDEBUG
        strcat(string, " > null");
    #endif

    system(string);

    ppDebug("BB_Set_Fpga:  fpga cmd string:  %s\n", string);

    return 1;
}
#else
static unsigned char BB_Set_Fpga(unsigned char out0, unsigned char out1, unsigned char out2)
{
    unsigned char flag = 0;
	int fd_fpga;
	void *map_base_fpga, *virt_addr_fpga;
	off_t target_fpga;
	static volatile unsigned short * reg_fpga = 0;
	unsigned short value=0;

	if (reg_fpga == 0)
	{
		target_fpga = (off_t)0x3800000;	//fpga_addr

		if ( ( fd_fpga = open( "/dev/mem", O_RDWR | O_SYNC ) ) == -1 ) return 0;
            // Map one page //
		map_base_fpga = mmap( 0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,fd_fpga, target_fpga & ~MAP_MASK );
		if ( map_base_fpga == ( void * ) -1 ) return 0;
		virt_addr_fpga = (void *)(((unsigned int)map_base_fpga) + ( target_fpga & MAP_MASK ));
		reg_fpga = (volatile unsigned short*)virt_addr_fpga;
	}

    if (out2)
    {
        flag = 1;
		value|=(out2&0x0F);
    }
	value=(value<<4);
    if (out1 || flag)
    {
		value|=(out1&0x0F);
    }
	value=(value<<4);
	value|=(out0&0x0F);

	reg_fpga[4]=(unsigned short)value;	

    ppDebug("BB_Set_Fpga:  fpga value: 0%04X\n", value);

    return 1;
}
#endif

static unsigned char BB_Set_3rd(unsigned char source)
{
	int fd;
	void *map_base, *virt_addr;
	off_t target;
	static volatile unsigned int * reg = 0;

        if (reg == 0)
        {
            target = (off_t)0x19001100;                                                     //SYS_CFG0

            if ( ( fd = open( "/dev/mem", O_RDWR | O_SYNC ) ) == -1 ) return 0;
            // Map one page //
            map_base = mmap( 0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,fd, target & ~MAP_MASK );
            if ( map_base == ( void * ) -1 ) return 0;
            virt_addr = (void *)(((unsigned int)map_base) + ( target & MAP_MASK ));
            reg = (volatile unsigned int*)virt_addr;
        }
        
        if      (source == 0)   reg[0] = 0x2;
        else if (source == 1)   reg[0] = 0x6;
        else if (source == 2)   reg[0] = 0x0;
        else return 0;

        ppDebug("BB_Set_3rd:  Reg 0x19001100:  %02X\n", reg[0]);

	return 1;
}

unsigned char BlackBoxGetLastChannels(bb_channels *channels)
{
    memcpy((unsigned char*)channels, (unsigned char*)&bb_last_channels, sizeof(bb_last_channels));

    return 1;
}

#define TSM_STREAM_ON       (1 << 7)
#define TSM_RAM_OVERFLOW    (1 << 2)
#define TSM_CHANNEL_RESET   (1 << 0)

class BB_Timer : public Object
{
public:
    ePtr<eTimer> tsmchecker_timer;
    volatile unsigned int * base_reg;

    unsigned int TSM_STREAM_CONF(unsigned char n)                   {return *((unsigned int*)&(((unsigned char*)base_reg)[0x20*n+0x00]));}
    unsigned int TSM_STREAM_STATUS(unsigned char n)                 {return *((unsigned int*)&(((unsigned char*)base_reg)[0x20*n+0x10]));}
    unsigned int TSM_STREAM_CONF2(unsigned char n)                  {return *((unsigned int*)&(((unsigned char*)base_reg)[0x20*n+0x18]));}
    void TSM_STREAM_CONF_write(unsigned char n, unsigned int val)   {*((unsigned int*)&(((unsigned char*)base_reg)[0x20*n+0x00])) = val;}
    void TSM_STREAM_STATUS_write(unsigned char n, unsigned int val) {*((unsigned int*)&(((unsigned char*)base_reg)[0x20*n+0x10])) = val;}
    void TSM_STREAM_CONF2_write(unsigned char n, unsigned int val)  {*((unsigned int*)&(((unsigned char*)base_reg)[0x20*n+0x18])) = val;}
    void TSM_STREAM_RST_write(unsigned int val)                     {*((unsigned int*)&(((unsigned char*)base_reg)[0x830])) = val;}

    void BB_tsm_reset_channel (unsigned int n)
    {
        unsigned int status;

        //ppDebug("BB_tsm_reset_channel: %d start\n", n);
        status = TSM_STREAM_STATUS(n);

        TSM_STREAM_CONF_write(n, TSM_STREAM_CONF(n) & ~TSM_STREAM_ON);
        usleep(500);
        TSM_STREAM_STATUS_write(n, status & ~TSM_RAM_OVERFLOW);
        usleep(500);
        TSM_STREAM_CONF2_write(n, TSM_CHANNEL_RESET);
        usleep(500);
        TSM_STREAM_CONF2_write(n, 0);
        usleep(500);
        TSM_STREAM_CONF_write(n, TSM_STREAM_CONF(n) | TSM_STREAM_ON);
        //ppDebug("BB_tsm_reset_channel: %d end\n", n);

        usleep(10000);                                                          //Put to avoid possible player2 deadlock during zaping

//        TSM_STREAM_RST_write(0);
//        TSM_STREAM_RST_write(6);
    }

    void BB_TsmChecker()
    {
        unsigned char input_num;

        //ppDebug("BlackBoxTsmChecker: start\n");
        tsmchecker_timer->stop();

        for(input_num=0;input_num<3;input_num++)
        {
            if(TSM_STREAM_STATUS(input_num) & TSM_RAM_OVERFLOW)
            {
                BB_tsm_reset_channel(input_num);
                ppDebug("--------------------->>>>>>>>>>>>>>>>>>>>> BlackBoxTsmChecker: Ram overflow detected on TSMerger channel %d\n", input_num);
//eDebug("--------------------->>>>>>>>>>>>>>>>>>>>> BlackBoxTsmChecker: Ram overflow detected on TSMerger channel %d\n", input_num);
            }
        }

#ifdef QBOXHD_MINI
        tsmchecker_timer->start(150);
#else
         tsmchecker_timer->start(100);
#endif
        //ppDebug("BlackBoxTsmChecker: end\n");
    }

    BB_Timer(void)
    {
	int fd;
	void *map_base, *virt_addr;
	off_t target;

	target = (off_t)0x19242000;                                                     //TSMergerBaseAddress

	if ( ( fd = open( "/dev/mem", O_RDWR | O_SYNC ) ) == -1 ) base_reg = 0;
	// Map one page //
	map_base = mmap( 0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,fd, target & ~MAP_MASK );
	if ( map_base == ( void * ) -1 ) base_reg = 0;
	virt_addr = (void *)(((unsigned int)map_base) + ( target & MAP_MASK ));
	base_reg = (volatile unsigned int*)virt_addr;

        if (base_reg)
        {
            tsmchecker_timer = eTimer::create(eApp);
            CONNECT(tsmchecker_timer->timeout, BB_Timer::BB_TsmChecker);
#ifdef QBOXHD_MINI
            tsmchecker_timer->start(150);
#else
            tsmchecker_timer->start(100);
#endif

        }
    }

};



unsigned char BlackBoxSwitch(bb_channels *channels)
{
    unsigned int i;
    bb_channels *ch = channels;
    Ci_Mode_BackBox starci_params;
    unsigned char streams[2] = {0xFF, 0xFF};
    unsigned char cams[2] = {0, 0};
    unsigned char third_flag = 0, third_cam = 0, tsin_1 = 0, tsin_2 = 0;
    unsigned char camB_flag = 0, camB_val=0;
    static BB_Timer *bb_timer = 0;
    static unsigned char fpgacsa_bypass = 0;

    if(bb_timer == 0)
    {
        bb_timer = new BB_Timer;                                                //Prepare timer for checking TSMerger inputs
    }

    if (!fpgacsa_bypass)
    {
        #ifdef PPDEBUG
            system("testreg 43 1");
        #else
            system("testreg 43 1 > null");
        #endif
        
        fpgacsa_bypass = 1;
    }

    bb_last_channels = *ch;

    if (ch->channels_quant == 0)    return 0;

    for(i=0;i<ch->channels_quant;i++)                                           //Scan all current channels
    {
        ppDebug("BlackBoxSwitch: ch_tuner %d   ch_phys %d   ch_cam %d\n", ch->ch_tuner[i], ch->ch_phys[i], ch->ch_cam[i]);
    }


    BB_Map_tuner_physnum(ch);

  //Streams configuration building
    ch->channels_quant = ch->channels_quant>2 ? 2 : ch->channels_quant;         //channels_quant must be max 2

    for(i=0;i<ch->channels_quant;i++)                                           //Scan all current channels
    {
        if (ch->ch_tuner[i] > 2)    return 0;                                       //Illegal tuner number!!

        if (ch->ch_tuner[i] == 2)                                                   //Case of channel from 3rd tuner
        {
            if (third_flag == 0)                                                        //If first time for 3rd tuner
            {
                third_flag = 1;
                third_cam = ch->ch_cam[i];
            }
            else if (ch->ch_cam[i])                                                     //If NOT first time for 3rd tuner
            {
                third_cam |= ch->ch_cam[i];                                                 //Case 2 "cam scrambled" channels on 3rd tuner
            }
        }
        else
        {
            if (streams[ch->ch_tuner[i]] == 0xFF)
            {
                streams[ch->ch_tuner[i]] = ch->ch_tuner[i];
                cams[ch->ch_tuner[i]] = ch->ch_cam[i];
            }
            else                                                                        //Case of 2 channels on same tuner
            {
                if (!cams[ch->ch_tuner[i]]) cams[ch->ch_tuner[i]] = ch->ch_cam[i];
                else                        cams[ch->ch_tuner[i]] = 3;                      //Case 2 "cam scrambled" channels on same tuner
            }
        }
    }

    if (third_flag)                                                              //If at least one channel from 3rd tuner
    {
        if (streams[0]==0xFF)        i = 0;
        else if (streams[1]==0xFF)   i = 1;
        else return 0;                                                              //Illegal channels

        streams[i] = 2;
        cams[i] = third_cam;
    }

        //ppDebug("----------------------------------\n");
        //ppDebug("BlackBoxSwitch: bb_streams[0] %d  bb_cams[0] %d    bb_streams[1] %d  bb_cams[1] %d\n", bb_streams[0], bb_cams[0], bb_streams[1], bb_cams[1]);
        //ppDebug("----------------------------------\n");
        //ppDebug("BlackBoxSwitch:    streams[0] %d     cams[0] %d       streams[1] %d     cams[1] %d\n", streams[0], cams[0], streams[1], cams[1]);
        //ppDebug("----------------------------------\n");

  //Streams configuration setting
    memset((unsigned char *)&starci_params, 0, sizeof(starci_params));
    if (streams[0]==0xFF || streams[1]==0xFF)                                   //Single Mode
    {
        starci_params.mode = SINGLE_M;

        i = (streams[0]==0xFF) ? 1 : 0;
        if (cams[i])                                                                //If at least 1 cam required
        {
            if (cams[i] == 3)                                                           //Case of 2 "cam scrambled" channels from 1 tuner
            {
                starci_params.cam_1 = CAM_A;
                starci_params.cam_2 = CAM_B;
            }
            else starci_params.cam_1 = (cams[i]==1) ? CAM_A : CAM_B;                    //Cam A or Cam B
        }
        
        if (streams[i]!=2 || streams[i]==bb_streams[i])                             //If tuner 0 or tuner 1   or    tuner 2 in same position as before
        {
            if (i==0)   {tsin_1 = (streams[i]!=2) ? TS1 : TS3; starci_params.tsin_1 = TS1;}
            else        {tsin_2 = (streams[i]!=2) ? TS2 : TS3; starci_params.tsin_2 = TS2;}
            
            BB_Set_Fpga(BB_GetPhysNum((tsin_1==TS1)?0:2), BB_GetPhysNum((tsin_2==TS2)?1:2), 0);   //Tuner 0 & 2  or  2 & 1 from FPGA
            ioctl ( ci_info[0].fd_ci, IOCTL_CI_MODE_FROM_BLACKBOX, &starci_params );        //Set StarCi parameters
            if (streams[i]==2) BB_Set_3rd(i);                                               //ST 3rd input from StarCi (from streaming routed from tuner 2)
            else               BB_Set_3rd(2);                                               //ST 3rd input from PAD 2
        }
        else                                                                        //If tuner 2
        {
          //Tuner 2 must be moved from one stream to another.
          //To avoid data loss, entire operation must be splitted in more steps.
          //First of all fpga must route tunner 2 to both outputs, then StarCi must be set to single mode (both output streams become equal).
          //Afterwards it is possible to route tuner 2 to right fpga output and change ST tsmerger input 3 source.
            BB_Set_Fpga(BB_GetPhysNum(2), BB_GetPhysNum(2), 0);                                 //Tuner 2 on both FPGA outputs

            if (i==0)   {tsin_1 = TS3; starci_params.tsin_1 = TS1;}
            else        {tsin_2 = TS3; starci_params.tsin_2 = TS2;}

            ioctl ( ci_info[0].fd_ci, IOCTL_CI_MODE_FROM_BLACKBOX, &starci_params );            //Set StarCi parameters
            BB_Set_3rd(i);                                                                      //ST 3rd input from StarCi (from streaming routed from tuner 2)
            BB_Set_Fpga(BB_GetPhysNum((tsin_1==TS1)?0:2), BB_GetPhysNum((tsin_2==TS2)?1:2), 0);   //Tuner 0 & 2  or  2 & 1 from FPGA
        }
    }
    else                                                                        //Twin mode
    {
        starci_params.mode = TWIN_M;

      //First leave last cam settings to avoid possible wrong temporary cam chaining mode selection, in case of change from single to twin mode
        if (bb_cams[0])                                                             //Leave same cam 1 setting as before
        {
            if (bb_cams[0] == 3)                                                        //Case of 2 "cam scrambled" channels from 1 tuner
            {
                starci_params.cam_1 = CAM_AB;
            }
            else starci_params.cam_1 = (bb_cams[0]==1) ? CAM_A : CAM_B;                 //Cam A or Cam B
        }
        if (bb_cams[1])                                                             //Leave same cam 2 setting as before
        {
            if (bb_cams[1] == 3)                                                        //Case of 2 "cam scrambled" channels from 1 tuner
            {
                starci_params.cam_2 = CAM_AB;
            }
            else starci_params.cam_2 = (bb_cams[1]==1) ? CAM_A : CAM_B;                 //Cam A or Cam B
        }

        if ( !((streams[0]==2 && bb_streams[1]==2) || (streams[1]==2 && bb_streams[0]==2)) ) //If Tuner 2 has not swaped
        {
            tsin_1 = (streams[0]==0) ? TS1 : TS3; starci_params.tsin_1 = TS1;
            tsin_2 = (streams[1]==1) ? TS2 : TS3; starci_params.tsin_2 = TS2;

            BB_Set_Fpga(BB_GetPhysNum((tsin_1==TS1)?0:2), BB_GetPhysNum((tsin_2==TS2)?1:2), 0);   //Tuner 0 & 2  or  2 & 1 from FPGA

            ioctl ( ci_info[0].fd_ci, IOCTL_CI_MODE_FROM_BLACKBOX, &starci_params );    //Set StarCi parameters with same cam settings as before

            starci_params.cam_1 = starci_params.cam_2 = NONE_S;
            if (cams[0])    starci_params.cam_1 = (cams[0]==1) ? CAM_A : ((cams[0]==2) ? CAM_B : CAM_AB);
            if (cams[1])    starci_params.cam_2 = (cams[1]==1) ? CAM_A : ((cams[1]==2) ? CAM_B : CAM_AB);
            ioctl ( ci_info[0].fd_ci, IOCTL_CI_MODE_FROM_BLACKBOX, &starci_params );    //Set StarCi parameters with new cam settings

            if      (streams[0]==2) BB_Set_3rd(0);                                    //ST 3rd input from StarCi output 1
            else if (streams[1]==2) BB_Set_3rd(1);                                    //ST 3rd input from StarCi output 2
            else                    BB_Set_3rd(2);                                    //ST 3rd input from PAD 2 (3rd PAD)
        }
        else                                                                        //If Tuner 2 required and swaped
        {
            BB_Set_Fpga(BB_GetPhysNum(2), BB_GetPhysNum(2), 0);                         //Tuner 2 on both FPGA outputs

            if (bb_streams[0]==2)   starci_params.cam_2 = NONE_S;                       //Disable Cam on stream routed from other tuner than Tuner 2
            else                    starci_params.cam_1 = NONE_S;                       //Disable Cam on stream routed from other tuner than Tuner 2

            if (streams[0]==2)  {tsin_1 = TS3; starci_params.tsin_1 = TS1;}
            else                {tsin_2 = TS3; starci_params.tsin_2 = TS2;}

            starci_params.mode = SINGLE_M;
            ioctl ( ci_info[0].fd_ci, IOCTL_CI_MODE_FROM_BLACKBOX, &starci_params );    //Set StarCi in Single mode to double output stream

            tsin_1 = (streams[0]==0) ? TS1 : TS3; starci_params.tsin_1 = TS1;
            tsin_2 = (streams[1]==1) ? TS2 : TS3; starci_params.tsin_2 = TS2;

            starci_params.cam_1 = starci_params.cam_2 = NONE_S;
            if (streams[0]==2)
            {
                if (cams[0])    starci_params.cam_1 = (cams[0]==1) ? CAM_A : ((cams[0]==2) ? CAM_B : CAM_AB);
                if (starci_params.cam_1 == CAM_B)
                {
                    camB_flag = 1;                                                                  //Postpone call to BB_Set_3rd to try to avoid data loss
                    camB_val = 0;                                                                   //due to Cam B limit (no doubled output) in StarCI
                }
                else BB_Set_3rd(0);                                                              //ST 3rd input from StarCi output 1
            }
            else if (streams[1]==2)
            {
                if (cams[1])    starci_params.cam_2 = (cams[1]==1) ? CAM_A : ((cams[1]==2) ? CAM_B : CAM_AB);
                if (starci_params.cam_2 == CAM_B)
                {
                    camB_flag = 1;                                                                  //Postpone call to BB_Set_3rd to try to avoid data loss
                    camB_val = 1;                                                                   //due to Cam B limit (no doubled output) in StarCI
                }
                else BB_Set_3rd(1);                                                              //ST 3rd input from StarCi output 2
            }

            starci_params.mode = TWIN_M;
            ioctl ( ci_info[0].fd_ci, IOCTL_CI_MODE_FROM_BLACKBOX, &starci_params );    //Set StarCi in Twin mode without cam for stream from other tuner than Tuner 2

            if (camB_flag)  BB_Set_3rd(camB_val);

            if (cams[0])    starci_params.cam_1 = (cams[0]==1) ? CAM_A : ((cams[0]==2) ? CAM_B : CAM_AB);
            if (cams[1])    starci_params.cam_2 = (cams[1]==1) ? CAM_A : ((cams[1]==2) ? CAM_B : CAM_AB);

            BB_Set_Fpga(BB_GetPhysNum((tsin_1==TS1)?0:2), BB_GetPhysNum((tsin_2==TS2)?1:2), 0);   //Tuner 0 & 2  or  2 & 1 from FPGA

            ioctl ( ci_info[0].fd_ci, IOCTL_CI_MODE_FROM_BLACKBOX, &starci_params );    //Set StarCi with definitive parameters
        }
    }

    memcpy(bb_streams, streams, sizeof(streams));                               //Save current streams state
    memcpy(bb_cams, cams, sizeof(cams));                                        //Save current cams state

    return 1;
}

#ifdef QBOXHD_MINI
void start_check_TSM(void)
{
	static BB_Timer *bb_timer = 0;
	if(bb_timer == 0)
	{
		eDebug("\nCreate the check timer\n");
		bb_timer = new BB_Timer;	//Prepare timer for checking TSMerger inputs
	}
}
#endif

void set_tuner_to_cam ( int tuner_no, data_source source )
{
	Module_and_ts_t mod;
    int ts_in = TS_0;

    if (tuner_no == first_tuner_found)
        ts_in = TS_0;
    else if (tuner_no == (first_tuner_found + 1))
        ts_in = TS_1;
    else
        eDebug("TS input not supported by Starci2win");

	eDebug("Called '%s' with %d %d\n", __func__, tuner_no, source);
	if( (ts_in==TS_0) && (source==CI_A) )
	{
		mod.ts_number=TS_0;
		mod.module=MODULE_A;
		mod.configuration=TS_TO_CAM;
		ioctl(ci_info[0].fd_ci,IOCTL_CI_SET_TS_MODE,&mod);
	}
	else if( (ts_in==TS_0) && (source==CI_B) )
	{
		mod.ts_number=TS_0;
		mod.module=MODULE_B;
		mod.configuration=TS_TO_CAM;
		ioctl(ci_info[1].fd_ci,IOCTL_CI_SET_TS_MODE,&mod);
	}
	else if( (ts_in==TS_1) && (source==CI_A) )
	{
		mod.ts_number=TS_1;
		mod.module=MODULE_A;
		mod.configuration=TS_TO_CAM;
		ioctl(ci_info[0].fd_ci,IOCTL_CI_SET_TS_MODE,&mod);
	}
	else if( (ts_in==TS_1) && (source==CI_B) )
	{
		mod.ts_number=TS_1;
		mod.module=MODULE_B;
		mod.configuration=TS_TO_CAM;
		ioctl(ci_info[1].fd_ci,IOCTL_CI_SET_TS_MODE,&mod);
	}
	else if( (ts_in==TS_0) && (source==TUNER_A) )
	{
		mod.module=MODULE_A;
		ioctl(ci_info[0].fd_ci,IOCTL_CI_GET_TS_MODE,&mod);
		if(mod.ts_number==TS_0)	/* TS0->ciA */
		{
			mod.module=MODULE_A;
			mod.configuration=TS_BY_PASS;
			mod.ts_number=TS_0;
			ioctl ( ci_info[0].fd_ci,IOCTL_CI_SET_TS_MODE,&mod );
		}
		else if(mod.ts_number==TS_1)	/*TS0->ciB */
		{
			mod.module=MODULE_B;
			mod.configuration=TS_BY_PASS;
			mod.ts_number=TS_0;
			ioctl ( ci_info[1].fd_ci,IOCTL_CI_SET_TS_MODE,&mod );
		}
        else
		  eDebug("Called '%s' with %d %d but it doesn't work[ts_number:%d]\n", 
            __func__,tuner_no, source, mod.ts_number);

	}
	else if( (ts_in==TS_1) && (source==TUNER_B) )
	{
		mod.module=MODULE_A;
		ioctl(ci_info[0].fd_ci,IOCTL_CI_GET_TS_MODE,&mod);
		if(mod.ts_number==TS_1)	/* TS1->ciA */
		{
			mod.module=MODULE_A;
			mod.configuration=TS_BY_PASS;
			mod.ts_number=TS_1;
			ioctl ( ci_info[0].fd_ci,IOCTL_CI_SET_TS_MODE,&mod );
		}
		else if(mod.ts_number==TS_0)	/*TS1->ciB */
		{
			mod.module=MODULE_B;
			mod.configuration=TS_BY_PASS;
			mod.ts_number=TS_1;
			ioctl ( ci_info[1].fd_ci,IOCTL_CI_SET_TS_MODE,&mod );
		}
	}
}

void switch_tuner_to_ts(Enum_cam_tuner p)
{
	eDebug("Called '%s' with %d\n",__FUNCTION__,p);
	switch(p) /* Disable by-pass */
	{
		case CAM_BYPASS_ON: { /* Enable by-pass: tuner0 with TS0 and tuner1 with TS1 */
            set_tuner_to_cam (first_tuner_found, TUNER_A);
            set_tuner_to_cam (first_tuner_found + 1, TUNER_B);
			break;
        }
		case CAM_BYPASS_OFF: /*TODO Disable by-pass: tuner0 with CamA/B and tuner1 with CamA/B TODO*/
		default:
			eDebug("Not implemented\n");
	}

}


unsigned char IsInserted_modA ( void )
{
	Module_t mod;
	mod.module=MODULE_A;
	mod.configuration=0xFF;
	ioctl ( ci_info[0].fd_ci,IOCTL_DETECT_MODULE,&mod );
	if ( mod.configuration==MODULE_PRESENT )
		return 1;
	else
		return 0;
}

unsigned char IsInserted ( unsigned char id )
{
	Module_t mod;
	mod.configuration=0xFF;

	if ( id==MODULE_A )
		mod.module=MODULE_A;
	else if ( id==MODULE_B )
		mod.module=MODULE_B;
	else
	{
		printf ( "Wrong module\n" );
		return 0;
	}

	ioctl ( ci_info[id].fd_ci,IOCTL_DETECT_MODULE,&mod );
	if ( mod.configuration==MODULE_PRESENT )
		return 1;
	else
		return 0;
}

#endif // QBOXHD
