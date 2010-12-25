#ifdef QBOXHD

#ifndef _EMMA_CIDEF_H
#define _EMMA_CIDEF_H

#define	UI8		unsigned char
#define	UI16	unsigned short
#define	UI32	unsigned int

#define	I8		char
#define	I16		short
#define	I32		int

#define STARCI2WIN



#define     KO          0x07
#define     SO0         0x01
#define     SO1         0x02
#define     SO2         0x04
#define     SO3         0x08
#define     SO4         0x10
#define     SO5         0x20
#define     SO6         0x40
#define     SO7         0x80

#define     NOTUSE      0x00
#define     ATTRIBUTE   0x01
#define     IOACCESS    0x02

#define     SLOTA_PWR   0x01
#define     SLOTB_PWR   0x10
#define     SLOTA_RES   0x02
#define     SLOTB_RES   0x40
#define     BYPASS_ALL  0x28
#define     SLOTB_LINK  0x24
#define     SLOTA_LINK  0x88
#define     ALL_LINK    0x84
#define     AOE_A       0x10
#define     AOE_B       0x20
#define     DOE_A       0x40
#define     DOE_B       0x80
#define     EN_X        0x01
#define     IO_X        0x02    /*  0:mem,1:IO */
#define     REG_X       0x04    /*  0:attribute,1:common */
#define     CARDA_CK    0x08

#define     EN_CARDB    0x01
#define     CARDB_OP    0x02    /*  0:mem,1:IO */
#define     CARDB_ACCESS        0x04    /*  0:attribute,1:common */
#define     CARDB_CK    0x08

/* CIS management */
/* tuples */
#define CISTPL_DEVICE                   0x01
#define CISTPL_DEVICE_A                 0x17
#define CISTPL_DEVICE_0A                0x1d
#define CISTPL_DEVICE_0C                0x1c
#define CISTPL_VERS_1                   0x15
#define CISTPL_MANFID                   0x20
#define CISTPL_CONFIG                   0x1a
#define CISTPL_CFTABLE_ENTRY    0x1b
#define CISTPL_NOLINK                   0x14
#define CISTPL_END                              0xff

/* subtuples */
#define CCST_CIF                                0xc0
#define STCE_EV                                 0xc0
#define STCE_PD                                 0xc1

/* offset in tuple parameters (without the 2 first bytes tuple and link) */
/* CISTPL_VERS_1 */
#define TPLLV1_MAJOR            0
#define TPLLV1_MINOR            1

/* CISTPL_CONFIG */
#define TPCC_SZ                         0
#define TPCC_LAST                       1
#define TPCC_RADR                       2

/* CISTPL_CFTABLE_ENTRY */
#define TPCE_INDX                       0
#define TPCE_IF                         1       /* If field present ! */
#define TPCE_FS                         2       /* If TPCE_IF field present, else 1 */
#define TPCE_PD                         3       /* If TPCE_IF field present, else 2 */

/* bit mask */
/* CISTPL_CONFIG */
#define TPCC_RASZ                       0x03
#define TPCC_RMSZ                       0x3c

/* CISTPL_CFTABLE_ENTRY */
#define TPCE_INDX_MASK          0xc0
#define TPCE_INTFACE            0x80
#define TPCE_IF_DVB_VAL         0x04                    /* Imposed in CENELEC Errata document. */
#define TPCE_FS_DVB_MASK        0x1b                    /* Previously TPCE_FS_MASK set to 0x0b wrong */
#define TPCE_FS_IO_IRQ_MASK     0x18
#define TPCE_FS_PD_MASK         0x03                    /* Previously TPCE_PD_MASK set to 0x01 wrong */
#define TPCE_EXT                        0x80
#define TPCE_TIMING                     0x04
#define TPCE_TIMING_WAIT        0x03
#define TPCE_TIMING_READY       0x1c
#define TPCE_TIMING_RESERV      0xe0
#define TPCE_IO                         0x08
#define TPCE_IO_DVB_VAL         0x22                    /* Imposed in CENELEC Errata document. */
#define TPCE_IO_RANGE           0x80
#define TPCE_IO_SLN                     0xc0
#define TPCE_IO_SLNR            0x06
#define TPCE_IO_SAN                     0x30
#define TPCE_IO_SANR            0x04
#define TPCE_IRQ                        0x10
#define TPCE_IRQ_MASK           0x10
#define TPCE_IR_DVB_VAL         0x20                    /* Imposed in CENELEC Errata document. */
#define TPCE_MEM                        0x60
#define TPCE_MEM_TWO_BYTES      0x20
#define TPCE_MEM_FOUR_BYTES     0x40
#define TPCE_MEM_DESCRIPTOR     0x60
#define TPCE_MEM_SL                     0x18
#define TPCE_MEM_SLR            0x03
#define TPCE_MEM_SCA            0x60
#define TPCE_MEM_SCAR           0x05
#define TPCE_MEM_HPA            0x80
#define TPCE_MISC                       0x80

/* value */
/* CISTPL_VERS_1 */
#define TPLLV1_MAJOR_VAL        0x05
#define TPLLV1_MINOR_VAL        0x00

/* CISTPL_CONFIG */
#define TPCC_RMSK                       0x01
#define NBRBYTES                        0x01
#define MAXADDR                         0xffe

/* CISTPL_CFTABLE_ENTRY */
#define TPCE_IF_VAL                     0x04

/* CISTPL_NOLINK */
#define CISTPL_NOLINKLEN        0x00

/* CCST_CIF */
#define CCST_CIFLEN                     0x0e
#define CCST_CIF1                       0x41
#define CCST_CIF2                       0x02
#define STCI_STR                        "DVB_CI_V"
#define STCI_STRP                       0x2e

/* STCE_EV */
#define STCE_EV_VAL                     "DVB_HOST"

/* STCE_PD */
#define STCE_PD_VAL                     "DVB_CI_MODULE"


#define         CXD1957CNTL     *(volatile UI8 *)0x60000001
#define         CXD1957SO       *(volatile UI8 *)0x60000003
#define         CXD1957MODEA    *(volatile UI8 *)0x60000005
#define         CXD1957GPIOCNTL *(volatile UI8 *)0x60000007
#define         CXD1957MODEB    *(volatile UI8 *)0x60000009

#define OGIO_0_BaseAddress	0xB4000000
//#define OGIO_0_BaseAddress	0x94000000

//#define OGIO_0_BaseAddress	0xB4000001

#define     CIMODULE_A  *(volatile UI8 *)OGIO_0_BaseAddress
#define     CIMA_DATA   *(volatile UI8 *)OGIO_0_BaseAddress	+ 0
#define     CIMA_CMDST  *(volatile UI8 *)OGIO_0_BaseAddress + 2
#define     CIMA_LS     *(volatile UI8 *)OGIO_0_BaseAddress + 4
#define     CIMA_MS     *(volatile UI8 *)OGIO_0_BaseAddress + 6
#define     CIMA_COR    *(volatile UI8 *)OGIO_0_BaseAddress	+ 0x400

#define     CIMODULE_B  *(volatile UI8 *)OGIO_0_BaseAddress
#define     CIMB_DATA   *(volatile UI8 *)OGIO_0_BaseAddress	+ 0
#define     CIMB_CMDST  *(volatile UI8 *)OGIO_0_BaseAddress	+ 2
#define     CIMB_LS     *(volatile UI8 *)OGIO_0_BaseAddress	+ 4
#define     CIMB_MS     *(volatile UI8 *)OGIO_0_BaseAddress	+ 6
#define     CIMB_COR    *(volatile UI8 *)OGIO_0_BaseAddress	+ 0x400
/*
#define     CIMODULE_A  *(volatile UI16 *)OGIO_0_BaseAddress
#define     CIMA_DATA   *(volatile UI16 *)OGIO_0_BaseAddress + 0
#define     CIMA_CMDST  *(volatile UI16 *)OGIO_0_BaseAddress + 4
#define     CIMA_LS     *(volatile UI16 *)OGIO_0_BaseAddress + 8
#define     CIMA_MS     *(volatile UI16 *)OGIO_0_BaseAddress + 12
#define     CIMA_COR    *(volatile UI16 *)OGIO_0_BaseAddress + 0x400

#define     CIMODULE_B  *(volatile UI16 *)OGIO_0_BaseAddress
#define     CIMB_DATA   *(volatile UI16 *)OGIO_0_BaseAddress + 0
#define     CIMB_CMDST  *(volatile UI16 *)OGIO_0_BaseAddress + 4
#define     CIMB_LS     *(volatile UI16 *)OGIO_0_BaseAddress + 8
#define     CIMB_MS     *(volatile UI16 *)OGIO_0_BaseAddress + 12
#define     CIMB_COR    *(volatile UI16 *)OGIO_0_BaseAddress + 0x400
*/
/*
#define     CIMODULE_A  *(volatile UI8 *)0x60040001
#define     CIMA_DATA   *(volatile UI8 *)0x60040001
#define     CIMA_CMDST  *(volatile UI8 *)0x60040003
#define     CIMA_LS     *(volatile UI8 *)0x60040005
#define     CIMA_MS     *(volatile UI8 *)0x60040007

#define     CIMODULE_B  *(volatile UI8 *)0x60080001
#define     CIMB_DATA   *(volatile UI8 *)0x60080001
#define     CIMB_CMDST  *(volatile UI8 *)0x60080003
#define     CIMB_LS     *(volatile UI8 *)0x60080005
#define     CIMB_MS     *(volatile UI8 *)0x60080007
#define     CIMB_COR    *(volatile UI8 *)0x60080401
*/
/* module status information */
#define     MODULE_DA   0x80
#define     MODULE_FR   0x40
#define     MODULE_WE   0x02
#define     MODULE_RE   0x01
/* module command definition */
#define     MODULE_RS   0x08
#define     MODULE_SR   0x04
#define     MODULE_SW   0x02
#define     MODULE_HC   0x01
#define     CONTROL    1//0
#define     STATUS_m   1//0
#define     DATA_m     0//1
#define     SIZE_H     3//2
#define     SIZE_L     2//3
#define     ENABLE     1
#define     DISABLE    0
#define     RESET       5

typedef struct session{
    UI16 SessionID;
    UI32 ResourceID;
    UI8  Status;
} SESSION_t;

typedef struct appli{
    UI8 ApplicaLen;
    UI32 AppID;
} APPLICA_t;

typedef struct tsi{
    UI8 tc_id;
    UI8 C_or_D;
    UI32 ResourceID;
    I8  MenuTitle[80];
    I8  MenuSub[160];
    I8  MenuBottom[160];
    I8  Text[25][256];
    UI8 ItemNB_MENU;
    UI8 MENU_LIST;
    UI8 DispCMD;
    UI8 DispMode;
    UI8 TimeInterval;
    UI16 Close_MMI_SessNum;
    UI8  Close_Delay;
    UI8  choice_ref;

    UI8  AnswID;
    UI8  Blind;
    UI8  AnswLeng;
    I8   AnswText[10];

    SESSION_t Session[50];
    APPLICA_t Application[50];

} TSI_t;

typedef struct ca_en{
    UI16 Pid;
    UI16 CA_id;
    UI8  CA_enable;
} CA_EN;

typedef struct ca_module {
    UI8  module_active;
    UI8  AppType;
    UI16 AppManufacture;
    UI16 ManufactureCode;
    I8   SystemName[30];
    /* UI16  ca_system_id; */
    UI8  MenuMessage;
    UI16 WorkingTimer;
    UI8  TS_LevelCA;
    UI16 TS_CA_id;
    CA_EN  ES_CA[22];
    TSI_t  Tsi;
} CA_MODULE;

typedef struct ca_buff{
    UI16 TxSize;
    UI16 RxSize;
    UI16 Access;
    UI8  TxBuff[1024];
    UI8  RxBuff[2048];
    UI8  SessBuff[1024];
    UI8  AppBuff[1024];
} CA_BUFF;

/* DVB TSI command */
#define  Tsb            0x80
#define  Trcv           0x81
#define  Tcreate_t_c    0x82
#define  Tc_t_c_reply   0x83
#define  Tdelete_t_c    0x84
#define  Td_t_c_reply   0x85
#define  Trequest_t_c   0x86
#define  Tnew_t_c       0x87
#define  Tt_c_error     0x88
#define  Tdata_last     0xA0
#define  Tdata_more     0xA1
/*  DVB Session command */
#define  Topen_session_request		0x91
#define  Topen_session_response		0x92
#define  Tcreate_session                0x93
#define	 Tcreate_session_response	0x94
#define  Tclose_session_request		0x95
#define  Tclose_session_response	0x96
#define  Tsession_number                0x90
/*  DVB application command */
/*  Resource - resource mgr */
#define         RMGR_header             0x9F80
#define         Tprofile_enq            0x10
#define         Tprofile_reply          0x11
#define  	Tprofile_change	    	0x12
/*  resource - application info */
#define         APPINFO_header          0x9F80
#define  	Tapplication_info_enq	0x20
#define         Tapplication_info       0x21
#define         Tenter_menu             0x22
#define         Tca_info_enq            0x30
#define         Tca_info                0x31
#define         Tca_pmt                 0x32
#define         Tca_pmt_reply           0x33
#define         Tca_pmt_last            0x48
#define         Tca_pmt_test            0x49

/*  resource - host control */
#define         HOSTCNTL_header         0x9F84
#define         DATETIME_header         0x9F84
#define         Ttune                   0x00
#define         Treplace                0x01
#define         Tclear_replace          0x02
#define         Task_release            0x03
#define         Tdate_time_enq          0x40
#define         Tdate_time              0x41
/*  resource - mmi */
#define         MMI_header              0x9F88
#define         Tclose_mmi              0x00
#define         Tdisplay_control        0x01
#define         Tdisplay_reply          0x02
#define         Ttext_last              0x03
#define         Ttext_more              0x04
#define         Tkeypad_control         0x05
#define         Tkeypress               0x06
#define         Tenq                    0x07
#define         Tansw                   0x08
#define         Tmenu_last              0x09
#define         Tmenu_more              0x0a
#define         Tmenu_answ              0x0b
#define         Tlist_last              0x0c
#define         Tlist_more              0x0d
#define		Tsubtitle_segment_last	0x0e
#define		Tsubtitle_segment_more	0x0f
#define         Tdisplay_message        0x10
#define         Tscene_end_mark         0x11
#define         Tscene_done             0x12
#define         Tscene_control          0x13
#define		Tsubtitle_download_last	0x14
#define         Tsubtitle_download_more 0x15
#define         Tflush_download         0x16
#define         Tdownload_reply         0x17
/*  resource - low speed comms */
#define         COMMS_header            0x9F8C
#define         Tcomms_cmd              0x00
#define		Tconnection_descriptor	0x01
#define         Tcomms_reply            0x02
#define         Tcomms_send_last        0x03
#define         Tcomms_send_more        0x04
#define         Tcomms_rev_last         0x05
#define         Tcomms_rev_more         0x06

/*  resource smart card */
#define         SCR_header              0x9F8E
#define         Tsmart_card_cmd         0x00
#define         Tsmart_card_reply       0x01
#define         Tsmart_card_send        0x02
#define         Tsmart_card_rcv         0x03


typedef struct {
    UI16 addr;
    UI16 len;
    UI8 *pbytes;
    UI16 rlen;
} DRV_stMem;

typedef struct {
    UI8 Main;
    UI8 TSI_mode;
    UI8 SS_mode;
    UI8 APP_mode;
    UI8 APP_modeBack;
} CI_MODE;


#define  TITLE		0x01
#define  SUBTITLE	0x02
#define  BOTTOM		0x03
#define  MESSAGE_c      0x04

#define  TSI_LAST	0x00
#define  TSI_MORE	0x80
#define  OK		1
#define  NG		0

#define  EMPTY          0
#define  INSERT         1
#define  INITIALIZE     2
#define  WORKING        3
#define  WORK_DELAY     4
#define  CAM_LIST       5
/* #define  INIT_FAIL      6 */

#define  SLOTA	    0
#define  SLOTB      1
#define  SLOTC      2

//#ifdef EVAL_HACK
#define  MAX_SLOT	3
//#else
//#define  MAX_SLOT	2
//#endif
/*  CI Main Mode */
#define  DETECT_m       10
#define  PWRCNTL        11
#define  RESET_m        12
#define  READ_CIS       13
#define  NEGOBUFF       14
#define  IDLE_CI      0xff
#define  PWRON_w        16
#define  NO_CIMODULE    17
/*  CI sub mode */
#define  PWRON_m        30
#define  PWROFF_m       31
#define  RES_ON         32
#define  RES_OFF        33
#define  RESET_l			34

struct dvb_entry_bundle
{
   int     Is_TPCE_INDX_Ok;        /* Intface + Default bits in TPCE_INDX if first Entry */
   int     Is_TPCE_IF;             /* If TRUE, the field TPCE_IF is present */
   int     Is_TPCE_IF_Ok;          /* "Errata in EN 50221 from CENELEC..." TPCE_IF imposed. */
   int     IsMasked_TPCE_FS_Ok;    /* TPCE_FS (Features) must include Power + IO + IRQ */
   int     Is_TPCE_PD_Ok;          /* There is at least one Description structure for Vcc */
   int     Is_TPCE_IO_Ok;          /* "Errata in EN 50221 from CENELEC..." TPCE_IO imposed. */
   int     Is_TPCE_IRQ_Ok;         /* "Errata in EN 50221 from CENELEC..." TPCE_IR imposed. */
   int     Is_STCE_EV_Ok;          /* Subtuple of CISTPL_CFTABLE_ENTRY contains "DVB_HOST" */
   int     Is_STCE_PD_Ok;          /* Subtuple of CISTPL_CFTABLE_ENTRY contains "DVB_CI_MODULE" */

   UI8   EntryNumb;              /* Read value of Configuration Entry Number in TPCE_INDX */
   UI8   TPCE_IF_Value;          /* Read value of TPCE_IF field. */
};

#define	    RESOURCEMGR	    0x00010041
#define	    APPINFO	    0x00020041
#define	    CA_SUPPORT	    0x00030041
#define	    HOSTCNTL	    0x00200041
#define	    DATE_TIME	    0x00240041
#define	    MMI		    0x00400041
#define     COMMS           0x00600000
#define     SC_READER       0x00700041

#define	    CLOSE	    0
#define	    OPEN	    1
#define     DISCONNECT      0
#define     CONNECT         1

#define    DRV_CISLEN  256

#define    set_mmi_mode		0x01
#define    OK_DESCRAMBLE        0x01
#define    OK_MMI               0x02
#define    QUERY                0x03
#define    NOT_SELECTED         0x04


extern CA_MODULE  g_aCaInfo[MAX_SLOT];
extern CA_BUFF    CA_Com[MAX_SLOT];
extern CI_MODE    Mode[MAX_SLOT];
//extern CA_MODULE  FUN_INFO;

#define M_RESET	0xfe

#endif //_EMMA_CIDEF_H

#endif //QBOXHD