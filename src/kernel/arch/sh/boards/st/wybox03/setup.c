/*
 * Wyplay WyBox, PCB revision WYBOXA_MBRD_V0200
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
#include <linux/sflash.h>
#include <asm/io.h>
#include <asm/irl.h>

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

static struct bpa2_partition_desc bpa2_parts_table[] = {
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

static void wybox03_bpa2_init(void)
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

static inline void wybox03_bpa2_init(void) { return; }

#endif  /* CONFIG_BPA2 */


void __init wybox03_setup(char **cmdline_p)
{
	extern void wybox03_pio_init(void);
	int ascs[1] = { 1 | (STASC_FLAG_NORTSCTS << 8) };

	stx7100_early_device_init();
	stb7100_configure_asc(ascs, ARRAY_SIZE(ascs), 1);

	wybox03_pio_init();
	wybox03_bpa2_init();
}

/*
 * NOR flash
 */
#ifdef CONFIG_MTD_PHYSMAP
/* default partition table, overridden by mtdparts command line option */
static struct mtd_partition mtd_parts_table[] = {
	{
		.name		= "data",
		.size		= 0x00002000,
		.offset		= 0x007FA000,
		/* read-only partition, mask bit set => flag cleared */
		.mask_flags	= MTD_WRITEABLE,
	}, {
		.name		= "params_redund",
		.size		= 0x00002000,
		.offset		= 0x007FC000,
	}, {
		.name		= "params",
		.size		= 0x00002000,
		.offset		= 0x007FE000,
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

static int phy_reset(void* priv)
{
	struct stpio_pin* pio_ptr = NULL;

	pio_ptr = stpio_request_set_pin(4, 6, "ETH_PHY_RESET", STPIO_OUT, 1);
	if (pio_ptr == NULL)
		return 0;
	/* Reset SMSC LAN8700 PHY
	 * assert nRST (low) for at least 100usec,
	 * with MII clock fed in at the same time */
	udelay(1);
	stpio_set_pin(pio_ptr, 0);
	udelay(110);
	stpio_set_pin(pio_ptr, 1);
	udelay(1);
	stpio_free_pin(pio_ptr);
	return 0;
}

static struct plat_stmmacphy_data phy_private_data = {
	.bus_id = 0,
	.phy_addr = 0x1f,
	.phy_mask = ~(1UL << 0x1f),
	.interface = PHY_INTERFACE_MODE_MII,
	.phy_reset = phy_reset,
};

static struct platform_device stmmac_phy_device = {
	.name		= "stmmacphy",
	.id		= 0,
	.num_resources	= 1,
	.resource	= (struct resource[]) {
		{
			.name	= "phyirq",
			.start	= IRL0_IRQ,
			.end	= IRL0_IRQ,
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
	[0] = 3 * 8 + 3,
	[1] = 3 * 8 + 5
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

static struct spi_board_info wybox03_spi_devices[] __initdata = {
	{
		.modalias = "m25p80",		/* drivers/mtd/devices/m25p80 */
		.platform_data = &encoder_flash_data,
		.max_speed_hz = 25000000,	/* Max SPI clock frequency
		                         	 * The chip supports 50MHz */
		.bus_num = 0,			/* On the first SPI bus */
		.chip_select = spi_set_cs(4, 4),
		.mode = SPI_MODE_3,
	},
};
#endif

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

static struct platform_device *wybox03_devices[] __initdata = {
	&physmap_flash,
	&stmmac_phy_device,
	&spansion_vpp,
};

static int __init wybox03_device_init(void)
{
	physmap_flash_resource.start = emi_bank_base(0);
	physmap_flash_resource.end = emi_bank_base(1) - 1UL;

#if defined(CONFIG_MTD_M25P80) || defined(CONFIG_MTD_M25P80_MODULE)
	spi_register_board_info(wybox03_spi_devices,
	                        ARRAY_SIZE(wybox03_spi_devices));
#endif

	stx7100_configure_sata();
	stx7100_configure_pwm(&pwm_private_info);
	stx7100_configure_ssc(&ssc_private_info);
	stx7100_configure_usb();
	stx7100_configure_ethernet(0, 0, 0);
	stx7100_configure_lirc(NULL, lirc_enabled_pios,
	                       ARRAY_SIZE(lirc_enabled_pios));
	stx7100_configure_pata(2, 1, IRL1_IRQ);

	return platform_add_devices(wybox03_devices,
				    ARRAY_SIZE(wybox03_devices));
}

device_initcall(wybox03_device_init);
