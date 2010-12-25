#ifndef __lcd_h
#define __lcd_h

#include <asm/types.h>
#include <lib/gdi/esize.h>
#include <lib/gdi/erect.h>

#ifdef QBOXHD_MINI
	#include <dfb++.h>
#endif


#define LCD_CONTRAST_MIN 0
#define LCD_CONTRAST_MAX 63
#define LCD_BRIGHTNESS_MIN 0
#define LCD_BRIGHTNESS_MAX 255

// #define DLB_LCD_RGB_565
#ifdef QBOXHD

#define DISPLAY_WIDTH   320     /**< QVGA res: x = 320 */
#define DISPLAY_HEIGHT  240     /**< QVGA res: y = 240 */

#ifdef QBOXHD_MINI
	#define DISPLAY_WIDTH_MINI   160
	#define DISPLAY_HEIGHT_MINI  128
#endif


#define LCD_DEVICE_NAME     "lcd"

#define LCD_IOC_MAGIC				'l'
#define LCD_IOCTL_RS				_IOR(LCD_IOC_MAGIC, 0, int)		/* Set Register Select */
#define LCD_IOCTL_STANDBY			_IOWR(LCD_IOC_MAGIC, 1, int)	/* Stan-by on/off */
//#define LCD_IOCTL_NEWLINE			_IOR(LCD_IOC_MAGIC, 2, int)		/* New line in Y */
#define LCD_IOCTL_ON_OFF_BRIGHTNESS	_IOWR(LCD_IOC_MAGIC, 3, int)   	/* On Off brightness */
#define LCD_IOCTL_REG_BRIGHTNESS	_IOWR(LCD_IOC_MAGIC, 4, short)   	/* On Off brightness */
#define LCD_IOCTL_MMAP_WRITE		_IOWR(LCD_IOC_MAGIC, 5, short)   /* Start and stop to write on */ 
																	 /* display with mmap function */

#define LCD_IOC_NR					6

#define BRIGHTNESS_OFF				0x00
#define BRIGHTNESS_ON				0x01

#define	LCD_STOP					0x00
#define	LCD_START					0x01

#endif ///QBOXHD

class eLCD
{
#ifdef SWIG
	eLCD(eSize size);
	~eLCD();
#else
protected:
	eSize res;
	unsigned char *_buffer;
	int lcdfd;
#ifdef QBOXHD
	int sizelcd;
#endif ///QBOXHD
	int _stride;
	int locked;
#endif ///SWIG
public:
	int lock();
	void unlock();
	int islocked() { return locked; }
	bool detected() { return lcdfd >= 0; }
#ifdef QBOXHD
	void enable_update();
	void draw( unsigned char *buff );
#endif // QBOXHD
#ifndef SWIG
	eLCD(eSize size);
	virtual ~eLCD();
	__u8 *buffer() { return (__u8*)_buffer; }
	int stride() { return _stride; }
	eSize size() { return res; }
	
	virtual void update()=0;
#endif
};

#ifdef QBOXHD_MINI

#define NO_INIT		0x00
#define	INIT		0x01
#define SUSPEND		0x02

void suspend_display_dfb(int par);

typedef struct
{
	IDirectFBSurface		*surf;
	int						surface_pitch;
	void                    *surface_data;
	unsigned short 			*pBUFF;
}DFB_Surface_t;

typedef struct
{
	unsigned char			init_struct;
	IDirectFB				*dfb;
	IDirectFBDisplayLayer	*layer;
	DFB_Surface_t			primary;
	DFB_Surface_t			surf2;
	DFB_Surface_t			surf3;
	DFBRectangle			src_rect;
	DFBRectangle			dst_rect;
    DFBSurfaceDescription   dsc;
}DFB_Stretch_t;
#endif

class eDBoxLCD: public eLCD
{
	static eDBoxLCD *instance;
	unsigned char inverted;
	int is_oled;
#ifdef SWIG
	eDBoxLCD();
	~eDBoxLCD();
#endif
#ifdef QBOXHD_MINI
private:
	DFB_Stretch_t	strc;
	void init_DFB_for_display(void);
public:
	void deinit_DFB_for_display(int par);
#endif
public:
#ifndef SWIG
	eDBoxLCD();
	~eDBoxLCD();
#endif
	static eDBoxLCD *getInstance();
#ifndef QBOXHD
	int setLCDContrast(int contrast);
	void setInverted( unsigned char );
	bool isOled() const { return !!is_oled; }
#endif ///QBOXHD
	
	int setLCDBrightness(int brightness);

	void update();
#ifdef QBOXHD
	unsigned short *buff_mmap;
	void DrawRGB565();
#endif ///QBOXHD
};

#endif
