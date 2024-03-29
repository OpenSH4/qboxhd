/*
 * arch/ppc/kernel/pci_auto.c
 *
 * PCI autoconfiguration library
 *
 * Author: Matt Porter <mporter@mvista.com>
 *
 * Copyright 2000 MontaVista Software Inc.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#include <common.h>

#ifdef CONFIG_PCI

#include <pci.h>



#ifdef DEBUG
#define DEBUGF(x...) printf(x)
#else
#define DEBUGF(x...)
#endif /* DEBUG */

#define	PCIAUTO_IDE_MODE_MASK		0x05

/*
 *
 */

void pciauto_region_init(struct pci_region* res)
{
	res->bus_lower = res->bus_start;
}

void pciauto_region_align(struct pci_region *res, unsigned long size)
{
	res->bus_lower = ((res->bus_lower - 1) | (size - 1)) + 1;
}

int pciauto_region_allocate(struct pci_region* res, unsigned int size, unsigned int *bar)
{
	unsigned long addr;

	if (!res) {
		DEBUGF("No resource");
		goto error;
	}

	addr = ((res->bus_lower - 1) | (size - 1)) + 1;

	if (addr - res->bus_start + size > res->size) {
		DEBUGF("No room in resource");
		goto error;
	}

	res->bus_lower = addr + size;

	DEBUGF("address=0x%lx", addr);

	*bar = addr;
	return 0;

 error:
	*bar = 0xffffffff;
	return -1;
}

/*
 *
 */

void pciauto_setup_device(struct pci_controller *hose,
			  pci_dev_t dev, int bars_num,
			  struct pci_region *mem,
			  struct pci_region *io)
{
	unsigned int bar_value, bar_response, bar_size;
	unsigned int cmdstat = 0;
	struct pci_region *bar_res;
	int bar, bar_nr = 0;
	int found_mem64 = 0;

	pci_hose_read_config_dword(hose, dev, PCI_COMMAND, &cmdstat);
	cmdstat = (cmdstat & ~(PCI_COMMAND_IO | PCI_COMMAND_MEMORY)) | PCI_COMMAND_MASTER;

	for (bar = PCI_BASE_ADDRESS_0; bar <= PCI_BASE_ADDRESS_0 + (bars_num*4); bar += 4) {
		/* Tickle the BAR and get the response */
		pci_hose_write_config_dword(hose, dev, bar, 0xffffffff);
		pci_hose_read_config_dword(hose, dev, bar, &bar_response);

		/* If BAR is not implemented go to the next BAR */
		if (!bar_response)
			continue;

		found_mem64 = 0;

		/* Check the BAR type and set our address mask */
		if (bar_response & PCI_BASE_ADDRESS_SPACE) {
			bar_size = ~(bar_response & PCI_BASE_ADDRESS_IO_MASK) + 1;
			bar_res = io;

			DEBUGF("PCI Autoconfig: BAR %d, I/O, size=0x%x, ", bar_nr, bar_size);
		} else {
			if ( (bar_response & PCI_BASE_ADDRESS_MEM_TYPE_MASK) ==
			     PCI_BASE_ADDRESS_MEM_TYPE_64)
				found_mem64 = 1;

			bar_size = ~(bar_response & PCI_BASE_ADDRESS_MEM_MASK) + 1;
			bar_res = mem;

			DEBUGF("PCI Autoconfig: BAR %d, Mem, size=0x%x, ", bar_nr, bar_size);
		}

		if (pciauto_region_allocate(bar_res, bar_size, &bar_value) == 0) {
			/* Write it out and update our limit */
			pci_hose_write_config_dword(hose, dev, bar, bar_value);

			/*
			 * If we are a 64-bit decoder then increment to the
			 * upper 32 bits of the bar and force it to locate
			 * in the lower 4GB of memory.
			 */
			if (found_mem64) {
				bar += 4;
				pci_hose_write_config_dword(hose, dev, bar, 0x00000000);
			}

			cmdstat |= (bar_response & PCI_BASE_ADDRESS_SPACE) ?
				PCI_COMMAND_IO : PCI_COMMAND_MEMORY;
		}

		DEBUGF("\n");

		bar_nr++;
	}

	pci_hose_write_config_dword(hose, dev, PCI_COMMAND, cmdstat);
	pci_hose_write_config_byte(hose, dev, PCI_CACHE_LINE_SIZE, 0x08);
	pci_hose_write_config_byte(hose, dev, PCI_LATENCY_TIMER, 0x80);
}

static void pciauto_prescan_setup_bridge(struct pci_controller *hose,
					 pci_dev_t dev, int sub_bus)
{
	struct pci_region *pci_mem = hose->pci_mem;
	struct pci_region *pci_io = hose->pci_io;
	unsigned int cmdstat;

	pci_hose_read_config_dword(hose, dev, PCI_COMMAND, &cmdstat);

	/* Configure bus number registers */
	pci_hose_write_config_byte(hose, dev, PCI_PRIMARY_BUS, PCI_BUS(dev));
	pci_hose_write_config_byte(hose, dev, PCI_SECONDARY_BUS, sub_bus);
	pci_hose_write_config_byte(hose, dev, PCI_SUBORDINATE_BUS, 0xff);

	if (pci_mem) {
		/* Round memory allocator to 1MB boundary */
		pciauto_region_align(pci_mem, 0x100000);

		/* Set up memory and I/O filter limits, assume 32-bit I/O space */
		pci_hose_write_config_word(hose, dev, PCI_MEMORY_BASE,
					(pci_mem->bus_lower & 0xfff00000) >> 16);

		cmdstat |= PCI_COMMAND_MEMORY;
	}

	if (pci_io) {
		/* Round I/O allocator to 4KB boundary */
		pciauto_region_align(pci_io, 0x1000);

		pci_hose_write_config_byte(hose, dev, PCI_IO_BASE,
					(pci_io->bus_lower & 0x0000f000) >> 8);
		pci_hose_write_config_word(hose, dev, PCI_IO_BASE_UPPER16,
					(pci_io->bus_lower & 0xffff0000) >> 16);

		cmdstat |= PCI_COMMAND_IO;
	}

	/* We don't support prefetchable memory for now, so disable */
	pci_hose_write_config_word(hose, dev, PCI_PREF_MEMORY_BASE, 0x1000);
	pci_hose_write_config_word(hose, dev, PCI_PREF_MEMORY_LIMIT, 0x1000);

	/* Enable memory and I/O accesses, enable bus master */
	pci_hose_write_config_dword(hose, dev, PCI_COMMAND, cmdstat | PCI_COMMAND_MASTER);
}

static void pciauto_postscan_setup_bridge(struct pci_controller *hose,
					  pci_dev_t dev, int sub_bus)
{
	struct pci_region *pci_mem = hose->pci_mem;
	struct pci_region *pci_io = hose->pci_io;

	/* Configure bus number registers */
	pci_hose_write_config_byte(hose, dev, PCI_SUBORDINATE_BUS, sub_bus);

	if (pci_mem) {
		/* Round memory allocator to 1MB boundary */
		pciauto_region_align(pci_mem, 0x100000);

		pci_hose_write_config_word(hose, dev, PCI_MEMORY_LIMIT,
					(pci_mem->bus_lower-1) >> 16);
	}

	if (pci_io) {
		/* Round I/O allocator to 4KB boundary */
		pciauto_region_align(pci_io, 0x1000);

		pci_hose_write_config_byte(hose, dev, PCI_IO_LIMIT,
					((pci_io->bus_lower-1) & 0x0000f000) >> 8);
		pci_hose_write_config_word(hose, dev, PCI_IO_LIMIT_UPPER16,
					((pci_io->bus_lower-1) & 0xffff0000) >> 16);
	}
}

/*
 *
 */

void pciauto_config_init(struct pci_controller *hose)
{
	int i;

	hose->pci_io = hose->pci_mem = NULL;

	for (i=0; i<hose->region_count; i++) {
		switch(hose->regions[i].flags) {
		case PCI_REGION_IO:
			if (!hose->pci_io ||
			    hose->pci_io->size < hose->regions[i].size)
				hose->pci_io = hose->regions + i;
			break;
		case PCI_REGION_MEM:
			if (!hose->pci_mem ||
			    hose->pci_mem->size < hose->regions[i].size)
				hose->pci_mem = hose->regions + i;
			break;
		}
	}


	if (hose->pci_mem) {
		pciauto_region_init(hose->pci_mem);

		DEBUGF("PCI Autoconfig: Memory region: [%lx-%lx]\n",
		    hose->pci_mem->bus_start,
		    hose->pci_mem->bus_start + hose->pci_mem->size - 1);
	}

	if (hose->pci_io) {
		pciauto_region_init(hose->pci_io);

		DEBUGF("PCI Autoconfig: I/O region: [%lx-%lx]\n",
		    hose->pci_io->bus_start,
		    hose->pci_io->bus_start + hose->pci_io->size - 1);
	}
}

/* HJF: Changed this to return int. I think this is required
 * to get the correct result when scanning bridges
 */
int pciauto_config_device(struct pci_controller *hose, pci_dev_t dev)
{
	unsigned int sub_bus = PCI_BUS(dev);
	unsigned short class;
	unsigned char prg_iface;
	int n;

	pci_hose_read_config_word(hose, dev, PCI_CLASS_DEVICE, &class);

	switch(class) {
	case PCI_CLASS_BRIDGE_PCI:
		hose->current_busno++;
		pciauto_setup_device(hose, dev, 2, hose->pci_mem, hose->pci_io);

		DEBUGF("PCI Autoconfig: Found P2P bridge, device %d\n", PCI_DEV(dev));

		/* Passing in current_busno allows for sibling P2P bridges */
		pciauto_prescan_setup_bridge(hose, dev, hose->current_busno);
		/*
		 * need to figure out if this is a subordinate bridge on the bus
		 * to be able to properly set the pri/sec/sub bridge registers.
		 */
		n = pci_hose_scan_bus(hose, hose->current_busno);

		/* figure out the deepest we've gone for this leg */
		sub_bus = max(n, sub_bus);
		pciauto_postscan_setup_bridge(hose, dev, sub_bus);

		sub_bus = hose->current_busno;
		break;

	case PCI_CLASS_STORAGE_IDE:
		pci_hose_read_config_byte(hose, dev, PCI_CLASS_PROG, &prg_iface);
		if (!(prg_iface & PCIAUTO_IDE_MODE_MASK)) {
			DEBUGF("PCI Autoconfig: Skipping legacy mode IDE controller\n");
			return sub_bus;
		}

		pciauto_setup_device(hose, dev, 6, hose->pci_mem, hose->pci_io);
		break;

	case PCI_CLASS_BRIDGE_CARDBUS:
		/* just do a minimal setup of the bridge, let the OS take care of the rest */
		pciauto_setup_device(hose, dev, 0, hose->pci_mem, hose->pci_io);

		DEBUGF("PCI Autoconfig: Found P2CardBus bridge, device %d\n", PCI_DEV(dev));

		hose->current_busno++;
		break;

#ifdef CONFIG_MPC5200
	case PCI_CLASS_BRIDGE_OTHER:
		DEBUGF("PCI Autoconfig: Skipping bridge device %d\n",
		       PCI_DEV(dev));
		break;
#endif

	default:
		pciauto_setup_device(hose, dev, 6, hose->pci_mem, hose->pci_io);
		break;
	}

	return sub_bus;
}

#endif /* CONFIG_PCI */
