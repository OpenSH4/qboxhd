/*
    Avilink avl2108 - DVBS/S2 Satellite demod driver with Sharp BS2S7HZ6360 tuner

    Copyright (C) 2009-2010 Duolabs Spa

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef _AVL2108_H
#define _AVL2108_H

#include <linux/dvb/frontend.h>

#if defined (CONFIG_KERNELVERSION)
#include <linux/stm/pio.h>
#else /* STLinux 2.2 kernel */
#include <linux/stpio.h>
#endif

#include "dvb_frontend.h"

#define eprintk(args...)  do {      \
	printk("avl2108: ERROR: " args);   \
} while (0)

struct avl2108_pllconf
{
	u16 m_r1;		/*< PLL register r1 */
	u16 m_r2;		/*< PLL register r2 */
	u16 m_r3;		/*< PLL register r3 */
	u16 m_r4;		/*< PLL register r4 */
	u16 m_r5;		/*< PLL register r5 */
	u16 ref_freq;	/*< Reference clock in kHz */
	u16 demod_freq;	/*< Demod clock in 10kHz */
	u16 fec_freq;	/*< FEC clock in 10kHz */
	u16 mpeg_freq;	/*< MPEG clock in 10kHz */
};

const struct avl2108_pllconf pll_conf[] = 
{
	/* For all parallel modes and all serial modes below 30M (symbol rate) */
	{10, 0, 335, 6, 5, 4000, 11200, 16800, 19200},	/*< Reference clock 4M;   --> 112, 168, 192 */
	{10, 0, 299, 6,5, 4500, 11250, 16875, 19286},	/*< Reference clock 4.5M; --> 112.5, 168.75, 192.86 */
	{10, 1, 269, 6, 5, 10000, 11250, 16875, 19286},	/*< Reference clock 10M;  --> 112.5, 168.75, 192.86 */
	{10, 0, 83, 6, 5, 16000, 11200, 16800, 19200},	/*< Reference clock 16M;  --> 112, 168, 192 */
	{10, 0, 49, 6,5, 27000, 11250, 16875, 19286},	/*< Reference clock 27M;  --> 112.5, 168.75, 192.86 */
	
	/* for all modes */
	{10, 0, 335, 6, 4, 4000, 11200, 16800, 22400},	/*< Reference clock 4M;   --> 112, 168, 224 */
	{10, 0, 299, 6,4, 4500, 11250, 16875, 22500},	/*< Reference clock 4.5M; --> 112.5, 168.75, 225 */
	{10, 1, 269, 6, 4, 10000, 11250, 16875, 22500},	/*< Reference clock 10M;  --> 112.5, 168.75, 225 */
	{10, 0, 83, 6, 4, 16000, 11200, 16800, 22400},	/*< Reference clock 16M;  --> 112, 168, 224 */
	{10, 0, 49, 6,4, 27000, 11250, 16875, 22500}	/*< Reference clock 27M;  --> 112.5, 168.75, 225 */
};

const unsigned short pll_array_size = sizeof(pll_conf) / sizeof(struct avl2108_pllconf);

/*struct Signal_Level*/
/*{*/
/*u16 SignalLevel;*/
/*short SignalDBM;*/
/*};*/

/*struct Signal_Level  SignalLevel[47] =*/
/*{*/
/*{8285,	-922},{10224, -902},{12538,	-882},{14890, -862},{17343,	-842},{19767, -822},{22178,	-802},{24618, -782},{27006,	-762},{29106, -742},*/
/*{30853,	-722},{32289, -702},{33577,	-682},{34625, -662},{35632,	-642},{36552, -622},{37467,	-602},{38520, -582},{39643,	-562},{40972, -542},*/
/*{42351,	-522},{43659, -502},{44812,	-482},{45811, -462},{46703,	-442},{47501, -422},{48331,	-402},{49116, -382},{49894,	-362},{50684, -342},*/
/*{51543,	-322},{52442, -302},{53407,	-282},{54314, -262},{55208,	-242},{56000, -222},{56789,	-202},{57544, -182},{58253,	-162},{58959, -142},*/
/*{59657,	-122},{60404, -102},{61181,	 -82},{62008,  -62},{63032,	 -42},{65483,  -22},{65535,	-12}*/
/*};*/

struct avl2108_config
{
	u8 demod_address; /*< the demodulator's i2c address */
	u8 tuner_address; /*< the tuner's i2c address */

	int (*set_ts_params)(struct dvb_frontend* fe, int is_punctured); /*< Need to set device param for start_dma */

	int (*reset_device)(struct dvb_frontend* fe); /* Need to reset device during firmware loading */

	u16 ref_freq;	/*< Reference clock in kHz units */
	u16 demod_freq;	/*< Demod clock in 10kHz units */
	u16 fec_freq;	/*< FEC clock in 10kHz units */
	u16 mpeg_freq;	/*< MPEG clock in 10kHz units */
};

struct dvb_frontend* avl2108_attach(const struct avl2108_config* config,
                                           struct i2c_adapter* i2c);
int avl2108_set_tone(struct dvb_frontend* fe, fe_sec_tone_mode_t tone);
int avl2108_set_voltage(struct dvb_frontend* fe, fe_sec_voltage_t voltage);

#endif /* _AVL2108_H */
