/*
 * (C) Copyright 2005
 * Andy Sturges, STMicroelectronics, andy.sturges@st.com.
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
 *
 */

#include <common.h>
#include <config.h>
#include <asm/io.h>

#if (CONFIG_COMMANDS & CFG_CMD_IDE)

#define HDDI_MODE  0x80
#define HDDI_ATARESET 0x84

#define HDD_FREQ_MHZ B_CLOCK_RATE

/* HDDI PIO timing registers! */
#define HDDI_DPIO_I        (CFG_ATA_BASE_ADDR + 0x00000090)
#define HDDI_DPIO_IORDY    (CFG_ATA_BASE_ADDR + 0x00000094)
#define HDDI_DPIO_WR       (CFG_ATA_BASE_ADDR + 0x00000098)
#define HDDI_DPIO_RD       (CFG_ATA_BASE_ADDR + 0x0000009C)
#define HDDI_DPIO_WREN     (CFG_ATA_BASE_ADDR + 0x000000A0)
#define HDDI_DPIO_AH       (CFG_ATA_BASE_ADDR + 0x000000A4)
#define HDDI_DPIO_WRRE     (CFG_ATA_BASE_ADDR + 0x000000A8)
#define HDDI_DPIO_RDRE     (CFG_ATA_BASE_ADDR + 0x000000AC)

typedef struct {
	unsigned int ir;
	unsigned int iordy;
	unsigned int wr;
	unsigned int rd;
	unsigned int wren;
	unsigned int ah;
	unsigned int wrre;
	unsigned int rdre;
} pio_t;

static pio_t pioTime[] = {
	/* Level:    Ir    IORdy   Wr    Rd    WrEn    AH    WrRe    RdRe   */
	/*   0 */ {70, 40, 120, 120, 10, 20, 340, 330},
	/*   1 */ {50, 40, 80, 80, 10, 20, 190, 190},
	/*   2 */ {30, 40, 50, 50, 10, 10, 100, 110},
	/*   3 */ {30, 40, 30, 30, 10, 10, 70, 70},
	/*   4 */ {30, 40, 20, 20, 10, 10, 30, 30}
};

static inline unsigned int _ata_adjust(unsigned int time)
{
	unsigned int v = (time * HDD_FREQ_MHZ) / 1000;
	/* round up to nearest greater integer */
	if ( (time * HDD_FREQ_MHZ % 1000) > 0 ) v++;
	/* subtract 1 before use...            */
	if (v > 0) v--;
	return(v);
}

static void set_pio_timings(int prate)
{

	/*
	 ** configure HDDI PIO register access timings
	 */
	ctrl_outl(_ata_adjust(pioTime[prate].ir), HDDI_DPIO_I);
	ctrl_outl(_ata_adjust(pioTime[prate].iordy), HDDI_DPIO_IORDY);
	ctrl_outl(_ata_adjust(pioTime[prate].wr), HDDI_DPIO_WR);
	ctrl_outl(_ata_adjust(pioTime[prate].rd), HDDI_DPIO_RD);
	ctrl_outl(_ata_adjust(pioTime[prate].ah), HDDI_DPIO_AH);
	ctrl_outl(_ata_adjust(pioTime[prate].wren), HDDI_DPIO_WREN);
	ctrl_outl(_ata_adjust(pioTime[prate].wrre), HDDI_DPIO_WRRE);
	ctrl_outl(_ata_adjust(pioTime[prate].rdre), HDDI_DPIO_RDRE);

#ifdef DEBUG
	printk
	    ("ST40-HDDI: set PIO %d timings (i %u, IOrdy %u, Wr %u, Rd %u, WrEn %u, Ah %u, WrRe %u, RdRe %u)\n",
	     PIO_RATE, ctrl_inl(HDDI_DPIO_I), ctrl_inl(HDDI_DPIO_IORDY),
	     ctrl_inl(HDDI_DPIO_WR), ctrl_inl(HDDI_DPIO_RD),
	     ctrl_inl(HDDI_DPIO_WREN), ctrl_inl(HDDI_DPIO_AH),
	     ctrl_inl(HDDI_DPIO_WRRE), ctrl_inl(HDDI_DPIO_RDRE));
#endif
}

#define SYSCON_REGS_BASE   0xB0300000
#define SYSCFG_HDDI        (SYSCON_REGS_BASE + 0x14) 
 
void stm8000_ide_init()
{

	writel(0x3FFFF800, SYSCFG_HDDI);
	writel(0, CFG_ATA_BASE_ADDR + HDDI_MODE);
	writel(1, CFG_ATA_BASE_ADDR + HDDI_ATARESET);
	writel(0, CFG_ATA_BASE_ADDR + HDDI_ATARESET);
	set_pio_timings(CFG_PIO_MODE);
}
#endif


int soc_init()
{
  return 0;
}
