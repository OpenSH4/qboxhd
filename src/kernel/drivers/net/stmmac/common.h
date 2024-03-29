#include "descs.h"

/* *********************************************
   DMA CRS Control and Status Register Mapping 
 * *********************************************/
#define DMA_BUS_MODE		0x00001000	/* Bus Mode */
#define DMA_XMT_POLL_DEMAND	0x00001004	/* Transmit Poll Demand */
#define DMA_RCV_POLL_DEMAND	0x00001008	/* Received Poll Demand */
#define DMA_RCV_BASE_ADDR	0x0000100c	/* Receive List Base */
#define DMA_TX_BASE_ADDR	0x00001010	/* Transmit List Base */
#define DMA_STATUS		0x00001014	/* Status Register */
#define DMA_CONTROL		0x00001018	/* Ctrl (Operational Mode) */
#define DMA_INTR_ENA		0x0000101c	/* Interrupt Enable */
#define DMA_MISSED_FRAME_CTR	0x00001020	/* Missed Frame Counter */
#define DMA_CUR_TX_BUF_ADDR	0x00001050	/* Current Host Tx Buffer */
#define DMA_CUR_RX_BUF_ADDR	0x00001054	/* Current Host Rx Buffer */

/* ********************************
   DMA Control register defines
 * ********************************/
#define DMA_CONTROL_ST		0x00002000	/* Start/Stop Transmission */
#define DMA_CONTROL_SR		0x00000002	/* Start/Stop Receive */

/* **************************************
   DMA Interrupt Enable register defines
 * **************************************/
/**** NORMAL INTERRUPT ****/
#define DMA_INTR_ENA_NIE 0x00010000	/* Normal Summary */
#define DMA_INTR_ENA_TIE 0x00000001	/* Transmit Interrupt */
#define DMA_INTR_ENA_TUE 0x00000004	/* Transmit Buffer Unavailable */
#define DMA_INTR_ENA_RIE 0x00000040	/* Receive Interrupt */
#define DMA_INTR_ENA_ERE 0x00004000	/* Early Receive */

#define DMA_INTR_NORMAL	(DMA_INTR_ENA_NIE | DMA_INTR_ENA_RIE | \
			DMA_INTR_ENA_TIE \
			/*| DMA_INTR_ENA_ERE | DMA_INTR_ENA_TUE*/)

/**** ABNORMAL INTERRUPT ****/
#define DMA_INTR_ENA_AIE 0x00008000	/* Abnormal Summary */
#define DMA_INTR_ENA_FBE 0x00002000	/* Fatal Bus Error */
#define DMA_INTR_ENA_ETE 0x00000400	/* Early Transmit */
#define DMA_INTR_ENA_RWE 0x00000200	/* Receive Watchdog */
#define DMA_INTR_ENA_RSE 0x00000100	/* Receive Stopped */
#define DMA_INTR_ENA_RUE 0x00000080	/* Receive Buffer Unavailable */
#define DMA_INTR_ENA_UNE 0x00000020	/* Tx Underflow */
#define DMA_INTR_ENA_OVE 0x00000010	/* Receive Overflow */
#define DMA_INTR_ENA_TJE 0x00000008	/* Transmit Jabber */
#define DMA_INTR_ENA_TSE 0x00000002	/* Transmit Stopped */

#define DMA_INTR_ABNORMAL	(DMA_INTR_ENA_AIE | DMA_INTR_ENA_FBE | \
				DMA_INTR_ENA_UNE)

/* DMA default interrupt mask */
#define DMA_INTR_DEFAULT_MASK	(DMA_INTR_NORMAL | DMA_INTR_ABNORMAL)
/* Disable DMA Rx IRQ (NAPI) */
#define DMA_INTR_NO_RX	(DMA_INTR_ENA_NIE | DMA_INTR_ENA_TIE | \
			DMA_INTR_ENA_TUE | DMA_INTR_ABNORMAL)

/* ****************************
 *  DMA Status register defines
 * ****************************/
#define DMA_STATUS_GPI		0x10000000	/* PMT interrupt */
#define DMA_STATUS_GMI		0x08000000	/* MMC interrupt */
#define DMA_STATUS_GLI		0x04000000	/* GMAC Line interface interrupt */
#define DMA_STATUS_GMI		0x08000000
#define DMA_STATUS_GLI		0x04000000
#define DMA_STATUS_EB_MASK	0x00380000	/* Error Bits Mask */
#define DMA_STATUS_EB_TX_ABORT	0x00080000	/* Error Bits - TX Abort */
#define DMA_STATUS_EB_RX_ABORT	0x00100000	/* Error Bits - RX Abort */
#define DMA_STATUS_TS_MASK	0x00700000	/* Transmit Process State */
#define DMA_STATUS_TS_SHIFT	20
#define DMA_STATUS_RS_MASK	0x000e0000	/* Receive Process State */
#define DMA_STATUS_RS_SHIFT	17
#define DMA_STATUS_NIS	0x00010000	/* Normal Interrupt Summary */
#define DMA_STATUS_AIS	0x00008000	/* Abnormal Interrupt Summary */
#define DMA_STATUS_ERI	0x00004000	/* Early Receive Interrupt */
#define DMA_STATUS_FBI	0x00002000	/* Fatal Bus Error Interrupt */
#define DMA_STATUS_ETI	0x00000400	/* Early Transmit Interrupt */
#define DMA_STATUS_RWT	0x00000200	/* Receive Watchdog Timeout */
#define DMA_STATUS_RPS	0x00000100	/* Receive Process Stopped */
#define DMA_STATUS_RU	0x00000080	/* Receive Buffer Unavailable */
#define DMA_STATUS_RI	0x00000040	/* Receive Interrupt */
#define DMA_STATUS_UNF	0x00000020	/* Transmit Underflow */
#define DMA_STATUS_OVF	0x00000010	/* Receive Overflow */
#define DMA_STATUS_TJT	0x00000008	/* Transmit Jabber Timeout */
#define DMA_STATUS_TU	0x00000004	/* Transmit Buffer Unavailable */
#define DMA_STATUS_TPS	0x00000002	/* Transmit Process Stopped */
#define DMA_STATUS_TI	0x00000001	/* Transmit Interrupt */

/* Other defines */
#define HASH_TABLE_SIZE 64
#define PAUSE_TIME 0x200

/* Flow Control defines */
#define FLOW_OFF	0
#define FLOW_RX		1
#define FLOW_TX		2
#define FLOW_AUTO	(FLOW_TX | FLOW_RX)

/* DMA STORE-AND-FORWARD Operation Mode */
#define SF_DMA_MODE 1

#define HW_CSUM 1
#define NO_HW_CSUM 0

/* GMAC TX FIFO is 8K, Rx FIFO is 16K */
#define BUF_SIZE_16KiB 16384
#define BUF_SIZE_8KiB 8192
#define BUF_SIZE_4KiB 4096
#define BUF_SIZE_2KiB 2048

/* Power Down and WOL */
#define PMT_NOT_SUPPORTED 0
#define PMT_SUPPORTED 1

/* Common MAC defines */
#define MAC_CTRL_REG		0x00000000	/* MAC Control */
#define MAC_ENABLE_TX		0x00000008	/* Transmitter Enable */
#define MAC_RNABLE_RX		0x00000004	/* Receiver Enable */

/* MAC Management Counters register */
#define MMC_CONTROL		0x00000100	/* MMC Control */
#define MMC_HIGH_INTR		0x00000104	/* MMC High Interrupt */
#define MMC_LOW_INTR		0x00000108	/* MMC Low Interrupt */
#define MMC_HIGH_INTR_MASK	0x0000010c	/* MMC High Interrupt Mask */
#define MMC_LOW_INTR_MASK	0x00000110	/* MMC Low Interrupt Mask */

#define MMC_CONTROL_MAX_FRM_MASK	0x0003ff8	/* Maximum Frame Size */
#define MMC_CONTROL_MAX_FRM_SHIFT	3
#define MMC_CONTROL_MAX_FRAME		0x7FF

struct stmmac_extra_stats {
	/* Transmit errors */
	unsigned long tx_underflow;
	unsigned long tx_carrier;
	unsigned long tx_losscarrier;
	unsigned long tx_heartbeat;
	unsigned long tx_deferred;
	unsigned long tx_vlan;
	unsigned long tx_jabber;
	unsigned long tx_frame_flushed;
	unsigned long tx_payload_error;
	unsigned long tx_ip_header_error;
	/* Receive errors */
	unsigned long rx_desc;
	unsigned long rx_partial;
	unsigned long rx_runt;
	unsigned long rx_toolong;
	unsigned long rx_collision;
	unsigned long rx_crc;
	unsigned long rx_lenght;
	unsigned long rx_mii;
	unsigned long rx_multicast;
	unsigned long rx_gmac_overflow;
	unsigned long rx_watchdog;
	unsigned long rx_filter;
	unsigned long rx_missed_cntr;
	unsigned long rx_overflow_cntr;
	/* Tx/Rx IRQ errors */
	unsigned long tx_undeflow_irq;
	unsigned long tx_process_stopped_irq;
	unsigned long tx_jabber_irq;
	unsigned long rx_overflow_irq;
	unsigned long rx_buf_unav_irq;
	unsigned long rx_process_stopped_irq;
	unsigned long rx_watchdog_irq;
	unsigned long tx_early_irq;
	unsigned long fatal_bus_error_irq;
	/* Extra info */
	unsigned long threshold; /* DMA tx/rx threshold (CSR6) */
	unsigned long tx_task_n; /* Tx tasklet invokations */
	unsigned long rx_poll_n; /* Rx poll method invokations */
	unsigned long tx_pkt_n; /* Frames transmitted */
	unsigned long rx_pkt_n; /* Frames received */
	/* average of the #frames/#schedul_times */
	unsigned long avg_tx_pkt_on_sched;
	unsigned long avg_rx_pkt_on_sched;
	unsigned long dma_tx_normal_irq; /* number of DMA normal TX irq */
	unsigned long dma_rx_normal_irq; /* number of DMA normal RX irq */
};
#define EXTRA_STATS 42

/* In case of GMAC, the device can compute the HW checksums and
 * found if the frame is corrupted. It can also decide to let the
 * upper layer to compute the Csum in Sw. */
enum rx_frame_status {
	good_frame = 0,
	discard_frame = 1,
	csum_none = 2,
};

/* Specific device structure VFP in order to mark the
 * difference between mac and gmac in terms of registers, descriptors etc.
 */
struct device_ops {
	/* MAC core initialization */
	void (*core_init) (unsigned long ioaddr);
	/* DMA core initialization */
	int (*dma_init) (unsigned long ioaddr, int pbl, u32 dma_tx, u32 dma_rx);
	/* Dump MAC registers */
	void (*dump_mac_regs) (unsigned long ioaddr);
	/* Dump DMA registers */
	void (*dump_dma_regs) (unsigned long ioaddr);
	/* Set tx/rx threshold in the csr6 register
	 * An invalid value enables the store-and-forward mode */
	void (*dma_mode) (unsigned long ioaddr, int txmode, int rxmode);
	/* To track extra statistic (if supported) */
	void (*dma_diagnostic_fr) (void *data, struct stmmac_extra_stats *x,
				   unsigned long ioaddr);
	/* RX descriptor ring initialization */
	void (*init_rx_desc) (struct dma_desc *p, unsigned int ring_size);
	/* TX descriptor ring initialization */
	void (*init_tx_desc) (struct dma_desc *p, unsigned int ring_size);
	/* Invoked by the xmit function to prepare the tx descriptor */
	void (*prepare_tx_desc) (struct dma_desc *p, int is_fs, int len,
				 int csum_flag);
	/* Invoked by the xmit function to close the tx descriptor */
	void (*close_tx_desc) (struct dma_desc *p);
	/* Clean the tx descriptor as soon as the tx irq is received */
	void (*release_tx_desc) (struct dma_desc *p);
	/* Clear interrupt on tx frame completion. When this bit is
	 * set an interrupt happens as soon as the frame is transmitted */
	void (*clear_tx_ic) (struct dma_desc *p);
	/* Interrupt after this number of packets have arrived. */
	void (*disable_rx_ic) (struct dma_desc *p, unsigned int ring_size,
			       int disable_ic);
	/* Last tx segment reports the transmit status */
	int (*get_tx_ls) (struct dma_desc *p);
	/* Set/get the owner of the descriptor */
	int (*get_tx_owner) (struct dma_desc *p);
	int (*get_rx_owner) (struct dma_desc *p);
	void (*set_tx_owner) (struct dma_desc *p);
	void (*set_rx_owner) (struct dma_desc *p);
	/* Get the receive frame size */
	int (*get_rx_frame_len) (struct dma_desc *p);
	/* Return the transmit status looking at the TDES1 */
	int (*tx_status) (void *data, struct stmmac_extra_stats *x,
			  struct dma_desc *p, unsigned long ioaddr);
	/* Return the reception status looking at the RDES1 */
	int (*rx_status) (void *data, struct stmmac_extra_stats *x,
			  struct dma_desc *p);
	/* Get the buffer size from the descriptor */
	int (*get_tx_len) (struct dma_desc *p);
	/* Multicast filter setting */
	void (*set_filter) (struct net_device * dev);
	/* Flow control setting */
	void (*flow_ctrl) (unsigned long ioaddr, unsigned int duplex,
			   unsigned int fc, unsigned int pause_time);
	/* Set power management mode (e.g. magic frame) */
	void (*pmt) (unsigned long ioaddr, unsigned long mode);
	/* Handle extra events on specific interrupts hw dependent */
	void (*host_irq_status) (unsigned long ioaddr);
};

struct mac_link {
	int port;
	int duplex;
	int speed;
};

struct mii_regs {
	unsigned int addr;	/* MII Address */
	unsigned int data;	/* MII Data */
};

struct hw_cap {
	unsigned int addr_high;	/* Multicast Hash Table High */
	unsigned int addr_low;	/* Multicast Hash Table Low */
	unsigned int version;	/* Core Version register (GMAC) */
	unsigned int pmt;	/* Power-Down mode (GMAC) */
	struct mac_link link;
	struct mii_regs mii;
};

struct mac_device_info {
	struct hw_cap hw;
	struct device_ops *ops;
};

struct mac_device_info *gmac_setup(unsigned long addr);
struct mac_device_info *mac100_setup(unsigned long addr);
