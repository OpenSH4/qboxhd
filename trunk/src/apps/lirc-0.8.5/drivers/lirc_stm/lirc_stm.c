/*
 * LIRC plugin for the STMicroelectronics IRDA devices
 *
 * Copyright (C) 2004-2005 STMicroelectronics
 *
 * June 2004:  first implementation for a 2.4 Linux kernel
 *             Giuseppe Cavallaro  <peppe.cavallaro@st.com>
 * Marc 2005:  review to support pure raw mode and to adapt to Linux 2.6
 *             Giuseppe Cavallaro  <peppe.cavallaro@st.com>
 * June 2005:  Change to a MODE2 receive driver and made into a generic
 *             ST driver.
 *             Carl Shaw <carl.shaw@st.com>
 * July 2005:  fix STB7100 MODE2 implementation and improve performance
 *             of STm8000 version. <carl.shaw@st.com>
 * Aug  2005:  Added clock autoconfiguration support.  Fixed module exit code.
 * 	       Added UHF support (kernel build only).
 * 	       Carl Shaw <carl.shaw@st.com>
 * Sep  2005:  Added first transmit support
 *             Added ability to set rxpolarity register
 * 	       Angelo Castello <angelo.castello@st.com>
 * 	       and Carl Shaw <carl.shaw@st.com>
 * Oct  2005:  Added 7100 transmit
 *             Added carrier width configuration
 * 	       Carl Shaw <carl.shaw@st.com>
 * Sept 2006:  Update:
 * 		fix timing issues (bugzilla 764)
 * 		Thomas Betker <thomas.betker@siemens.com>
 * 		allocate PIO pins in driver
 * 		update transmit
 * 		improve fault handling on init
 * 		Carl Shaw <carl.shaw@st.com>
 * Oct  2007:  Added both lirc-0.8.2 common interface and integrated out IRB driver  
 *             to be working for linux-2.6.23-rc7. Removed old platform support...
 *             Sti5528 STb8000. Added new IR rx intq mechanism to reduce the amount 
 *             intq needed to identify one button. Fix TX transmission loop setting up 
 *             correctly the irq clean register.
 * 	       Angelo Castello <angelo.castello@st.com>
 * Nov  2007:  Moved here all the platform
 *             dependences leaving clear of this task the common interface. (lirc_dev.c)
 *	       Code cleaning and optimization.
 * 	       Angelo Castello <angelo.castello@st.com>
 * Dec  2007:  Added device resource management support.
 * 	       Angelo Castello <angelo.castello@st.com>
 * Mar  2008:  Fix UHF support and general tidy up
 *             Carl Shaw <carl.shaw@st.com>
 * Mar  2008:  Fix insmod/rmmod actions. Added new PIO allocate mechanism
 *	       based on platform PIOs dependencies values (LIRC_PIO_ON,
 *	       LIRC_IR_RX, LIRC_UHF_RX so on )
 * 	       Angelo Castello <angelo.castello@st.com>
 *
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/clock.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/stm/pio.h>
#include <linux/stm/soc.h>
#include <linux/time.h>
#include <linux/lirc.h>
#include "lirc_dev.h"

#define LIRC_STM_NAME "lirc_stm"




/* General debugging */
#undef LIRC_STM_DEBUG
//#define LIRC_STM_DEBUG
//#ifdef  LIRC_STM_DEBUG
//#define DPRINTK(fmt, args...) printk(KERN_INFO LIRC_STM_NAME ": %s: " fmt, __FUNCTION__ , ## args)
//#else
#define DPRINTK(fmt, args...)
//#endif




/************************************************************************

				qbox_one			qbox_hd

MUTE			0x44				0xa1
POWER			0x0F				0xa0
0				0x00				0x00
1				0X01				0x01
2				0x02				0x02
3				0x03				0x03
4				0x04				0x04
5				0x05				0x05
6				0x06				0x06
7				0x07				0x07
8				0x08				0x08
9				0x09				0x09
<				0x50				0xa6
>               0x51				0xa7
HELP			0x28				0xb9
MENU			0x20				0xad
UP				0x21*				0xc1*
DOWN			0x22*				0xc5*
LEFT			0x23*				0xc2
RIGHT			0x24*				0xc4
OK				0x25				0xc3
AUDIO			0x26				0xa2
EXIT			0x52				0xb6
TEXT			0x45				0xb8
TV				0x0c*				0xb7
RADIO			0x53*				0xae
VIDEO			0x54				0xa4
RED				0x40*				0xb2*
GREEN			0x41*				0xb4*
YELLOW			0x42*				0xb5*
BLUE			0x43*				0xb3*
VOL+			0x24*				0xa3
VOL-			0x23*				0xa5
CH+				0x21*				0xc1*
CH-				0x22*				0xc5*
STOP			0x0c*				0xa9
REC				0x53*				0xab
BACKWARD		0x40*				0xb2*
PLAY			0x41*				0xb4*
PAUSE			0x42*				0xb5*
FORWARD			0x43*				0xb3*

BOUQ+			n/a					0xa8
BOUQ-			n/a					0xb1
INFO			n/a					0xac
EPG				n/a

*********************************************************************** */



/* ----------------------DUOLABS defines---------------------------- */
//#define DUOLABS
//#define SHOW_IMPULSES


/* --------------------QBOXHD_RC defines---------------------------- */
#define QBOXHD_RC_SUPPORT

#define QBOXHD_IR_REMOTE                	0x000022df

/* identify start header of impulses train */
#define QBOXHD_START_PULSES_MIN_LENGTH 		8800
#define QBOXHD_START_SPACE_MIN_LENGTH 		2000

/* define time interval and tollerance for interpreted bit */
#define QBOXHD_BIT_1_MIN_LENGTH				1600
#define QBOXHD_BIT_0_MIN_LENGTH				550	
#define QBOXHD_BIT_TOLLERANCE				200





/* --------------------QBOXONE_RC defines--------------------------- */
#define QBOXONE_RC_SUPPORT



/*
 * Infra Red: hardware register map
 */
#if defined(CONFIG_LIRC_STM_UHF_RX)
static int uhf_switch = 1;
#else
static int uhf_switch = 0;
#endif
static int ir_or_uhf_offset;
static int irb_irq = 0;		/* IR block irq */
static void *irb_base_address;	/* IR block register base address */

/* QBOX RC Variables */

typedef enum
{
    RC_QBOXONE = 0x00010000,
    RC_QBOXHD  = 0x00020000,
}
fp_rc_mapping;

#ifdef QBOXONE_RC_SUPPORT

static int first_simbol = 0;
unsigned char power_button_pressed = 0;
static const unsigned int threshold[]={1000, 1164, 1302, 1439, 1577, 1715, 1852, 1989, 2127, 2265, 2402, 2539, 2677, 2835, 2952};
#define IR_REMOTE_BIG      		        0x0E0F441A    
#define IR_REMOTE_LITTLE                0x2B1F441A
#define IR_BUTTON_MODEL					IR_REMOTE_LITTLE
#define IR_BUTTON_MODEL_MASK            0xffffffff

#endif

#ifdef QBOXHD_RC_SUPPORT
unsigned int last_button_pressed=0;
unsigned int prev_button_pressed=0;
unsigned int new_button_received=0;
#endif


/* IR transmitter registers */
#define IRB_TX_REG(x)		(irb_base_address + x)
#define IRB_TX_PRESCALAR	IRB_TX_REG(0x00)	/* clock prescalar       */
#define IRB_TX_SUBCARRIER	IRB_TX_REG(0x04)	/* subcarrier frequency  */
#define IRB_TX_SYMPERIOD	IRB_TX_REG(0x08)	/* symbol period         */
#define IRB_TX_ONTIME		IRB_TX_REG(0x0c)	/* symbol pulse time     */
#define IRB_TX_INT_ENABLE	IRB_TX_REG(0x10)	/* TX irq enable         */
#define IRB_TX_INT_STATUS	IRB_TX_REG(0x14)	/* TX irq status         */
#define IRB_TX_ENABLE		IRB_TX_REG(0x18)	/* TX enable             */
#define IRB_TX_INT_CLEAR	IRB_TX_REG(0x1c)	/* TX interrupt clear    */
#define IRB_TX_SUBCARRIER_WIDTH	IRB_TX_REG(0x20)	/* subcarrier freq width */
#define IRB_TX_STATUS		IRB_TX_REG(0x24)	/* TX status */

#define TX_INT_PENDING			0x01
#define TX_INT_UNDERRUN			0x02

#define TX_FIFO_DEPTH			7
#define TX_FIFO_USED			((readl(IRB_TX_STATUS) >> 8) & 0x07)

/* IR receiver registers */
#define IRB_RX_REG(x)		(irb_base_address + x + ir_or_uhf_offset)
#define IRB_RX_ON	    	IRB_RX_REG(0x40)	/* RX pulse time capture */
#define IRB_RX_SYS          	IRB_RX_REG(0x44)	/* RX sym period capture ------------------------------------ */ 
#define IRB_RX_INT_EN	    	IRB_RX_REG(0x48)	/* RX IRQ enable (R/W)   */
#define IRB_RX_INT_STATUS      	IRB_RX_REG(0x4C)	/* RX IRQ status (R/W)   */
#define IRB_RX_EN	    	IRB_RX_REG(0x50)	/* Receive enable (R/W)  */
#define IRB_MAX_SYM_PERIOD  	IRB_RX_REG(0x54)	/* end of sym. max value */
#define IRB_RX_INT_CLEAR 	IRB_RX_REG(0x58)	/* overrun status (W)    */
#define IRB_RX_STATUS	    	IRB_RX_REG(0x6C)	/* receive status        */
#define IRB_RX_NOISE_SUPPR  	IRB_RX_REG(0x5C)	/* noise suppression     */
#define IRB_RX_POLARITY_INV 	IRB_RX_REG(0x68)	/* polarity inverter     */

/* IRB and UHF common registers */
#define IRB_CM_REG(x)		(irb_base_address + x)
#define IRB_RX_RATE_COMMON   	IRB_CM_REG(0x64)	/* sample frequency divisor */
#define IRB_RX_CLOCK_SEL  	IRB_CM_REG(0x70)	/* clock select (LP mode)   */
#define IRB_RX_CLOCK_SEL_STATUS IRB_CM_REG(0x74)	/* clock selection status   */
#define IRB_RX_NOISE_SUPP_WIDTH IRB_CM_REG(0x9C)

#define RX_CLEAR_IRQ(x) 		writel((x), IRB_RX_INT_CLEAR)
#define RX_WORDS_IN_FIFO() 		(readl(IRB_RX_STATUS) & 0x0700)

#define LIRC_STM_MINOR			0
#define LIRC_STM_MAX_SYMBOLS		100
#define LIRC_STM_FIFO_READ_MAX_SYMBOLS		3
#define LIRC_STM_BUFSIZE		(LIRC_STM_MAX_SYMBOLS*sizeof(lirc_t))

/* Bit settings */
#define LIRC_STM_IS_OVERRUN	 	0x04
#define LIRC_STM_CLEAR_IRQ	 	0x38
#define LIRC_STM_CLEAR_OVERRUN	 	0x04
/* IRQ set: Enable full FIFO                 1  -> bit  3;
 *          Enable overrun IRQ               1  -> bit  2;
 *          Enable last symbol IRQ           1  -> bit  1:
 *          Enable RX interrupt              1  -> bit  0;
 */
#define LIRC_STM_ENABLE_IRQ		0x0f

/* RX graphical example to better understand the difference between ST IR block
 * output and standard definition used by LIRC (and most of the world!)
 *
 *           mark                                     mark
 *      |-IRB_RX_ON-|                            |-IRB_RX_ON-|
 *      ___  ___  ___                            ___  ___  ___             _
 *      | |  | |  | |                            | |  | |  | |             |
 *      | |  | |  | |         space 0            | |  | |  | |   space 1   |
 * _____| |__| |__| |____________________________| |__| |__| |_____________|
 *
 *      |--------------- IRB_RX_SYS -------------|------ IRB_RX_SYS -------|
 *
 *      |------------- encoding bit 0 -----------|---- encoding bit 1 -----|
 *
 * ST hardware returns mark (IRB_RX_ON) and total symbol time (IRB_RX_SYS), so
 * convert to standard mark/space we have to calculate space=(IRB_RX_SYS - mark)
 * The mark time represents the amount of time the carrier (usually 36-40kHz)
 * is detected.
 *
 * TX is the same but with opposite calculation.
 *
 * The above examples shows Pulse Width Modulation encoding where bit 0 is
 * represented by space>mark.
 */

/* SOC dependent section - these values are set in the appropriate 
 * arch/sh/kernel/cpu/sh4/setup-* files and
 * transfered when the lirc device is opened
 */

typedef struct lirc_stm_tx_data_s {
	wait_queue_head_t waitq;
	/* timing fine control */
	unsigned int mult;
	unsigned int div;
	unsigned int carrier_freq;
	/* transmit buffer */
	lirc_t *wbuf;
	volatile int off_wbuf;
} lirc_stm_tx_data_t;

typedef struct lirc_stm_rx_data_s {
	/* timing fine control */
	int symbol_mult;
	int symbol_div;
	int pulse_mult;
	int pulse_div;
	/* data configuration */
	unsigned int sampling_freq_div;
	lirc_t *rbuf;
	volatile int off_rbuf;
	unsigned int sumUs;
	int error;
	struct timeval sync;
} lirc_stm_rx_data_t;

typedef struct lirc_stm_plugin_data_s {
	int open_count;
	struct plat_lirc_data *p_lirc_d;
} lirc_stm_plugin_data_t;


static lirc_stm_plugin_data_t pd;	/* IR data config */
static lirc_stm_rx_data_t rx;	/* RX data config */
static lirc_stm_tx_data_t tx;	/* TX data config */

/* LIRC subsytem symbol buffer. managed only via common lirc routines
 * user process read symbols from here  */
struct lirc_buffer lirc_stm_rbuf;

static inline void lirc_stm_reset_rx_data(void)
{
	rx.error = 0;
	rx.off_rbuf = 0;
	rx.sumUs = 0;
	memset(rx.rbuf, 0, LIRC_STM_BUFSIZE);
}

static inline unsigned int lirc_stm_time_to_cycles(unsigned int microsecondtime)
{
	/* convert a microsecond time to the nearest number of subcarrier clock
	 * cycles
	 */
	microsecondtime *= tx.mult;
	microsecondtime /= tx.div;
	return (microsecondtime * tx.carrier_freq / 1000000);
}

static void lirc_stm_tx_interrupt(int irq, void *dev_id)
{
	unsigned int symbol, mark, done = 0;
	unsigned int tx_irq_status = readl(IRB_TX_INT_STATUS);

	if ((tx_irq_status & TX_INT_PENDING) != TX_INT_PENDING)
		return;

	while (done == 0) {
		if (unlikely((readl(IRB_TX_INT_STATUS) & TX_INT_UNDERRUN) ==
			     TX_INT_UNDERRUN)) {
			/* There has been an underrun - clear flag, switch
			 * off transmitter and signal possible exit
			 */
			printk(KERN_ERR "lirc_stm: transmit underrun!\n");
			writel(0x02, IRB_TX_INT_CLEAR);
			writel(0x00, IRB_TX_INT_ENABLE);
			writel(0x00, IRB_TX_ENABLE);
			done = 1;
			DPRINTK("disabled TX\n");
			wake_up_interruptible(&tx.waitq);
		} else {
			int fifoslots = TX_FIFO_USED;

			while (fifoslots < TX_FIFO_DEPTH) {
				mark = tx.wbuf[(tx.off_wbuf * 2)];
				symbol = mark + tx.wbuf[(tx.off_wbuf * 2) + 1];
				DPRINTK("TX raw m %d s %d ", mark, symbol);

				mark = lirc_stm_time_to_cycles(mark) + 1;
				symbol = lirc_stm_time_to_cycles(symbol) + 2;
				DPRINTK("cal m %d s %d\n", mark, symbol);

				if ((tx.wbuf[(tx.off_wbuf * 2)] == 0xFFFF) ||
				    (tx.wbuf[(tx.off_wbuf * 2) + 1] == 0xFFFF))
				{
					/* Dump out last symbol */
					writel(mark * 2, IRB_TX_SYMPERIOD);
					writel(mark, IRB_TX_ONTIME);

					DPRINTK("TX end m %d s %d\n",
						mark, mark * 2);

					/* flush transmit fifo */
					while (TX_FIFO_USED != 0) {
					};
					writel(0, IRB_TX_SYMPERIOD);
					writel(0, IRB_TX_ONTIME);
					/* spin until TX fifo empty */
					while (TX_FIFO_USED != 0) {
					};
					/* disable tx interrupts and
					 * transmitter */
					writel(0x07, IRB_TX_INT_CLEAR);
					writel(0x00, IRB_TX_INT_ENABLE);
					writel(0x00, IRB_TX_ENABLE);
					DPRINTK("TX disabled\n");
					tx.off_wbuf = 0;
					fifoslots = 999;
					done = 1;
				} else {
					writel(symbol, IRB_TX_SYMPERIOD);
					writel(mark, IRB_TX_ONTIME);

					DPRINTK("Nm %d s %d\n", mark, symbol);

					tx.off_wbuf++;
					fifoslots = TX_FIFO_USED;
				}
			}
		}
	}
}



void duodvb_rc_buttondown(fp_rc_mapping rc_mapping, int keycode)
{
	unsigned int mycode;

	if ( keycode != -1) {
		mycode = rc_mapping|keycode;
		lirc_buffer_write_1(&lirc_stm_rbuf, (unsigned char *)&(mycode));
		DPRINTK( "0x%04X\n", keycode );
	}

}


void duodvb_rc_buttonrepeat(fp_rc_mapping rc_mapping, int keycode)
{
	unsigned int mycode;

	if ( keycode != -1) {	

		mycode = rc_mapping|0x8000|keycode;
		lirc_buffer_write_1(&lirc_stm_rbuf, (unsigned char *)&(mycode));
		DPRINTK( "0x%04X\n", keycode );
	}
}


void duodvb_rc_buttonup(fp_rc_mapping rc_mapping, int keycode)
{
	unsigned int mycode;

	if ( keycode != -1) {	
		
		mycode = rc_mapping|0x8000|keycode;
		lirc_buffer_write_1(&lirc_stm_rbuf, (unsigned char *)&(mycode));
		DPRINTK( "0x%04X\n", mycode );

		mycode = rc_mapping|0x00FF;
		lirc_buffer_write_1(&lirc_stm_rbuf, (unsigned char *)&(mycode));
		DPRINTK( "0x%04X\n", mycode );		
	}
}







#ifdef QBOXONE_RC_SUPPORT


static unsigned char decodeValue( lirc_t space, lirc_t  pulse )
{
	unsigned char nibbleidx = 0;

	for ( nibbleidx = 0; nibbleidx < sizeof( threshold ); nibbleidx++)
	{
		/* Identify all nibble from threshold space
		*/		
		if ( threshold[nibbleidx] >= ( pulse+space ) )
			return nibbleidx;
		 
	}

	return nibbleidx;						
}




static int QBOXONE_RC_traslate_impulse_to_8Bytes(unsigned int total_pulses, unsigned int *first_4, unsigned int *last_4)
{
	unsigned int i, c;
	unsigned int button_value;
	unsigned int impulse_idx=0;
	unsigned int pulse_idx=0;
	unsigned int space_idx=0;
	lirc_t pulses[9];
	lirc_t spaces[8];	
	int isPulse = 0;
	int pulseReceived=0;
	unsigned char checksum=0;
		
	*first_4=0;
	*last_4=0;
		
	/* check if i have 9 pulses for minimal parse */
	if ( total_pulses < 9 ) 
	{
		DPRINTK("ERROR: total_pulses < 9. Discard it.\n");
		return 0;
	}
	
	for ( impulse_idx=0; impulse_idx< ((2*rx.off_rbuf)-1); impulse_idx++ )
	{
		/* check if it is a PULSE */
		if ( rx.rbuf[impulse_idx] & PULSE_BIT ) isPulse=1;
		else isPulse=0;

		if (isPulse) {

			if ( pulse_idx > 8 ) pulse_idx=0;

			pulses[pulse_idx]= rx.rbuf[impulse_idx] & ~PULSE_BIT;
			pulse_idx++;
			pulseReceived=1;
		}
		else {

			/* first you must receive a pulse and then space ...*/
			if ( pulseReceived )
			{
				if ( space_idx > 7 ) space_idx=0;
				
				spaces[space_idx] = rx.rbuf[impulse_idx];
				space_idx++;
				pulseReceived=0;
			}
		}

		if ( pulse_idx != 9 ) continue;


		/* invalid train of impulse */
		if ( ( pulse_idx == 9 ) && (space_idx != 8) ) {
			DPRINTK("ERROR: Pulse received %d space received %d. Discard it.\n", pulse_idx, space_idx );
			space_idx=0;
			pulse_idx=0;		
			pulseReceived=0;
			continue;
		}
		
		space_idx=0;
		pulse_idx=0;		
		pulseReceived=0;

		/* convert to code */		
		button_value=0;
		for ( i=0; i<8; i++)
			 button_value |= ( ( decodeValue( spaces[i], pulses[i] )&0x0f) << ((7-i)*4) );

		/* calculate checksum */
		checksum = 0;
		for (c=0;c<=28;c+=4) checksum += ((button_value>>c)&0x0f);
		
		if ( !(checksum&0x0f) )
		{	
			/* checksum ok */
			if ((button_value & 0xfffff) == (IR_REMOTE_BIG&0xFFFFF))
			{
				*first_4=button_value;
				first_simbol = 1;
			}
			else if ( first_simbol )
			{
				*last_4 = button_value;
				first_simbol = 0;

				DPRINTK("[first: %08x] last: %08x\n", *first_4, *last_4 );

				return 1;
			}
			
		}
		else
		{	
				DPRINTK("Checksum error. Discard it.\n");
		}
	}

	return 0;
}



unsigned int QBOXONE_TranslateIRButton( unsigned int first_8, unsigned int button_code)
{
	if ( first_8 == IR_REMOTE_LITTLE )
	{
/*
		switch( button_code )
		{
			case 0x50: return 0x51;                        //translate button: <
			case 0x51: return 0x50;						   //translate button: >
		//	case 0x54: return 0x27;						   //translate button "help" to button "video"
		}
*/
	}

	return button_code;
}


static void QBOXONE_RC_manageIRButton( unsigned int first_4, unsigned int last_4)
{
	unsigned int button_code, valid_code=0;
	static unsigned char last_button_code=0, my_last_button_pressed=0;
	
	do
	{

		/* Verify if first_4 is code of QBOX rc model */
		if ( ( first_4 & IR_BUTTON_MODEL_MASK ) != ( IR_BUTTON_MODEL & IR_BUTTON_MODEL_MASK )) break;
		
		power_button_pressed = 0;
	
	    /* get button code */
		button_code=(( last_4>>8) & 0xff );    
	 
	    /* translate button code to right code */
		button_code= QBOXONE_TranslateIRButton(first_4, button_code);
	
		if (( last_4&0x00900000) == 0x00900000 )
		{	
			/*Forget last pressed button */
			my_last_button_pressed=0;
			/* Button released */
			duodvb_rc_buttonup(RC_QBOXONE, button_code);
		}
		else
		{
			if (first_4 == IR_REMOTE_LITTLE )
			{
				if ( last_4  & 0x00800000 )
				{
					/* only for power button: Button being pressed */
					if ( button_code == 0x0f ) 
						 duodvb_rc_buttonrepeat(RC_QBOXONE, button_code); 
					else 
						 duodvb_rc_buttonup(RC_QBOXONE, button_code); 
				}
				else duodvb_rc_buttondown(RC_QBOXONE, button_code); 
	
				if ( button_code == 0x0f ) power_button_pressed=1; 
			}
			else
			{
				if ( last_4  & 0x00800000 )	
					/* Button being pressed */
					duodvb_rc_buttonrepeat(RC_QBOXONE, button_code); 
				else 
					/* Begin pressing button */
					duodvb_rc_buttondown(RC_QBOXONE, button_code); 
			}

			/* Record button pression event */
			my_last_button_pressed=1;
			/* Record pressed button code */
			last_button_code=button_code;
		}
		
		// Valid button code received		
		valid_code = 1;
	
	} while(0);
	
	if ( ( first_4 & IR_BUTTON_MODEL_MASK ) == ( IR_BUTTON_MODEL & IR_BUTTON_MODEL_MASK ) ) 
	{	
		if (!valid_code && my_last_button_pressed)
		{
			/* Force last pressed button */
			my_last_button_pressed=0;      
			/* Force last received button release */
			duodvb_rc_buttonup(RC_QBOXONE, last_button_code);
		}
	}
}




#endif /* QBOXONE_RC_SUPPORT */


#ifdef QBOXHD_RC_SUPPORT 


unsigned int QBOXHD_TranslateIRButton( unsigned int button_code)
{

	switch( button_code )
	{
		case 0xA1: return 0x44;                        // MUTE
		case 0xA0: return 0x0F;                        // POWER

		case 0xa6: return 0x50;                        // <
		case 0xa7: return 0x51;                        // >

		case 0xac: return 0x28;                        // INFO
		case 0xad: return 0x20;                        // MENU
		case 0xb9: return 0x27;                        // HELP

		case 0xC1: return 0x21;                        // UP
		case 0xC5: return 0x22;                        // DOWN
		case 0xC2: return 0x23;                        // LEFT
		case 0xC4: return 0x24;                        // RIGHT

		case 0xC3: return 0x25;                        // OK
		case 0xA2: return 0x26;                        // AUDIO

		case 0xb6: return 0x52;                        // EXIT
		case 0xb8: return 0x45;                        // TEXT
		case 0xb7: return 0x0C;                        // TV
		case 0xae: return 0x53;                        // RADIO
		case 0xa4: return 0x54;                        // VIDEO
		case 0xB2: return 0x40;                        // RED
		case 0xB4: return 0x41;                        // GREEN
		case 0xB5: return 0x42;                        // YELLOW
		case 0xB3: return 0x43;                        // BLUE

	}
	return button_code;
}


static int QBOXHD_RC_traslate_impulse(unsigned char *repeat_button)
{
	unsigned int button_value=0;
	unsigned int impulse_idx=0;
	unsigned int pulse_idx=0;
	unsigned int space_idx=0;
	unsigned int total_pulses=0;
	unsigned int total_spaces=0;
	lirc_t pulses[34];
	lirc_t spaces[33];	
	int pulseReceived=0;
	unsigned char value=0;
	unsigned char checknot=0;

	*repeat_button=0;
		
	for ( impulse_idx=0; impulse_idx< ((2*rx.off_rbuf)-1); impulse_idx++ )
	{
		/* check if it is a PULSE */
		if (rx.rbuf[impulse_idx] & PULSE_BIT) {

			if ( pulse_idx > 33 ) pulse_idx=0;

			pulses[pulse_idx]= rx.rbuf[impulse_idx] & ~PULSE_BIT;
#ifdef SHOW_IMPULSES
			DPRINTK( "pulse time %d\n", rx.rbuf[impulse_idx] & ~PULSE_BIT);
#endif
			pulse_idx++;
			pulseReceived=1;
		}
		else {

			/* first you must receive a pulse and then space ...*/
			if ( pulseReceived )
			{
				if ( space_idx > 32 ) space_idx=0;
				
				spaces[space_idx] = rx.rbuf[impulse_idx];
#ifdef SHOW_IMPULSES
        		DPRINTK( "space time %d\n", rx.rbuf[impulse_idx] );
#endif
				space_idx++;
				pulseReceived=0;
			}
		}
	}

	/* invalid train of impulse */
	if ( pulse_idx != (space_idx+1) ) {
		DPRINTK("ERROR: Pulse received %d space received %d. Discard it.\n", pulse_idx, space_idx );
		return 0;
	}				

    total_pulses = pulse_idx;
	total_spaces = space_idx;

	/* check if first pulse and space are a sequence of start */
	if (!( ( pulses[0] >= QBOXHD_START_PULSES_MIN_LENGTH ) && ( spaces[0] >= QBOXHD_START_SPACE_MIN_LENGTH ) ))
	{
		DPRINTK("ERROR: Not a start sequence. Discard all impulse train.\n");
		return 0;
	}

	/* check if it is a repeat code */
	if ((total_pulses == 2) && (total_spaces == 1))
	{
		// Repeater code here ...
		DPRINTK( "repeat button_code\n");
		new_button_received += 1;
		*repeat_button=1;
		return 1;
	}

	
	/* decoding al impulses train */
	button_value = 0;
	for ( space_idx = 1; space_idx<total_spaces; space_idx++) {
		/* check timing for 0 bit */
		if ((spaces[space_idx] >= (QBOXHD_BIT_0_MIN_LENGTH-QBOXHD_BIT_TOLLERANCE)) && (spaces[space_idx] <= (QBOXHD_BIT_0_MIN_LENGTH+QBOXHD_BIT_TOLLERANCE)) ) {
			button_value |=  0 <<(space_idx-1);
		}
		/* check timing for 1 bit */
		else if ((spaces[space_idx] >= (QBOXHD_BIT_1_MIN_LENGTH-QBOXHD_BIT_TOLLERANCE)) && (spaces[space_idx] <= (QBOXHD_BIT_1_MIN_LENGTH+QBOXHD_BIT_TOLLERANCE)) ) {
			button_value |=  1<<(space_idx-1);
		}
		else {
				DPRINTK("ERROR: Space length not valid idx= %d [%d]. Discard all impulse train.\n", space_idx, spaces[space_idx]);
				return 0;
		}
	}	

	button_value = ((button_value & 0x000000ff)<<24) | ((button_value & 0x0000ff00)<<8) | ((button_value & 0x00ff0000)>>8) | ((button_value & 0xff000000)>>24);

	DPRINTK("Code: 0x%08x.\n", button_value);

	/* check manufacture id from code */
	if ( QBOXHD_IR_REMOTE != (button_value >> 16) ) {
			DPRINTK("ERROR: Unknown ID code [0x%04x]. Discard all impulse train.\n", (button_value >> 16));
			return 0;
	}
	
	/* check valid code */
	checknot = button_value & 0x000000FF;
	value = ((button_value & 0x0000FF00) >> 8);

	DPRINTK("Button Value: 0x%04x cheknot: 0x%04x.\n", value, checknot);

	if ( checknot != (unsigned char)(~value) ) {
			DPRINTK("ERROR: Checknot not valid. Discard all impulse train.\n");
			return 0;
	}

	last_button_pressed = QBOXHD_TranslateIRButton(value);

	new_button_received=0;

	return 1;
}


static void QBOXHD_RC_manageIRButton( unsigned char repeat_button )
{

	/* The first repeat code after button code is too fast. I discard it and i keep other one. */
	if ((new_button_received < 4) && repeat_button ) {
		return;
	}

	duodvb_rc_buttonup(RC_QBOXHD,prev_button_pressed);

    duodvb_rc_buttondown(RC_QBOXHD, last_button_pressed);
	/* if power pressed not send double code */
	if ( last_button_pressed != 0x0f )
	    duodvb_rc_buttondown(RC_QBOXHD, last_button_pressed);

	prev_button_pressed = last_button_pressed;
}


#endif /* QBOXHD_RC_SUPPORT  */




static void lirc_stm_rx_interrupt(int irq, void *dev_id)
{
	unsigned int symbol, mark = 0;
	int lastSymbol, clear_irq = 1;
	unsigned int first_4=0;
	unsigned int last_4=0;

	while (RX_WORDS_IN_FIFO()) {

		unsigned int total_pulses=0, total_spaces=0;
		unsigned char repeat_button=0;

		/* discard the entire collection in case of errors!  */
		if (unlikely(readl(IRB_RX_INT_STATUS) & LIRC_STM_IS_OVERRUN)) {
			printk(KERN_INFO "lirc_stm: IR RX overrun\n");
			writel(LIRC_STM_CLEAR_OVERRUN, IRB_RX_INT_CLEAR);
			rx.error = 1;
		}

		/* get the symbol times from FIFO */
		symbol = (readl(IRB_RX_SYS));
		mark = (readl(IRB_RX_ON));

		if (clear_irq) {
			/*  Clear the interrupt
			 * and leave only the overrun irq enabled */
			RX_CLEAR_IRQ(LIRC_STM_CLEAR_IRQ);
			writel(0x07, IRB_RX_INT_EN);
			clear_irq = 0;
		}

		if (rx.off_rbuf >= LIRC_STM_MAX_SYMBOLS) {
			printk
			    ("lirc_stm: IR too many symbols (max %d)\n",
			     LIRC_STM_MAX_SYMBOLS);
			rx.error = 1;
		}

		/* now handle the data depending on error condition */
		if (rx.error) {
			/*  Try again */
			lirc_stm_reset_rx_data();
			continue;
		}

		if (symbol == 0xFFFF)
			lastSymbol = 1;
		else
			lastSymbol = 0;

		/* A sequence seems to start with a constant time symbol (1us)
		 * pulse and symbol time length, both of 1us. We ignore this.
		 */
		if ((mark > 2) && (symbol > 1)) {
			/* Make fine adjustments to timings */
			symbol -= mark;	/* to get space timing */
			symbol *= rx.symbol_mult;
			symbol /= rx.symbol_div;
			mark *= rx.pulse_mult;
			mark /= rx.pulse_div;

			/* The ST hardware returns the pulse time and the
			 * period, which is the pulse time + space time, so
			 * we need to subtract the pulse time from the period
			 * to get the space time.
			 * For a pulse in LIRC MODE2, we need to set the
			 * PULSE_BIT ON
			 */
			rx.rbuf[(rx.off_rbuf * 2)] = mark | PULSE_BIT;
			rx.rbuf[(rx.off_rbuf * 2) + 1] = symbol;
			rx.sumUs += mark + symbol;
			rx.off_rbuf++;

			if (lastSymbol) {
				/* move the entire collection into user
				 * buffer if enough space, drop otherwise
				 * (perhaps too crude a recovery?)
				 */


#ifndef DUOLABS
				if (likely(lirc_buffer_available
					   (&lirc_stm_rbuf) >=
					   (2 * rx.off_rbuf))) {
#endif
					struct timeval now;
					lirc_t syncSpace;

					DPRINTK("W symbols = %d\n",
						rx.off_rbuf);

					/*  Calculate and write the leading
					 *  space. All spaces and pulses
					 *  together sum up to the
					 *  microseconds elapsed since we
					 *  sent the previous block of data
					 */

					do_gettimeofday(&now);
					if (now.tv_sec - rx.sync.tv_sec < 0)
						syncSpace = 0;
					else if (now.tv_sec -
						 rx.sync.tv_sec >
						 PULSE_MASK / 1000000)
						syncSpace = PULSE_MASK;
					else {
						syncSpace =
						    (now.tv_sec -
						     rx.sync.tv_sec) *
						    1000000 +
						    (now.tv_usec -
						     rx.sync.tv_usec);
						syncSpace -=
						    (rx.sumUs -
						     rx.
						     rbuf[((rx.
							    off_rbuf -
							    1) * 2) + 1]);
						if (syncSpace < 0)
							syncSpace = 0;
						else if (syncSpace > PULSE_MASK)
							syncSpace = PULSE_MASK;
					}

					rx.sync = now;

#ifdef DUOLABS
					{
						unsigned int impulse_idx;

						/* count number of pulses in this train of impulse */
						for ( impulse_idx=0; impulse_idx< ((2*rx.off_rbuf)-1); impulse_idx++ )
							if ( rx.rbuf[impulse_idx] & PULSE_BIT ) {
#ifdef SHOW_IMPULSES
								printk("|");
#endif /*SHOW_IMPULSES*/
								total_pulses++;
							}
							else
							{
#ifdef SHOW_IMPULSES
								printk("_");
#endif /*SHOW_IMPULSES*/
								total_spaces++;
							}
#ifdef SHOW_IMPULSES
						printk(" ");
						printk("total_pulses: %04d total_spaces %04d\n", total_pulses, total_spaces);
						printk("\n");

#endif /*SHOW_IMPULSES*/
					}
#ifdef QBOXONE_RC_SUPPORT
					/* decoding impulse */					
					if ( QBOXONE_RC_traslate_impulse_to_8Bytes(total_pulses, &first_4, &last_4) )
						QBOXONE_RC_manageIRButton( first_4, last_4 );
#endif /*QBOXONE_RC_SUPPORT*/

#ifdef QBOXHD_RC_SUPPORT
					/* decoding impulse */		
								
					if ( QBOXHD_RC_traslate_impulse(&repeat_button) )
						QBOXHD_RC_manageIRButton( repeat_button );
#endif /*QBOXHD_RC_SUPPORT*/

#else /*DUOLABS*/					

					lirc_buffer_write_1
					    (&lirc_stm_rbuf, (unsigned char *)
					     &syncSpace);
					rx.sync = now;

					/*  Now write the pulse / space pairs
					 *  EXCEPT FOR THE LAST SPACE
					 *  The last space value should be
					 *  0xFFFF to denote a timeout
					 */
					lirc_buffer_write_n
					    (&lirc_stm_rbuf,
					     (unsigned char *)rx.rbuf,
					     (2 * rx.off_rbuf) - 1);

#endif /*DUOLABS*/					

					wake_up_interruptible
					    (&lirc_stm_rbuf.wait_poll);
#ifndef DUOLABS
				} 
				else
				{
					printk(KERN_ERR
					       "lirc_stm: not enough space "
					       "in user buffer\n");
				}
#endif
				lirc_stm_reset_rx_data();
			}
		}
	}			/* while */

	RX_CLEAR_IRQ(LIRC_STM_CLEAR_IRQ | 0x02);
	writel(LIRC_STM_ENABLE_IRQ, IRB_RX_INT_EN);
}

static irqreturn_t lirc_stm_interrupt(int irq, void *dev_id)
{
	lirc_stm_tx_interrupt(irq, dev_id);
	lirc_stm_rx_interrupt(irq, dev_id);

	return IRQ_HANDLED;
}

static int lirc_stm_open_inc(void *data)
{
	lirc_stm_plugin_data_t *lpd = (lirc_stm_plugin_data_t *) data;
	DPRINTK("entering open\n");

	if (lpd->open_count++ == 0) {
		unsigned long flags;
		DPRINTK("plugin enabled\n");
		local_irq_save(flags);

		/* enable interrupts and receiver */
		writel(LIRC_STM_ENABLE_IRQ, IRB_RX_INT_EN);
		writel(0x01, IRB_RX_EN);
		lirc_stm_reset_rx_data();
		local_irq_restore(flags);
	} else
		DPRINTK("plugin already open\n");

	return 0;
}

static void lirc_stm_flush_rx(void)
{
	/* Disable receiver */
	writel(0x00, IRB_RX_EN);
	/* Disable interrupt */
	writel(0x20, IRB_RX_INT_EN);
	/* clean the buffer */
	lirc_stm_reset_rx_data();
}

/*
** Called by lirc_dev as a last action on a real close
*/
static void lirc_stm_close_dec(void *data)
{
	lirc_stm_plugin_data_t *lpd = (lirc_stm_plugin_data_t *) data;
	DPRINTK("entering close\n");

	/* The last close disable the receiver */
	if (--lpd->open_count == 0)
		lirc_stm_flush_rx();
}

static int lirc_stm_ioctl(struct inode *node, struct file *filep,
			  unsigned int cmd, unsigned long arg)
{
	int retval = 0;
	unsigned long value = 0;
	char *msg = "";

	switch (cmd) {
	case LIRC_GET_FEATURES:
		/*
		 * Our driver can receive in mode2 and send in pulse mode.
		 * TODO: We can generate our own carrier freq
		 *      (LIRC_CAN_SET_SEND_CARRIER) and also change duty
		 *      cycle (LIRC_CAN_SET_SEND_DUTY_CYCLE)
		 */
		DPRINTK
		    ("LIRC_GET_FEATURES return REC_MODE2|SEND_PULSE\n");
		retval =
		    put_user(LIRC_CAN_REC_MODE2 | LIRC_CAN_SEND_PULSE,
			     (unsigned long *)arg);
		break;

	case LIRC_GET_REC_MODE:
		DPRINTK("LIRC_GET_REC_MODE return LIRC_MODE_MODE2\n");
		retval = put_user(LIRC_MODE_MODE2, (unsigned long *)arg);
		break;

	case LIRC_SET_REC_MODE:
		retval = get_user(value, (unsigned long *)arg);
		DPRINTK("LIRC_SET_REC_MODE to 0x%lx\n", value);
		if (value != LIRC_MODE_MODE2)
			retval = -ENOSYS;
		break;

	case LIRC_GET_SEND_MODE:
		DPRINTK
		    ("LIRC_GET_SEND_MODE return LIRC_MODE_PULSE\n");
		retval = put_user(LIRC_MODE_PULSE, (unsigned long *)arg);
		break;

	case LIRC_SET_SEND_MODE:
		retval = get_user(value, (unsigned long *)arg);
		DPRINTK("LIRC_SET_SEND_MODE to 0x%lx\n", value);
		/* only LIRC_MODE_PULSE supported */
		if (value != LIRC_MODE_PULSE)
			return (-ENOSYS);
		break;

	case LIRC_GET_REC_RESOLUTION:
		msg = "LIRC_GET_REC_RESOLUTION";
		goto _not_supported;

	case LIRC_GET_REC_CARRIER:
		msg = "LIRC_GET_REC_CARRIER";
		goto _not_supported;

	case LIRC_SET_REC_CARRIER:
		msg = "LIRC_SET_REC_CARRIER";
		goto _not_supported;

	case LIRC_GET_SEND_CARRIER:
		msg = "LIRC_GET_SEND_CARRIER";
		goto _not_supported;

	case LIRC_SET_SEND_CARRIER:
		msg = "LIRC_SET_SEND_CARRIER";
		goto _not_supported;

	case LIRC_GET_REC_DUTY_CYCLE:
		msg = "LIRC_GET_REC_DUTY_CYCLE";
		goto _not_supported;

	case LIRC_SET_REC_DUTY_CYCLE:
		msg = "LIRC_SET_REC_DUTY_CYCLE";
		goto _not_supported;

	case LIRC_GET_SEND_DUTY_CYCLE:
		msg = "LIRC_GET_SEND_DUTY_CYCLE";
		goto _not_supported;

	case LIRC_SET_SEND_DUTY_CYCLE:
		msg = "LIRC_SET_SEND_DUTY_CYCLE";
		goto _not_supported;

	case LIRC_GET_LENGTH:
		msg = "LIRC_GET_LENGTH";
		goto _not_supported;

	default:
		msg = "???";
	      _not_supported:
		DPRINTK("command %s (0x%x) not supported\n", msg, cmd);
		retval = -ENOIOCTLCMD;
	}

	return retval;
}


#ifdef DUOLABS

static ssize_t lirc_stm_read(struct file *file, char *buf, size_t n, loff_t * ppos)
{
	unsigned char mybuf[lirc_stm_rbuf.chunk_size];
	int ret=0, written=0;
	DECLARE_WAITQUEUE(wait, current);

	/* we add ourselves to the task queue before buffer check 
     * to avoid losing scan code (in case when queue is awaken somewhere 
	 * beetwen while condition checking and scheduling)
	 */
	add_wait_queue(&lirc_stm_rbuf.wait_poll, &wait);
	set_current_state(TASK_INTERRUPTIBLE);

	while (written < n && ret == 0) { 
		if (lirc_buffer_empty(&lirc_stm_rbuf)) {
			/* According to the read(2) man page, 'written' can be
			 * returned as less than 'length', instead of blocking
			 * again, returning -EWOULDBLOCK, or returning
			 * -ERESTARTSYS */
			if (written) break;
			if (file->f_flags & O_NONBLOCK) {
				ret = -EWOULDBLOCK;
				break;
			}
			if (signal_pending(current)) {
				ret = -ERESTARTSYS;
				break;
			}
			schedule();
			set_current_state(TASK_INTERRUPTIBLE);
		} else {
			lirc_buffer_read_1(&lirc_stm_rbuf, mybuf);
			ret = copy_to_user((void *)buf+written, mybuf,
					   lirc_stm_rbuf.chunk_size);
			written += lirc_stm_rbuf.chunk_size;

		}
	}


	remove_wait_queue(&lirc_stm_rbuf.wait_poll, &wait);
	set_current_state(TASK_RUNNING);
	
	return ret ? ret : written;
}

#endif /* DUOLABS */

static ssize_t lirc_stm_write(struct file *file, const char *buf,
			      size_t n, loff_t * ppos)
{
	int i;
	size_t rdn = n / sizeof(size_t);
	unsigned int symbol, mark;
	int fifosyms, num_pio_pins;
	struct lirc_pio *p;

	num_pio_pins = pd.p_lirc_d->num_pio_pins;
	while (num_pio_pins > 0) {
		p = &(pd.p_lirc_d->pio_pin_arr[num_pio_pins - 1]);
		if (!(p->pinof ^ (LIRC_IR_TX | LIRC_PIO_ON)))
			break;
		else
			num_pio_pins--;
	}
	if (!num_pio_pins) {
		printk(KERN_ERR "lirc_stm: write operation unsupported.\n");
		return -ENOTSUPP;
	}

	if (n % sizeof(lirc_t))
		return -EINVAL;

	if (tx.off_wbuf != 0 && (file->f_flags & O_NONBLOCK))
		return -EAGAIN;

	/* Wait for transmit to become free... */
	if (wait_event_interruptible(tx.waitq, tx.off_wbuf == 0))
		return -ERESTARTSYS;

	/* Prevent against buffer overflow... */
	if (rdn > LIRC_STM_MAX_SYMBOLS)
		rdn = LIRC_STM_MAX_SYMBOLS;

	n -= rdn * sizeof(size_t);

	if (copy_from_user((char *)tx.wbuf, buf, rdn * sizeof(size_t))) {
		return -EFAULT;
	}

	if (n == 0)
		tx.wbuf[rdn - 1] = 0xFFFF;

	/* load the first words into the FIFO */
	fifosyms = rdn;

	if (fifosyms > TX_FIFO_DEPTH)
		fifosyms = TX_FIFO_DEPTH;

	for (i = 0; i < fifosyms; i++) {
		mark = tx.wbuf[(i * 2)];
		symbol = mark + tx.wbuf[(i * 2) + 1];
		DPRINTK("TX raw m %d s %d ", mark, symbol);

		mark = lirc_stm_time_to_cycles(mark) + 1;
		symbol = lirc_stm_time_to_cycles(symbol) + 2;
		DPRINTK("cal m %d s %d\n", mark, symbol);

		tx.off_wbuf++;
		writel(mark, IRB_TX_ONTIME);
		writel(symbol, IRB_TX_SYMPERIOD);
	}

	/* enable the transmit */
	writel(0x07, IRB_TX_INT_ENABLE);
	writel(0x01, IRB_TX_ENABLE);
	DPRINTK("TX enabled\n");

	return n;
}

static void lirc_stm_calc_tx_clocks(unsigned int clockfreq,
				    unsigned int carrierfreq,
				    unsigned int subwidthpercent)
{
	/*  We know the system base clock and the required IR carrier frequency
	 *  We now want a divisor of the system base clock that gives the nearest
	 *  integer multiple of the carrier frequency
	 */

	const unsigned int clkratio = clockfreq / carrierfreq;
	unsigned int scalar, n;
	int delta;
	unsigned int diffbest = clockfreq, nbest = 0, scalarbest = 0;
	unsigned int nmin = clkratio / 255;

	if ((nmin & 0x01) == 1)
		nmin++;

	for (n = nmin; n < clkratio; n += 2) {
		scalar = clkratio / n;
		if ((scalar & 0x01) == 0 && scalar != 0) {
			delta = clockfreq - (scalar * carrierfreq * n);
			if (delta < 0)
				delta *= -1;

			if (delta < diffbest) {	/* better set of parameters ? */
				diffbest = delta;
				nbest = n;
				scalarbest = scalar;
			}
			if (delta == 0)	/* an exact multiple */
				break;
		}
	}

	scalarbest /= 2;
	nbest *= 2;

	DPRINTK("TX clock scalar = %d\n", scalarbest);
	DPRINTK("TX subcarrier scalar = %d\n", nbest);

	/*  Set the registers now  */

	writel(scalarbest, IRB_TX_PRESCALAR);
	writel(nbest, IRB_TX_SUBCARRIER);
	writel(nbest * subwidthpercent / 100, IRB_TX_SUBCARRIER_WIDTH);

	/*  Now calculate timing to subcarrier cycles factors which compensate
	 *  for any remaining difference between our clock ratios and real times
	 *  in microseconds
	 */

	if (diffbest == 0) {
		/* no adjustment required - our clock is running at the required
		 * speed */
		tx.mult = 1;
		tx.div = 1;
	} else {
		/* adjustment is required */
		delta = scalarbest * carrierfreq * nbest;
		tx.mult = delta / (clockfreq / 10000);

		if (delta < clockfreq) {	/* our clock is running too fast */
			DPRINTK("clock running slow at %d\n", delta);
			tx.div = tx.mult;
			tx.mult = 10000;
		} else {	/* our clock is running too slow */

			DPRINTK("clock running fast at %d\n", delta);
			tx.div = 10000;
		}
	}

	DPRINTK("TX fine adjustment mult = %d\n", tx.mult);
	DPRINTK("TX fine adjustment div  = %d\n", tx.div);
}

static int lirc_stm_hardware_init(struct platform_device *pdev)
{
	struct plat_lirc_data *lirc_private_data = NULL;
	struct clk *clk;
	int baseclock;
	unsigned int scwidth;
	unsigned int rx_max_symbol_per;

	/*  set up the hardware version dependent setup parameters */
	lirc_private_data = (struct plat_lirc_data *)pdev->dev.platform_data;

	tx.carrier_freq = 38000;	// in Hz

	/* Set the polarity inversion bit to the correct state */
	writel(lirc_private_data->rxpolarity, IRB_RX_POLARITY_INV);

	/*  Get or calculate the clock and timing adjustment values.
	 *  We can auto-calculate these in some cases
	 */

	if (lirc_private_data->irbclock == 0) {
		clk = clk_get(NULL, "comms_clk");
		baseclock = clk_get_rate(clk) / lirc_private_data->sysclkdiv;
	} else
		baseclock = lirc_private_data->irbclock;

	if (lirc_private_data->irbclkdiv == 0) {
		/* Auto-calculate clock divisor */

		int freqdiff;

		rx.sampling_freq_div = baseclock / 10000000;

		/* Work out the timing adjustment factors */
		freqdiff = baseclock - (rx.sampling_freq_div * 10000000);

		/* freqdiff contains the difference between our clock and a
		 * true 10 MHz clock which the IR block wants
		 */

		if (freqdiff == 0) {
			/* no adjustment required - our clock is running at the
			 * required speed
			 */
			rx.symbol_mult = 1;
			rx.pulse_mult = 1;
			rx.symbol_div = 1;
			rx.pulse_div = 1;
		} else {
			/* adjustment is required */
			rx.symbol_mult =
			    baseclock / (10000 * rx.sampling_freq_div);

			if (freqdiff > 0) {
				/* our clock is running too fast */
				rx.pulse_mult = 1000;
				rx.pulse_div = rx.symbol_mult;
				rx.symbol_mult = rx.pulse_mult;
				rx.symbol_div = rx.pulse_div;
			} else {
				/* our clock is running too slow */
				rx.symbol_div = 1000;
				rx.pulse_mult = rx.symbol_mult;
				rx.pulse_div = 1000;
			}

		}

	} else {
		rx.sampling_freq_div = (lirc_private_data->irbclkdiv);
		rx.symbol_mult = (lirc_private_data->irbperiodmult);
		rx.symbol_div = (lirc_private_data->irbperioddiv);
		rx.pulse_mult = (lirc_private_data->irbontimemult);
		rx.pulse_div = (lirc_private_data->irbontimediv);
	}

	writel(rx.sampling_freq_div, IRB_RX_RATE_COMMON);
	DPRINTK("IR clock is %d\n", baseclock);
	DPRINTK("IR clock divisor is %d\n", rx.sampling_freq_div);
	DPRINTK("IR clock divisor readlack is %d\n",
		readl(IRB_RX_RATE_COMMON));
	DPRINTK("IR period mult factor is %d\n", rx.symbol_mult);
	DPRINTK("IR period divisor factor is %d\n", rx.symbol_div);
	DPRINTK("IR pulse mult factor is %d\n", rx.pulse_mult);
	DPRINTK("IR pulse divisor factor is %d\n", rx.pulse_div);

	/* maximum symbol period.
	 * Symbol periods longer than this will generate
	 * an interrupt and terminate a command
	 */
	if ((lirc_private_data->irbrxmaxperiod) != 0)
		rx_max_symbol_per =
		    (lirc_private_data->irbrxmaxperiod) *
		    rx.symbol_mult / rx.symbol_div;
	else
		rx_max_symbol_per = 0;

	DPRINTK("RX Maximum symbol period register 0x%x\n",
		rx_max_symbol_per);
	writel(rx_max_symbol_per, IRB_MAX_SYM_PERIOD);

	/*  Set up the transmit timings  */
	if (lirc_private_data->subcarrwidth != 0)
		scwidth = lirc_private_data->subcarrwidth;
	else
		scwidth = 50;

	if (scwidth > 100)
		scwidth = 50;

	DPRINTK("subcarrier width set to %d %%\n", scwidth);
	lirc_stm_calc_tx_clocks(baseclock, tx.carrier_freq, scwidth);

	return 0;
}

static int lirc_stm_remove(struct platform_device *pdev)
{
	DPRINTK("lirc_stm_remove called\n");
	return 0;
}

static int lirc_stm_probe(struct platform_device *pdev)
{
	int ret = -EINVAL;
	int num_pio_pins;
	struct lirc_pio *p;
	struct device *dev = &pdev->dev;
	struct resource *res;

	if (pdev->name == NULL) {
		printk(KERN_ERR
		       "lirc_stm: probe failed. Check kernel SoC config.\n");
		return -ENODEV;
	}

	printk(KERN_INFO
	       "lirc_stm: probe found data for platform device %s\n",
	       pdev->name);
	pd.p_lirc_d = (struct plat_lirc_data *)pdev->dev.platform_data;

	if ((irb_irq = platform_get_irq(pdev, 0)) == 0) {
		printk(KERN_ERR "lirc_stm: IRQ configuration not found\n");
		return -ENODEV;
	}

	if (devm_request_irq(dev, irb_irq, lirc_stm_interrupt, IRQF_DISABLED,
			     LIRC_STM_NAME, (void *)&pd) < 0) {
		printk(KERN_ERR "lirc_stm: IRQ register failed\n");
		return -EIO;
	}

	/* Configure for ir or uhf. uhf_switch==1 is UHF */
	if (uhf_switch)
		ir_or_uhf_offset = 0x40;
	else
		ir_or_uhf_offset = 0x00;

	/* Hardware IR block setup - the PIO ports should already be set up
	 * in the board-dependent configuration.  We need to remap the
	 * IR registers into kernel space - we do this in one chunk
	 */

	if ((rx.rbuf = (lirc_t *) devm_kzalloc(dev,
					       LIRC_STM_BUFSIZE,
					       GFP_KERNEL)) == NULL)
		return -ENOMEM;

	if ((tx.wbuf = (lirc_t *) devm_kzalloc(dev,
					       LIRC_STM_BUFSIZE,
					       GFP_KERNEL)) == NULL)
		return -ENOMEM;

	memset(rx.rbuf, 0, LIRC_STM_BUFSIZE);
	memset(tx.wbuf, 0, LIRC_STM_BUFSIZE);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		printk(KERN_ERR "lirc_stm: IO MEM not found\n");
		return -ENODEV;
	}

	if (!devm_request_mem_region(dev, res->start,
				     res->end - res->start, LIRC_STM_NAME)) {
		printk(KERN_ERR "lirc_stm: request_mem_region failed\n");
		return -EBUSY;
	}

	irb_base_address =
	    devm_ioremap_nocache(dev, res->start, res->end - res->start);

	if (irb_base_address == NULL) {
		printk(KERN_ERR "lirc_stm: ioremap failed\n");
		ret = -ENOMEM;
	} else {
		DPRINTK("ioremapped register block at 0x%x\n",
			res->start);
		DPRINTK("ioremapped to 0x%x\n",
			(unsigned int)irb_base_address);

		printk(KERN_INFO "lirc_stm: STM LIRC plugin has IRQ %d",
		       irb_irq);

		/* Allocate the PIO pins */
		num_pio_pins = pd.p_lirc_d->num_pio_pins;
		while (num_pio_pins > 0) {
			ret = 0;
			p = &(pd.p_lirc_d->pio_pin_arr[num_pio_pins - 1]);
			/* Can I satisfy the IR-RX request ? */
			if ((ir_or_uhf_offset == 0x00)
			    && (p->pinof & LIRC_IR_RX)) {
				if (p->pinof & LIRC_PIO_ON)
					printk(" using IR-RX mode\n");
				else {
					printk
					    (" with IR-RX mode unsupported\n");
					ret = -1;
				}
			}

			/* Can I satisfy the UHF-RX request ? */
			if ((ir_or_uhf_offset == 0x40)
			    && (p->pinof & LIRC_UHF_RX)) {
				if (p->pinof & LIRC_PIO_ON)
					printk(" using UHF-RX mode\n");
				else {
					printk
					    (" with UHF-RX mode unsupported\n");
					ret = -1;
				}
			}

			/* Try to satisfy the request */
			if ((!ret) && (p->pinof & LIRC_PIO_ON))
				if (!(p->pinaddr = stpio_request_pin(p->bank,
								     p->pin,
								     LIRC_STM_NAME,
								     p->dir))) {
					printk(KERN_ERR
					       "\nlirc_stm: PIO[%d,%d] request failed\n",
					       p->bank, p->pin);
					ret = -1;

				}

			/* something bad is happened */
			if (ret) {
				while (num_pio_pins < pd.p_lirc_d->num_pio_pins) {
					stpio_free_pin(pd.p_lirc_d->
						       pio_pin_arr
						       [num_pio_pins].pinaddr);
					pd.p_lirc_d->pio_pin_arr[num_pio_pins].
					    pinaddr = NULL;
					num_pio_pins++;
				}
				return -EIO;
			}

			num_pio_pins--;
		}

		/* reset and then harware initialisation */
		init_waitqueue_head(&tx.waitq);
		/* enable signal detection */
		ret = lirc_stm_hardware_init(pdev);

		if (!ret)
			printk(KERN_INFO
		       		"STMicroelectronics LIRC driver configured.\n");
	}

	return ret;
}

static struct platform_driver lirc_device_driver = {
	.driver.name = "lirc",
	.probe = lirc_stm_probe,
	.remove = lirc_stm_remove,
};

static struct file_operations lirc_stm_fops = {
      .write=lirc_stm_write,
#ifdef DUOLABS
      .read=lirc_stm_read,
#endif
};

static struct lirc_plugin lirc_stm_plugin = {
	.name = LIRC_STM_NAME,
	.minor = LIRC_STM_MINOR,
	.code_length = 1,
	.sample_rate = 0,
	/* plugin can receive raw pulse and space timings for each symbol */
	.features = LIRC_CAN_REC_MODE2,
	/* plugin private data  */
	.data = (void *)&pd,
	/* buffer handled by upper layer */
	.add_to_buf = NULL,
	.get_queue = NULL,
	.set_use_inc = lirc_stm_open_inc,
	.set_use_dec = lirc_stm_close_dec,
	.ioctl = lirc_stm_ioctl,
	.fops = &lirc_stm_fops,
	.rbuf = &lirc_stm_rbuf,
	.owner = THIS_MODULE,
};

static int __init lirc_stm_init(void)
{
	DPRINTK("initializing the IR receiver...\n");

	/* inform the top level driver that we use our own user buffer */
#ifdef DUOLABS
	if (lirc_buffer_init(&lirc_stm_rbuf, sizeof(lirc_t),
				 (sizeof(lirc_t) * LIRC_STM_FIFO_READ_MAX_SYMBOLS)))
#else
	if (lirc_buffer_init(&lirc_stm_rbuf, sizeof(lirc_t),
			     (2 * LIRC_STM_MAX_SYMBOLS)))
#endif
	{
		printk(KERN_ERR "lirc_stm: buffer init failed\n");
		return -EINVAL;
	}

	request_module("lirc_dev");
	if (lirc_register_plugin(&lirc_stm_plugin) < 0) {
		printk(KERN_ERR "lirc_stm: plugin registration failed\n");
		lirc_buffer_free(&lirc_stm_rbuf);
		return -EINVAL;
	}

	if (platform_driver_register(&lirc_device_driver)) {
		printk(KERN_ERR "lirc_stm: platform driver register failed\n");
		lirc_buffer_free(&lirc_stm_rbuf);
		lirc_unregister_plugin(LIRC_STM_MINOR);
		return -EINVAL;
	}
	return 0;
}

void __exit lirc_stm_release(void)
{
	int num_pio_pins;

	DPRINTK("removing STM lirc plugin\n");

	/* unregister the plugin */
	lirc_stm_flush_rx();
	platform_driver_unregister(&lirc_device_driver);

	/* unplug the lirc stm driver */
	if (lirc_unregister_plugin(LIRC_STM_MINOR) < 0)
		printk(KERN_ERR "lirc_stm: plugin unregister failed\n");
	/* free buffer */
	lirc_buffer_free(&lirc_stm_rbuf);

	/* deallocate the STPIO pins */
	num_pio_pins = pd.p_lirc_d->num_pio_pins;
	while (num_pio_pins > 0)
		if (pd.p_lirc_d->pio_pin_arr[num_pio_pins - 1].pinaddr)
			stpio_free_pin(pd.p_lirc_d->pio_pin_arr[--num_pio_pins].
				       pinaddr);
		else
			--num_pio_pins;

	printk(KERN_INFO "STMicroelectronics LIRC driver removed\n");
}

module_param(uhf_switch, bool, 0664);
MODULE_PARM_DESC(ir_or_uhf_offset, "Enable uhf mode");

module_init(lirc_stm_init);
module_exit(lirc_stm_release);
MODULE_DESCRIPTION
    ("Linux InfraRed receiver plugin for STMicroelectronics platforms");
MODULE_AUTHOR("Carl Shaw <carl.shaw@st.com>");
MODULE_AUTHOR("Angelo Castello <angelo.castello@st.com>");
MODULE_LICENSE("GPL");
