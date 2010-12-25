/*
* STM TSMerger driver
*
* Copyright (c) STMicroelectronics 2006
*
*   Author:Peter Bennett <peter.bennett@st.com>
*
*      This program is free software; you can redistribute it and/or
*      modify it under the terms of the GNU General Public License as
*      published by the Free Software Foundation; either version 2 of
*      the License, or (at your option) any later version.
*/

#include <linux/slab.h>
#include <linux/pagemap.h>
#include <linux/dma-mapping.h>
#include <linux/stm/stm-dma.h>
#include <asm/io.h>

#include <linux/stm/stm-frontend.h>
#include "tsmerger.h"
#include "dvb_module.h"
#if defined(CONFIG_SH_QBOXHD_1_0) || defined(CONFIG_SH_QBOXHD_MINI_1_0)
#include "pti.h"
#else
#include "st-common.h"
#endif

#include <linux/delay.h>

//__TDT__: many modifications in this file

#ifndef TSM_NUM_PTI_ALT_OUT
unsigned long TSM_NUM_PTI_ALT_OUT;
unsigned long TSM_NUM_1394_ALT_OUT;
#define LOAD_TSM_DATA
#endif

#define TSMergerBaseAddress   	0x19242000

#define TSM_STREAM0_CFG      	0x0000
#define TSM_STREAM1_CFG      	0x0020
#define TSM_STREAM2_CFG      	0x0040
#define TSM_STREAM3_CFG      	0x0060
#define TSM_STREAM4_CFG      	0x0080

/* for all 7109er */
#define TSM_STREAM5_CFG      	0x00a0
#define TSM_STREAM6_CFG      	0x00c0


#define TSM_STREAM0_SYNC   	0x0008
#define TSM_STREAM1_SYNC   	0x0028
#define TSM_STREAM2_SYNC   	0x0048
#define TSM_STREAM3_SYNC   	0x0068
#define TSM_STREAM4_SYNC   	0x0088

/* for all 7109er */
#define TSM_STREAM5_SYNC        0x00a8
#define TSM_STREAM6_SYNC        0x00c8


#define TSM_STREAM0_STA      	0x0010
#define TSM_STREAM1_STA      	0x0030
#define TSM_STREAM2_STA      	0x0050
#define TSM_STREAM3_STA      	0x0070
#define TSM_STREAM4_STA      	0x0090


#define TSM_PTI_DEST      	0x0200
#define TSM_PTI_SEL      	TSM_PTI_DEST

#define TSM_1394_DEST      	0x0210
#define TSM_1394_SEL      	TSM_1394_DEST

#define TSM_PROG_CNT0      	0x0400
#define TSM_PROG_CNT1      	0x0410

#define SWTS_CFG(x)   (0x0600 + (x*0x10))

#define PTI_ALT_OUT_CFG      	0x0800
#define TS_1394_CFG      	0x0810
#define TSM_SYS_CFG      	0x0820
#define TSM_SW_RST      	0x0830

#define TSM_SWTS      		0x010BE000

#define SysConfigBaseAddress    0x19001000
#define SYS_CFG0      		0x100
#define SYS_CFG1      		0x104

unsigned long tsm_io;

extern int highSR;

#if defined(CONFIG_SH_QBOXHD_1_0) || defined(CONFIG_SH_QBOXHD_MINI_1_0)
void paceSwtsByPti(void);
#endif

/* ****************************************
 * Dagobert:
 * Taken from new player - frontend
 * ****************************************
 */

#define SWTS_FDMA_ALIGNMENT 127

/* Maximum number of pages we send in 1 FDMA transfer */
#define MAX_SWTS_PAGES 260

struct stm_tsm_handle {
  unsigned long tsm_io;
  unsigned long tsm_swts;

  int swts_channel;

  /* FDMA for SWTS */
  int                        fdma_channel;
  struct stm_dma_params      fdma_params;
  unsigned long              fdma_reqline;
  struct stm_dma_req        *fdma_req;

  struct page               *swts_pages[MAX_SWTS_PAGES];
  struct stm_dma_params      swts_params[MAX_SWTS_PAGES];
  struct scatterlist         swts_sg[MAX_SWTS_PAGES];
};

static struct stm_tsm_handle tsm_handle;

struct stm_dma_req_config fdma_req_config = {
  .rw        = REQ_CONFIG_WRITE,
  .opcode    = REQ_CONFIG_OPCODE_32,
  .count     = 4,
  .increment = 0,
  .hold_off  = 0,
  .initiator = 0,
};

static const char *fdmac_id[]    = { STM_DMAC_ID, NULL };
static const char *fdma_cap_hb[] = { STM_DMA_CAP_HIGH_BW, NULL };

void stm_tsm_inject_data(struct stm_tsm_handle *handle, u32 *data, off_t size)
{
  int blocks = (size + 127) / 128;
  int count  = size;
  int words;
  u32 *p = data;
  int n;
  int m;
  u32 *addr = (u32*)handle->tsm_swts;

//   DVB_DEBUG("%s > size = %d, block %d\n", __FUNCTION__, size, blocks);
  DVB_DEBUG("%s > size = %lu, block %d\n", __FUNCTION__, size, blocks);

  for (n=0;n<blocks;n++) {
    while( !(readl(handle->tsm_io + SWTS_CFG(0)) & TSM_SWTS_REQ) ) {
      printk("%s: Waiting for FIFO %x\n",__FUNCTION__,readl(handle->tsm_io));
      msleep(10);
    }

    if (count > 128)
      words = 128/4;
    else
      words = count / 4;

    count -= words * 4;

    for (m=0;m<words;m++)
      *addr = *p++;
  }

  DVB_DEBUG("%s < \n", __FUNCTION__);

}

int stm_tsm_inject_user_data(const char __user *data, off_t size)
{
  int nr_pages;
  int ret = 0;
  int page_offset;
  int remaining;
  int n;
  struct stm_dma_params *in_param;
  struct scatterlist    *curr_sg;
  int sg_count;
  unsigned long start = (unsigned long)data;
  unsigned long len = size;
  unsigned long taddr,tlen;
  int extra = 0;
  struct stm_tsm_handle *handle = &tsm_handle;

//   DVB_DEBUG("%s > size = %d\n", __FUNCTION__, size);
  DVB_DEBUG("%s > size = %lu\n", __FUNCTION__, size);

  DVB_DEBUG("status: 0x%08x",  readl (tsm_io + TSM_STREAM3_STA));

  paceSwtsByPti();

  if (start & SWTS_FDMA_ALIGNMENT) {
    int hand = (SWTS_FDMA_ALIGNMENT + 1) - (start & SWTS_FDMA_ALIGNMENT);
    // we need a copy to user...
    DVB_DEBUG("inject 1\n");
    stm_tsm_inject_data(handle, (u32*)data, hand);
    start += hand;
    len   -= hand;
  }

  if (len & SWTS_FDMA_ALIGNMENT) {
    extra = len & SWTS_FDMA_ALIGNMENT;
    len = len & ~SWTS_FDMA_ALIGNMENT;
  }

  nr_pages = (PAGE_ALIGN(start + len) -
	      (start & PAGE_MASK)) >> PAGE_SHIFT;

  down_read(&current->mm->mmap_sem);
  ret = get_user_pages(current, current->mm, start,
		       nr_pages, READ, 0, handle->swts_pages, NULL);
  up_read(&current->mm->mmap_sem);

  if (ret < nr_pages) {
    nr_pages = ret;
    ret = -EINVAL;
    DVB_DEBUG("ret = %d < nr_pages %d\n", ret, nr_pages);
    goto out_unmap;
  }

  page_offset = start & (PAGE_SIZE-1);
  remaining = len;

  for (n=0; n<nr_pages; n++) {
    int copy = min_t(int, PAGE_SIZE - page_offset, remaining);
    handle->swts_sg[n].page   = handle->swts_pages[n];
    handle->swts_sg[n].offset = page_offset;
    handle->swts_sg[n].length = copy;

    page_offset = 0;
    remaining -= copy;
//DVB_DEBUG("nr_pages %d, length %d, remaining %d\n", nr_pages, copy, remaining);
  }

  sg_count = dma_map_sg(NULL, &handle->swts_sg[0], nr_pages, DMA_TO_DEVICE);

  in_param = &handle->swts_params[0];
  curr_sg = &handle->swts_sg[0];

  /* Go through the list and unscatter it  */
  taddr = sg_dma_address(curr_sg);
  tlen  = sg_dma_len(curr_sg);

  for (n=0; n<sg_count; n++) {
    unsigned long naddr,nlen;
    dma_params_addrs(in_param, taddr, 0x1a300000 , tlen);
    dma_params_link(in_param,in_param+1);

    curr_sg++;

    naddr = sg_dma_address(curr_sg);
    nlen  = sg_dma_len(curr_sg);

    if (taddr + tlen == naddr && ((n+1) != sg_count)) {
      tlen += nlen;
    } else {
      taddr = naddr;
      tlen  = nlen;
      in_param++;
    }
  }

  in_param--;
  dma_params_link(in_param,NULL);

  dma_compile_list(handle->fdma_channel,&handle->swts_params[0] , GFP_ATOMIC);

  ret = dma_xfer_list(handle->fdma_channel, &handle->swts_params[0]);

  if (ret)
  {
    printk("xfer ret = %d\n", ret);
    goto out_unmap;
  }
  dma_wait_for_completion(handle->fdma_channel);

  dma_unmap_sg(NULL, sg, nr_pages, DMA_TO_DEVICE);

  if (extra)
  {
    DVB_DEBUG("inject 2\n");
    stm_tsm_inject_data(handle,(u32*)(data + size - extra), extra);
  }

out_unmap:
  for (n = 0; n < nr_pages; n++)
    page_cache_release(handle->swts_pages[n]);

  DVB_DEBUG("%s < ret = %d\n", __FUNCTION__, ret);

  return ret;
}
EXPORT_SYMBOL(stm_tsm_inject_user_data);

/* ****************************************
 * END: Taken from new player - frontend
 * ****************************************
 */

void stm_tsm_interrupt ( void )
{
  int n;
  for (n=0;n<2;n++)
    {
      int status = readl(tsm_io + TSM_STREAM_STATUS(n));

      if (status & TSM_RAM_OVERFLOW)
      {
      writel( readl(tsm_io + TSM_STREAM_CONF(n)) & ~TSM_STREAM_ON,
         tsm_io + TSM_STREAM_CONF(n));

      udelay(500);

      writel( status & ~TSM_RAM_OVERFLOW, tsm_io + TSM_STREAM_STATUS(n) );
      printk(KERN_WARNING"%s: TSMerger RAM Overflow on channel %d(%x), deploying work around\n",__FUNCTION__,n,status);
      writel( TSM_CHANNEL_RESET, tsm_io + TSM_STREAM_CONF2(n) );

      udelay(500);

      writel( readl(tsm_io + TSM_STREAM_CONF(n)) | TSM_STREAM_ON,
          tsm_io + TSM_STREAM_CONF(n));
      }
    }
}


int read_tsm(int a)
{
  return readl(tsm_io + TSM_STREAM_STATUS(a));
}

#if defined(CONFIG_SH_QBOXHD_1_0) || defined(CONFIG_SH_QBOXHD_MINI_1_0)
void stm_tsm_init (int use_cimax, struct plat_frontend_config *config)
#else
void stm_tsm_init (int use_cimax)
#endif
{

#if defined(HL101) // TODO fix cimax first
	use_cimax = 0;
#endif

   if (use_cimax != 0)
   {

      //route tsmerger to cimax and then to pti
      //first configure sysconfig
      unsigned long    reg_sys_config = 0;
      unsigned long    reg_config = 0;
      unsigned int     ret;
      int              n;

      /*
       * 0xbc4733
       * sync = 3
       * drop = 3
       * soap toker = 0x47
       * packet len = 188
       */

#if !defined(FORTIS_HDBOX)
      unsigned int stream_sync = 0xbc4733;
#else
      unsigned int stream_sync = 0xbc4722;
#endif

      /* Streamconfig + jeweils ram start s.u.
       * 0x20020 =
       * add_tag_bytes = 1
       * ram = 0
       * pri = 2 (binaer 10)

       * 0x30020 =
       * add_tag_bytes = 1
       * ram = 0
       * pri = 3 (binaer 11)
       */

      printk("Routing streams through cimax\n");

	  reg_sys_config = (unsigned long)ioremap(SysConfigBaseAddress, 0x0900 /* ??? */);

      /*
	 ->TSIN0 routed to TSIN2
	 ->TSMerger TSIN2 receives TSIN0 (based on config before)
	 ->TS interface is as indicated by TSMerger configuration bits
       */

#if  !defined(TF7700) && !defined(UFS922) && !defined(FORTIS_HDBOX)  && !defined(HL101) \
	&& !defined(CONFIG_SH_QBOXHD_1_0) && !defined(CONFIG_SH_QBOXHD_MINI_1_0)
      /*
	 The UFS910 hardware requires the following connections:
	 ->TSIN1 routed to TSIN2
	 ->TSMerger TSIN2 receives TSIN1 (based on config before)
	 ->TS interface is as indicated by TSMerger configuration bits
       */

      ctrl_outl(0x6, reg_sys_config + SYS_CFG0);
/* from fw 202 rc ->see also pti */
	  ctrl_outl(0x2, reg_sys_config + SYS_CFG0);

#elif  defined(FORTIS_HDBOX)
	/* ->TSIN0 routes to TSIN2
	 */
      ctrl_outl(0x2, reg_sys_config + SYS_CFG0);
#else
      /*
	 The TF7700 hardware requires the following connections:
	 - StarCI2Win TSO1 => TSIN0 => IN0 => OUT (to PTI)
	 - StarCI2Win TSO2 => TSIN1 => IN1 => OUT (to PTI)
	 - TS1394 (TSIN2) => StarCI2Win TSI3
      */
      ctrl_outl(0x0, reg_sys_config + SYS_CFG0);
#endif
      ctrl_outl(0x0, reg_sys_config + SYS_CFG1);

      /* set up tsmerger */
	  tsm_handle.tsm_io = tsm_io = reg_config = (unsigned long)ioremap(TSMergerBaseAddress, 0x0900);

      /* 1. Reset */
      ctrl_outl(0x0, reg_config + TSM_SW_RST);
      ctrl_outl(0x6, reg_config + TSM_SW_RST);

      /* set all registers to a defined state */
      ctrl_outl(0x0, reg_config + TSM_STREAM0_CFG);
      ctrl_outl(0x0, reg_config + TSM_STREAM0_SYNC);
      ctrl_outl(0x0, reg_config + TSM_STREAM0_STA);
      ctrl_outl(0x0, reg_config + 0x18 /* reserved ??? */);

      ctrl_outl(0x0, reg_config + TSM_STREAM1_CFG);
      ctrl_outl(0x0, reg_config + TSM_STREAM1_SYNC);
      ctrl_outl(0x0, reg_config + TSM_STREAM1_STA);
      ctrl_outl(0x0, reg_config + 0x38 /* reserved ??? */);

      ctrl_outl(0x0, reg_config + TSM_STREAM2_CFG);
      ctrl_outl(0x0, reg_config + TSM_STREAM2_SYNC);
      ctrl_outl(0x0, reg_config + TSM_STREAM2_STA);
      ctrl_outl(0x0, reg_config + 0x58 /* reserved ??? */);

      ctrl_outl(0x0, reg_config + TSM_STREAM3_CFG);
      ctrl_outl(0x0, reg_config + TSM_STREAM3_SYNC);
      ctrl_outl(0x0, reg_config + TSM_STREAM3_STA);
      ctrl_outl(0x0, reg_config + 0x78 /* reserved ??? */);

      ctrl_outl(0x0, reg_config + TSM_STREAM4_CFG);
      ctrl_outl(0x0, reg_config + TSM_STREAM4_SYNC);
      ctrl_outl(0x0, reg_config + TSM_STREAM4_STA);
      ctrl_outl(0x0, reg_config + 0x98 /* reserved ??? */);

      ctrl_outl(0x0, reg_config + TSM_PROG_CNT0);
      ctrl_outl(0x0, reg_config + TSM_PROG_CNT1);

      ctrl_outl(0x0, reg_config + TSM_PTI_SEL);
      ctrl_outl(0x0, reg_config + TSM_1394_SEL);
      ctrl_outl(0x0, reg_config + PTI_ALT_OUT_CFG);
      ctrl_outl(0x0, reg_config + TS_1394_CFG);

      ctrl_outl(0x0, reg_config + SWTS_CFG(0));

#if  defined(FORTIS_HDBOX) || defined(UFS922) || defined(TF7700) || defined(HL101) \
	|| defined(CONFIG_SH_QBOXHD_1_0) || defined(CONFIG_SH_QBOXHD_MINI_1_0)
      ctrl_outl(0x0, reg_config + SWTS_CFG(1));
      ctrl_outl(0x0, reg_config + SWTS_CFG(2));
#endif

      ctrl_outl(0x0, reg_config + TSM_SYS_CFG);
      ctrl_outl(0x0, reg_config + TSM_SYS_CFG); /* 2 times ? */

      /* RAM partitioning of streams */
      ctrl_outl(0x0, reg_config + TSM_STREAM0_CFG);
      ctrl_outl(0x500, reg_config + TSM_STREAM1_CFG);
      ctrl_outl(0xa00, reg_config + TSM_STREAM2_CFG);
      ctrl_outl(0xe00, reg_config + TSM_STREAM3_CFG);
      ctrl_outl(0x1300, reg_config + TSM_STREAM4_CFG);

#if  defined(FORTIS_HDBOX) || defined(UFS922) || defined(HL101)
      ctrl_outl(0x1700, reg_config + TSM_STREAM5_CFG);
      ctrl_outl(0x1a00, reg_config + TSM_STREAM6_CFG);
#endif

#if  defined(FORTIS_HDBOX)
      /* configure streams: */
      /* add tag bytes to stream + stream priority */
      ret = ctrl_inl(reg_config + TSM_STREAM0_CFG);
      ctrl_outl(ret | (0x40020), reg_config + TSM_STREAM0_CFG);

	  ret = ctrl_inl(reg_config + TSM_STREAM4_CFG);
	  ctrl_outl(ret | (0x40020), reg_config + TSM_STREAM4_CFG);

	  ret = ctrl_inl(reg_config + TSM_STREAM5_CFG);
	  ctrl_outl(ret | (0x40020), reg_config + TSM_STREAM5_CFG);

#else
      /* configure streams: */
      /* add tag bytes to stream + stream priority */
      ret = ctrl_inl(reg_config + TSM_STREAM0_CFG);
      ctrl_outl(ret | (0x20020), reg_config + TSM_STREAM0_CFG);
#endif

      ctrl_outl(stream_sync, reg_config + TSM_STREAM0_SYNC);
      ctrl_outl(0x0, reg_config + 0x18 /* reserved ??? */);

#if defined(FORTIS_HDBOX)
      /* add tag bytes to stream + stream priority */
      ret = ctrl_inl(reg_config + TSM_STREAM1_CFG);
      ctrl_outl(ret | (0x40020), reg_config + TSM_STREAM1_CFG);
#else
      /* add tag bytes to stream + stream priority */
      ret = ctrl_inl(reg_config + TSM_STREAM1_CFG);
      ctrl_outl(ret | (0x20020), reg_config + TSM_STREAM1_CFG);
#endif

      ctrl_outl(stream_sync, reg_config + TSM_STREAM1_SYNC);
      ctrl_outl(0x0, reg_config + 0x38 /* reserved ??? */);

      /* add tag bytes to stream + stream priority */
#if defined(FORTIS_HDBOX)
      ret = ctrl_inl(reg_config + TSM_STREAM2_CFG);
      ctrl_outl(ret | (0x40020), reg_config + TSM_STREAM2_CFG);
#else
      ret = ctrl_inl(reg_config + TSM_STREAM2_CFG);
      ctrl_outl(ret | (0x30020), reg_config + TSM_STREAM2_CFG);
#endif

      ctrl_outl(stream_sync, reg_config + TSM_STREAM2_SYNC);
      ctrl_outl(0x0, reg_config + 0x58 /* reserved ??? */);

      /* add tag bytes to stream + stream priority */
#if defined(FORTIS_HDBOX)
      ret = ctrl_inl(reg_config + TSM_STREAM3_CFG);
      ctrl_outl(ret | (0x40020), reg_config + TSM_STREAM3_CFG);
#else
      ret = ctrl_inl(reg_config + TSM_STREAM3_CFG);
      ctrl_outl(ret | (0x30020), reg_config + TSM_STREAM3_CFG);
#endif

      ctrl_outl(stream_sync, reg_config + TSM_STREAM3_SYNC);
      ctrl_outl(0x0, reg_config + 0x78 /* reserved ??? */);

#if !defined(FORTIS_HDBOX)
      /* swts_req_trigger + pace cycles (1101) */
      ctrl_outl(0x800000d, reg_config + SWTS_CFG(0));
#else
      ctrl_outl(0x8000010, reg_config + SWTS_CFG(0));
#endif

      /* auto count */
      ctrl_outl(0x0, reg_config + TSM_PROG_CNT0);

#if  !defined(TF7700) && !defined(UFS922) && !defined(FORTIS_HDBOX) && !defined(HL101) \
	&& !defined(CONFIG_SH_QBOXHD_1_0) && !defined(CONFIG_SH_QBOXHD_MINI_1_0)
      /* UFS910 stream configuration */
      /* route stream 2 to PTI */
      ret = ctrl_inl(reg_config + TSM_PTI_SEL);
      ctrl_outl(ret | 0x4, reg_config + TSM_PTI_SEL);

      /* set stream on */
      ret = ctrl_inl(reg_config + TSM_STREAM2_CFG);
      ctrl_outl(ret | 0x80,reg_config + TSM_STREAM2_CFG);

      /* set firewire clock */

      /* This solves the SR 30000 Problem */	
/* this must be 0x70010 for SR 30000. for some modules
 * this must be 0x70012 or 0x70014 :-(
 * ->also the original pvrmain (fw 106) makes CC trouble
 * with this setting and some modules.
 * 0xf from 106 pvrmain
 */
#ifdef FW1XX
      if (highSR)	
         ctrl_outl(0x7000f ,reg_config + TS_1394_CFG);
      else
         ctrl_outl(0x70014 ,reg_config + TS_1394_CFG);
#else
/* logged from fw 202rc ->see also pti */
	  ctrl_outl(0x50014 ,reg_config + TS_1394_CFG);
#endif

      	
      /* connect TSIN0 to TS1394 for routing tuner TS through the CIMAX */
      ret = ctrl_inl(reg_config + TSM_1394_DEST);
      ctrl_outl(ret | 0x1 , reg_config + TSM_1394_DEST);
#elif  defined(TF7700) || defined(UFS922) || defined(HL101) \
	|| defined(CONFIG_SH_QBOXHD_1_0) || defined(CONFIG_SH_QBOXHD_MINI_1_0)

      /* TF7700 stream configuration */
      /* route stream 1 to PTI */
      ret = ctrl_inl(reg_config + TSM_PTI_SEL);
      ctrl_outl(ret | 0x2,reg_config + TSM_PTI_SEL);

      /* set stream 1 on */
      ret = ctrl_inl(reg_config + TSM_STREAM1_CFG);
      ctrl_outl(ret | 0x80,reg_config + TSM_STREAM1_CFG);

      /* route stream 0 to PTI */
      ret = ctrl_inl(reg_config + TSM_PTI_SEL);
      ctrl_outl(ret | 0x1, reg_config + TSM_PTI_SEL);

      /* set firewire clock */
      //ctrl_outl(0x70014 ,reg_config + TS_1394_CFG);

      /* connect SWTS to TS1394 for routing through the StarCI2Win */
      //ret = ctrl_inl(reg_config + TSM_1394_DEST);
      //ctrl_outl(ret | 0x8 , reg_config + TSM_1394_DEST);
#elif defined(FORTIS_HDBOX)
	  /* route stream 1 to PTI */
	  ret = ctrl_inl(reg_config + TSM_PTI_SEL);
	  ctrl_outl(ret | 0x2,reg_config + TSM_PTI_SEL);

	  /* route stream 2 to PTI */
	  ret = ctrl_inl(reg_config + TSM_PTI_SEL);
	  ctrl_outl(ret | 0x4, reg_config + TSM_PTI_SEL);

	  /* set stream on */
	  ret = ctrl_inl(reg_config + TSM_STREAM5_CFG);
	  ctrl_outl(ret | 0x80,reg_config + TSM_STREAM5_CFG);

	  /* set stream on */
	  ret = ctrl_inl(reg_config + TSM_STREAM4_CFG);
	  ctrl_outl(ret | 0x80,reg_config + TSM_STREAM4_CFG);

	  /* set stream on */
	  ret = ctrl_inl(reg_config + TSM_STREAM3_CFG);
	  ctrl_outl(ret | 0x80,reg_config + TSM_STREAM3_CFG);

	  /* set stream on */
	  ret = ctrl_inl(reg_config + TSM_STREAM2_CFG);
	  ctrl_outl(ret | 0x80,reg_config + TSM_STREAM2_CFG);

	  /* set stream 1 on */
	  ret = ctrl_inl(reg_config + TSM_STREAM1_CFG);
	  ctrl_outl(ret | 0x80,reg_config + TSM_STREAM1_CFG);

	  /* route stream 0 to PTI */
	  ret = ctrl_inl(reg_config + TSM_PTI_SEL);
	  ctrl_outl(ret | 0x1, reg_config + TSM_PTI_SEL);

	  ctrl_outl(stream_sync, reg_config + TSM_STREAM4_SYNC);
	  ctrl_outl(stream_sync, reg_config + TSM_STREAM5_SYNC);
	  ctrl_outl(0x00, reg_config + TSM_STREAM6_SYNC);

	  ret = ctrl_inl(reg_config + TSM_1394_DEST);
	  ctrl_outl(ret | 0x38 , reg_config + TSM_1394_DEST);

#endif

      /* set stream on */
      ret = ctrl_inl(reg_config + TSM_STREAM0_CFG);
      ctrl_outl(ret | 0x80,reg_config + TSM_STREAM0_CFG);

      /* Dagobert: set-up swts */
      ctrl_outl( TSM_SWTS_REQ_TRIG(128/16) | 0x10, reg_config + TSM_SWTS_CFG(0));

      /* SWTS0 to PTI */
      ret = ctrl_inl(reg_config + TSM_PTI_SEL);
      ctrl_outl(ret | 0x8, reg_config + TSM_PTI_SEL);

      ret = ctrl_inl(reg_config + TSM_STREAM3_CFG);
      ctrl_outl( (ret & TSM_RAM_ALLOC_START(0xff)) |
                  TSM_PRIORITY(0x7) | TSM_STREAM_ON | TSM_ADD_TAG_BYTES | TSM_SYNC_NOT_ASYNC | TSM_ASYNC_SOP_TOKEN, reg_config + TSM_STREAM3_CFG);

      tsm_handle.swts_channel = 3;
      tsm_handle.tsm_swts = (unsigned long)ioremap (0x1A300000, 0x1000);

      /* Now lets get the SWTS info and setup an FDMA channel */
#if  !defined(TF7700) && !defined(UFS922) && !defined(FORTIS_HDBOX) \
	&& !defined(CONFIG_SH_QBOXHD_1_0) && !defined(CONFIG_SH_QBOXHD_MINI_1_0)
			//nit2005, ufs910 use dma request id 30 für swts, do'nt know what other boxes use
      tsm_handle.fdma_reqline = 30;
#else
      tsm_handle.fdma_reqline = 28;
#endif
      tsm_handle.fdma_channel = request_dma_bycap(fdmac_id, fdma_cap_hb, "swts0");
      tsm_handle.fdma_req     = dma_req_config(tsm_handle.fdma_channel,tsm_handle.fdma_reqline,&fdma_req_config);

      /* Initilise the parameters for the FDMA SWTS data injection */
      for (n=0;n<MAX_SWTS_PAGES;n++) {
         dma_params_init(&tsm_handle.swts_params[n], MODE_PACED, STM_DMA_LIST_OPEN);
         dma_params_DIM_1_x_0(&tsm_handle.swts_params[n]);
         dma_params_req(&tsm_handle.swts_params[n],tsm_handle.fdma_req);
      }
   } 
#if defined(CONFIG_SH_QBOXHD_1_0) || defined(CONFIG_SH_QBOXHD_MINI_1_0)
	else {
		/* bypass cimax */
		int n;
		unsigned long size;

		printk("Bypass cimax\n");

		tsm_io = (unsigned long)ioremap (config->tsm_base_address, 0x1000);

#ifdef LOAD_TSM_DATA
		TSM_NUM_PTI_ALT_OUT  = config->tsm_num_pti_alt_out;
		TSM_NUM_1394_ALT_OUT = config->tsm_num_1394_alt_out;
#endif

		writel( 0x0, tsm_io + TSM_SW_RESET);
		writel( 0x6, tsm_io + TSM_SW_RESET);

		size = (config->tsm_sram_buffer_size / config->nr_nims) & ~31;

		for (n=0;n<5;n++)
			writel( TSM_RAM_ALLOC_START( 0x3 *n ), tsm_io + TSM_STREAM_CONF(n));

		for (n = 0; n < config->nr_nims; n++)
		{
			enum plat_frontend_options options = config->nims[n].option;
			int chan = STM_GET_CHANNEL(options);// / STM_TSM_CHANNEL_1) & 0xf;

			/*printk("---> %s(): options: 0x%X\n", __func__, options);*/
			writel(readl(tsm_io + TSM_DESTINATION(0)) | (1 << chan), tsm_io + TSM_DESTINATION(0));

			writel((readl(tsm_io + TSM_STREAM_CONF(chan)) & TSM_RAM_ALLOC_START(0xff))	|
					(options & STM_SERIAL_NOT_PARALLEL ? TSM_SERIAL_NOT_PARALLEL : 0 )	|
					(options & STM_INVERT_CLOCK        ? TSM_INVERT_BYTECLK : 0 )		| 
					(options & STM_PACKET_CLOCK_VALID  ? TSM_SYNC_NOT_ASYNC : 0 )		|
					TSM_ALIGN_BYTE_SOP 													| 
					TSM_PRIORITY(0xf) 													|
					TSM_STREAM_ON 														|
					TSM_ADD_TAG_BYTES,
					tsm_io + TSM_STREAM_CONF(chan));

			writel( TSM_SYNC(config->nims[n].lock)	|    
					TSM_DROP(config->nims[n].drop)	|
					TSM_SOP_TOKEN(0x47)				|
					TSM_PACKET_LENGTH(188), 
					tsm_io + TSM_STREAM_SYNC(chan));
		}
	}
#else
	else
   {
   	/* bypass cimax */
      int n;
	  /*unsigned long size;*/

      printk("Bypass cimax\n");

	  tsm_io = (unsigned long)ioremap (/* config->tsm_base_address */ 0x19242000,0x1000);

#ifdef LOAD_TSM_DATA
  	TSM_NUM_PTI_ALT_OUT  = 1/* config->tsm_num_pti_alt_out*/;
  	TSM_NUM_1394_ALT_OUT = 1/*config->tsm_num_1394_alt_out */;
#endif

      writel( 0x0, tsm_io + TSM_SW_RESET);
      writel( 0x6, tsm_io + TSM_SW_RESET);

/*
channels = streams = 5 ? wohl 4 s.u.
tsm_sram_buffer_size  ?
options? ->s.u.
lock ->muss es als register geben ->ist lt. meinen logs beides 3
drop ->dito _>also nachlesen
#define STM_GET_CHANNEL(x)       ((x & 0x0f0000) >> 16)

Dies sind die Options (also wohl auch view channel):
       STM_TSM_CHANNEL_0       = 0x000000, ->ok dann liefer stm_get_channel 0,1,2,3 ;-)
       STM_TSM_CHANNEL_1       = 0x010000,
       STM_TSM_CHANNEL_2       = 0x020000,
       STM_TSM_CHANNEL_3       = 0x030000,

*/
  	for (n=0;n<5;n++)
    	   writel( TSM_RAM_ALLOC_START( 0x3 *n ), tsm_io + TSM_STREAM_CONF(n));

  	for (n=0;n< 4/* config->nr_channels */;n++)
  	{
#ifdef alt
    		enum plat_frontend_options options = config->channels[n].option;
    		int chan = STM_GET_CHANNEL(options);// / STM_TSM_CHANNEL_1) & 0xf;
#endif
    		int chan = n;
			int options = n * 0x10000;

    		writel( readl(tsm_io + TSM_DESTINATION(0)) | (1 << chan), tsm_io + TSM_DESTINATION(0));

    		writel( (readl(tsm_io + TSM_STREAM_CONF(chan)) & TSM_RAM_ALLOC_START(0xff)) |
       			(options & STM_SERIAL_NOT_PARALLEL ? TSM_SERIAL_NOT_PARALLEL : 0 ) |
       			(options & STM_INVERT_CLOCK        ? TSM_INVERT_BYTECLK : 0 ) |
       			(options & STM_PACKET_CLOCK_VALID  ? TSM_SYNC_NOT_ASYNC : 0 ) |
       			TSM_ALIGN_BYTE_SOP |
       			TSM_PRIORITY(0xf) | TSM_STREAM_ON | TSM_ADD_TAG_BYTES ,
       			tsm_io + TSM_STREAM_CONF(chan));

#ifdef alt
    writel( TSM_SYNC(config->channels[n].lock) |
       TSM_DROP(config->channels[n].drop) |
       TSM_SOP_TOKEN(0x47)                |
       TSM_PACKET_LENGTH(188)
       ,tsm_io + TSM_STREAM_SYNC(chan));
#endif
    		writel( TSM_SYNC(3 /* lock */) |
       			TSM_DROP(3 /*drop*/) |
       			TSM_SOP_TOKEN(0x47)                |
       			TSM_PACKET_LENGTH(188)
       			,tsm_io + TSM_STREAM_SYNC(chan));
  	}
	}
#endif

	/* Put TSMERGER into normal mode */
	writel( TSM_CFG_BYPASS_NORMAL, tsm_io + TSM_SYSTEM_CFG);
}

void stm_tsm_release (void)
{
  iounmap((void*)tsm_io );
}

