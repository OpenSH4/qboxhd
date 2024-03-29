/*
 * (C) Copyright 2004 STMicroelectronics.
 *
 * Andy Sturges <andy.sturges@st.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>

#include <asm/hardware.h>

#define TMU_TICKS_PER_SEC CFG_HZ
#define TMU_START0 0x01
#define TMU_MAX_COUNTER (~0UL)
#define TMU_INPUT 0x0006

#define TMU_OFF() *TMU_TSTR = 0;
#define TMU_ON() *TMU_TSTR = *TMU_TSTR | 1;
#define TMU_CLEAR() *TMU_TCOR0 = TMU_MAX_COUNTER; *TMU_TCNT0 = TMU_MAX_COUNTER;
#define TMU_SET(v) *TMU_TCNT0 = (v ^ TMU_MAX_COUNTER);
#define TMU_GET()  (*TMU_TCNT0 ^ TMU_MAX_COUNTER)

/* RTC clock not connected on this board use pclock */
int timer_init (void)
{
	TMU_OFF ();
	/* Take clock from PCLOCK/1024 */
	*TMU_TCR0 = *TMU_TCR0 & 0xfff8;
	*TMU_TCR0 = *TMU_TCR0 | 0x4;
	*TMU_TCOR0 = TMU_MAX_COUNTER;
	TMU_ON ();
	return 0;
}
void reset_timer (void)
{
	TMU_OFF ();
	TMU_CLEAR ();
	TMU_ON ();
}

ulong get_timer (ulong base)
{
  ulong now = TMU_GET();
	return ((int)now - base) < 0 ? (TMU_MAX_COUNTER - (base - now)) : (now - base);
}

void set_timer (ulong t)
{
	TMU_SET (t);
}

void udelay (unsigned long usec)
{
  ulong tmo;
  ulong start = TMU_GET ();
  if (usec > 1000000)
    tmo = ((usec/100000) * CFG_HZ) / 10; 
  else if (usec > 1000)
    tmo = ((usec/100) * CFG_HZ) / 10000; 
  else 
    tmo = (usec * CFG_HZ) / 1000000; 
  while (get_timer(start) < tmo)
		/*NOP*/;
}

ulong get_tbclk (void)
{
	return CFG_HZ;
}
