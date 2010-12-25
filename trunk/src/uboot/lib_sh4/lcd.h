/*****************************
 * MACROS
 *****************************/

#ifndef CONFIG_QBOXHD_mini
 #define DISPLAY_WIDTH				320     /**< QVGA res: x = 320 */
 #define DISPLAY_HEIGHT				240     /**< QVGA res: y = 240 */
#else
 #define DISPLAY_WIDTH				160     
 #define DISPLAY_HEIGHT				128     
#endif

#define DISPLAY_BYPP				2


#define LCD_DEVICE_NAME				"lcd"	

#define LCD_IOC_MAGIC				'l'
#define LCD_IOCTL_RS				_IOR(LCD_IOC_MAGIC, 0, int)      /* Set Register Select */
#define LCD_IOCTL_STANDBY			_IOWR(LCD_IOC_MAGIC, 1, int)     /* Stan-by on/off */
#define LCD_IOCTL_NEWLINE			_IOR(LCD_IOC_MAGIC, 2, int)      /* New line in Y */
#define LCD_IOCTL_PIN_BRIGHTNESS	_IOWR(LCD_IOC_MAGIC, 3, int)   

#define LCD_IOC_NR					4

#define BRIGHTNESS_OFF				0x00
#define BRIGHTNESS_ON				0x01

/*****************************
 * DATA TYPES
 *****************************/

typedef char                    INT8;
typedef unsigned char           UINT8;

typedef short int               INT16; 
typedef unsigned short int      UINT16;

typedef int                     INT32;
typedef unsigned int            UINT32;

typedef long long int           INT64;
typedef unsigned long long int  UINT64;

typedef double                  FINT64;

/*****************************
 * several color
 *****************************/
/*
   R  |   G   |   B
1111|1 000|000 0|0000| -> RED 
0000|0 111|111 0|0000| -> GREEN
0000|0 000|000 1|1111| -> BLUE
*/

//RGB-565
#define	BLACK		0x0000
#define	WHITE		0xFFFF
#define	NICE_BLUE	0x561E

#define	RED			0xF800
#define	GREEN		0x07E0
#define	BLUE		0x001F

#define	YELLOW		0xFFE0//red + green
#define	PURPLE		0xF81F//red + blue
#define	CLEAR_BLUE	0x07FF//green + blue




