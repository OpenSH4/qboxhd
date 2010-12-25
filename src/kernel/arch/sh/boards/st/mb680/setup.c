/*
 * arch/sh/boards/st/mb680/setup.c
 *
 * Copyright (C) 2008 STMicroelectronics Limited
 * Author: Stuart Menefy (stuart.menefy@st.com)
 *
 * May be copied or modified under the terms of the GNU General Public
 * License.  See linux/COPYING for more information.
 *
 * STMicroelectronics STx7105 Mboard support.
 */

#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/leds.h>
#include <linux/stm/pio.h>
#include <linux/stm/soc.h>
#include <linux/stm/emi.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/physmap.h>
#include <linux/mtd/partitions.h>
#include <linux/bpa2.h>
#include <sound/stm.h>
#include <linux/phy.h>
#include <linux/gpio_keys.h>
#include <linux/input.h>
#include <asm/irq-ilc.h>
#include <asm/irl.h>
#include <asm/io.h>
#include "../common/common.h"

static int ascs[2] __initdata = { 2, 3 };

const char *LMI_SYS_partalias[] = { "BPA2_Region0", "v4l2-coded-video-buffers",  
		"BPA2_Region1", "v4l2-video-buffers" ,"coredisplay-video", NULL };

//const char *GFX_partalias[] = { "gfx-memory", NULL };

static struct bpa2_partition_desc bpa2_parts_table[] = {
	{
		.name  = "LMI_SYS",
		.start = 0x44400000,
		.size  = 0x04400000,
		.flags = 0,
		.aka   = LMI_SYS_partalias
	},
	{
		.name  = "bigphysarea",
		.start = 0x48800000,
		.size  = 0x01800000,
		.flags = 0,
		.aka   = NULL
	},
	{
		.name  = "gfx-memory-1",
		.start = 0x4a000000,
		.size  = 0x02000000,
		.flags = 0,
		.aka   = NULL
	},
	{
		.name  = "gfx-memory-0",
		.start = 0x4c000000,
		.size  = 0x04000000,
		.flags = 0,
		.aka   = NULL
	},	
};

static void __init mb680_setup(char** cmdline_p)
{
	printk("STMicroelectronics STx7105 Mboard initialisation\n");

	stx7105_early_device_init();
	stx7105_configure_asc(ascs, 2, 0);
	bpa2_init(bpa2_parts_table, ARRAY_SIZE(bpa2_parts_table));
}

static struct plat_stm_pwm_data pwm_private_info = {
	.flags		= PLAT_STM_PWM_OUT0,
	.routing	= PWM_OUT0_PIO13_0,
};

static struct plat_ssc_data ssc_private_info = {
	.capability  =
		ssc0_has(SSC_I2C_CAPABILITY) |
		ssc1_has(SSC_I2C_CAPABILITY) |
		ssc2_has(SSC_I2C_CAPABILITY) |
		ssc3_has(SSC_I2C_CAPABILITY),
	.routing =
		SSC3_SCLK_PIO3_6 | SSC3_MTSR_PIO3_7 | SSC3_MRST_PIO3_7,
};

static struct usb_init_data usb_init[2] __initdata = {
	{
		.oc_en = 1,
		.oc_actlow = 0,
		.oc_pinsel = USB0_OC_PIO4_4,
		.pwr_en = 1,
		.pwr_pinsel = USB0_PWR_PIO4_5,
	}, {
		.oc_en = 1,
		.oc_actlow = 0,
		.oc_pinsel = USB1_OC_PIO4_6,
		.pwr_en = 1,
		.pwr_pinsel = USB1_PWR_PIO4_7,
	}
};

static struct platform_device mb680_leds = {
	.name = "leds-gpio",
	.id = 0,
	.dev.platform_data = &(struct gpio_led_platform_data) {
		.num_leds = 2,
		.leds = (struct gpio_led[]) {
			{
				.name = "LD5",
				.default_trigger = "heartbeat",
				.gpio = stpio_to_gpio(2, 4),
			},
			{
				.name = "LD6",
				.gpio = stpio_to_gpio(2, 3),
			},
		},
	},
};

static struct plat_stmmacphy_data phy_private_data = {
	/* National Semiconductor DP83865 */
	.bus_id = 0,
	.phy_addr = 1,
	.phy_mask = 0,
	.interface = PHY_INTERFACE_MODE_MII,
};

static struct platform_device mb680_phy_device = {
	.name		= "stmmacphy",
	.id		= 0,
	.num_resources	= 1,
	.resource	= (struct resource[]) {
		{
			.name	= "phyirq",
			.start	= -1,/*FIXME, should be ILC_EXT_IRQ(6), */
			.end	= -1,
			.flags	= IORESOURCE_IRQ,
		},
	},
	.dev = {
		.platform_data = &phy_private_data,
	}
};

static struct mtd_partition nand1_parts = {
		.name	= "NAND1 root",
		.offset	= 0,
		.size 	= MTDPART_SIZ_FULL,
};

/* Timing data for onboard NAND */
static struct emi_timing_data nand_timing_data = {
	.rd_cycle_time	 = 40,		 /* times in ns */
	.rd_oee_start	 = 0,
	.rd_oee_end	 = 10,
	.rd_latchpoint	 = 10,

	.busreleasetime  = 10,
	.wr_cycle_time	 = 40,
	.wr_oee_start	 = 0,
	.wr_oee_end	 = 10,
		
	.wait_active_low = 0,
};

static struct nand_config_data mb680_nand_config = {
	.emi_bank		= 1,
	.emi_withinbankoffset	= 0,

	.emi_timing_data	= &nand_timing_data,

	.chip_delay		= 25,
	.mtd_parts		= &nand1_parts,
	.nr_parts		= 1,
	.rbn_port		= 2,
	.rbn_pin		= 7,
};


static struct platform_device *mb680_devices[] __initdata = {
	&mb680_leds,
	&mb680_phy_device,
};

/* Configuration based on Futarque-RC signals train. */
lirc_scd_t lirc_scd = {
	.code = 0x3FFFC028,
	.codelen = 0x1e,
	.alt_codelen = 0,
	.nomtime = 0x1f4,
	.noiserecov = 0,
};

static int __init device_init(void)
{
	stx7105_configure_sata();
	stx7105_configure_pwm(&pwm_private_info);
	stx7105_configure_ssc(&ssc_private_info);

	/*
	 * Note that USB port configuration depends on jumper
	 * settings:
	 *		  PORT 0  SW		PORT 1	SW
	 *		+----------------------------------------
	 * OC	normal	|  4[4]	J5A 2-3		 4[6]	J10A 2-3
	 *	alt	| 12[5]	J5A 1-2		14[6]	J10A 1-2
	 * PWR	normal	|  4[5]	J5B 2-3		 4[7]	J10B 2-3
	 *	alt	| 12[6]	J5B 1-2		14[7]	J10B 1-2
	 */

	stx7105_configure_usb(0, &usb_init[0]);
	stx7105_configure_usb(1, &usb_init[1]);
	stx7105_configure_ethernet(0, 0, 0, 1, 0, 0);
	stx7105_configure_lirc(&lirc_scd, NULL, 0);
	stx7105_configure_tsin(0);
	stx7105_configure_tsin(1);
	
	return platform_add_devices(mb680_devices, ARRAY_SIZE(mb680_devices));
}
arch_initcall(device_init);

static void __iomem *mb680_ioport_map(unsigned long port, unsigned int size)
{
	/* However picking somewhere safe isn't as easy as you might think.
	 * I used to use external ROM, but that can cause problems if you are
	 * in the middle of updating Flash. So I'm now using the processor core
	 * version register, which is guaranted to be available, and non-writable.
	 */
	return (void __iomem *)CCN_PVR;
}

static void __init mb680_init_irq(void)
{
#ifndef CONFIG_SH_ST_MB705
	/* Configure STEM interrupts as active low. */
	set_irq_type(ILC_EXT_IRQ(1), IRQ_TYPE_LEVEL_LOW);
	set_irq_type(ILC_EXT_IRQ(2), IRQ_TYPE_LEVEL_LOW);
#endif
}

struct sh_machine_vector mv_mb680 __initmv = {
	.mv_name		= "mb680",
	.mv_setup		= mb680_setup,
	.mv_nr_irqs		= NR_IRQS,
	.mv_init_irq		= mb680_init_irq,
	.mv_ioport_map		= mb680_ioport_map,
};
