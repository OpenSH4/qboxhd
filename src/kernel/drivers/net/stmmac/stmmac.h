#define ETH_RESOURCE_NAME	"stmmaceth"
#define PHY_RESOURCE_NAME	"stmmacphy"
#define DRV_MODULE_VERSION	"Nov_08"

#if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)
#define STMMAC_VLAN_TAG_USED
#endif

#include "common.h"
#ifdef CONFIG_STMMAC_TIMER
#include "stmmac_timer.h"
#endif

struct stmmac_priv {
	struct net_device *dev;
	struct device *device;

	int pbl;
	int is_gmac;
	void (*fix_mac_speed) (void *priv, unsigned int speed);
	void *bsp_priv;

	int bus_id;
	int phy_addr;
	int phy_mask;
	phy_interface_t phy_interface;
	int (*phy_reset) (void *priv);
	int phy_irq;

	struct phy_device *phydev;
	int oldlink;
	int speed;
	int oldduplex;

	struct mii_bus *mii;

	spinlock_t lock;
	spinlock_t tx_lock;

	struct dma_desc *dma_tx	____cacheline_aligned;
	dma_addr_t dma_tx_phy;
	struct dma_desc *dma_rx	____cacheline_aligned;
	dma_addr_t dma_rx_phy;
	struct sk_buff **tx_skbuff;
	struct sk_buff **rx_skbuff;
	dma_addr_t *rx_skbuff_dma;

	unsigned int cur_rx, dirty_rx;
	unsigned int cur_tx, dirty_tx;
	unsigned int dma_tx_size;
	unsigned int dma_rx_size;
	unsigned int dma_buf_sz;
	unsigned int rx_buff;
	struct tasklet_struct tx_task;
	struct stmmac_extra_stats xstats;
	struct mac_device_info *mac_type;
	unsigned int flow_ctrl;
	unsigned int pause;
	u32 msg_enable;
	int rx_csum;
	int tx_coe;
	int wolopts;
	int wolenabled;
	int shutdown;
	int tx_coalesce;
#ifdef CONFIG_STMMAC_TIMER
	struct stmmac_timer *tm;
#endif
#ifdef STMMAC_VLAN_TAG_USED
	struct vlan_group *vlgrp;
#endif
};
