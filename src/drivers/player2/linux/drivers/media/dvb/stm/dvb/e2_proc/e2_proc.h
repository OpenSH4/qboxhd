#ifndef E2_PROC_H_
#define E2_PROC_H_

int proc_video_aspect_get(void);
int proc_video_policy_get(void);
#if defined CONFIG_SH_QBOXHD_1_0 || defined CONFIG_SH_QBOXHD_MINI_1_0
int proc_video_policy2_get(void);
#endif
int e2_proc_audio_getPassthrough(void);

#endif
