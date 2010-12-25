/*
 * 	Copyright (C) 2010 Duolabs Srl
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */
/*****************************
 * INCLUDES
 *****************************/
#include <common.h>
#include "io.h"
#include "lcd.h"
#include "lcd_private.h"
#include "font.h"

#ifdef CONFIG_QBOXHD_mini
#include "sim_i2c_pin.h"
#endif

#define mdelay(n)       udelay((n)*1000)
#define	ssize_t			int
#define	EINVAL			0xFF

unsigned int last_position; // reference to hight of font

int bkg_color;
int pen_color;

/*****************************
 * MACROS
 *****************************/

INT32 debug;                    /**< Turn on/off LCD debugging */

static struct cdev lcd_cdev;    /**< LCD character device */

LCD_CONTROL_BLOCK lcd_cb;       /**< LCD Control Block */

#ifndef CONFIG_QBOXHD_mini
enum lcd_model {
	LCD_YM220T,
	LCD_YLM682A
};

enum lcd_model lcd_id = LCD_YM220T;		/**< LCD ID for recognizing different models */
#endif


void display_rgbw(unsigned char, unsigned char);

/**************************************************************************
 * Register ops
 **************************************************************************/

/**
 * @brief  Write a WO register
 * @param  base_address The GPIO base address
 * @param  reg The GPIO's register(offset) starting from the base address
 * @param  val 16-bit data
 */
static void reg_writeonly_l(UINT32 base_address, UINT32 reg, unsigned int val)
{
    UINT32 reg_address;
    
    reg_address = base_address + reg;
    ctrl_outl(val, reg_address);
}

/**
 * @brief Set the Register Select. RS input is used to judge serial input data <br>
 *        as display data when RS = H (1) the data is display data <br>
 *        and when RS = L (0) the data is command data.
 * @param rs Register Select
 */
static void reg_set_rs(UINT32 rs)
{
    /* Command data */
    if (!rs)
        reg_writeonly_l(lcd_cb.base_address_pio5, PIO_CLR_PnOUT, 0x20);
    /* Display data */
    else
        reg_writeonly_l(lcd_cb.base_address_pio5, PIO_SET_PnOUT, 0x20);
}

#ifndef CONFIG_QBOXHD_mini
/*****************************
 * YM220T
 *****************************/
/**
 * @brief  Write a 16-bit command
 * @param  addr First 8-bit data
 * @param  val Second 8-bit data
 */
static void reg_write_command_ym220t(UINT8 addr, UINT8 val) 
{
    ctrl_outw((addr << 8 | val), lcd_cb.base_address_emi);
}

/**
 * @brief  Write a 16-bit data
 * @param  data_1 First 8-bit data
 * @param  data_2 Second 8-bit data
 */
static void reg_write_data_ym220t(unsigned char data_1,unsigned char data_2)
{ 
    ctrl_outw((data_1 << 8 | data_2), lcd_cb.base_address_emi);
}

/*****************************
 * YLM682a
 *****************************/
/**
 * @brief LCD YLM682a write address
 * @param index Address
 */
static void reg_write_command_ylm682a(UINT16 index)
{
    reg_set_rs(0);
	ctrl_outw((index), lcd_cb.base_address_emi);
    reg_set_rs(1);
}

/**
 * @brief LCD YLM682a write data
 * @param instruction Data
 */
static void reg_write_data_ylm682a(UINT16 instruction)
{
	ctrl_outw((instruction), lcd_cb.base_address_emi);
}

 /**
 * @brief Read the LCD ID and save it in the global var lcd_id
 * 		  The YM220T does not support this op
 */
static void lcd_readid(void)
{
	UINT16 res;

    reg_set_rs(0);
	ctrl_outw((0x0), lcd_cb.base_address_emi);
    reg_set_rs(1);
	res = ctrl_inw(lcd_cb.base_address_emi);
	if (res == 0x9325)
		lcd_id = LCD_YLM682A;
	else
		lcd_id = LCD_YM220T;

	switch (lcd_id) {
		case LCD_YLM682A:
			break;
		case LCD_YM220T:
		default:
			break;
	}
}


/*****************************
 * YLM682a
 *****************************/

/**
 * @brief LCD YLM682a write address and data
 * @param index Address
 * @param instruction Data
 */
static void Init_data(UINT16 index, UINT16 instruction)
{
	reg_write_command_ylm682a(index);
    reg_write_data_ylm682a(instruction);
}

/**
 * @brief Set LCD YLM682a initial configuration
 */
static void init_lcd_ylm682a(void)
{
	Init_data(0x00F0,0x0001);
	Init_data(0x001A,0x288A);
	Init_data(0x00F0,0x0000);
	mdelay(50);
	Init_data(0x00E3, 0x3008);	/* Set internal timing */
	Init_data(0x00E7, 0x0012); 	/* Set internal timing */
	Init_data(0x00EF, 0x1231); 	/* Set internal timing */

	//Init_data(0x0001, 0x0100); 	/* set SS and SM bit */
	Init_data(0x0001, 0x0000); 	/* set SS and SM bit */

	Init_data(0x0002, 0x0700); 	/* set 1 line inversion */

	//Init_data(0x0003, 0x1030);	/*0x1030); // set GRAM write direction and BGR=1. */
	Init_data(0x0003, 0x1038);	/*0x1030); // set GRAM write direction and BGR=1. */


	//Init_data(0x0004, 0x0000); /* Resize register */
	//Init_data(0x0008, 0x0207); /*0x0202); // set the back porch and front porch */
	Init_data(0x0008, 0x0202); 	/*0x0202); // set the back porch and front porch */
	Init_data(0x0009, 0x0000); 	/* set non-display area refresh cycle ISC[3:0] */
	//Init_data(0x000A, 0x0000); /* FMARK function */
	//Init_data(0x000C, 0x0000); /* RGB interface setting */
	//Init_data(0x000D, 0x0000); /* Frame marker Position */
	//Init_data(0x000F, 0x0000); /* RGB interface polarity */

	/* Power On sequence */
	Init_data(0x0010, 0x0000);	/* SAP, BT[3:0], AP, DSTB, SLP, STB */
	Init_data(0x0011, 0x0007);	/* DC1[2:0], DC0[2:0], VC[2:0] */
	Init_data(0x0012, 0x0000);	/* VREG1OUT voltage */
	Init_data(0x0013, 0x0000);	/* VDV[4:0] for VCOM amplitude */
	Init_data(0x0007, 0x0001);
	mdelay(200); /* Dis-charge capacitor power voltage */
	//Init_data(0x0010, 0x1490);  /*0x1290); // SAP, BT[3:0], AP, DSTB, SLP, STB */
	Init_data(0x0010, 0x1290);  /*0x1290); // SAP, BT[3:0], AP, DSTB, SLP, STB */
	Init_data(0x0011, 0x0227); 	/* Set DC1[2:0], DC0[2:0], VC[2:0] */
	mdelay(50); 				/* Delay 50ms */
	//Init_data(0x0012, 0x0019); /*0x001a);  //0x001d); // External reference voltage= Vci; */
	Init_data(0x0012, 0x001a); 	/*0x001a);  //0x001d); // External reference voltage= Vci; */
	mdelay(50); 				/* Delay 50ms */
	//Init_data(0x0013, 0x1600); /*0x1400); //0x1000); // Set VDV[4:0] for VCOM amplitude */
	Init_data(0x0013, 0x1800); 	/*0x1400); //0x1000); // Set VDV[4:0] for VCOM amplitude */
	Init_data(0x0029, 0x002A); 	/*//0x002E); // 0x0019); //0x0008); // SetVCM[5:0] for VCOMH    */
	Init_data(0x002B, 0x000c); 	/*0x000d); // Set Frame Rate    //test by xen  */
	mdelay(50); 				/* Delay 50ms */
	Init_data(0x0020, 0x0000); 	/* GRAM horizontal Address */
	Init_data(0x0021, 0x0000); 	/* GRAM Vertical Address */

	/* Adjust the Gamma Curve */
	Init_data(0x0030, 0x0004);
	Init_data(0x0031, 0x0607);
	Init_data(0x0032, 0x0006);
	Init_data(0x0035, 0x0302);
	Init_data(0x0036, 0x0004);
	Init_data(0x0037, 0x0107);
	Init_data(0x0038, 0x0001);
	Init_data(0x0039, 0x0307);
	Init_data(0x003C, 0x0203);
	Init_data(0x003D, 0x0004);

	/* Set GRAM area */
	Init_data(0x0050, 0x0000); 	/* Horizontal GRAM Start Address */
	Init_data(0x0051, 0x00EF); 	/* Horizontal GRAM End Address */
	Init_data(0x0052, 0x0000); 	/* Vertical GRAM Start Address */
	Init_data(0x0053, 0x013F); 	/* Vertical GRAM Start Address */
	Init_data(0x0060, 0xa700); 	/* Gate Scan Line */
	Init_data(0x0061, 0x0001); 	/* NDL,VLE, REV */
	Init_data(0x006A, 0x0000); 	/* set scrolling line */

	/* Partial Display Control */
	Init_data(0x0080, 0x0000);
	Init_data(0x0081, 0x0000);
	Init_data(0x0082, 0x0000);
	Init_data(0x0083, 0x0000);
	Init_data(0x0084, 0x0000);
	Init_data(0x0085, 0x0000);

	/* Panel Control */
	Init_data(0x0090, 0x0010);
	Init_data(0x0092, 0x0600);
	Init_data(0x0007, 0x0133); 	/* 262K color and display ON */
}

/**
 * @brief Set LCD initial configuration
 */
static void init_lcd_ym220t(void)
{
    reg_set_rs(0);
    
    reg_write_command_ym220t(0x02,0x00);
    reg_write_command_ym220t(0x03,0x01);   /* Reset */
    reg_write_command_ym220t(0x62,0x05);
    mdelay(20);
    
    reg_write_command_ym220t(0x00,0x40);   /* Output all data as 0 */
    reg_write_command_ym220t(0x4e,0x20);
    mdelay(10);

    reg_write_command_ym220t(0x4e,0x60);
    reg_write_command_ym220t(0x01,0x00);
    mdelay(5);
    
    reg_write_command_ym220t(0x22,0x01);
    mdelay(5);
    
    reg_write_command_ym220t(0x22,0x00);
    mdelay(15);
    reg_write_command_ym220t(0x01,0x02);
    reg_write_command_ym220t(0x27,0x57);
	mdelay(25);
    reg_write_command_ym220t(0x28,0x60);
    mdelay(25);
    reg_write_command_ym220t(0x29,0x69);
    mdelay(25);
    reg_write_command_ym220t(0x2a,0x00);
    mdelay(25);
    reg_write_command_ym220t(0x3c,0x00);
    mdelay(25);
    reg_write_command_ym220t(0x3d,0x01);
    mdelay(25);
    reg_write_command_ym220t(0x3e,0x5c);
    mdelay(25);
    reg_write_command_ym220t(0x3f,0x87);
    mdelay(25);
    reg_write_command_ym220t(0x40,0x2f);
    mdelay(25);

	reg_write_command_ym220t(0x4b,0x03);
	reg_write_command_ym220t(0x4c,0x01);
	reg_write_command_ym220t(0x4f,0x04);
	reg_write_command_ym220t(0x50,0x26);
	reg_write_command_ym220t(0x53,0x19);
	reg_write_command_ym220t(0x54,0x22);
	reg_write_command_ym220t(0x55,0x0f);
	reg_write_command_ym220t(0x56,0x18);
	reg_write_command_ym220t(0x57,0x05);
	reg_write_command_ym220t(0x58,0x0e);

	reg_write_command_ym220t(0x34,0x70);
	reg_write_command_ym220t(0x30,0x02);
	reg_write_command_ym220t(0x35,0x00);
	reg_write_command_ym220t(0x31,0x00);
	reg_write_command_ym220t(0x33,0x57);
	reg_write_command_ym220t(0x32,0x07);
	reg_write_command_ym220t(0x37,0x60);
	reg_write_command_ym220t(0x36,0x60);	
	reg_write_command_ym220t(0x05,0x00);

	reg_write_command_ym220t(0x06,0x00);
    reg_write_command_ym220t(0x00,0x00);

// 	reg_write_command(0x07,0x00);

	reg_write_command_ym220t(0x4d,0x10);
	reg_write_command_ym220t(0x26,0xef);
	mdelay(10);

	reg_write_command_ym220t(0x4d,0x00);
	reg_write_command_ym220t(0x5d,0x18);
	mdelay(15);

	reg_write_command_ym220t(0x01,0x12);
	reg_write_command_ym220t(0x3d,0x05);
	mdelay(25);

	reg_write_command_ym220t(0x4d,0x28);
	reg_write_command_ym220t(0x5d,0xf8);
    mdelay(25);
    
    reg_write_command_ym220t(0x00,0x00);
    mdelay(10);
    
	reg_write_command_ym220t(0x08,0x00);  /* Set the min values of X addr */
	reg_write_command_ym220t(0x00,0x00);
	reg_write_command_ym220t(0x09,0x00);  /* Set the max values of X addr */
	reg_write_command_ym220t(0x00,0xef);
	reg_write_command_ym220t(0x0a,0x00);  /* Set the min values of Y addr */
	reg_write_command_ym220t(0x00,0x00);
	reg_write_command_ym220t(0x0b,0x00);  /* Set the max values of Y addr */
	reg_write_command_ym220t(0x01,0x3f);
	
//    reg_write_command_ym220t(0x01,0x80);    /* Invert X addr, ADX = 1 */
    reg_write_command_ym220t(0x01,0x40);  /* Invert Y addr, ADR = 1 */
    
	reg_write_command_ym220t(0x06,0x00);  /* Set X addr */
	reg_write_command_ym220t(0x00,0x00);    
	reg_write_command_ym220t(0x07,0x00);  /* Set Y addr */
	reg_write_command_ym220t(0x00,0x00);
	
	reg_write_command_ym220t(0x05,0x10);
    reg_write_command_ym220t(0x05,0x04);  /* Increment Y addr: rotate display */
}
#else
/* Because the bus EMI is inverted  (D7->D0, D0->D7) */
//#define SWAP_BIT(x)	((x<<7)&0x80) | ((x<<5)&0x40) |  ((x<<3)&0x20) |  ((x<<1)&0x10) |  ((x>>7)&0x01) |  ((x>>5)&0x02) |  ((x>>3)&0x04) |  ((x>>1)&0x08)
#define SWAP_BIT(x)	(x)

void WriteCOM(unsigned char a)
{
	unsigned char data=0;
	data=SWAP_BIT(a);
    reg_set_rs(0);
	ctrl_outb((data), lcd_cb.base_address_emi);
    reg_set_rs(1);
}

void WriteDAT(unsigned char b)
{
	unsigned char data=0;
	data=SWAP_BIT(b);
    reg_set_rs(1);
	ctrl_outb((data), lcd_cb.base_address_emi);
}
/****************************/
/*	Read id for QBoxHD Mini */
/****************************/
static unsigned short read_DA_id_reg(void)
{
	unsigned short res_m=0;
	unsigned short res_da=0;
	unsigned int lcd_id_m=0;

    /* Reset LCD */
    reg_writeonly_l(lcd_cb.base_address_pio5, PIO_SET_PnOUT, 0x10);
    mdelay(100);
    reg_writeonly_l(lcd_cb.base_address_pio5, PIO_CLR_PnOUT, 0x10);
    mdelay(100);
    reg_writeonly_l(lcd_cb.base_address_pio5, PIO_SET_PnOUT, 0x10);
    mdelay(120);
	WriteCOM(0x11); //Exit Sleep
	mdelay(100);

	/* DA */
	WriteCOM(0xDA);	//manufacturer ID
	res_da=res_m=ctrl_inw(lcd_cb.base_address_emi);
	lcd_id_m|=((res_m>>8)&0xFF);
	printf("Value of DA register (manufacturer ID): 0x%04X\n",(res_m>>8));
	lcd_id_m=(lcd_id_m<<8);

	/* I don't known why the following read it isn't work */
#if 0
	/* DB */
	WriteCOM(0xDB);
	res_m=0;
	res_m=ctrl_inw(lcd_cb.base_address_emi);
	lcd_id_m|=((res_m>>8)&0xFF);
	printf("Value of DB register: 0x%04X\n",res_m);
	lcd_id_m=(lcd_id_m<<8);

	/* DC */
	WriteCOM(0xDC);
	res_m=0;
	res_m=ctrl_inw(lcd_cb.base_address_emi);
	lcd_id_m|=((res_m>>8)&0xFF);
	printf("Value of DC register: 0x%04X\n",res_m);
	printf("Value of LCD id: 0x%08X\n", lcd_id_m);
#endif

	return res_da;
}
/*****************************
 * FDG177
 ****************************/
static void init_lcd_fdg177(void)
{
	printf("\n");
	printf("Init of fdg177 display\n");
	printf("\n");

    /* Reset LCD */
    reg_writeonly_l(lcd_cb.base_address_pio5, PIO_SET_PnOUT, 0x10);
    mdelay(100);
    reg_writeonly_l(lcd_cb.base_address_pio5, PIO_CLR_PnOUT, 0x10);
    mdelay(100);
    reg_writeonly_l(lcd_cb.base_address_pio5, PIO_SET_PnOUT, 0x10);
    mdelay(120);

	WriteCOM(0x11);          //SLEEP OUT&BOOST ON

	mdelay(50);
	WriteCOM(0xff);	//Vcom 4  Level  control
	WriteDAT(0x40);
	WriteDAT(0x01);	//01
	WriteDAT(0x1a);	

	WriteCOM(0xd9);	//close IDLE  EEPROM Control Status
	WriteDAT(0x60);
	WriteCOM(0xc7);	//EEPROM Control Status
	WriteDAT(0x90);
	mdelay(100);

	WriteCOM(0xB1);	//Frame Rate Control normal mode/full color
	WriteDAT(0x00);
	WriteDAT(0x06);
	WriteDAT(0x03);

	WriteCOM(0xB2);	//Frame Rate Control 8-colors
	WriteDAT(0x00);
	WriteDAT(0x06);
	WriteDAT(0x03);

	WriteCOM(0xB3);	//Frame Rate Control partial mode/full colors
	WriteDAT(0x00);
	WriteDAT(0x06);
	WriteDAT(0x03);
	WriteDAT(0x00);
	WriteDAT(0x06);
	WriteDAT(0x03);

	WriteCOM(0xB4);	//Display Inversion Control
	WriteDAT(0x02);
	
	WriteCOM(0xB6);	//Display Function set 5
	WriteDAT(0x15);
	WriteDAT(0x02);

	WriteCOM(0xC0);	//Power Control 1
	WriteDAT(0x02);
	WriteDAT(0x70);

	WriteCOM(0xC1);	//Power Control 2
	WriteDAT(0x05);

	WriteCOM(0xC2);	//Power Control 3
	WriteDAT(0x01);
	WriteDAT(0x02);

	WriteCOM(0xC3);	//Power Control 4
	WriteDAT(0x02);
	WriteDAT(0x07);

	WriteCOM(0xC4);	//Power Control 5
	WriteDAT(0x01);
	WriteDAT(0x02);

	WriteCOM(0xFC);	//
	WriteDAT(0x11);
	WriteDAT(0x15);

	WriteCOM(0xC5);	//VCOM Control 1
	WriteDAT(0x39);
	WriteDAT(0x46);

	WriteCOM(0xe0);    //GAMCTRP1
	WriteDAT(0x09);
	WriteDAT(0x16);
	WriteDAT(0x09);
	WriteDAT(0x20); //27
	WriteDAT(0x21); //2E
	WriteDAT(0x1B); //25
	WriteDAT(0x13); //1C
	WriteDAT(0x19); //20
	WriteDAT(0x17); //1E
	WriteDAT(0x15); //1A
	WriteDAT(0x1E); //24
	WriteDAT(0x2B); //2D
	WriteDAT(0x04);
	WriteDAT(0x05);
	WriteDAT(0x02);
	WriteDAT(0x0e);

	WriteCOM(0xe1);   //GAMCTRN1
	WriteDAT(0x0b);
	WriteDAT(0x14);
	WriteDAT(0x08);  //09
	WriteDAT(0x1E);  //26
	WriteDAT(0x22);  //27
	WriteDAT(0x1D);  //22
	WriteDAT(0x18);  //1C
	WriteDAT(0x1E);  //20
	WriteDAT(0x1B);  //1D
	WriteDAT(0x1A);  //1A
	WriteDAT(0x24);  //25
	WriteDAT(0x2B);  //2D
	WriteDAT(0x06);
	WriteDAT(0x06);
	WriteDAT(0x02);
	WriteDAT(0x0f);

	WriteCOM(0x36);   //Memory data access control MY MX MV ML RGB MH - -
	WriteDAT(0x60);//(0xc0);   //C8 1=BGR 0=RGB

	WriteCOM(0x2A);   //CASET (2Ah): Column Address Set
	WriteDAT(0x00);
	WriteDAT(0x00);
	WriteDAT(0x00);
	//WriteDAT(0x7f);
	WriteDAT(0x9f);

	WriteCOM(0x2B);    //RASET (2Bh): Row Address Set
	WriteDAT(0x00);
	WriteDAT(0x00);
	WriteDAT(0x00);
	//WriteDAT(0x9f);
	WriteDAT(0x7f);

	WriteCOM(0x3a);   //Interface pixel format
	WriteDAT(0x05);   //011-12-bit/pixel  101-16bit   110-18bit

	WriteCOM(0x29);	// Display on
	//delayms(100);
	mdelay(100);
	WriteCOM(0x2C);	//Write memory
}

/*****************************
 * ILI9163
 ****************************/
static void init_lcd_ili9163(void)
{
	printf("\n");
	printf("Init of ili9163 display\n");
	printf("\n");

    /* Reset LCD */
    reg_writeonly_l(lcd_cb.base_address_pio5, PIO_SET_PnOUT, 0x10);
    mdelay(100);
    reg_writeonly_l(lcd_cb.base_address_pio5, PIO_CLR_PnOUT, 0x10);
    mdelay(100);
    reg_writeonly_l(lcd_cb.base_address_pio5, PIO_SET_PnOUT, 0x10);

    mdelay(120);

	WriteCOM(0x11); //Exit Sleep
	mdelay(20);

	WriteCOM(0x26); //Set Default Gamma
	WriteDAT(0x04);

	WriteCOM(0xB1);//Set Frame Rate
	WriteDAT(0x0e); 
	WriteDAT(0x14);

	WriteCOM(0xC0); //Set VRH1[4:0] & VC[2:0] for VCI1 & GVDD
	WriteDAT(0x08);//08  4.5V
	WriteDAT(0x05);//00//2.75V

	WriteCOM(0xC1); //Set BT[2:0] for AVDD & VCL & VGH & VGL
	WriteDAT(0x05);///6* 3*

	WriteCOM(0xC5); //Set VMH[6:0] & VML[6:0] for VOMH & VCOML
	WriteDAT(35); //38   3.9V
	WriteDAT(90);  //40//3d   -0.9V

	WriteCOM(0xC7); //Set VMH[6:0] & VML[6:0] for VOMH & VCOML
	WriteDAT(0xb8); //38

	WriteCOM(0x36); 
	WriteDAT(0x68);// rotate 90° and RGB //(0x60);//(0x08);  //08
	//WriteCOM(0xb7); 
	//WriteDAT(0x01);  //08
	
	WriteCOM(0x3a); //Set Color Format
	WriteDAT(0x05);

	WriteCOM(0x2A); //Set Column Address
	WriteDAT(0x00);
	WriteDAT(0x00);
	WriteDAT(0x00);
//	WriteDAT(0x7F);
	WriteDAT(0x9F);

	WriteCOM(0x2B); //Set Page Address
	WriteDAT(0x00);
	WriteDAT(0x00);
	WriteDAT(0x00);
//	WriteDAT(0x9F);
	WriteDAT(0x7F);
	//WriteCOM(0xEc);
	//WriteDAT(0x0c);//p1

	WriteCOM(0xB4); 
	WriteDAT(0x00);

	WriteCOM(0xf2); //Enable Gamma bit
	WriteDAT(0x01);

	WriteCOM(0xE0);
	WriteDAT(0x36);//p1
	WriteDAT(0x29);//p2
	WriteDAT(0x12);//p3
	WriteDAT(0x22);//p4
	WriteDAT(0x1C);//p5
	WriteDAT(0x15);//p6
	WriteDAT(0x42);//p7
	WriteDAT(0xB7);//p8
	WriteDAT(0x2F);//p9
	WriteDAT(0x13);//p10
	WriteDAT(0x12);//p11
	WriteDAT(0x0A);//p12
	WriteDAT(0x11);//p13
	WriteDAT(0x0B);//p14
	WriteDAT(0x06);//p15

	WriteCOM(0xE1);
	WriteDAT(0x09);//p1
	WriteDAT(0x16);//p2
	WriteDAT(0x2D);//p3
	WriteDAT(0x0D);//p4
	WriteDAT(0x13);//p5
	WriteDAT(0x15);//p6
	WriteDAT(0x40);//p7
	WriteDAT(0x48);//p8
	WriteDAT(0x53);//p9
	WriteDAT(0x0C);//p10
	WriteDAT(0x1D);//p11
	WriteDAT(0x25);//p12
	WriteDAT(0x2E);//p13
	WriteDAT(0x34);//p14
	WriteDAT(0x39);//p15

	WriteCOM(0x29); // Display On
	WriteCOM(0x2c); // Display On
}
#endif

/**************************************************************************
 * Init/Uninit funcs
 **************************************************************************/
 
/**
 * @brief Map registers, configure GPIOs and initialize LCD
 */
int device_init(void)
{
    lcd_cb.base_address_emi  = (unsigned long)EMI_BASE_ADDRESS;
    lcd_cb.base_address_pio5 = (unsigned long)PIO5_BASE_ADDRESS;
    lcd_cb.fpga_base_address = (unsigned long)FPGA_BASE_ADDRESS;

    /* Configure LCD reset. Set GPIO 5[4]. Datasheet page 213-217 */
    reg_writeonly_l(lcd_cb.base_address_pio5, PIO_CLR_PnC0, 0x10);
    reg_writeonly_l(lcd_cb.base_address_pio5, PIO_SET_PnC1, 0x10);
    reg_writeonly_l(lcd_cb.base_address_pio5, PIO_CLR_PnC2, 0x10);
    
    /* Configure LCD brightness. Set GPIO 5[7] -> output */
//    reg_writeonly_l(lcd_cb.base_address_pio5, PIO_CLR_PnC0, 0x80);
//    reg_writeonly_l(lcd_cb.base_address_pio5, PIO_SET_PnC1, 0x80);
//    reg_writeonly_l(lcd_cb.base_address_pio5, PIO_CLR_PnC2, 0x80);

	/* Configure LCD brightness. Set GPIO 5[7] -> input No use it but PWM of FPGA */
    reg_writeonly_l(lcd_cb.base_address_pio5, PIO_SET_PnC0, 0x80);
    reg_writeonly_l(lcd_cb.base_address_pio5, PIO_CLR_PnC1, 0x80);
    reg_writeonly_l(lcd_cb.base_address_pio5, PIO_SET_PnC2, 0x80);

    /* Reset LCD */
    reg_writeonly_l(lcd_cb.base_address_pio5, PIO_CLR_PnOUT, 0x10);
    mdelay(150);
    reg_writeonly_l(lcd_cb.base_address_pio5, PIO_SET_PnOUT, 0x10);
    mdelay(150);

    /* Configure Register Select (RS). Set GPIO 5[5]. Datasheet page 213-217 */
    reg_writeonly_l(lcd_cb.base_address_pio5, PIO_CLR_PnC0, 0x20);
    reg_writeonly_l(lcd_cb.base_address_pio5, PIO_SET_PnC1, 0x20);
    reg_writeonly_l(lcd_cb.base_address_pio5, PIO_CLR_PnC2, 0x20);

#ifndef CONFIG_QBOXHD_mini
	/* Init LCD */
	lcd_readid();
	switch (lcd_id) {
		case LCD_YLM682A: /* New LCD */
			init_lcd_ylm682a();
			mdelay(20);
			display_rgbw(0x00,0x00);
			//lcd_ylm682a_test();
			break;
		case LCD_YM220T:
		default:
    		init_lcd_ym220t();
			/* Display a nice blue screen */
			//display_rgbw(0x56,0x1E);
			display_rgbw(0x00,0x00);//black
			break;
	}
#else
	unsigned short id=0;
	id=read_DA_id_reg();
	if( (id>>8) == 0x54)
		init_lcd_ili9163();
	else	//0x5Cxxxx
		init_lcd_fdg177();
#endif

    /* Default: brightness ON */
	/* If you use PWM, no set it */
//    reg_writeonly_l(lcd_cb.base_address_pio5, PIO_CLR_PnOUT, 0x80);

    return LCD_NO_ERROR;
}

/**
 * @brief Unmap registers
 */
static void device_uninit(void)
{
    lcd_cb.base_address_emi=0;
    lcd_cb.base_address_pio5=0;
    lcd_cb.fpga_base_address=0;
}

/**************************************************************************
 * Driver File Operations
 **************************************************************************/
#ifndef CONFIG_QBOXHD_mini
/**
 * @brief Paint the whole LCD with a solid colour
 * @param data1 First 8-bit data
 * @param data2 Second 8-bit data
 */
void display_rgbw(unsigned char data1, unsigned char data2)
{
    UINT32 i,j;
    UINT16 data;

    reg_set_rs(1);

	/* The followed commands are used to set the orig (0,0) and to create the area to draw */
	if (lcd_id == LCD_YLM682A) {
		Init_data(0x0020,0x0000);
		Init_data(0x0021,0x0000);//AC
		Init_data(0x0050,0x0000);
		Init_data(0x0051,0x00EF);//H address
		Init_data(0x0052,0x0000);
		Init_data(0x0053,0x013F);//V address	
		reg_write_command_ylm682a(0x0022);	
	}
	data = data1 << 8 | data2;

	for (i = 0; i < DISPLAY_WIDTH; i++) {
		for (j = 0; j < DISPLAY_HEIGHT; j++) {
			if (lcd_id == LCD_YLM682A)
				reg_write_data_ylm682a(data);
			else
				reg_write_data_ym220t(data1, data2);
		}
	}
}

//U-boot routine
static ssize_t lcd_write(const char *buf, size_t len)
{
    /* Since we write two pixels at once, we only accept 4 bytes */
    if (len != 4)
        return -EINVAL;
        
    reg_set_rs(1);
    /*        pixel 2, msb        | pixel 2, lsb        | pixel 1, msb       | pixel 2, lsb */
    ctrl_outl((UINT8)buf[3] << 24 | (UINT8)buf[2] << 16 | (UINT8)buf[1] << 8 | (UINT8)buf[0],
        lcd_cb.base_address_emi);

    return len;
}


/*		RGB 565		*/

/********************************************************************/
/* 'x' and 'y' are refered to the hw orig. For us they are inverted */
/********************************************************************/
int set_pos(int x,int y)
{
	unsigned int y_pos=0;

	reg_set_rs(0);

	switch (lcd_id) {
		case LCD_YLM682A:
			/* Set the X addr. When in qvga this is equivalent to Y (240 pixels) */
			Init_data(0x0020,x);// Horizontal -> for us Vertical 

			/* Set the Y addr. When in qvga this is equivalent to Y (240 pixels) */
			Init_data(0x0021,y);// Vertical -> for us Horizontal

			reg_write_command_ylm682a(0x0022);
			break;
		case LCD_YM220T:
		default:
			/* Set the X addr. When in qvga this is equivalent to Y (240 pixels) */
			reg_write_command_ym220t(0x06,0x00);
			reg_write_command_ym220t(0x00,239 - (UINT8)x);

			/* Set the Y addr. When in qvga this is equivalent to Y (240 pixels) */
			y_pos=y;

			reg_write_command_ym220t(0x07,0x00);
			reg_write_command_ym220t((y_pos>>8),(y_pos&0xFF));
			break;
	}

	return 0;
}
#else
void display_rgbw(unsigned char data1, unsigned char data2)
{
    UINT32 i,j;
    UINT16 data;

	WriteCOM(0x2C);	//Write memory
    reg_set_rs(1);
//	data = data1 << 8 | data2;
	for (i = 0; i < DISPLAY_WIDTH; i++)
	{
		for (j = 0; j < DISPLAY_HEIGHT; j++)
		{
			ctrl_outb(SWAP_BIT(data1),lcd_cb.base_address_emi);
			ctrl_outb(SWAP_BIT(data2),lcd_cb.base_address_emi);
		}
	}
}
/********************************************************************/
/* 'x' and 'y' are refered to the hw orig. For us they are inverted */
/********************************************************************/
int set_pos(int x,int y)
{
	unsigned int y_pos=0;

	reg_set_rs(0);
	WriteCOM(0x2A);   //CASET (2Ah): Column Address Set
	WriteDAT(0x00);
	WriteDAT(y);
	WriteDAT(0x00);
	WriteDAT(0x9f);

	WriteCOM(0x2B);    //RASET (2Bh): Row Address Set
	WriteDAT(0x00);
	WriteDAT(x);
	WriteDAT(0x00);
	WriteDAT(0x7f);
	WriteCOM(0x2C);	//Write memory
	reg_set_rs(1);

	return 0;
}
#endif

int set_bkg_color(unsigned short color)
{
	bkg_color=color;
	return 0;
}

int set_pen_color(unsigned short color)
{
	pen_color=color;
	return 0;
}


#ifndef CONFIG_QBOXHD_mini
int set_pixel_color(unsigned short color)
{
    reg_set_rs(1);
	if(lcd_id==LCD_YM220T)
		reg_write_data_ym220t((color>>8), (color&0xFF));
	else
		reg_write_data_ylm682a(color);
	return 0;
}
#else
int set_pixel_color(unsigned short color)
{
    reg_set_rs(1);
	ctrl_outb(SWAP_BIT((color>>8)),lcd_cb.base_address_emi);
	ctrl_outb((SWAP_BIT(color&0xFF)),lcd_cb.base_address_emi);

	return 0;
}
#endif

SetPixel(int xp,int yp,int presence)
{
	if(presence!=0)
	{
//		set_pos(xp,yp);
		set_pixel_color(pen_color);//red
	}
	else
		set_pixel_color(bkg_color);
}

void LCDSoftChar(unsigned char ch, int xpos, int ypos)
{
	unsigned char by,mask;
	unsigned char i,j,k;
 
	if((ch==10) || (ch==13) )//10 LF il carattere '\n'; 13 CR
		last_position+=fontheight;
	for(i=0; i<8; i++) 
	{
		if(fontheight<=8)
		{
			//by=font[ch*fontwidth+i];
////////////////////////			
			//if the font is for horizontal display
			//by=font[ch][i]; 
			//if the font is for vertical display
			by=0;
			for(k=0;k<8;k++)
			{
				if( ( ((font[ch][k]) & (1<<i)) << k ) != 0 )
					by |= ( 1 << k );
			}
////////////////////////
			mask=0x01;      
			set_pos(xpos+i,ypos);    
			for(j=0; j<8; j++)
			{
				SetPixel(xpos+i,ypos+j,(by&mask));
				mask<<=1; 
			}
		}
    /*
//for 16x8
		if(fontheight>8 && fontheight<=16)
		{
			by=font[ch*2*fontwidth+i]; 
			mask=0x01;      
			for(j=0; j<8; j++)
			{
				SetPixel(xpos+i,ypos+j,(by&mask));
				mask<<=1;
			}
			by=font[ch*2*fontwidth+i+8]; 
			mask=0x01;      
			for(j=0; j<8; j++)
			{
				SetPixel(xpos+i,ypos+j+8,(by&mask));
				mask<<=1; 
			}
		}
	*/
	}
}
#ifndef CONFIG_QBOXHD_mini
 #define		LEN_STR		39
#else
 #define		LEN_STR		25
#endif
void LCDSoftString(unsigned char * str, int xpos, int ypos)
{
	unsigned int i,j;
	unsigned int len=0;
	unsigned int volte=0;
	if(strlen(str)<LEN_STR)
	{
		for(i=0;i<strlen(str);i++)
		{
			LCDSoftChar(str[i],xpos+0,ypos+(fontwidth/*8*/*i));
		}
	}
	else
	{
		len=strlen(str);
		volte=len/LEN_STR;
		for(i=0;i<volte;i++)
		{
			for(j=0;j<LEN_STR;j++)
			{
				LCDSoftChar(str[j+(i*LEN_STR)],xpos+0+(i*8),ypos+(fontwidth/*8*/*j));
			}
			last_position+=fontheight;
		}	
		len=len%LEN_STR;
		last_position+=fontheight;
		for(j=0;j<len;j++)
		{	
			LCDSoftChar(str[j+(volte*LEN_STR)],last_position,ypos+(fontwidth/*8*/*j));
		}
		last_position-=fontheight;// work around: there is a inexplicable '\n'  
	}
}

int display_print(const char *format, ...)
{
	char debugTxt[1024];
	va_list ap;
	int rv;
	va_start(ap, format);
	rv = vsprintf(debugTxt, format, ap);
	va_end(ap);
	
	if(last_position>225)//FIXME
	{
		display_rgbw((bkg_color>>8),(bkg_color&0xFF));
		last_position=0;
	}
	LCDSoftString(debugTxt,last_position+8,fontwidth/*8*/);
	return rv;
}

unsigned short convert_hsb2rgb(float Hue, float Saturation, float Brightness)
{
	float r=0,g=0,b=0,temp1,temp2;
	unsigned char i;
	float clr[]={0,0,0};
	float t3[3];
	unsigned short res=0,ri=0,gi=0,bi=0;
Brightness/=100;
Saturation/=100;
	Hue/=360;
	t3[0]=Hue + 1.0/3.0;
	t3[1]=Hue;
	t3[2]=Hue - 1.0/3.0;
	if(Brightness==0)
	{
		r = 0;
        g = 0;
        b = 0;
	}
    else
	{
		if(Saturation==0)
		{
			r = Brightness;
            g = Brightness;
            b = Brightness;
		}
		else
		{
			if(Brightness<=0.5)
			{
				temp2=Brightness * (1.0 + Saturation);
			}
			else
			{
				temp2=Brightness + Saturation - Brightness * Saturation;
			}
			temp1=2.0 * Brightness - temp2;

			for(i=0;i<=2;i++)
			{
				if(t3[i]<0) t3[i]+=1.0;
				if(t3[i]>1) t3[i]-=1.0;
				if( 6.0*t3[i] <1.0)
					clr[i]=temp1+(temp2 - temp1)*t3[i] *6.0;
				else if (2.0* t3[i] <1.0) 
					clr[i]=temp2;
				else if (3.0* t3[i] <2.0)
					clr[i]=temp1 + (temp2 - temp1) * (2.0 / 3.0 - t3[i]) * 6.0;
				else
					clr[i]=temp1;
			}
	        r = clr[0];
	        g = clr[1];
	        b = clr[2];
		}
	}

	ri=r*32;
	gi=g*64;
	bi=b*32;
	
	res=ri;
	res=res<<6;
	res|=gi;
	res=res<<5;
	res|=bi;
	
	
	return res;
	
}

#if 0
void create_scale(void)
{
	UINT32 i,j;
	float z=0,z1=100;
	unsigned short t=0;
	
	z1=z1/240;
	z=0;
	set_pos(0,0);
//    reg_set_rs(1);
    for (i = 0; i < 240/*(DISPLAY_WIDTH)*/; i++)//320
    {
        for (j = 0; j < (DISPLAY_HEIGHT); j++)//240
        {
			set_pos(i,j);
			z=z+z1;
			t=convert_hsb2rgb(j,99,(float)(i/2.7));
			reg_set_rs(1);
			reg_write_data_ym220t((t>>8),(t&0xFF) );
		}
	}
}
#endif	            

/********************************************************************/

void test_display(void)
{
	unsigned short current_color=0;
	unsigned char i=0;

	int str;

	current_color=WHITE;
	last_position=0;
	bkg_color=BLACK;
	pen_color=WHITE;

	/* Set all background */
	display_rgbw((bkg_color>>8),(bkg_color&0xFF));
	
	for(i=0;i<25;i++)
	{
		display_print("%d - Test display\n",i);
	}

	for(i=25;i<31;i++)
	{
		display_print("%d - Test display\n",i);
		mdelay(1000);
	}


	display_print("\n");
	display_print("Last position: %d\n",last_position);
set_pen_color(GREEN);
	display_print("# GREEN #\n");
set_pen_color(RED);
	display_print("# RED #\n");
set_pen_color(BLUE);
	display_print("# BLUE #\n");
set_pen_color(YELLOW);
	display_print("# YELLOW #\n");
set_pen_color(PURPLE);
	display_print("# PURPLE #\n");
set_pen_color(CLEAR_BLUE);
	display_print("# CLEAR_BLUE# \n");
set_pen_color(WHITE);
display_print("WHITE\n");
display_print("\n");

udelay(2*1000*1000);
//create_scale();
udelay(5*1000*1000);
//display_rgbw((bkg_color>>8),(bkg_color&0xFF));

}

extern flash_info_t flash_info[];	/* info for FLASH chips */
extern unsigned char flash_read_char_duo(flash_info_t * info, ulong addr);
#ifndef CONFIG_QBOXHD_mini
void display_image(void)
{
	flash_info_t *info;
	UINT32 i,j,cnt=0;
	unsigned char data1=0,data2=0;
	UINT16 data;

	info = &flash_info[0];
	reg_set_rs(1);

	for (i = 0; i < (DISPLAY_HEIGHT); i++)
	{
		if (lcd_id != LCD_YLM682A)
		{
			/* NEW LINE */
			reg_set_rs(0);
			reg_write_command_ym220t(0x06,0x00);
			reg_write_command_ym220t(0x00,(DISPLAY_HEIGHT-1)-(UINT8)i);
			/* Send Data */
			reg_set_rs(1);

		}
		for (j = 0; j < (DISPLAY_WIDTH); j++)
		{
			data1=flash_read_char_duo(info,LOGO_ADDRESS+cnt);
			data2=flash_read_char_duo(info,LOGO_ADDRESS+cnt+1);
			if(lcd_id == LCD_YM220T)
				reg_write_data_ym220t(data1,data2);
			else
			{
				data = data1 << 8 | data2;
				reg_write_data_ylm682a(data);
			}
			cnt+=2;
		}
	}
}
#else
void display_image(void)
{
	flash_info_t *info;
	UINT32 i,j,cnt=0;
	unsigned char data1=0,data2=0;
	UINT16 data;

	set_pos(0,0);
	
	info = &flash_info[0];
	reg_set_rs(1);

	for (i = 0; i < (DISPLAY_HEIGHT); i++)
	{
		for (j = 0; j < (DISPLAY_WIDTH); j++)
		{
			data1=flash_read_char_duo(info,LOGO_ADDRESS+cnt);
			data2=flash_read_char_duo(info,LOGO_ADDRESS+cnt+1);	//for u-boot is big end -> little end.. fixme!
			ctrl_outb(SWAP_BIT(data1),lcd_cb.base_address_emi);
			ctrl_outb(SWAP_BIT(data2),lcd_cb.base_address_emi);
			cnt+=2;
		}
	}
}
#endif


void demo_print(void)
{
	unsigned short current_color=0;
	unsigned char i=0;
	unsigned short data_brg=52/*63*/;	/* Max brg */
	int str;

	current_color=WHITE;
#ifndef CONFIG_QBOXHD_mini
	last_position=(8*16);
#else
	last_position=(8*7);
#endif

	bkg_color=BLACK;
	pen_color=WHITE;

	display_image();

#ifdef CONFIG_QBOXHD_mini
	i=set_brg(27);//(31);
	if(i>=2)
		ctrl_outw(data_brg, lcd_cb.fpga_base_address+(REG_BRIGHTNESS*2));
#else
	/* Set ON the brightness */
	ctrl_outw(data_brg, lcd_cb.fpga_base_address+(REG_BRIGHTNESS*2));
#endif

	mdelay(1);

#ifndef CONFIG_QBOXHD_mini
	display_print(" WELCOME TO QboxHD\n");

	/* To print the ip */
	last_position+=(8*3);//4);
	pen_color=RED;
	display_print(" Booting.......\n");
	last_position+=(8*1);//2);

#else
	display_print("WELCOME TO QboxHD\n");
	display_print("       mini\n");
	last_position+=(8*1);
	pen_color=RED;
	display_print("Booting...\n");
	/* To print the ip */
	last_position+=(8*0);//2);
#endif
}


