/* 
 * drivers/net/stmmac/mac100.c
 *
 * This is a driver for the MAC 10/100 on-chip
 * Ethernet controller currently present on STb7109 and 7200 SoCs.
 *
 * Copyright (C) 2007 by STMicroelectronics
 * Author: Giuseppe Cavallaro <peppe.cavallaro@st.com>
 *
*/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/if_ether.h>
#include <linux/crc32.h>
#include <linux/mii.h>
#include <linux/phy.h>
#include <asm/io.h>

#include "common.h"
#include "mac100.h"

#undef MAC100_DEBUG
/*#define MAC100_DEBUG*/
#ifdef MAC100_DEBUG
#define DBG(fmt,args...)  printk(fmt, ## args)
#else
#define DBG(fmt, args...)  do { } while(0)
#endif
static void mac100_core_init(unsigned long ioaddr)
{
	u32 value = readl(ioaddr + MAC_CONTROL);
	writel((value | MAC_CORE_INIT), ioaddr + MAC_CONTROL);

#if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)
	/* VLAN1 Tag identifier register is programmed to 
	 * the 802.1Q VLAN Extended Header (0x8100). */
	writel(ETH_P_8021Q, ioaddr + MAC_VLAN1);
#endif
	return;
}

static void mac100_dump_mac_regs(unsigned long ioaddr)
{
	printk("\t----------------------------------------------\n"
	       "\t  MAC100 CSR (base addr = 0x%8x)\n"
	       "\t----------------------------------------------\n",
	       (unsigned int)ioaddr);
	printk("\tcontrol reg (offset 0x%x): 0x%08x\n", MAC_CONTROL,
	       readl(ioaddr + MAC_CONTROL));
	printk("\taddr HI (offset 0x%x): 0x%08x\n ", MAC_ADDR_HIGH,
	       readl(ioaddr + MAC_ADDR_HIGH));
	printk("\taddr LO (offset 0x%x): 0x%08x\n", MAC_ADDR_LOW,
	       readl(ioaddr + MAC_ADDR_LOW));
	printk("\tmulticast hash HI (offset 0x%x): 0x%08x\n", MAC_HASH_HIGH,
	       readl(ioaddr + MAC_HASH_HIGH));
	printk("\tmulticast hash LO (offset 0x%x): 0x%08x\n", MAC_HASH_LOW,
	       readl(ioaddr + MAC_HASH_LOW));
	printk("\tflow control (offset 0x%x): 0x%08x\n", MAC_FLOW_CTRL,
	       readl(ioaddr + MAC_FLOW_CTRL));
	printk("\tVLAN1 tag (offset 0x%x): 0x%08x\n", MAC_VLAN1,
	       readl(ioaddr + MAC_VLAN1));
	printk("\tVLAN2 tag (offset 0x%x): 0x%08x\n", MAC_VLAN2,
	       readl(ioaddr + MAC_VLAN2));
	printk("\n\tMAC management counter registers\n");
	printk("\t MMC crtl (offset 0x%x): 0x%08x\n",
	       MMC_CONTROL, readl(ioaddr + MMC_CONTROL));
	printk("\t MMC High Interrupt (offset 0x%x): 0x%08x\n",
	       MMC_HIGH_INTR, readl(ioaddr + MMC_HIGH_INTR));
	printk("\t MMC Low Interrupt (offset 0x%x): 0x%08x\n",
	       MMC_LOW_INTR, readl(ioaddr + MMC_LOW_INTR));
	printk("\t MMC High Interrupt Mask (offset 0x%x): 0x%08x\n",
	       MMC_HIGH_INTR_MASK, readl(ioaddr + MMC_HIGH_INTR_MASK));
	printk("\t MMC Low Interrupt Mask (offset 0x%x): 0x%08x\n",
	       MMC_LOW_INTR_MASK, readl(ioaddr + MMC_LOW_INTR_MASK));
	return;
}

static int mac100_dma_init(unsigned long ioaddr, int pbl, u32 dma_tx,
			   u32 dma_rx)
{
	u32 value = readl(ioaddr + DMA_BUS_MODE);
	/* DMA SW reset */
	value |= DMA_BUS_MODE_SFT_RESET;
	writel(value, ioaddr + DMA_BUS_MODE);
	while ((readl(ioaddr + DMA_BUS_MODE) & DMA_BUS_MODE_SFT_RESET)) {
	}

	/* Enable Application Access by writing to DMA CSR0 */
	writel(DMA_BUS_MODE_DEFAULT | (pbl << DMA_BUS_MODE_PBL_SHIFT),
	       ioaddr + DMA_BUS_MODE);

	/* Mask interrupts by writing to CSR7 */
	writel(DMA_INTR_DEFAULT_MASK, ioaddr + DMA_INTR_ENA);

	/* The base address of the RX/TX descriptor lists must be written into
	 * DMA CSR3 and CSR4, respectively. */
	writel(dma_tx, ioaddr + DMA_TX_BASE_ADDR);
	writel(dma_rx, ioaddr + DMA_RCV_BASE_ADDR);

	return 0;
}

/* Store and Forward capability is not used at all..
 * The transmit threshold can be programmed by
 * setting the TTC bits in the DMA control register.*/
static void mac100_dma_operation_mode(unsigned long ioaddr, int txmode,
				      int rxmode)
{
	u32 csr6 = readl(ioaddr + DMA_CONTROL);

	if (txmode <= 32)
		csr6 |= DMA_CONTROL_TTC_32;
	else if (txmode <= 64)
		csr6 |= DMA_CONTROL_TTC_64;
	else
		csr6 |= DMA_CONTROL_TTC_128;

	writel(csr6, ioaddr + DMA_CONTROL);

	return;
}

static void mac100_dump_dma_regs(unsigned long ioaddr)
{
	int i;

	DBG(KERN_DEBUG "MAC100 DMA CSR \n");
	for (i = 0; i < 9; i++) {
		printk(KERN_DEBUG "\t CSR%d (offset 0x%x): 0x%08x\n", i,
		       (DMA_BUS_MODE + i * 4),
		       readl(ioaddr + DMA_BUS_MODE + i * 4));
	}
	DBG(KERN_DEBUG "\t CSR20 (offset 0x%x): 0x%08x\n",
	    DMA_CUR_TX_BUF_ADDR, readl(ioaddr + DMA_CUR_TX_BUF_ADDR));
	DBG(KERN_DEBUG "\t CSR21 (offset 0x%x): 0x%08x\n",
	    DMA_CUR_RX_BUF_ADDR, readl(ioaddr + DMA_CUR_RX_BUF_ADDR));
	return;
}

/* The DMA controller maintains two counters to track the number of
   the receive missed frames. */
static void mac100_dma_diagnostic_fr(void *data, struct stmmac_extra_stats *x,
				     unsigned long ioaddr)
{
	struct net_device_stats *stats = (struct net_device_stats *)data;
	u32 csr8 = readl(ioaddr + DMA_MISSED_FRAME_CTR);

	if (unlikely(csr8)) {
		if (csr8 & DMA_MISSED_FRAME_OVE) {
			stats->rx_over_errors += 0x800;
			x->rx_overflow_cntr += 0x800;
		} else {
			unsigned int ove_cntr;
			ove_cntr = ((csr8 & DMA_MISSED_FRAME_OVE_CNTR) >> 17);
			stats->rx_over_errors += ove_cntr;
			x->rx_overflow_cntr += ove_cntr;
		}

		if (csr8 & DMA_MISSED_FRAME_OVE_M) {
			stats->rx_missed_errors += 0xffff;
			x->rx_missed_cntr += 0xffff;
		} else {
			unsigned int miss_f = (csr8 & DMA_MISSED_FRAME_M_CNTR);
			stats->rx_missed_errors += miss_f;
			x->rx_missed_cntr += miss_f;
		}
	}
	return;
}

static int mac100_get_tx_frame_status(void *data, struct stmmac_extra_stats *x,
				      struct dma_desc *p, unsigned long ioaddr)
{
	int ret = 0;
	struct net_device_stats *stats = (struct net_device_stats *)data;

	if (unlikely(p->des01.tx.error_summary)) {
		if (unlikely(p->des01.tx.underflow_error)) {
			x->tx_underflow++;
			stats->tx_fifo_errors++;
		}
		if (unlikely(p->des01.tx.no_carrier)) {
			x->tx_carrier++;
			stats->tx_carrier_errors++;
		}
		if (unlikely(p->des01.tx.loss_carrier)) {
			x->tx_losscarrier++;
			stats->tx_carrier_errors++;
		}
		if (unlikely((p->des01.tx.excessive_deferral) ||
			     (p->des01.tx.excessive_collisions) ||
			     (p->des01.tx.late_collision))) {
			stats->collisions += p->des01.tx.collision_count;
		}
		ret = -1;
	}
	if (unlikely(p->des01.tx.heartbeat_fail)) {
		x->tx_heartbeat++;
		stats->tx_heartbeat_errors++;
		ret = -1;
	}
	if (unlikely(p->des01.tx.deferred)) {
		x->tx_deferred++;
	}

	return (ret);
}

static int mac100_get_tx_len(struct dma_desc *p)
{
	return (p->des01.tx.buffer1_size);
}

/* This function verifies if the incoming frame has some errors 
 * and, if required, updates the multicast statistics.
 * In case of success, it returns  csum_none becasue the device
 * is not able to compute the csum in HW. */
static int mac100_get_rx_frame_status(void *data, struct stmmac_extra_stats *x,
				      struct dma_desc *p)
{
	int ret = csum_none;
	struct net_device_stats *stats = (struct net_device_stats *)data;

	if (unlikely(p->des01.rx.last_descriptor == 0)) {
		printk(KERN_WARNING "mac100 Error: Oversized Ethernet "
		       "frame spanned multiple buffers\n");
		stats->rx_length_errors++;
		return discard_frame;
	}

	if (unlikely(p->des01.rx.error_summary)) {
		if (unlikely(p->des01.rx.descriptor_error)) {
			x->rx_desc++;
		}
		if (unlikely(p->des01.rx.partial_frame_error)) {
			x->rx_partial++;
		}
		if (unlikely(p->des01.rx.run_frame)) {
			x->rx_runt++;
		}
		if (unlikely(p->des01.rx.frame_too_long)) {
			x->rx_toolong++;
		}
		if (unlikely(p->des01.rx.collision)) {
			x->rx_collision++;
			stats->collisions++;
		}
		if (unlikely(p->des01.rx.crc_error)) {
			x->rx_crc++;
			stats->rx_crc_errors++;
		}
		ret = discard_frame;
	}
	if (unlikely(p->des01.rx.dribbling))
		ret = discard_frame;

	if (unlikely(p->des01.rx.length_error)) {
		x->rx_lenght++;
		ret = discard_frame;
	}
	if (unlikely(p->des01.rx.mii_error)) {
		x->rx_mii++;
		ret = discard_frame;
	}
	if (p->des01.rx.multicast_frame) {
		x->rx_multicast++;
		stats->multicast++;
		/* no error! */
	}
	return (ret);
}

static void mac100_irq_status(unsigned long ioaddr)
{
	return;
}

static void mac100_set_filter(struct net_device *dev)
{
	unsigned long ioaddr = dev->base_addr;
	u32 value = readl(ioaddr + MAC_CONTROL);

	if (dev->flags & IFF_PROMISC) {
		value |= MAC_CONTROL_PR;
		value &= ~(MAC_CONTROL_PM | MAC_CONTROL_IF | MAC_CONTROL_HO |
			   MAC_CONTROL_HP);
	} else if ((dev->mc_count > HASH_TABLE_SIZE)
		   || (dev->flags & IFF_ALLMULTI)) {
		value |= MAC_CONTROL_PM;
		value &= ~(MAC_CONTROL_PR | MAC_CONTROL_IF | MAC_CONTROL_HO);
		writel(0xffffffff, ioaddr + MAC_HASH_HIGH);
		writel(0xffffffff, ioaddr + MAC_HASH_LOW);
	} else if (dev->mc_count == 0) {	/* Just get our own stuff .. no multicast?? */
		value &= ~(MAC_CONTROL_PM | MAC_CONTROL_PR | MAC_CONTROL_IF |
			   MAC_CONTROL_HO | MAC_CONTROL_HP);
	} else {		/* Store the addresses in the multicast HW filter */
		int i;
		u32 mc_filter[2];
		struct dev_mc_list *mclist;

		/* Perfect filter mode for physical address and Hash
		   filter for multicast */
		value |= MAC_CONTROL_HP;
		value &= ~(MAC_CONTROL_PM | MAC_CONTROL_PR | MAC_CONTROL_IF
			   | MAC_CONTROL_HO);

		memset(mc_filter, 0, sizeof(mc_filter));
		for (i = 0, mclist = dev->mc_list;
		     mclist && i < dev->mc_count; i++, mclist = mclist->next) {
			/* The upper 6 bits of the calculated CRC are used to 
			 * index the contens of the hash table */
			int bit_nr =
			    ether_crc(ETH_ALEN, mclist->dmi_addr) >> 26;
			/* The most significant bit determines the register to 
			 * use (H/L) while the other 5 bits determine the bit 
			 * within the register. */
			mc_filter[bit_nr >> 5] |= 1 << (bit_nr & 31);
		}
		writel(mc_filter[0], ioaddr + MAC_HASH_LOW);
		writel(mc_filter[1], ioaddr + MAC_HASH_HIGH);
	}

	writel(value, ioaddr + MAC_CONTROL);

	DBG(KERN_INFO "%s: CTRL reg: 0x%08x Hash regs: "
	    "HI 0x%08x, LO 0x%08x\n",
	    __FUNCTION__, readl(ioaddr + MAC_CONTROL),
	    readl(ioaddr + MAC_HASH_HIGH), readl(ioaddr + MAC_HASH_LOW));
	return;
}

static void mac100_flow_ctrl(unsigned long ioaddr, unsigned int duplex,
			     unsigned int fc, unsigned int pause_time)
{
/* No riscrittura ETH_FLOW_CTRL basata su stato porta 1 (si basa su porta 5 sistemata in uboot) */
/* Per il QboxHD_mini questo va bene perchè ha un'unica porta (con addr 0x1F) */
#ifndef CONFIG_SH_QBOXHD_1_0
	unsigned int flow = MAC_FLOW_CTRL_ENABLE;

	if (duplex)
		flow |= (pause_time << MAC_FLOW_CTRL_PT_SHIFT);
	writel(flow, ioaddr + MAC_FLOW_CTRL);
#endif /* CONFIG_SH_QBOXHD_1_0 */
	return;
}

static void mac100_pmt(unsigned long ioaddr, unsigned long mode)
{
	/* There is no PMT module in the stb7109 so no wake-up-on-Lan hw feature
	 * is supported. 
	 */
	return;
}

static void mac100_init_rx_desc(struct dma_desc *p, unsigned int ring_size)
{
	int i;
	for (i = 0; i < ring_size; i++) {
		p->des01.rx.own = 1;
		p->des01.rx.buffer1_size = BUF_SIZE_2KiB - 1;
		if (i == ring_size - 1) {
			p->des01.rx.end_ring = 1;
		}
		p++;
	}
	return;
}

static void mac100_disable_rx_ic(struct dma_desc *p, unsigned int ring_size,
				 int disable_ic)
{
	int i;
	for (i = 0; i < ring_size; i++) {
		if (i % disable_ic)
			p->des01.rx.disable_ic = 1;
		p++;
	}
	return;
}

static void mac100_init_tx_desc(struct dma_desc *p, unsigned int ring_size)
{
	int i;
	for (i = 0; i < ring_size; i++) {
		p->des01.tx.own = 0;
		if (i == ring_size - 1) {
			p->des01.tx.end_ring = 1;
		}
		p++;
	}
	return;
}

static int mac100_get_tx_owner(struct dma_desc *p)
{
	return p->des01.tx.own;
}

static int mac100_get_rx_owner(struct dma_desc *p)
{
	return p->des01.rx.own;
}

static void mac100_set_tx_owner(struct dma_desc *p)
{
	p->des01.tx.own = 1;
}

static void mac100_set_rx_owner(struct dma_desc *p)
{
	p->des01.rx.own = 1;
}

static int mac100_get_tx_ls(struct dma_desc *p)
{
	return p->des01.tx.last_segment;
}

static void mac100_release_tx_desc(struct dma_desc *p)
{
	int ter = p->des01.tx.end_ring;

/*	memset(p, 0, sizeof(struct dma_desc));*/
	/* clean field used within the xmit */
	p->des01.tx.first_segment = 0;
	p->des01.tx.last_segment = 0;
	p->des01.tx.buffer1_size = 0;

	/* clean status reported */
	p->des01.tx.error_summary = 0;
	p->des01.tx.underflow_error = 0;
	p->des01.tx.no_carrier = 0;
	p->des01.tx.loss_carrier = 0;
	p->des01.tx.excessive_deferral = 0;
	p->des01.tx.excessive_collisions = 0;
	p->des01.tx.late_collision = 0;
	p->des01.tx.heartbeat_fail = 0;
	p->des01.tx.deferred = 0;

	/* set termination field */
	p->des01.tx.end_ring = ter;

	return;
}

static void mac100_prepare_tx_desc(struct dma_desc *p, int is_fs, int len,
				   int csum_flag)
{
	p->des01.tx.first_segment = is_fs;
	p->des01.tx.buffer1_size = len;
}

static void mac100_clear_tx_ic(struct dma_desc *p)
{
	p->des01.tx.interrupt = 0;
}

static void mac100_close_tx_desc(struct dma_desc *p)
{
	p->des01.tx.last_segment = 1;
	p->des01.tx.interrupt = 1;
}

static int mac100_get_rx_frame_len(struct dma_desc *p)
{
	return p->des01.rx.frame_length;
}

struct device_ops mac100_driver = {
	.core_init = mac100_core_init,
	.dump_mac_regs = mac100_dump_mac_regs,
	.dma_init = mac100_dma_init,
	.dump_dma_regs = mac100_dump_dma_regs,
	.dma_mode = mac100_dma_operation_mode,
	.dma_diagnostic_fr = mac100_dma_diagnostic_fr,
	.tx_status = mac100_get_tx_frame_status,
	.rx_status = mac100_get_rx_frame_status,
	.get_tx_len = mac100_get_tx_len,
	.set_filter = mac100_set_filter,
	.flow_ctrl = mac100_flow_ctrl,
	.pmt = mac100_pmt,
	.init_rx_desc = mac100_init_rx_desc,
	.init_tx_desc = mac100_init_tx_desc,
	.get_tx_owner = mac100_get_tx_owner,
	.get_rx_owner = mac100_get_rx_owner,
	.release_tx_desc = mac100_release_tx_desc,
	.prepare_tx_desc = mac100_prepare_tx_desc,
	.clear_tx_ic = mac100_clear_tx_ic,
	.close_tx_desc = mac100_close_tx_desc,
	.get_tx_ls = mac100_get_tx_ls,
	.set_tx_owner = mac100_set_tx_owner,
	.set_rx_owner = mac100_set_rx_owner,
	.get_rx_frame_len = mac100_get_rx_frame_len,
	.host_irq_status = mac100_irq_status,
	.disable_rx_ic = mac100_disable_rx_ic,
};

struct mac_device_info *mac100_setup(unsigned long ioaddr)
{
	struct mac_device_info *mac;

	mac = kmalloc(sizeof(const struct mac_device_info), GFP_KERNEL);
	memset(mac, 0, sizeof(struct mac_device_info));

	printk(KERN_INFO "\tMAC 10/100\n");

	mac->ops = &mac100_driver;
	mac->hw.pmt = PMT_NOT_SUPPORTED;
	mac->hw.addr_high = MAC_ADDR_HIGH;
	mac->hw.addr_low = MAC_ADDR_LOW;
	mac->hw.link.port = MAC_CONTROL_PS;
	mac->hw.link.duplex = MAC_CONTROL_F;
	mac->hw.link.speed = 0;
	mac->hw.mii.addr = MAC_MII_ADDR;
	mac->hw.mii.data = MAC_MII_DATA;

	return mac;
}
