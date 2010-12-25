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

/***********************************/
/*     For program FPGA module     */
/***********************************/

/*****************************
 * INCLUDES
 *****************************/
#include <common.h>
#include "fpga_module.h"
#include "io.h"

#define PRINT	printf


#define FPGA_SET_PROGB			ctrl_outl(PIN_RPROG_B_MASK,fpga_p.pio2_base_address+PIO_SET_P2OUT)
#define FPGA_CLR_PROGB			ctrl_outl(PIN_RPROG_B_MASK,fpga_p.pio2_base_address+PIO_CLR_P2OUT)


#define FPGA_SET_INIT_B			ctrl_outl(PIN_INIT_B_MASK,fpga_p.pio2_base_address+PIO_SET_P2OUT)
#define FPGA_CLR_INIT_B			ctrl_outl(PIN_INIT_B_MASK,fpga_p.pio2_base_address+PIO_CLR_P2OUT)
#define FPGA_DONE_STATE			(ctrl_inl(fpga_p.pio2_base_address+PIO_P2IN) & (PIN_DONE_MASK) )

#define SWAP_BIT(x)	((x<<7)&0x80) | ((x<<5)&0x40) |  ((x<<3)&0x20) |  ((x<<1)&0x10) |  ((x>>7)&0x01) |  ((x>>5)&0x02) |  ((x>>3)&0x04) |  ((x>>1)&0x08)

#define mdelay(n)       udelay((n)*1000)

static struct cdev fpga_cdev;
unsigned long fpga_driver_init;
fpga_t fpga_p;

/************************/
/*	Auxiliar functions	*/
/************************/
static int device_init(void)
{
    fpga_p.fpga_base_address = (unsigned long)FPGA_BASE_ADDRESS;
	fpga_p.pio2_base_address = (unsigned long)PIO2_BASE_ADDRESS;

	/* Set the pin of PIO2 */
	ctrl_outl(0x40,fpga_p.pio2_base_address+PIO_SET_P2C0);
	ctrl_outl(0xA0,fpga_p.pio2_base_address+PIO_CLR_P2C0);

	ctrl_outl(0xA0,fpga_p.pio2_base_address+PIO_SET_P2C1);
	ctrl_outl(0x40,fpga_p.pio2_base_address+PIO_CLR_P2C1);

	ctrl_outl(0x40,fpga_p.pio2_base_address+PIO_SET_P2C2);
	ctrl_outl(0xA0,fpga_p.pio2_base_address+PIO_CLR_P2C2);

    return 0;
}


int fpga_operation(unsigned int cmd, unsigned char * param)
{
	switch (cmd)
	{
		case PREPARE_FOR_PROGRAM://1
			FPGA_SET_INIT_B;
			mdelay(100);
			FPGA_CLR_PROGB;
			mdelay(1);
			FPGA_SET_PROGB;
		break;
		case TEST_DONE_PIN://2
			*param=ctrl_inb(fpga_p.pio2_base_address+PIO_P2IN);
			*param&=PIN_DONE_MASK;
		break;
		case WRITE_ADDR_FPGA://3
			ctrl_outb(*param, fpga_p.fpga_base_address);
		break;

	}

	return 0;
}


extern flash_info_t flash_info[];	/* info for FLASH chips */
extern unsigned char flash_read_char_duo(flash_info_t * info, ulong addr);

void fpga_prog (void)
{
	unsigned int cnt=0;
	unsigned char data=0,value=0;
	flash_info_t *info;
	
	info = &flash_info[0];
	value=ctrl_inb(fpga_p.pio2_base_address+PIO_P2IN);
	value&=PIN_DONE_MASK;
	PRINT("DONE PIN STATUS:0x%02X\n",value);
	
	/* Prepare to program */
	FPGA_SET_INIT_B;
	mdelay(100);
	FPGA_SET_PROGB;
	mdelay(1);
	FPGA_CLR_PROGB;
	mdelay(7);
	FPGA_SET_PROGB;
	mdelay(1000);
//#if 0
	for(cnt=0;cnt<FPGA_BITSTREAM_SIZE;cnt++)
	{
		data=flash_read_char_duo(info,FPGA_BITSTREAM_ADDR+cnt);//0x90000
		data=SWAP_BIT(data);
		ctrl_outw((unsigned short)data, fpga_p.fpga_base_address);
	}
	
	if (FPGA_DONE_STATE)	PRINT("Program Complete. \n");
	else 					PRINT("### ERROR:Program Fault. \n");
	/* Clear the init pin, so the link between fpga and flash of the module... */
	FPGA_CLR_INIT_B;
	
	value=ctrl_inb(fpga_p.pio2_base_address+PIO_P2IN);
	value&=PIN_DONE_MASK;
	PRINT("DONE PIN STATUS:0x%02X\n",value);
	
//#endif	
}	

#ifdef CONFIG_QBOXHD_mini
void fpga_set_input_spare(void)
{
	unsigned long spare_base_addr;

	/* Set in input Port 0 Pin 2 */
	spare_base_addr=PIO0_BASE_ADDRESS;
	ctrl_outl(0x4,spare_base_addr+PIO_SET_P2C0);
	ctrl_outl(0x4,spare_base_addr+PIO_CLR_P2C1);
	ctrl_outl(0x4,spare_base_addr+PIO_SET_P2C2);

	/* Set in input Port 1 Pin 2 */
	spare_base_addr=PIO1_BASE_ADDRESS;
	ctrl_outl(0x4,spare_base_addr+PIO_SET_P2C0);
	ctrl_outl(0x4,spare_base_addr+PIO_CLR_P2C1);
	ctrl_outl(0x4,spare_base_addr+PIO_SET_P2C2);

	/* Set in input Port 3 Pin 2-4-5 */
	spare_base_addr=PIO3_BASE_ADDRESS;
	ctrl_outl(0x34,spare_base_addr+PIO_SET_P2C0);
	ctrl_outl(0x34,spare_base_addr+PIO_CLR_P2C1);
	ctrl_outl(0x34,spare_base_addr+PIO_SET_P2C2);

	/* Set in input Port 4 Pin 4 */
	spare_base_addr=PIO4_BASE_ADDRESS;
	ctrl_outl(0x10,spare_base_addr+PIO_SET_P2C0);
	ctrl_outl(0x10,spare_base_addr+PIO_CLR_P2C1);
	ctrl_outl(0x10,spare_base_addr+PIO_SET_P2C2);

	/* Set in input Port 5 Pin 7 */
	spare_base_addr=PIO5_BASE_ADDRESS;
	ctrl_outl(0x80,spare_base_addr+PIO_SET_P2C0);
	ctrl_outl(0x80,spare_base_addr+PIO_CLR_P2C1);
	ctrl_outl(0x80,spare_base_addr+PIO_SET_P2C2);
}
#endif

void fpga_module_init(void)
{
#ifdef CONFIG_QBOXHD_mini
	/* Set spare */
	fpga_set_input_spare();
#endif

	/* Init and program the fpga */
	device_init();
	fpga_prog();
	mdelay(1000);

#ifdef CONFIG_QBOXHD_mini
	/* Set the initial value of led */	
	ctrl_outw(0x0F0F,fpga_p.fpga_base_address+(0x40*2));
	ctrl_outw(0x0F,fpga_p.fpga_base_address+(0x41*2));
#endif

}


