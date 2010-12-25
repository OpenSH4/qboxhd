/*
 * Copyright (C) 2007 STMicroelectronics Limited
 * Author: Stuart Menefy <stuart.menefy@st.com>
 *
 * May be copied or modified under the terms of the GNU General Public
 * License.  See linux/COPYING for more information.
 */

#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/io.h>
#include <linux/stm/emi.h>


#define EMI_GEN_CFG			0x0028
#define BANK_BASEADDRESS(b)		(0x800 + (0x10 * b))
#define BANK_EMICONFIGDATA(b, r)	(0x100 + (0x40 * b) + (8 * r))



static char emi_initialised;
static unsigned long emi_memory_base;
static void __iomem *emi_control;



int __init emi_init(unsigned long memory_base, unsigned long control_base)
{
	BUG_ON(emi_initialised);

	if (!request_mem_region(control_base, 0x864, "EMI"))
		return -EBUSY;

	emi_control = ioremap(control_base, 0x864);
	if (emi_control == NULL)
		return -ENOMEM;

	emi_memory_base = memory_base;

	emi_initialised = 1;

	return 0;
}

unsigned long emi_bank_base(int bank)
{
	unsigned long reg;

	BUG_ON(!emi_initialised);

	reg = readl(emi_control + BANK_BASEADDRESS(bank));

	return emi_memory_base + (reg << 22);
}

void __init emi_bank_configure(int bank, unsigned long data[4])
{
	int i;

	BUG_ON(!emi_initialised);

	for (i = 0; i < 4; i++)
		writel(data[i], emi_control + BANK_EMICONFIGDATA(bank, i));
}



/*
 *                ______________________________
 * EMIADDR    ___/                              \________
 *               \______________________________/
 *
 * (The cycle time specified in nano seconds)
 *
 *                |-----------------------------| cycle_time
 *                 ______________                ___________
 * CYCLE_TIME     /              \______________/
 *
 *
 * (IORD_start the number of nano seconds after the start of the cycle the
 * RD strobe is asserted IORD_end the number of nano seconds before the
 * end of the cycle the RD strobe is de-asserted.)
 *                   _______________________
 * IORD       ______/                       \________
 *
 *               |--|                       |---|
 *                 ^--- IORD_start            ^----- IORD_end
 *
 * (RD_latch the number of nano seconds at the end of the cycle the read
 * data is latched)
 *                                  __
 * RD_LATCH  ______________________/__\________
 *
 *                                 |------------|
 *                                      ^---------- RD_latch
 *
 * (IOWR_start the number of nano seconds after the start of the cycle the
 * WR strobe is asserted IOWR_end the number of nano seconds before the
 * end of the cycle the WR strobe is de-asserted.)
 *                   _______________________
 * IOWR       ______/                       \________
 *
 *               |--|                       |---|
 *                 ^--- IOWR_start            ^----- IOWR_end
 */



/* NOTE: these calculations assume a 100MHZ clock */



static void __init set_pata_read_timings(int bank, int cycle_time,
		int IORD_start, int IORD_end, int RD_latch)
{
	cycle_time = cycle_time / 10;
	IORD_start = IORD_start / 5;
	IORD_end = IORD_end / 5;
	RD_latch = RD_latch / 10;

	writel((cycle_time << 24) | (IORD_start << 8) | (IORD_end << 12),
			emi_control + BANK_EMICONFIGDATA(bank, 1));
	writel(0x791 | (RD_latch << 20),
			emi_control + BANK_EMICONFIGDATA(bank, 0));
}

static void __init set_pata_write_timings(int bank, int cycle_time,
		int IOWR_start, int IOWR_end)
{
	cycle_time = cycle_time / 10;
	IOWR_start = IOWR_start / 5;
	IOWR_end = IOWR_end / 5;

	writel((cycle_time << 24) | (IOWR_start << 8) | (IOWR_end << 12),
			emi_control + BANK_EMICONFIGDATA(bank, 2));
}

void __init emi_config_pata(int bank, int pc_mode)
{
	int mask;

	BUG_ON(!emi_initialised);

	/* Set timings for PIO4 */
	set_pata_read_timings(bank, 120, 35, 30, 20);
	set_pata_write_timings(bank, 120, 35, 30);

	switch (bank) {
	case 2:	/* Bank C */
		mask = 1<<3;
		break;
	case 3:	/* Bank D */
		mask = 1<<4;
		break;
	default:
		mask = 0;
		break;
	}

	if (mask) {
		u32 val = readl(emi_control + EMI_GEN_CFG);
		if (pc_mode)
			val |= mask;
		else
			val &= (~mask);
		writel(val, emi_control + EMI_GEN_CFG);
	}
}

static void __init set_nand_read_timings(int bank, int cycle_time,
		int IORD_start, int IORD_end,
		int RD_latch, int busreleasetime,
		int wait_active_low )
{
	cycle_time = cycle_time / 10;		/* cycles */
	IORD_start = IORD_start / 5;		/* phases */
	IORD_end = IORD_end / 5;		/* phases */
	RD_latch = RD_latch / 10;		/* cycles */
	busreleasetime = busreleasetime / 10;   /* cycles */

	writel(0x04000699 | (busreleasetime << 11) | (RD_latch << 20) | (wait_active_low << 25),
			emi_control + BANK_EMICONFIGDATA(bank, 0));

	writel((cycle_time << 24) | (IORD_start << 12) | (IORD_end << 8),
			emi_control + BANK_EMICONFIGDATA(bank, 1));
}

static void __init set_nand_write_timings(int bank, int cycle_time,
		int IOWR_start, int IOWR_end)
{
	cycle_time = cycle_time / 10;		/* cycles */
	IOWR_start = IOWR_start / 5;		/* phases */
	IOWR_end   = IOWR_end / 5;		/* phases */

	writel((cycle_time << 24) | (IOWR_start << 12) | (IOWR_end << 8),
			emi_control + BANK_EMICONFIGDATA(bank, 2));
}

void __init emi_config_nand(int bank, struct emi_timing_data *timing_data)
{
	BUG_ON(!emi_initialised);

	set_nand_read_timings(bank,
			timing_data->rd_cycle_time,
			timing_data->rd_oee_start,
			timing_data->rd_oee_end,
			timing_data->rd_latchpoint,
			timing_data->busreleasetime,
			timing_data->wait_active_low);

	set_nand_write_timings(bank,
			timing_data->wr_cycle_time,
			timing_data->wr_oee_start,
			timing_data->wr_oee_end);
}
