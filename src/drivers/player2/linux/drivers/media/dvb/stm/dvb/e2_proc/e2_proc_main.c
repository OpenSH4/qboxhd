/* 
 * Dagobert:
 * e2 proc handling rebuilding.
 * We do not rebuild all files from dm800 but those we need from e2 view.
 * the rest seems to be receiver & driver specific.
 *
 *
 * progress
 *  |
 * bus
 *  |
 *  ----------- nim_sockets
 *  |
 * stb
 *  |
 *  ----------- audio
 *  |             |
 *  |             --------- ac3
 *  |             |
 *  |             --------- audio_delay_pcm
 *  |             |
 *  |             --------- audio_delay_bitstream
 *  |             |
 *  |             --------- audio_j1_mute
 *  |             |
 *  |             --------- ac3_choices
 *  |             |
 *  |             ---------
 *  |
 *  ----------- video
 *  |             |
 *  |             --------- alpha
 *  |             |
 *  |             --------- aspect
 *  |             |
 *  |             --------- aspect_choices
 *  |             |
 *  |             --------- force_dvi
 *  |             |
 *  |             --------- policy
 *  |             |
 *  |             --------- policy2 
 *  |             |
 *  |             --------- policy_choices
 *  |             |
 *  |             --------- videomode
 *  |             |
 *  |             --------- videomode_50hz
 *  |             |
 *  |             --------- videomode_60hz
 *  |             |
 *  |             --------- videomode_choices
 *  |             |
 *  |             --------- videomode_preferred
 *  |             |
 *  |             --------- pal_v_start
 *  |             |
 *  |             --------- pal_v_end
 *  |             |
 *  |             --------- pal_h_start
 *  |             |
 *  |             --------- pal_h_end
 *  |  
 *  ---------- avs
 *  |           |
 *  |           --------- 0
 *  |               |
 *  |               --------- colorformat <-colorformat in generlell, hdmi and scart
 *  |               |
 *  |               --------- colorformat_choices
 *  |               |
 *  |               --------- fb <-fastblanking
 *  |               |
 *  |               --------- sb <-slowblanking
 *  |               |
 *  |               --------- volume
 *  |               |
 *  |               --------- input  <-Input, Scart VCR Input or Encoder
 *  |               |
 *  |               --------- input_choices
 *  |               |
 *  |               --------- standby
 *  |  
 *  ---------- denc
 *  |           |
 *  |           --------- 0
 *  |               |
 *  |               --------- wss
 *  |               |
 *  |               --------- 
 *  |               |
 *  |               --------- 
 *  |  
 *  ---------- fp (this is wrong used for e2 I think. on dm800 this is frontprocessor and there is another proc entry for frontend)
 *  |           |
 *  |           --------- lnb_sense1
 *  |           |
 *  |           --------- lnb_sense2
 *  |           |
 *  |           --------- led0_pattern
 *  |           |
 *  |           --------- led_pattern_speed
 *  |           |
 *  |           |
 *  |           --------- version
 *  |           |
 *  |           --------- wakeup_time <- dbox frontpanel wakeuptime 
 *  |           |
 *  |           --------- was_timer_wakeup
 *  |  
 *  |
 *  ---------- hdmi
 *  |           |
 *  |           --------- bypass_edid_checking
 *  |           |
 *  |           --------- enable_hdmi_resets
 *  |           |
 *  |           --------- audio_source
 *  |           |
 *  |           --------- 
 *  |
 *  ---------- info
 *  |           |
 *  |           --------- model <- Version String of out Box
 *  |
 *  ---------- tsmux
 *  |           |
 *  |           --------- input0
 *  |           |
 *  |           --------- input1
 *  |           |
 *  |           --------- ci0_input
 *  |           |
 *  |           --------- ci1_input
 *  |           |
 *  |           --------- lnb_b_input
 *  |           |
 *  |           ---------
 *  |
 *  ---------- misc
 *  |           |
 *  |           --------- 12V_output
 *  |
 *  ---------- tuner (dagoberts tuner entry ;-) )
 *  |           |
 *  |           --------- 
 *  |
 *  ---------- vmpeg
 *  |           |
 *  |           --------- 0/1
 *  |               |
 *  |               --------- dst_left   \
 *  |               |                     |
 *  |               --------- dst_top     | 
 *  |               |                      >  PIG WINDOW SIZE AND POSITION
 *  |               --------- dst_width   |
 *  |               |                     |
 *  |               --------- dst_height /
 *  |               |
 *  |               --------- dst_all (Dagobert: Dont confuse player by setting value one after each other)
 *  |               |
 *  |               --------- yres
 *  |               |
 *  |               --------- xres
 *  |               |
 *  |               --------- framerate
 *  |               |
 *  |               --------- aspect
 *  |                 |TODO
 *  |               | v
 *  |               --------- progressive
 *
 */

#ifdef m
Offen:
	    			proc = open("/sys/class/stmcoredisplay/display0/hdmi0.0/modes", "r")
#endif

#include <linux/proc_fs.h>  	/* proc fs */ 
#include <asm/uaccess.h>    	/* copy_from_user */

#include <linux/dvb/video.h>	/* Video Format etc */

#include <linux/dvb/audio.h>
#include <linux/smp_lock.h>
#include <linux/string.h>

#include "../backend.h"
#include "../dvb_module.h"
#include "linux/dvb/stm_ioctls.h"

/* external functions provided by the module e2_procfs */
extern int install_e2_procs(char *name, read_proc_t *read_proc, write_proc_t *write_proc, void *data);
extern int remove_e2_procs(char *name, read_proc_t *read_proc, write_proc_t *write_proc);


extern int proc_progress_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_progress_write(struct file *file, const char __user *buf, unsigned long count, void *data);

extern int proc_bus_nim_sockets_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);

extern int proc_info_model_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);

extern int proc_audio_ac3_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_audio_ac3_write(struct file *file, const char __user *buf, unsigned long count, void *data);
extern int proc_audio_delay_pcm_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_audio_delay_pcm_write(struct file *file, const char __user *buf, unsigned long count, void *data);
extern int proc_audio_delay_bitstream_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_audio_delay_bitstream_write(struct file *file, const char __user *buf, unsigned long count, void *data);
extern int proc_audio_j1_mute_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_audio_j1_mute_write(struct file *file, const char __user *buf, unsigned long count, void *data);
extern int proc_video_aspect_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_video_aspect_write(struct file *file, const char __user *buf, unsigned long count, void *data);
extern int proc_video_aspect_choices_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_video_policy_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_video_policy_write(struct file *file, const char __user *buf, unsigned long count, void *data);
#if defined CONFIG_SH_QBOXHD_1_0 || defined CONFIG_SH_QBOXHD_MINI_1_0
extern int proc_video_policy2_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_video_policy2_write(struct file *file, const char __user *buf, unsigned long count, void *data);
#endif
extern int proc_video_policy_choices_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_video_videomode_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_video_videomode_write(struct file *file, const char __user *buf, unsigned long count, void *data);
extern int proc_video_pal_h_start_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_video_pal_h_start_write(struct file *file, const char __user *buf, unsigned long count, void *data);
extern int proc_video_pal_h_end_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_video_pal_h_end_write(struct file *file, const char __user *buf, unsigned long count, void *data);
extern int proc_video_pal_v_start_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_video_pal_v_start_write(struct file *file, const char __user *buf, unsigned long count, void *data);
extern int proc_video_pal_v_end_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_video_pal_v_end_write(struct file *file, const char __user *buf, unsigned long count, void *data);
extern int proc_video_alpha_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_video_alpha_write(struct file *file, const char __user *buf, unsigned long count, void *data);
extern int proc_avs_0_colorformat_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_avs_0_colorformat_write(struct file *file, const char __user *buf, unsigned long count, void *data);
extern int proc_avs_0_colorformat_choices_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_avs_0_fb_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_avs_0_fb_write(struct file *file, const char __user *buf, unsigned long count, void *data);
extern int proc_avs_0_input_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_avs_0_input_write(struct file *file, const char __user *buf, unsigned long count, void *data);
extern int proc_avs_0_sb_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_avs_0_sb_write(struct file *file, const char __user *buf, unsigned long count, void *data);
extern int proc_avs_0_volume_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_avs_0_volume_write(struct file *file, const char __user *buf, unsigned long count, void *data);
extern int proc_avs_0_input_choices_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);

extern int proc_avs_0_standby_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_avs_0_standby_write(struct file *file, const char __user *buf, unsigned long count, void *data);

extern int proc_denc_0_wss_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_denc_0_wss_write(struct file *file, const char __user *buf, unsigned long count, void *data);

extern int proc_tsmux_input0_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_tsmux_input0_write(struct file *file, const char __user *buf, unsigned long count, void *data);
extern int proc_tsmux_input1_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_tsmux_input1_write(struct file *file, const char __user *buf, unsigned long count, void *data);
extern int proc_tsmux_ci0_input_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_tsmux_ci0_input_write(struct file *file, const char __user *buf, unsigned long count, void *data);
extern int proc_tsmux_ci1_input_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_tsmux_ci1_input_write(struct file *file, const char __user *buf, unsigned long count, void *data);
extern int proc_tsmux_lnb_b_input_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_tsmux_lnb_b_input_write(struct file *file, const char __user *buf, unsigned long count, void *data);
extern int proc_misc_12V_output_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_misc_12V_output_write(struct file *file, const char __user *buf, unsigned long count, void *data);

extern int proc_audio_ac3_choices_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_video_videomode_choices_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_video_videomode_preferred_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_video_videomode_preferred_write(struct file *file, const char __user *buf, unsigned long count, void *data);
extern int proc_fp_led0_pattern_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_fp_led0_pattern_write(struct file *file, const char __user *buf, unsigned long count, void *data);
extern int proc_fp_led_pattern_speed_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_fp_led_pattern_speed_write(struct file *file, const char __user *buf, unsigned long count, void *data);

extern int proc_vmpeg_0_dst_left_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_vmpeg_0_dst_left_write(struct file *file, const char __user *buf, unsigned long count, void *data);
extern int proc_vmpeg_0_dst_top_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_vmpeg_0_dst_top_write(struct file *file, const char __user *buf, unsigned long count, void *data);
extern int proc_vmpeg_0_dst_width_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_vmpeg_0_dst_width_write(struct file *file, const char __user *buf, unsigned long count, void *data);
extern int proc_vmpeg_0_dst_height_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_vmpeg_0_dst_height_write(struct file *file, const char __user *buf, unsigned long count, void *data);
extern int proc_vmpeg_0_dst_all_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_vmpeg_0_dst_all_write(struct file *file, const char __user *buf, unsigned long count, void *data);

extern int proc_vmpeg_0_yres_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_vmpeg_0_xres_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_vmpeg_0_aspect_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_vmpeg_0_framerate_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);


extern int proc_hdmi_audio_source_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_hdmi_audio_source_write(struct file *file, const char __user *buf, unsigned long count, void *data);
extern int proc_hdmi_audio_source_choices_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);

extern int proc_hdmi_edid_handling_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_hdmi_edid_handling_write(struct file *file, const char __user *buf, unsigned long count, void *data);

extern int proc_stream_AV_SYNC_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_stream_AV_SYNC_write(struct file *file, const char __user *buf, unsigned long count, void *data);
extern int proc_stream_TRICK_MODE_AUDIO_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_stream_TRICK_MODE_AUDIO_write(struct file *file, const char __user *buf, unsigned long count, void *data);
extern int proc_stream_PLAY_24FPS_VIDEO_AT_25FPS_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_stream_PLAY_24FPS_VIDEO_AT_25FPS_write(struct file *file, const char __user *buf, unsigned long count, void *data);
extern int proc_stream_MASTER_CLOCK_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_stream_MASTER_CLOCK_write(struct file *file, const char __user *buf, unsigned long count, void *data);
extern int proc_stream_EXTERNAL_TIME_MAPPING_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_stream_EXTERNAL_TIME_MAPPING_write(struct file *file, const char __user *buf, unsigned long count, void *data);
extern int proc_stream_DISPLAY_FIRST_FRAME_EARLY_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_stream_DISPLAY_FIRST_FRAME_EARLY_write(struct file *file, const char __user *buf, unsigned long count, void *data);
extern int proc_stream_STREAM_ONLY_KEY_FRAMES_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_stream_STREAM_ONLY_KEY_FRAMES_write(struct file *file, const char __user *buf, unsigned long count, void *data);
extern int proc_stream_STREAM_SINGLE_GROUP_BETWEEN_DISCONTINUITIES_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_stream_STREAM_SINGLE_GROUP_BETWEEN_DISCONTINUITIES_write(struct file *file, const char __user *buf, unsigned long count, void *data);
extern int proc_stream_PLAYOUT_ON_TERMINATE_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_stream_PLAYOUT_ON_TERMINATE_write(struct file *file, const char __user *buf, unsigned long count, void *data);
extern int proc_stream_PLAYOUT_ON_SWITCH_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_stream_PLAYOUT_ON_SWITCH_write(struct file *file, const char __user *buf, unsigned long count, void *data);
extern int proc_stream_PLAYOUT_ON_DRAIN_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_stream_PLAYOUT_ON_DRAIN_write(struct file *file, const char __user *buf, unsigned long count, void *data);
extern int proc_stream_TRICK_MODE_DOMAIN_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_stream_TRICK_MODE_DOMAIN_write(struct file *file, const char __user *buf, unsigned long count, void *data);
extern int proc_stream_DISCARD_LATE_FRAMES_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_stream_DISCARD_LATE_FRAMES_write(struct file *file, const char __user *buf, unsigned long count, void *data);
extern int proc_stream_REBASE_ON_DATA_DELIVERY_LATE_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_stream_REBASE_ON_DATA_DELIVERY_LATE_write(struct file *file, const char __user *buf, unsigned long count, void *data);
extern int proc_stream_REBASE_ON_FRAME_DECODE_LATE_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_stream_REBASE_ON_FRAME_DECODE_LATE_write(struct file *file, const char __user *buf, unsigned long count, void *data);
extern int proc_stream_LOWER_CODEC_DECODE_LIMITS_ON_FRAME_DECODE_LATE_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_stream_LOWER_CODEC_DECODE_LIMITS_ON_FRAME_DECODE_LATE_write(struct file *file, const char __user *buf, unsigned long count, void *data);
extern int proc_stream_H264_ALLOW_NON_IDR_RESYNCHRONIZATION_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_stream_H264_ALLOW_NON_IDR_RESYNCHRONIZATION_write(struct file *file, const char __user *buf, unsigned long count, void *data);
extern int proc_stream_MPEG2_IGNORE_PROGESSIVE_FRAME_FLAG_read(char *page, char **start, off_t off, int count,int *eof, void *data_unused);
extern int proc_stream_MPEG2_IGNORE_PROGESSIVE_FRAME_FLAG_write(struct file *file, const char __user *buf, unsigned long count, void *data);

#define cProcDir	1
#define cProcEntry	2 

#define cProcNotExist 		1
#define cProcAlreadyExist 2

struct vmpeg_procs
{
  int   type;
  int proc_dir_entry_exists;
  char *name;
  struct proc_dir_entry* entry;
  read_proc_t *read_proc;
  write_proc_t *write_proc;
} vmpeg_procs[] = 
{
  {cProcEntry, cProcNotExist, "dst_left",  NULL,            proc_vmpeg_0_dst_left_read,             proc_vmpeg_0_dst_left_write},
  {cProcEntry, cProcNotExist, "dst_top",  NULL,             proc_vmpeg_0_dst_top_read,              proc_vmpeg_0_dst_top_write},
  {cProcEntry, cProcNotExist, "dst_width", NULL,           proc_vmpeg_0_dst_width_read,            proc_vmpeg_0_dst_width_write},
  {cProcEntry, cProcNotExist, "dst_height",   NULL,        proc_vmpeg_0_dst_height_read,           proc_vmpeg_0_dst_height_write},
  {cProcEntry, cProcNotExist, "dst_all",  NULL,                NULL,                                   proc_vmpeg_0_dst_all_write},
  {cProcEntry, cProcNotExist, "yres",   NULL,                  proc_vmpeg_0_yres_read,                 NULL},
  {cProcEntry, cProcNotExist, "xres",   NULL,                  proc_vmpeg_0_xres_read,                 NULL},
  {cProcEntry, cProcNotExist, "aspect",  NULL,                 proc_vmpeg_0_aspect_read,               NULL},
  {cProcEntry, cProcNotExist, "framerate",  NULL,              proc_vmpeg_0_framerate_read,            NULL}
};

struct e2_procs
{
  int   type;
  int proc_dir_entry_exists;
  char *name;
  struct proc_dir_entry* entry;
  read_proc_t *read_proc;
  write_proc_t *write_proc;
  int context;
} e2_procs[] =
{
  {cProcEntry, cProcNotExist, "progress",                          NULL, proc_progress_read,                     proc_progress_write, 0},

  {cProcEntry, cProcNotExist, "bus/nim_sockets",                   NULL, proc_bus_nim_sockets_read,              NULL, 0},
  {cProcEntry, cProcNotExist, "stb/audio/ac3",                      NULL, proc_audio_ac3_read,                    proc_audio_ac3_write, 0},
  {cProcEntry, cProcNotExist, "stb/audio/audio_delay_pcm",          NULL, proc_audio_delay_pcm_read,              proc_audio_delay_pcm_write, 0},
  {cProcEntry, cProcNotExist, "stb/audio/audio_delay_bitstream",   NULL, proc_audio_delay_bitstream_read,        proc_audio_delay_bitstream_write, 0},
  {cProcEntry, cProcNotExist, "stb/audio/j1_mute",                  NULL, proc_audio_j1_mute_read,                proc_audio_j1_mute_write, 0},
  {cProcEntry, cProcNotExist, "stb/audio/ac3_choices",              NULL, proc_audio_ac3_choices_read,            NULL, 0},

  {cProcEntry, cProcNotExist, "stb/video/alpha",                    NULL,  proc_video_alpha_read,                  proc_video_alpha_write, 0},
  {cProcEntry, cProcNotExist, "stb/video/aspect",                   NULL, proc_video_aspect_read,                 proc_video_aspect_write, 0},
  {cProcEntry, cProcNotExist, "stb/video/aspect_choices",           NULL, proc_video_aspect_choices_read,         NULL, 0},
/*
  {"stb/video/force_dvi", NULL, NULL, 0},
*/
  {cProcEntry, cProcNotExist, "stb/video/policy",                   NULL, proc_video_policy_read,                 proc_video_policy_write, 0},
#if defined CONFIG_SH_QBOXHD_1_0 || defined CONFIG_SH_QBOXHD_MINI_1_0
  {cProcEntry, cProcNotExist, "stb/video/policy2",                   NULL, proc_video_policy2_read,                 proc_video_policy2_write, 0},
#endif
  {cProcEntry, cProcNotExist, "stb/video/policy_choices",           NULL, proc_video_policy_choices_read,         NULL, 0},
  {cProcEntry, cProcNotExist, "stb/video/videomode",                NULL, proc_video_videomode_read,              proc_video_videomode_write, 0},
  {cProcEntry, cProcNotExist, "stb/video/videomode_50hz",           NULL, proc_video_videomode_read,              proc_video_videomode_write, 0},
  {cProcEntry, cProcNotExist, "stb/video/videomode_60hz",           NULL, proc_video_videomode_read,              proc_video_videomode_write, 0},
  {cProcEntry, cProcNotExist, "stb/video/videomode_choices",        NULL, proc_video_videomode_choices_read,      NULL, 0},
  {cProcEntry, cProcNotExist, "stb/video/videomode_preferred",      NULL, proc_video_videomode_preferred_read,    proc_video_videomode_preferred_write, 0},
  {cProcEntry, cProcNotExist, "stb/video/pal_v_start",     	 NULL, proc_video_pal_v_start_read,    	proc_video_pal_v_start_write, 0},
  {cProcEntry, cProcNotExist, "stb/video/pal_v_end",     		 NULL, proc_video_pal_v_end_read,    		proc_video_pal_v_end_write, 0},
  {cProcEntry, cProcNotExist, "stb/video/pal_h_start",     	 NULL, proc_video_pal_h_start_read,    	proc_video_pal_h_start_write, 0},
  {cProcEntry, cProcNotExist, "stb/video/pal_h_end",     		 NULL, proc_video_pal_h_end_read,    		proc_video_pal_h_end_write, 0},

  {cProcEntry, cProcNotExist, "stb/avs/0/colorformat",              NULL, proc_avs_0_colorformat_read,            proc_avs_0_colorformat_write, 0},
  {cProcEntry, cProcNotExist, "stb/avs/0/colorformat_choices",      NULL, proc_avs_0_colorformat_choices_read,    NULL, 0},
  {cProcEntry, cProcNotExist, "stb/avs/0/fb",                       NULL, proc_avs_0_fb_read,                     proc_avs_0_fb_write, 0},
  {cProcEntry, cProcNotExist, "stb/avs/0/input",                    NULL, proc_avs_0_input_read,                  proc_avs_0_input_write, 0},
  {cProcEntry, cProcNotExist, "stb/avs/0/sb",                       NULL, proc_avs_0_sb_read,                     proc_avs_0_sb_write, 0},
  {cProcEntry, cProcNotExist, "stb/avs/0/volume",                   NULL, proc_avs_0_volume_read,                 proc_avs_0_volume_write, 0},
  {cProcEntry, cProcNotExist, "stb/avs/0/input_choices",            NULL, proc_avs_0_input_choices_read,          NULL, 0},
  {cProcEntry, cProcNotExist, "stb/avs/0/standby",                  NULL, proc_avs_0_standby_read,                proc_avs_0_standby_write, 0},

  {cProcEntry, cProcNotExist, "stb/denc/0/wss",                     NULL, proc_denc_0_wss_read,                   proc_denc_0_wss_write, 0},

  {cProcEntry, cProcNotExist, "stb/tsmux/input0",                   NULL, proc_tsmux_input0_read,                 proc_tsmux_input0_write, 0},
  {cProcEntry, cProcNotExist, "stb/tsmux/input1",                   NULL, proc_tsmux_input1_read,                 proc_tsmux_input1_write, 0},
  {cProcEntry, cProcNotExist, "stb/tsmux/ci0_input",                NULL, proc_tsmux_ci0_input_read,              proc_tsmux_ci0_input_write, 0},
  {cProcEntry, cProcNotExist, "stb/tsmux/ci1_input",                NULL, proc_tsmux_ci1_input_read,              proc_tsmux_ci1_input_write, 0},
  {cProcEntry, cProcNotExist, "stb/tsmux/lnb_b_input",              NULL, proc_tsmux_lnb_b_input_read,            proc_tsmux_lnb_b_input_write, 0},
  {cProcEntry, cProcNotExist, "stb/misc/12V_output",                NULL, proc_misc_12V_output_read,              proc_misc_12V_output_write, 0},

  {cProcEntry, cProcNotExist, "stb/hdmi/bypass_edid_checking",      NULL, proc_hdmi_edid_handling_read,           proc_hdmi_edid_handling_write, 0},
/*
  {"stb/hdmi/enable_hdmi_resets", NULL, NULL, 0},
*/
  {cProcEntry, cProcNotExist, "stb/hdmi/audio_source",              NULL, proc_hdmi_audio_source_read,            proc_hdmi_audio_source_write, 0},
  {cProcEntry, cProcNotExist, "stb/hdmi/audio_source_choices",      NULL, proc_hdmi_audio_source_choices_read,    NULL, 0},

  {cProcEntry, cProcNotExist, "stb/stream/policy/AV_SYNC",                                         NULL,  proc_stream_AV_SYNC_read, proc_stream_AV_SYNC_write, 0},
  {cProcEntry, cProcNotExist, "stb/stream/policy/TRICK_MODE_AUDIO"                               ,  NULL, proc_stream_TRICK_MODE_AUDIO_read, proc_stream_TRICK_MODE_AUDIO_write, 0},
  {cProcEntry, cProcNotExist, "stb/stream/policy/PLAY_24FPS_VIDEO_AT_25FPS"                      ,  NULL,  proc_stream_PLAY_24FPS_VIDEO_AT_25FPS_read, proc_stream_PLAY_24FPS_VIDEO_AT_25FPS_write, 0},
  {cProcEntry, cProcNotExist, "stb/stream/policy/MASTER_CLOCK"                                   ,  NULL, proc_stream_MASTER_CLOCK_read, proc_stream_MASTER_CLOCK_write, 0},
  {cProcEntry, cProcNotExist, "stb/stream/policy/EXTERNAL_TIME_MAPPING"                          ,  NULL, proc_stream_EXTERNAL_TIME_MAPPING_read, proc_stream_EXTERNAL_TIME_MAPPING_write, 0},
  {cProcEntry, cProcNotExist, "stb/stream/policy/DISPLAY_FIRST_FRAME_EARLY"                      , NULL,  proc_stream_DISPLAY_FIRST_FRAME_EARLY_read, proc_stream_DISPLAY_FIRST_FRAME_EARLY_write, 0},
  {cProcEntry, cProcNotExist, "stb/stream/policy/STREAM_ONLY_KEY_FRAMES"                         , NULL,  proc_stream_STREAM_ONLY_KEY_FRAMES_read, proc_stream_STREAM_ONLY_KEY_FRAMES_write, 0},
  {cProcEntry, cProcNotExist, "stb/stream/policy/STREAM_SINGLE_GROUP_BETWEEN_DISCONTINUITIES"    , NULL,  proc_stream_STREAM_SINGLE_GROUP_BETWEEN_DISCONTINUITIES_read, proc_stream_STREAM_SINGLE_GROUP_BETWEEN_DISCONTINUITIES_write, 0},
  {cProcEntry, cProcNotExist, "stb/stream/policy/PLAYOUT_ON_TERMINATE"                            ,  NULL, proc_stream_PLAYOUT_ON_TERMINATE_read, proc_stream_PLAYOUT_ON_TERMINATE_write, 0},
  {cProcEntry, cProcNotExist, "stb/stream/policy/PLAYOUT_ON_SWITCH"                              ,  NULL, proc_stream_PLAYOUT_ON_SWITCH_read, proc_stream_PLAYOUT_ON_SWITCH_write, 0},
  {cProcEntry, cProcNotExist, "stb/stream/policy/PLAYOUT_ON_DRAIN"                               ,  NULL, proc_stream_PLAYOUT_ON_DRAIN_read, proc_stream_PLAYOUT_ON_DRAIN_write, 0},
  {cProcEntry, cProcNotExist, "stb/stream/policy/TRICK_MODE_DOMAIN"                              ,  NULL, proc_stream_TRICK_MODE_DOMAIN_read, proc_stream_TRICK_MODE_DOMAIN_write, 0},
  {cProcEntry, cProcNotExist, "stb/stream/policy/DISCARD_LATE_FRAMES"                            ,  NULL, proc_stream_DISCARD_LATE_FRAMES_read, proc_stream_DISCARD_LATE_FRAMES_write, 0},
  {cProcEntry, cProcNotExist, "stb/stream/policy/REBASE_ON_DATA_DELIVERY_LATE"                   , NULL,  proc_stream_REBASE_ON_DATA_DELIVERY_LATE_read, proc_stream_REBASE_ON_DATA_DELIVERY_LATE_write, 0},
  {cProcEntry, cProcNotExist, "stb/stream/policy/REBASE_ON_FRAME_DECODE_LATE"                    , NULL,  proc_stream_REBASE_ON_FRAME_DECODE_LATE_read, proc_stream_REBASE_ON_FRAME_DECODE_LATE_write, 0},
  {cProcEntry, cProcNotExist, "stb/stream/policy/LOWER_CODEC_DECODE_LIMITS_ON_FRAME_DECODE_LATE" , NULL,  proc_stream_LOWER_CODEC_DECODE_LIMITS_ON_FRAME_DECODE_LATE_read, proc_stream_LOWER_CODEC_DECODE_LIMITS_ON_FRAME_DECODE_LATE_write, 0},
  {cProcEntry, cProcNotExist, "stb/stream/policy/H264_ALLOW_NON_IDR_RESYNCHRONIZATION"           , NULL,  proc_stream_H264_ALLOW_NON_IDR_RESYNCHRONIZATION_read, proc_stream_H264_ALLOW_NON_IDR_RESYNCHRONIZATION_write, 0},
  {cProcEntry, cProcNotExist, "stb/stream/policy/MPEG2_IGNORE_PROGESSIVE_FRAME_FLAG"             , NULL,  proc_stream_MPEG2_IGNORE_PROGESSIVE_FRAME_FLAG_read, proc_stream_MPEG2_IGNORE_PROGESSIVE_FRAME_FLAG_write}

};

struct DeviceContext_s* ProcDeviceContext = NULL;


struct proc_dir_entry *exist_proc_dir(char *name, struct proc_dir_entry *entry) 
{
	int len;
	struct proc_dir_entry *child_proc_dir, *parent_entry;

	if (!entry)
		parent_entry = &proc_root;
	else
		parent_entry = entry;

	len = strlen(name);

	for (child_proc_dir = parent_entry->subdir; child_proc_dir; child_proc_dir=child_proc_dir->next) 
	{
		 	if ((child_proc_dir->namelen == len) && (! memcmp(child_proc_dir->name, name, len)))
				return child_proc_dir;
	}

	return NULL;
}

void init_e2_proc(struct DeviceContext_s* DC)
{
  int i;

  for(i = 0; i < sizeof(e2_procs)/sizeof(e2_procs[0]); i++)
  {
    install_e2_procs(e2_procs[i].name, e2_procs[i].read_proc,
                        e2_procs[i].write_proc,
			&DC->DvbContext->DeviceContext[e2_procs[i].context]);
  }

  /* save players device context */
  ProcDeviceContext = DC;
}

void cleanup_e2_proc(void)
{
  int i;

  for(i = sizeof(e2_procs)/sizeof(e2_procs[0]) - 1; i >= 0; i--)
  {
    remove_e2_procs(e2_procs[i].name, e2_procs[i].read_proc,
                        e2_procs[i].write_proc);
  }
}


void install_vmpeg_procs(struct DvbCurrDecoder_s* DvbCurrDecoder, unsigned char decoder)
{
	int i;
	char procname[50];
	struct proc_dir_entry *stbEntry, *vmpegEntry, *contextEntry;

	/* Create proc path stb/vmpeg/x/ */
	stbEntry = exist_proc_dir("stb", NULL);
	if 	( !stbEntry )
		stbEntry = proc_mkdir("stb", NULL);

	vmpegEntry = exist_proc_dir("vmpeg", stbEntry);
	if 	( !vmpegEntry )
		vmpegEntry = proc_mkdir("vmpeg", stbEntry);

	sprintf(procname, "%d", decoder );
	contextEntry = exist_proc_dir(procname, vmpegEntry);
	if 	( !contextEntry )
		contextEntry = proc_mkdir(procname, vmpegEntry);

	for(i = 0; i < sizeof(vmpeg_procs)/sizeof(vmpeg_procs[0]); i++)
	{
		vmpeg_procs[i].entry = create_proc_entry(vmpeg_procs[i].name, 0, contextEntry);
		vmpeg_procs[i].entry->read_proc = vmpeg_procs[i].read_proc;
		vmpeg_procs[i].entry->write_proc = vmpeg_procs[i].write_proc;
		vmpeg_procs[i].entry->data = DvbCurrDecoder;
  	}
}


void cleanup_vmpeg_proc(void)
{
	int i, decoder;
	char procname[50];
	struct proc_dir_entry *stbEntry, *vmpegEntry, *contextEntry;

	/* Verify if exists proc/stb/vmpeg path */
	stbEntry = exist_proc_dir("stb", NULL);
	if 	( !stbEntry ) return;

	vmpegEntry = exist_proc_dir("vmpeg", stbEntry);
	if 	( !vmpegEntry ) return;

	decoder=0;
	while (1) {
		sprintf(procname, "%d", decoder );
		contextEntry = exist_proc_dir(procname, vmpegEntry);
		if 	( !contextEntry ) break;
	
		for(i = 0; i < sizeof(vmpeg_procs)/sizeof(vmpeg_procs[0]); i++)
			remove_proc_entry(vmpeg_procs[i].name, vmpeg_procs[i].entry->parent);

		remove_proc_entry( procname, vmpegEntry );
		decoder++;
	}

	remove_proc_entry("vmpeg", stbEntry);
}