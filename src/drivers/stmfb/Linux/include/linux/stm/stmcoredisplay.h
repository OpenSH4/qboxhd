/***********************************************************************
 *
 * File: stgfb/Linux/video/stmcoredisplay.h
 * Copyright (c) 2007 STMicroelectronics Limited.
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive for
 * more details.
 *
\***********************************************************************/

#ifndef _STMCOREDISPLAY_H
#define _STMCOREDISPLAY_H

#if !defined(_STM_HDMI_DEF)
struct stm_hdmi;
#endif

#define _ALIGN_DOWN(addr,size)  ((addr)&(~((size)-1)))

#define STMCORE_MAX_PLANES      8

#define STMCORE_PLANE_VIDEO     (1L<<0)
#define STMCORE_PLANE_GFX       (1L<<1)
#define STMCORE_PLANE_PREFERRED (1L<<2)
#define STMCORE_PLANE_SHARED    (1L<<3)
#define STMCORE_PLANE_MEM_VIDEO (1L<<4)
#define STMCORE_PLANE_MEM_SYS   (1L<<5)
#define STMCORE_PLANE_MEM_ANY   (STMCORE_PLANE_MEM_VIDEO|STMCORE_PLANE_MEM_SYS)

typedef enum { STMCORE_BLITTER_GAMMA, STMCORE_BLITTER_BDISPII } stmcore_blitter_t;

typedef void *stm_vsync_context_handle_t;
typedef void (*stm_vsync_callback) (stm_vsync_context_handle_t context,
                                    stm_field_t                field);

struct stmcore_plane_data
{
  stm_plane_id_t id;
  unsigned int   flags;
};


struct stmcore_vsync_cb
{
  struct list_head           node;
  struct module             *owner;
  stm_vsync_context_handle_t context;
  stm_vsync_callback         cb;
};


struct stmcore_vsync_cb_list
{
  rwlock_t         lock;
  struct list_head list; // of type struct stmcore_vsync_cb
};


struct stmcore_display
{
  spinlock_t        spinlock;
  /*
   * The vsync wait queue is for public use by any driver needing to
   * block, waiting on the next vsync generated by this display pipeline.
   */
  wait_queue_head_t vsync_wait_queue;

  /*
   * For core driver use, if you need a handle to the main output then
   * get your own from the device interface using main_output_id in the
   * platform device data below.
   */
  stm_display_output_t *main_output;
  stm_display_output_t *hdmi_output;

  /*
   * Set if we need to poll hotplug status on this platform
   */
  struct stpio_pin *hotplug_poll_pio;

  /*
   * Allow another driver to hook into the vsync processing.
   */
  struct stmcore_vsync_cb_list vsync_cb_list;

  volatile s64 last_display_processing_us;
  volatile s64 min_display_processing_us;
  volatile s64 max_display_processing_us;
  volatile s64 avg_display_processing_us;
};


struct stmcore_display_pipeline_data
{
  struct module            *owner;
  char                     *name;
  int                       vtg_irq;
  char                      vtg_name[7]; /* for request_irq */
  int                       blitter_irq;
  int                       hdmi_irq;
  int                       hdmi_i2c_adapter_id;
  int                       main_output_id;
  int                       aux_output_id;
  int                       hdmi_output_id;
  int                       dvo_output_id;

  int                       blitter_id;
  stmcore_blitter_t         blitter_type;

  stm_plane_id_t            preferred_graphics_plane;
  stm_plane_id_t            preferred_video_plane;
  struct stmcore_plane_data planes[STMCORE_MAX_PLANES];

  const unsigned long      *whitelist;
  unsigned long             whitelist_size;
  unsigned long             io_offset;
  unsigned long             mmio;
  unsigned long             mmio_len;

  stm_display_device_t     *device;
  stm_display_blitter_t    *blitter;
  char                      blitter_name[9]; /* for request_irq */
  struct stm_hdmi          *hdmi_data;
  struct stmcore_display   *display_runtime;
  struct platform_device   *platform_device;
  struct class_device      *class_device;
};


int stmcore_get_display_pipeline(int pipeline, struct stmcore_display_pipeline_data *data);
int stmcore_register_vsync_callback(struct stmcore_display *, struct stmcore_vsync_cb *);
int stmcore_unregister_vsync_callback(struct stmcore_display *, struct stmcore_vsync_cb *);


#ifdef DEBUG
#define DPRINTK(a,b...) printk(KERN_WARNING "stmcoredisplay: %s: " a, __FUNCTION__ ,##b)
#define DPRINTK_NOPREFIX(a,b...) printk(a,##b)
#else
#define DPRINTK(a,b...)
#define DPRINTK_NOPREFIX(a,b...)
#endif

#endif /* _STMCOREDISPLAY_H */
