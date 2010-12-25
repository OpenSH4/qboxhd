/*
 * arch/sh/boards/st/qboxhd_mini/setup.c
 *
 * Copyright (C) 2009 Duolabs Spa
 * Author: Pedro Aguilar (pedro@duolabs.com)
 *
 * Based on arch/sh/boards/st/mb442/setup.c
 *
 * Copyright (C) 2005 STMicroelectronics Limited
 * Author: Stuart Menefy (stuart.menefy@st.com)
 *
 * May be copied or modified under the terms of the GNU General Public
 * License.  See linux/COPYING for more information.
 *
 * STMicroelectronics STb7100 Reference board support.
 */

#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/stm/pio.h>
#include <linux/stm/soc.h>
#include <linux/delay.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/physmap.h>
#include <linux/mtd/partitions.h>
#include <linux/phy.h>
#include <linux/lirc.h>
#include <asm/irl.h>

static int ascs[2] __initdata = { 2 };

#ifdef CONFIG_BPA2

#include <linux/bpa2.h>

const char *LMI_VID_partalias[] = { 
	"BPA2_Region1", 
	"coredisplay-video", 
	"gfx-memory", 
	"v4l2-video-buffers", 
	NULL 
};

const char *LMI_SYS_partalias[] = { 
	"BPA2_Region0", 
	"bigphysarea", 
	"v4l2-coded-video-buffers", 
	NULL 
};

static struct bpa2_partition_desc bpa2_parts_table[] = {
	{
		.name  = "LMI_VID",
		.start = 0x10800000,
		.size  = 0x03800000,
		.flags = 0,
		.aka   = LMI_VID_partalias
	},
	{
		.name  = "LMI_SYS",
		.start = 0,
		.size  = 0x01400000, /* 0x08000000 - 0x01400000 = 0x06C00000 = 113,246,208 B */
		.flags = 0,
		.aka   = LMI_SYS_partalias
	}
};

static void qboxhd_bpa2_init(void)
{
    unsigned long sysconf;

    /* Find out the amount of DDR-SDRAM on system then video LMI
	 * Read UBA field in LMI_SDRA0 registers
	 *
	 * FIXME: accessing LMI registers through P4 is buggy, dunno
	 * why...
	 */
	//sysconf = ctrl_inl(ST40_LMI_SDRA0(SYS)) & 0xFFE00000;
    sysconf = *(volatile u32 *)P2SEGADDR(ST40_LMI_SDRA0(SYS)) & 0xFFE00000;
    sysconf = (sysconf - 0x04000000UL) / (1024UL * 1024UL);
    printk("%luMB system DDR-SDRAM, ", sysconf);
	//sysconf = ctrl_inl(ST40_LMI_SDRA0(VID)) & 0xFFE00000;
    sysconf = *(volatile u32 *)P2SEGADDR(ST40_LMI_SDRA0(VID)) & 0xFFE00000;
    sysconf = (sysconf - 0x10000000UL) / (1024UL * 1024UL);
    printk("%luMB video DDR-SDRAM\n", sysconf);

    bpa2_init(bpa2_parts_table, ARRAY_SIZE(bpa2_parts_table));
}
#else

static inline void qboxhd_bpa2_init(void) { return; }

#endif  /* CONFIG_BPA2 */



void __init qboxhd_setup(char** cmdline_p)
{
	printk("Duolabs QBoxHD mini initialisation\n");

	stx7100_early_device_init();
	stb7100_configure_asc(ascs, 1, 0);
	qboxhd_bpa2_init();
}

static struct plat_ssc_data ssc_private_info = {
	.capability  =
		ssc0_has(SSC_I2C_CAPABILITY|SSC_I2C_CLK_UNIDIR) |
		ssc1_has(SSC_I2C_CAPABILITY|SSC_I2C_CLK_UNIDIR) |
		ssc2_has(SSC_I2C_CAPABILITY|SSC_I2C_CLK_UNIDIR),
};

static struct resource smc91x_resources[] = {
	[0] = {
		.start	= 0x02000300,
		.end	= 0x02000300 + 0xff,
		.flags	= IORESOURCE_MEM,
	},
};

static struct platform_device smc91x_device = {
	.name		= "smc91x",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(smc91x_resources),
	.resource	= smc91x_resources,
};

static struct mtd_partition mtd_parts_table[8] = {
    {
        .name = "Genesis",
        .size = 0x0000F000,
        .offset = 0x00000000,
    }, {
        .name = "MAC",
        .size = 0x00001000,
        .offset = 0x0000F000,
    }, {
        .name = "U-Boot",
        .size = 0x00050000,
        .offset = 0x00010000,
    }, {
        .name = "Logo",
        .size = 0x00030000,
        .offset = 0x00060000,
    }, {
        .name = "Bs",
        .size = 0x00080000,
        .offset = 0x00090000,
    }, {
        .name = "Kernel",
        .size = 0x00300000,
        .offset = 0x00110000,
    }, {
        .name = "Ramfs",
        .size = MTDPART_SIZ_FULL,
        .offset = 0x00410000,
    }, {
        .name = "All_Flash",
        .size = MTDPART_SIZ_FULL,
        .offset = 0x00010000,
	}	
};

static struct physmap_flash_data physmap_flash_data = {
	.width		= 2,
	.set_vpp	= NULL,
	.nr_parts	= ARRAY_SIZE(mtd_parts_table),
	.parts		= mtd_parts_table
};

static struct resource physmap_flash_resource = {
	.start		= 0x00000000,
	.end		= 0x00800000 - 1,
	.flags		= IORESOURCE_MEM,
};

static struct platform_device physmap_flash = {
	.name		= "physmap-flash",
	.id			= -1,
	.dev		= {
		.platform_data	= &physmap_flash_data,
	},
	.num_resources	= 1,
	.resource	= &physmap_flash_resource,
};

static struct plat_stmmacphy_data phy_private_data = {
	.bus_id 	= 0,
	.phy_addr 	= 0x1F,
	.phy_mask 	= 1,
	.interface 	= PHY_INTERFACE_MODE_MII,
};

static struct platform_device qboxhd_phy_device = {
	.name		= "stmmacphy",
	.id			= 0,
	.num_resources	= 1,
	.resource	= (struct resource[]) {
                {
			.name	= "phyirq",
			.start	= IRL3_IRQ,
			.end	= IRL3_IRQ,
			.flags	= IORESOURCE_IRQ,
		},
	},
	.dev = {
		.platform_data = &phy_private_data,
	 }
};

static struct platform_device *qboxhd_devices[] __initdata = {
	&smc91x_device,
	&physmap_flash,
	&qboxhd_phy_device,
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
	stx7100_configure_sata();
	stx7100_configure_ssc(&ssc_private_info);
	stx7100_configure_usb();
	stx7100_configure_lirc(&lirc_scd, NULL, 0);
	stx7100_configure_ethernet(0, 0, 0);

	return platform_add_devices(qboxhd_devices,
				    ARRAY_SIZE(qboxhd_devices));
}

device_initcall(device_init);
