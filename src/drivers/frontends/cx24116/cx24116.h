/*
    Conexant cx24116/cx24118 - DVBS/S2 Satellite demod/tuner driver

    Copyright (C) 2006 Steven Toth <stoth@linuxtv.com>

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

#ifndef CX24116_H
#define CX24116_H

#include <linux/dvb/frontend.h>

#if defined (CONFIG_KERNELVERSION)
#include <linux/stm/pio.h>
#else /* STLinux 2.2 kernel */
#include <linux/stpio.h>
#endif

#include "dvb_frontend.h"


struct cx24116_config {
	/* the demodulator's i2c address */
	u8 demod_address;

	/* Need to set device param for start_dma */
	int (*set_ts_params)(struct dvb_frontend *fe, int is_punctured);

	/* Need to reset device during firmware loading */
	int (*reset_device)(struct dvb_frontend *fe);

	/* Need to set MPEG parameters */
	u8 mpg_clk_pos_pol:0x02;

    struct i2c_adapter  *i2c_adap; /* i2c bus of the tuner */
    u8          i2c_addr; /* i2c address of the tuner */
    struct stpio_pin*   tuner_enable_pin;
    struct stpio_pin*   lnb_enable_pin;
    struct stpio_pin*   lnb_vsel_pin;
    u8          tuner_enable_act; /* active state of the pin */
    u8          lnb_enable_act; /* active state of the pin */
    u8          lnb_vsel_act; /* active state of the pin */
};

extern struct dvb_frontend* cx24116_attach(const struct cx24116_config* config,
										   struct i2c_adapter* i2c);
int cx24116_set_tone(struct dvb_frontend* fe, fe_sec_tone_mode_t tone);
#if defined(CONFIG_SH_QBOXHD_1_0) || defined(CONFIG_SH_QBOXHD_MINI_1_0)
int cx24116_set_voltage(struct dvb_frontend* fe, fe_sec_voltage_t voltage);
#endif

#endif /* CX24116_H */
