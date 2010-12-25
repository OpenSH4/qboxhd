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

#include <common.h>
#include "sim_i2c_pin.h"
#include "io.h"

#ifdef CONFIG_QBOXHD_mini
unsigned long pio_addr;

/* Fo SCL */
#define SET_SCL			ctrl_outl(SCL,pio_addr+PIO_SET_P3OUT)
#define CLR_SCL			ctrl_outl(SCL,pio_addr+PIO_CLR_P3OUT)

/* For SDA */
#define SET_SDA			ctrl_outl(SDA,pio_addr+PIO_SET_P3OUT)
#define CLR_SDA			ctrl_outl(SDA,pio_addr+PIO_CLR_P3OUT)

/* To read SDA */
#define READ_SDA			(ctrl_inl(pio_addr+PIO_P3IN) & (SDA) )

/* To read SCL */
#define READ_SCL			(ctrl_inl(pio_addr+PIO_P3IN) & (SCL) )


void i2c_dly(void)
{
	udelay(1000); //1ms
}

void i2c_start(void)
{
	SET_SDA;             // i2c start bit sequence
	i2c_dly();
	SET_SCL;
	i2c_dly();
	CLR_SDA;
	i2c_dly();
	CLR_SCL;
	i2c_dly();
}

void i2c_stop(void)
{
	CLR_SDA;             // i2c stop bit sequence
	i2c_dly();
	SET_SCL;
	i2c_dly();
	SET_SDA;
	i2c_dly();
}

unsigned char i2c_rx(char ack) //set 'ack'=1 if it isn't the last byte....I think
{
	char x, d=0;
	SET_SDA; 
	for(x=0; x<8; x++)
	{
		d <<= 1;
		//do {
			SET_SCL;
		//}while(READ_SCL==0);    // wait for any SCL clock stretching
		i2c_dly();
		if(READ_SDA) d |= 1;
		CLR_SCL;
		i2c_dly();
	} 
	if(ack) CLR_SDA;
	else SET_SDA;
	SET_SCL;
	i2c_dly();             // send (N)ACK bit
	CLR_SCL;
	SET_SDA;
	return d;
}

unsigned char i2c_tx(unsigned char d)
{
	char x;
	unsigned char b;
	b=d;
	for(x=8; x; x--)
	{
		if(b&0x80) SET_SDA;
		else CLR_SDA;
		SET_SCL;
		b <<= 1;
		i2c_dly();
		CLR_SCL;
		i2c_dly();
	}
	SET_SDA;
	SET_SCL;
	i2c_dly();
	b = READ_SDA;          // possible ACK bit
	CLR_SCL;
	return b;
}

void i2c_read_version(unsigned char * ver)
{
	i2c_start();
	i2c_tx(0x34);
	i2c_tx(0x01);	//2 -> temp; 1 -> version
	i2c_start();
	i2c_tx(0x35);
	*ver = i2c_rx(0);
	i2c_stop();      
}

void lpc_init(void)
{
	pio_addr=PIO3_BASE_ADDRESS;

	ctrl_outl((SDA|SCL),pio_addr+PIO_SET_P3C0);
	ctrl_outl((SDA|SCL),pio_addr+PIO_CLR_P3C1);
	ctrl_outl((SDA|SCL),pio_addr+PIO_CLR_P3C2);
//	ctrl_outl(SCL,pio_addr+PIO_SET_P3C2);

	SET_SDA;
	SET_SCL;

	/* Set led to red */
	printf("Set red the led\n");
	i2c_start();
	i2c_tx(0x34);	//holtek addr
	i2c_tx(0x17);	//register addr
	i2c_tx(0x1F);	//red
	i2c_tx(0x00);	//green
	i2c_tx(0x00);	//blue
	i2c_tx(0x00);	//default
	i2c_stop();

}

unsigned char set_brg(unsigned char b)
{
	unsigned char version=0;
	i2c_read_version(&version);

	if(version>=2)
	{
		printf("Version is: %d\n",version);
		/* Activeted the backlight of display */
		i2c_start();
		i2c_tx(0x34);	//holtek addr
		i2c_tx(0x23);	//register addr
		i2c_tx(b);	//blacklight (0<value<0x1F)
		i2c_tx(0x00);	//default
		i2c_tx(0x00);	//default
		i2c_tx(0x00);	//default
		i2c_stop();
	}
	return version;
}
#endif








