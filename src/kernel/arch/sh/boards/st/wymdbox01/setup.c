/*
 * Wyplay MediaBox
 * PCB revision MDBOXA_MBRD_V0100
 * PCB revision MDBOXB_MBRD_V0100
 * PCB revision MDBOXB_MBRD_V0200
 *
 * Derived from STMicroelectronics STb7100 Reference board support.
 *
 * (C) Copyright 2005 STMicroelectronics Limited.
 * Stuart Menefy <stuart.menefy@st.com>
 *
 * (C) Copyright 2006-2008 WyPlay SAS.
 * Aubin Constans <aconstans@wyplay.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; version 2 of
 * the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 * TODO:
 *      - sort out the USBPWR stuff from stb7100ref/setup.c
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/stm/pio.h>
#include <linux/stm/soc.h>
#include <linux/stm/emi.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/physmap.h>
#include <linux/mtd/partitions.h>
#include <linux/i2c.h>
#include <linux/spi/spi.h>
#include <linux/spi/flash.h>
#include <linux/phy.h>
#include <linux/marvell_m88e6xxx.h>
#include <linux/sflash.h>
#include <linux/vmii.h>
#include <linux/usb/usbc.h>
#include <asm/io.h>
#include <asm/irl.h>

extern int usbc_setup(struct platform_device *);
extern void usbc_freed(struct platform_device *);
extern int usbc_get_state(void);

#ifdef CONFIG_BPA2

#include <linux/bpa2.h>

static const char *LMI_VID_partalias[] = {
	"BPA2_Region1",
	"coredisplay-video",
	"gfx-memory",
	"v4l2-video-buffers",
	NULL
};

static const char *LMI_SYS_partalias[] = {
	"BPA2_Region0",
	"bigphysarea",
	"v4l2-coded-video-buffers",
	NULL
};

static struct bpa2_partition_desc bpa2_parts_table_vid64[] = {
	{
		.name	= "LMI_VID",
		.start	= 0x10800000,
		.size	= 0x03800000,	/* 64-8=56MB */
		.flags	= 0,
		.aka	= LMI_VID_partalias,
	}, {
		.name	= "LMI_SYS",
		.start	= 0,
		.size	= 0x03800000,	/* 56MB */
		.flags	= 0,
		.aka	= LMI_SYS_partalias,
	},
};

#ifndef CONFIG_BPA_PROFILE_VID_64
static struct bpa2_partition_desc bpa2_parts_table_vid112[] = {
	{
		.name	= "LMI_VID",
		.start	= 0x10800000,
		.size	= 0x06800000,	/* 112-8=104MB */
		.flags	= 0,
		.aka	= LMI_VID_partalias,
	}, {
		.name	= "LMI_SYS",
		.start	= 0,
		.size	= 0x03800000,	/* 56MB */
		.flags	= 0,
		.aka	= LMI_SYS_partalias,
	},
};
#else
#define bpa2_parts_table_vid112		bpa2_parts_table_vid64
#endif

static void wymdbox_bpa2_init(void)
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
	if (sysconf >= 112UL)
		bpa2_init(bpa2_parts_table_vid112, ARRAY_SIZE(bpa2_parts_table_vid112));
	else
		bpa2_init(bpa2_parts_table_vid64, ARRAY_SIZE(bpa2_parts_table_vid64));
}

#else

static inline void wymdbox_bpa2_init(void) { return; }

#endif  /* CONFIG_BPA2 */


void __init wymdbox01_setup(char **cmdline_p)
{
	extern void wymdbox_pio_init(void);
	int ascs[1] = { 1 | (STASC_FLAG_NORTSCTS << 8) };

	stx7100_early_device_init();
	stb7100_configure_asc(ascs, ARRAY_SIZE(ascs), 1);

	wymdbox_pio_init();
	wymdbox_bpa2_init();
}

/*
 * NOR flash
 */
#ifdef CONFIG_MTD_PHYSMAP
/* default partition table, overridden by mtdparts command line option */
static struct mtd_partition mtd_parts_table[] = {
	{
		.name		= "params_redund",
		.size		= 0x00002000,
		.offset		= 0x007F8000,
	}, {
		.name		= "params",
		.size		= 0x00002000,
		.offset		= 0x007FA000,
	}, {
		.name		= "data",
		.size		= 0x00002000,
		.offset		= 0x007FE000,
		/* read-only partition, mask bit set => flag cleared */
		.mask_flags	= MTD_WRITEABLE,
	},
};

static struct physmap_flash_data physmap_flash_data = {
	.width		= 2,
	.set_vpp	= NULL,
	.nr_parts	= ARRAY_SIZE(mtd_parts_table),
	.parts		= mtd_parts_table,
};
#endif

static struct resource physmap_flash_resource = {
	/* completed at runtime */
	.flags	= IORESOURCE_MEM,
};

static struct platform_device physmap_flash = {
	.name	= "physmap-flash",
	.id	= -1,
	.dev	= {
#ifdef CONFIG_MTD_PHYSMAP
		.platform_data = &physmap_flash_data,
#endif
	},
	.num_resources	= 1,
	.resource	= &physmap_flash_resource,
};

static struct plat_stmmacphy_data phy_private_data = {
	.bus_id = 0,
	.phy_addr = 0x1d,
	.phy_mask = ~(1UL << 0x1d),
	.interface = PHY_INTERFACE_MODE_MII,
	.phy_reset = NULL,
};

static struct platform_device stmmac_phy_device = {
	.name		= "stmmacphy",
	.id		= 0,
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

/*
 * Infrared interfaces
 */
static unsigned lirc_enabled_pios[] = {
	[0] = 3 * 8 + 3
};

/*
 * PWM
 */
static struct plat_stm_pwm_data pwm_private_info = {
	.flags		= PLAT_STM_PWM_OUT1,
	.pwmclkdiv	= 3, /* 26kHz, for the fan */
};

/*
 * SSC
 */
static struct plat_ssc_data ssc_private_info = {
	.capability  = ssc0_has(SSC_I2C_CAPABILITY) |
		ssc1_has(SSC_SPI_CAPABILITY) |
		ssc2_has(SSC_I2C_CAPABILITY),
};

/*
 * I2C
 */
static struct i2c_board_info wymdbox01_i2c_devices[] __initdata = {
	{ I2C_BOARD_INFO("rtc-m41t80", 0x68), .type = "m41t80", .irq = IRL2_IRQ },
};

/*
 * SPI
 */
#if defined(CONFIG_MTD_M25P80) || defined(CONFIG_MTD_M25P80_MODULE)
static struct mtd_partition encoder_flash_partitions[] = {
	{
		.name = "ViXSfirmware",
		.size = 0x00100000,		/* 1MB only so both M25P80 and
		                   		 * M25P16 are supported */
		.offset = 0x00000000,
		.mask_flags = MTD_CAP_ROM,
	},
};

static struct flash_platform_data encoder_flash_data = {
	.name = "ViXS",
	.parts = encoder_flash_partitions,
	.nr_parts = ARRAY_SIZE(encoder_flash_partitions),
	.type = "m25p16",			/* The actual chip model */
};

static struct spi_board_info wymdbox01_spi_devices[] __initdata = {
	{
		.modalias = "m25p80",		/* drivers/mtd/devices/m25p80 */
		.platform_data = &encoder_flash_data,
		.max_speed_hz = 6000000,	/* Max SPI clock frequency
		                         	 * The chip supports 50MHz */
		.bus_num = 0,			/* On the first SPI bus */
		.chip_select = spi_set_cs(3, 4),
		.mode = SPI_MODE_3,
	},
};
#endif

/*
 * UDC (net2272)
 */
static struct resource net2272_resources[] = {
	[0] = {	/* offset only, base-shifted at runtime */
		.start	= 0,
		.end	= 0x2C,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRL1_IRQ,
		.end	= IRL1_IRQ,
		.flags	= IORESOURCE_IRQ,
	}
};

static struct platform_device net2272_device = {
	.name		= "net2272",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(net2272_resources),
	.resource	= net2272_resources,
};

static int rt2880_reset(int state)
{
	struct stpio_pin *pio_ptr = NULL;
	int rc = 0;

	pio_ptr = stpio_request_pin(5, 5, "WLAN_RESET", STPIO_OUT);
	if(!pio_ptr) {
		printk("%s:-EIO\n",__FUNCTION__);
		return -EIO;
	}
	switch(state) {
		case 0:
		case 1:
			printk("%s:state=%d\n", __FUNCTION__, state);
			stpio_set_pin(pio_ptr, state);
			break;
		default:
			rc = -EIO;
	}
	stpio_free_pin(pio_ptr);
	return rc;
}

static int marvell_phy0_set_status(int status)
{
#ifdef CONFIG_MARVELL_M88E6XXX_PHY
	if(status > 1)
		m88e6xxx_disable_wlan_port();
	else
		m88e6xxx_enable_wlan_port();

	status = status % 2;

	return m88e6xxx_set_phy_port_status(0x0, status);
#else
	return 1;
#endif
}

static struct vmii_dev vmii_bus_devices_info[] = {
	{
		.master = "eth0",
		.phy_port = 4,
		.name = "rt2880",
		.reset = rt2880_reset,
		.is_clone_mac = 1,
		.devid = VMII_RT2880,
		.set_phy_status = marvell_phy0_set_status,
	},
	{
		.master = "eth0",
		.phy_port = 0,
		.phy_addr = 0,
		.bus_id = 10,
		.phy_irq = 11,
		.switch_id = 1,
		.name = "generic",
		.is_clone_mac = 1,
		.set_phy_status = marvell_phy0_set_status,
		.prefix = "vmii",
	},
};

static struct vmii_devs vmii_bus_devices = {
	.nb = ARRAY_SIZE(vmii_bus_devices_info),
	.devs = vmii_bus_devices_info,
};

static struct platform_device vmii_bus = {
	.name = "vmii",
	.dev = {
		.platform_data = &vmii_bus_devices,
	}
};

static int spansion_set_vpp(int state)
{
	struct stpio_pin *pio_ptr = NULL;
	int rc = 0;

	pio_ptr = stpio_request_pin(3, 6, "SFVPP", STPIO_OUT);
	if(!pio_ptr) {
		printk("%s:-EIO\n", __FUNCTION__);
		return -EIO;
	}
	switch(state) {
		case 0:
		case 1:
			printk("%s:state=%d\n", __FUNCTION__, state);
			stpio_set_pin(pio_ptr, state);
			break;
		default:
			rc = -EIO;
	}
	stpio_free_pin(pio_ptr);
	return rc;
}

static struct sflash_info _sflash_info = {
	.set_vpp = spansion_set_vpp,
};

static struct platform_device spansion_vpp = {
	.name = "sflash_vpp",
	.dev = {
		.platform_data = &_sflash_info,
	}
};

static struct usbc_data _usbc_data= {
	.init = usbc_setup,
	.remove = usbc_freed,
	.get_state = usbc_get_state,
};

static struct resource usbc_res[] = {
	[0] = {
		.name	= "usbcableirq",
		.start	= IRL1_IRQ,
		.end	= IRL1_IRQ,
		.flags	= IORESOURCE_IRQ,
	}
};

static struct platform_device usbcable_device = {
	.name = "usbcable",
	.num_resources	= 1,
	.resource	= usbc_res,
	.dev = {
		.platform_data = &_usbc_data,
	},
};

static struct platform_device *wymdbox01_devices[] __initdata = {
	&physmap_flash,
	&stmmac_phy_device,
	&net2272_device,
	&vmii_bus,
	&spansion_vpp,
	&usbcable_device,
};

static int __init wymdbox01_device_init(void)
{
	const unsigned long emi_bank1_base = emi_bank_base(1);

	physmap_flash_resource.start = emi_bank_base(0);
	physmap_flash_resource.end = emi_bank1_base - 1UL;
	net2272_resources[0].start += emi_bank1_base;
	net2272_resources[0].end += emi_bank1_base;

	i2c_register_board_info(0, wymdbox01_i2c_devices,
				ARRAY_SIZE(wymdbox01_i2c_devices));
#if defined(CONFIG_MTD_M25P80) || defined(CONFIG_MTD_M25P80_MODULE)
	spi_register_board_info(wymdbox01_spi_devices,
	                        ARRAY_SIZE(wymdbox01_spi_devices));
#endif

	stx7100_configure_sata();
	stx7100_configure_pwm(&pwm_private_info);
	stx7100_configure_ssc(&ssc_private_info);
	stx7100_configure_usb();
	stx7100_configure_ethernet(0, 0, 0);
	stx7100_configure_lirc(NULL, lirc_enabled_pios,
	                       ARRAY_SIZE(lirc_enabled_pios));
	stx7100_configure_pata(2, 1, -1);

	return platform_add_devices(wymdbox01_devices,
				    ARRAY_SIZE(wymdbox01_devices));
}

device_initcall(wymdbox01_device_init);
