/*
 * PCI-RDK version 1 registers (PLX 9054, EPLD)
 *
 * Copyright (C) 2005 PLX Technology, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* PCI-RDK EPLD Registers */
#define RDK_EPLD_IO_REGISTER1				0x00000000
#define		RDK_EPLD_USB_RESET				0
#define		RDK_EPLD_USB_POWERDOWN				1
#define		RDK_EPLD_USB_WAKEUP				2
#define		RDK_EPLD_USB_EOT				3
#define		RDK_EPLD_DPPULL					4
#define RDK_EPLD_IO_REGISTER2				0x00000004
#define		RDK_EPLD_BUSWIDTH				0
#define		RDK_EPLD_USER					2
#define		RDK_EPLD_RESET_INTERRUPT_ENABLE			3
#define		RDK_EPLD_DMA_TIMEOUT_ENABLE			4
#define RDK_EPLD_STATUS_REGISTER			0x00000008
#define		RDK_EPLD_USB_LRESET				0
#define RDK_EPLD_REVISION_REGISTER			0x0000000c

/* PCI-RDK PLX 9054 Registers */
#define INTCSR						0x68
#define		PCI_INTERRUPT_ENABLE				8
#define		LOCAL_INTERRUPT_INPUT_ENABLE			11
#define		LOCAL_INPUT_INTERRUPT_ACTIVE			15
#define		LOCAL_DMA_CHANNEL_0_INTERRUPT_ENABLE		18
#define		LOCAL_DMA_CHANNEL_1_INTERRUPT_ENABLE		19
#define		DMA_CHANNEL_0_INTERRUPT_ACTIVE			21
#define		DMA_CHANNEL_1_INTERRUPT_ACTIVE			22
#define CNTRL						0x6C
#define		RELOAD_CONFIGURATION_REGISTERS			29
#define		PCI_ADAPTER_SOFTWARE_RESET			30
#define DMAMODE0					0x80
#define		LOCAL_BUS_WIDTH					0
#define		INTERNAL_WAIT_STATES				2
#define		TA_READY_INPUT_ENABLE				6
#define		LOCAL_BURST_ENABLE				8
#define		SCATTER_GATHER_MODE				9
#define		DONE_INTERRUPT_ENABLE				10
#define		LOCAL_ADDRESSING_MODE				11
#define		DEMAND_MODE					12
#define		DMA_EOT_ENABLE					14
#define		FAST_SLOW_TERMINATE_MODE_SELECT			15
#define		DMA_CHANNEL_INTERRUPT_SELECT			17
#define DMAPADR0					0x84
#define DMALADR0					0x88
#define DMASIZ0						0x8c
#define DMADPR0						0x90
#define		DESCRIPTOR_LOCATION				0
#define		END_OF_CHAIN					1
#define		INTERRUPT_AFTER_TERMINAL_COUNT			2
#define		DIRECTION_OF_TRANSFER				3
#define DMACSR0						0xa8
#define		CHANNEL_ENABLE					0
#define		CHANNEL_START					1
#define		CHANNEL_ABORT					2
#define		CHANNEL_CLEAR_INTERRUPT				3
#define		CHANNEL_DONE					4
#define DMATHR						0xb0
#define LBRD1						0xf8
#define		MEMORY_SPACE_LOCAL_BUS_WIDTH			0
#define		W8_BIT							0
#define		W16_BIT							1

/* Special OR'ing of INTCSR bits */
#define LOCAL_INTERRUPT_TEST \
((1 << LOCAL_INPUT_INTERRUPT_ACTIVE) | \
 (1 << LOCAL_INTERRUPT_INPUT_ENABLE))

#define DMA_CHANNEL_0_TEST \
((1 << DMA_CHANNEL_0_INTERRUPT_ACTIVE) | \
 (1 << LOCAL_DMA_CHANNEL_0_INTERRUPT_ENABLE))

#define DMA_CHANNEL_1_TEST \
((1 << DMA_CHANNEL_1_INTERRUPT_ACTIVE) | \
 (1 << LOCAL_DMA_CHANNEL_1_INTERRUPT_ENABLE))

/* EPLD Registers */
#define RDK_EPLD_IO_REGISTER1			0x00000000
#define 	RDK_EPLD_USB_RESET			0
#define 	RDK_EPLD_USB_POWERDOWN			1
#define 	RDK_EPLD_USB_WAKEUP			2
#define 	RDK_EPLD_USB_EOT			3
#define 	RDK_EPLD_DPPULL				4
#define RDK_EPLD_IO_REGISTER2			0x00000004
#define 	RDK_EPLD_BUSWIDTH			0
#define 	RDK_EPLD_USER				2
#define 	RDK_EPLD_RESET_INTERRUPT_ENABLE		3
#define 	RDK_EPLD_DMA_TIMEOUT_ENABLE		4
#define RDK_EPLD_STATUS_REGISTER		0x00000008
#define RDK_EPLD_USB_LRESET				0
#define RDK_EPLD_REVISION_REGISTER		0x0000000c

#define EPLD_IO_CONTROL_REGISTER		0x400
#define 	NET2272_RESET				0
#define 	BUSWIDTH				1
#define 	MPX_MODE				3
#define 	USER					4
#define 	DMA_TIMEOUT_ENABLE			5
#define 	DMA_CTL_DACK				6
#define 	EPLD_DMA_ENABLE				7
#define EPLD_DMA_CONTROL_REGISTER		0x800
#define 	SPLIT_DMA_MODE				0
#define 	SPLIT_DMA_DIRECTION			1
#define 	SPLIT_DMA_ENABLE			2
#define 	SPLIT_DMA_INTERRUPT_ENABLE		3
#define 	SPLIT_DMA_INTERRUPT			4
#define 	EPLD_DMA_MODE				5
#define		EPLD_DMA_CONTROLLER_ENABLE		7
#define SPLIT_DMA_ADDRESS_LOW			0xc00
#define SPLIT_DMA_ADDRESS_HIGH			0x1000
#define SPLIT_DMA_BYTE_COUNT_LOW		0x1400
#define SPLIT_DMA_BYTE_COUNT_HIGH		0x1800
#define EPLD_REVISION_REGISTER			0x1c00
#define SPLIT_DMA_RAM				0x4000
#define DMA_RAM_SIZE				0x1000

