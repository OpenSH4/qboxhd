#ifdef QBOXHD

#ifndef _STARCI2WIN_H
#define _STARCI2WIN_H

#include <linux/ioctl.h>

typedef struct
{
        unsigned char mode;                     //1: Single     2: Twin
        unsigned char tsin_1;                   //0: None       1: Ts0      2: Ts2
        unsigned char tsin_2;                   //0: None       1: Ts1      2: Ts2
        unsigned char cam_1;                    //0: None       1: A        2: B        3: A+B
        unsigned char cam_2;                    //0: None       1: A        2: B        3: A+B
}Ci_Mode_BackBox;


#define SINGLE_M    1
#define TWIN_M      2
#define NONE_S      0
#define TS1         1
#define TS2         1
#define TS3         2
#define CAM_A       1
#define CAM_B       2
#define CAM_AB      3



#define TS_0        0
#define TS_1        1



#define STARCI_DEVICE_NAME              "STARCI2WIN"
#define CI_DEVICE_NAME                  "ci"
//#define INPUT_DEVICE_NAME             "ci_input"

#define CI_NUMBER_OF_CONTROLLERS        2


#define STARCI_MAJOR_NUM                170
#define STARCI_MINOR_START              0

#define IOCTL_STARCI_MAGIC              't'
#define IOCTL_SET_MODE_MODULE           _IOWR(IOCTL_STARCI_MAGIC, 1, Module_t)
#define IOCTL_GET_MODE_MODULE           _IOWR(IOCTL_STARCI_MAGIC, 2, Module_t)
#define IOCTL_READ_CIS                  _IOWR(IOCTL_STARCI_MAGIC, 3, Module_t)
#define IOCTL_WRITE_ADDR                _IOWR(IOCTL_STARCI_MAGIC, 4, Module_t)
#define IOCTL_READ_ADDR                 _IOWR(IOCTL_STARCI_MAGIC, 5, Module_t)
#define IOCTL_DETECT_MODULE             _IOWR(IOCTL_STARCI_MAGIC, 6, Module_t)

#define IOCTL_CI_SET_POWER              _IOWR(IOCTL_STARCI_MAGIC, 7, Module_t)
#define IOCTL_CI_GET_POWER              _IOWR(IOCTL_STARCI_MAGIC, 8, Module_t)
#define IOCTL_CI_SWF_RESET              _IOWR(IOCTL_STARCI_MAGIC, 9, Module_t)
#define IOCTL_CI_SET_CIS_IO_SPACE_MODE  _IOWR(IOCTL_STARCI_MAGIC, 10, Module_t)
#define IOCTL_CI_SET_TS_MODE            _IOWR(IOCTL_STARCI_MAGIC, 11, Module_and_ts_t)
#define IOCTL_CI_GET_TS_MODE            _IOWR(IOCTL_STARCI_MAGIC, 12, Module_and_ts_t)
#define IOCTL_SWITCH_MODULE             _IOWR(IOCTL_STARCI_MAGIC, 13, Module_t)
#define IOCTL_RST_ALL_MODE_MODULE       _IOWR(IOCTL_STARCI_MAGIC, 14, Module_t)

#define IOCTL_ACTIVATION_MODULE_RST     _IOWR(IOCTL_STARCI_MAGIC, 15, Module_t)

#define	IOCTL_CI_MODE_FROM_BLACKBOX	_IOWR(IOCTL_STARCI_MAGIC, 16, Ci_Mode_BackBox)

#define IOCTL_READ_I2C                  _IOWR(IOCTL_STARCI_MAGIC, 100, Module_t)
#define IOCTL_WRITE_I2C                 _IOWR(IOCTL_STARCI_MAGIC, 101, Module_t)



/*
#define IOCTL_CI_MEMORY_READ        _IOW(IOCTL_STARCI_MAGIC, 8, Module_t)
#define IOCTL_CI_ENABLE_CI_MODE     _IOW(IOCTL_STARCI_MAGIC, 9, Module_t)
#define IOCTL_CI_SET_TS_MODE        _IOW(IOCTL_STARCI_MAGIC, 10, Module_t)
#define IOCTL_CI_GET_TS_MODE        _IOW(IOCTL_STARCI_MAGIC, 11, Module_t)
#define IOCTL_CI_SWF_RESET          _IOW(IOCTL_STARCI_MAGIC, 12, Module_t)
#define IOCTL_CI_SET_BUFFER_SIZE    _IOW(IOCTL_STARCI_MAGIC, 13, Module_t)
#define IOCTL_CI_SEND_LPDU          _IOW(IOCTL_STARCI_MAGIC, 14, Module_t)
#define IOCTL_CI_ENABLE_LPDU_RCV    _IOW(IOCTL_STARCI_MAGIC, 15, Module_t)
#define IOCTL_CI_ENABLE_CARD_DETECT _IOW(IOCTL_STARCI_MAGIC, 16, Module_t)
#define IOCTL_CI_GET_PORT_STATUS    _IOW(IOCTL_STARCI_MAGIC, 17, Module_t)
*/




#define STARTCI_IO_SIZE             0x1000
#define STARCI_BASE_ADDRESS         0x3000000
//#define STARCI_BASE_ADDRESS       0x1000000
#define STARCI_REG_DATA             0x0
#define STARCI_REG_CTRL_STATUS      0x1
#define STARCI_REG_SIZE_LSB         0x2
#define STARCI_REG_SIZE_MSB         0x3

/* STATUS REGISTER: -> | DA | FR | R | R | R | R | WE | RE | */
/*  Define of the bits Status Register  */
#define DA_BIT_STATUS               0x80
#define FR_BIT_STATUS               0x40
#define WE_BIT_STATUS               0x02
#define RE_BIT_STATUS               0x01

/* COMMAND REGISTER: -> | R | R | R | R | RS | SR | SW | HC | */
/*  Define of the bits Command Register */
#define RS_BIT_COMMAND              0x08
#define SR_BIT_COMMAND              0x04
#define SW_BIT_COMMAND              0x02
#define HC_BIT_COMMAND              0x01



#define     MODULE_PRESENT          0x01
#define     MODULE_NOT_PRESENT      0x00

#define     POWER_ON                0x01
#define     POWER_OFF               0x00




#define     CIS_LEN                 256



/******     Value compare       ******/

/******     Module indicator    ******/
#define         NONE                0xAA
#define         MODULE_A            0x00
#define         MODULE_B            0x01
#define         MODULE_EXT          0x02

/******     Access module mode  ******/
#define         IO_SPACE            0x00
#define         CIS                 0x01
#define         MODE_MASK           0x0C

#define         TS_BY_PASS          0x02    /* TSIN->TSOUT */
#define         TS_TO_CAM           0x03    /* TSIN->Module->TSOUT */

#define         DEST_MOD_A          0x02
#define         DEST_MOD_B          0x04


/* temporaney definition.....FIXME!!!!!!!!!!!!! */
#define MOD_A_CTRL          0x00
#define MOD_A_ACCESS_TIME   0x05
#define MOD_B_CTRL          0x09
#define MOD_B_ACCESS_TIME   0x0E
#define SINGLE_MODE_TS      0x10
#define TWIN_MODE_TS        0x11
#define DEST_MODULE         0x17
#define POWER_CTRL          0x18
#define MP_INTERFACE_CONFIG 0x1D
#define STARCI2WIN_CTRL     0x1F


typedef struct
{
	unsigned char module;			/* Module A, Module B, EXT*/
	unsigned char configuration;	/* CIS mode or "Normal" mode */
} Module_t;

typedef struct
{
	unsigned short offset_addr;			/* Module A, Module B, EXT*/
	unsigned char value;	/* CIS mode or "Normal" mode */
} Register_t;

typedef struct
{
	unsigned char module;			/* Module A, Module B, EXT*/
	unsigned char ts_number;				/* TS through Cam */
	unsigned char configuration;	/* CIS mode or "Normal" mode */
} Module_and_ts_t;

#endif  ///_STARCI2WIN_H

#endif ///QBOXHD
