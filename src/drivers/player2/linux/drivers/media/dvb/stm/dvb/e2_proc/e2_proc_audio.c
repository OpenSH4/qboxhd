/* 
 * e2_proc_audio
 */
 
#include <linux/proc_fs.h>  	/* proc fs */ 
#include <asm/uaccess.h>    	/* copy_from_user */

#include <linux/dvb/video.h>	/* Video Format etc */

#include <linux/dvb/audio.h>
#include <linux/smp_lock.h>
#include <linux/string.h>

#include "../backend.h"
#include "../dvb_module.h"
#include "linux/dvb/stm_ioctls.h"

#include <asm/io.h>

//Dagobert Hack
#include <sound/driver.h>
#include <sound/core.h>
#include <sound/control.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/rawmidi.h>
#include <sound/initval.h>
#include "../../../../../sound/pseudocard/pseudo_mixer.h"
#include <avs_core.h>

#define PSEUDO_ADDR(x) (offsetof(struct snd_pseudo_mixer_settings, x))
extern struct DeviceContext_s* ProcDeviceContext;

extern int AudioIoctlSetBypassMode (struct DeviceContext_s* Context, unsigned int Mode);
extern int AudioIoctlPause (struct DeviceContext_s* Context);
extern int AudioIoctlContinue (struct DeviceContext_s* Context);
extern int AudioIoctlClearBuffer (struct DeviceContext_s* Context);
extern int VideoIoctlFreeze (struct DeviceContext_s* Context);
extern int VideoIoctlContinue (struct DeviceContext_s* Context);
extern int VideoIoctlClearBuffer (struct DeviceContext_s* Context);

extern struct snd_kcontrol ** pseudoGetControls(int* numbers);
extern int snd_pseudo_integer_get(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol);
extern int snd_pseudo_integer_put(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol);
#if defined(CONFIG_SH_QBOXHD_1_0) || defined(CONFIG_SH_QBOXHD_MINI_1_0)
static int avs_command_kernel(unsigned int cmd, void *arg) { return 0; }
#else
int avs_command_kernel(unsigned int cmd, void *arg);
#endif

int proc_audio_delay_pcm_write(struct file *file, const char __user *buf,
                           unsigned long count, void *data)
{
	char 		*page;
	char		*myString;
	ssize_t 	ret = -ENOMEM;
	
	printk("%s %d - ", __FUNCTION__, (int) count);

	page = (char *)__get_free_page(GFP_KERNEL);
	if (page) 
	{
		ret = -EFAULT;
		if (copy_from_user(page, buf, count))
			goto out;

		myString = (char *) kmalloc(count + 1, GFP_KERNEL);
		strncpy(myString, page, count);
		myString[count] = '\0';

		printk("%s\n", myString);
		kfree(myString);
		//result = sscanf(page, "%3s %3s %3s %3s %3s", s1, s2, s3, s4, s5);
	}
	
	ret = count;
out:
	
	free_page((unsigned long)page);
	return ret;
}


int proc_audio_delay_pcm_read (char *page, char **start, off_t off, int count,
			  int *eof, void *data_unused)
{
	int len = 0;
	printk("%s %d\n", __FUNCTION__, count);

        return len;
}

int proc_audio_delay_bitstream_write(struct file *file, const char __user *buf,
                           unsigned long count, void *data)
{
	char 		*page;
	char		*myString;
	ssize_t 	ret = -ENOMEM;
	
	printk("%s %d - ", __FUNCTION__, (int) count);

	page = (char *)__get_free_page(GFP_KERNEL);
	if (page) 
	{
		ret = -EFAULT;
		if (copy_from_user(page, buf, count))
			goto out;

		myString = (char *) kmalloc(count + 1, GFP_KERNEL);
		strncpy(myString, page, count);
		myString[count] = '\0';

		printk("%s\n", myString);
		kfree(myString);
		//result = sscanf(page, "%3s %3s %3s %3s %3s", s1, s2, s3, s4, s5);
	}
	
	ret = count;
out:
	
	free_page((unsigned long)page);
	return ret;
}


int proc_audio_delay_bitstream_read (char *page, char **start, off_t off, int count,
			  int *eof, void *data_unused)
{
	int len = 0;
	printk("%s %d\n", __FUNCTION__, ProcDeviceContext->AudioState.mute_state);
	len = sprintf(page, "%d\n", ProcDeviceContext->AudioState.mute_state);
        return len;
}

int volume = 0;

static unsigned long ReadRegister(volatile unsigned long *reg)
{
  return readl((unsigned long)reg);
}

static void WriteRegister(volatile unsigned long *reg,unsigned long val)
{
  writel(val, (unsigned long)reg);
}

#if defined(UFS912)
void spdif_out_mute(int mute)
{
#warning fixme search the audio mute register for 7111 arch
   printk("warning: spdif_out_mute currently not implemented for 7111 arch\n");
}
#else

#define AUD_IO_CTRL		0x200
#define SPDIF_EN		(1L<<3)
#define SPDIF_DIS		0xFFFFFFF7
#define STb7100_REGISTER_BASE	0x18000000
#define STb7100_AUDIO_BASE	0x01210000
#define STb7100_REG_ADDR_SIZE	0x02000000


void spdif_out_mute(int mute)
{
	unsigned long val;
	unsigned long *RegMap;
	RegMap = (unsigned long*)ioremap(STb7100_REGISTER_BASE,STb7100_REG_ADDR_SIZE);
	if(mute==AVS_MUTE)
	{
		val = ReadRegister( RegMap + ( (STb7100_AUDIO_BASE+AUD_IO_CTRL) >>2) );
		WriteRegister(RegMap + ((STb7100_AUDIO_BASE+AUD_IO_CTRL)>>2),val & SPDIF_DIS);
	}
	else
	{
		val = ReadRegister( RegMap + ( (STb7100_AUDIO_BASE+AUD_IO_CTRL) >>2) );
		WriteRegister(RegMap + ((STb7100_AUDIO_BASE+AUD_IO_CTRL)>>2),val|SPDIF_EN);
	}
	iounmap(RegMap);
}

#endif

int proc_audio_j1_mute_write(struct file *file, const char __user *buf,
                           unsigned long count, void *data)
{
	char 		*page;
	char		*myString;
	ssize_t 	ret = -ENOMEM;
	unsigned int 	State;
	
	printk("%s %d - ", __FUNCTION__, (int) count);
	mutex_lock (&(ProcDeviceContext->DvbContext->Lock));

	page = (char *)__get_free_page(GFP_KERNEL);
	if (page) 
	{
		ret = -EFAULT;
		if (copy_from_user(page, buf, count))
			goto out;

		myString = (char *) kmalloc(count + 1, GFP_KERNEL);
		strncpy(myString, page, count);
		myString[count] = '\0';

		printk("%s\n", myString);
		sscanf(myString, "%d", &State);
		
		if (State == 1) { //MUTE
			int number = 0;
			struct snd_kcontrol *single_control = NULL;
			struct snd_kcontrol ** kcontrol = pseudoGetControls(&number);
			int vLoop;
		
			for (vLoop = 0; vLoop < number; vLoop++)
			{
				if (kcontrol[vLoop]->private_value == PSEUDO_ADDR(master_volume))
				{
					single_control = kcontrol[vLoop];
					//printk("Finde master_volume control at %p\n", single_control);
					break;
				}		
			}	
	
	                if ((kcontrol != NULL) && (single_control != NULL))
			{
				struct snd_ctl_elem_value ucontrol;
				//printk("Pseudo Mixer controls = %p\n", kcontrol);
				snd_pseudo_integer_get(single_control, &ucontrol);
	
				volume = ucontrol.value.integer.value[0];

				ucontrol.value.integer.value[0] = -63;
				ucontrol.value.integer.value[1] = -63;
				ucontrol.value.integer.value[2] = -63;
				ucontrol.value.integer.value[3] = -63;
				ucontrol.value.integer.value[4] = -63;
				ucontrol.value.integer.value[5] = -63;
			#if defined(CONFIG_SH_QBOXHD_1_0) || defined(CONFIG_SH_QBOXHD_MINI_1_0)
			/* For 'Side Left' and 'Side Right' in pseudo_card 'Master' */
				ucontrol.value.integer.value[6] = -63;
				ucontrol.value.integer.value[7] = -63;
			#endif
				snd_pseudo_integer_put(single_control, &ucontrol);

			} else
			{
				printk("Pseudo Mixer does not deliver controls\n");
			}
			spdif_out_mute(AVS_MUTE);
			avs_command_kernel(AVSIOSMUTE, (void*) AVS_MUTE);

		} else { //UNMUTE
			int number = 0;
			struct snd_kcontrol *single_control = NULL;
			struct snd_kcontrol ** kcontrol = pseudoGetControls(&number);
			int vLoop;
		
			for (vLoop = 0; vLoop < number; vLoop++) {
				if (kcontrol[vLoop]->private_value == PSEUDO_ADDR(master_volume)) {
					single_control = kcontrol[vLoop];
					//printk("Find master_volume control at %p\n", single_control);
					break;
				}		
			}	
	
			if ((kcontrol != NULL) && (single_control != NULL) ){
				struct snd_ctl_elem_value ucontrol;
				//printk("Pseudo Mixer controls = %p\n", kcontrol);
				
				//if volume has changed or is not in mute do nothing
				snd_pseudo_integer_get(single_control, &ucontrol);
				if(ucontrol.value.integer.value[0] == -63)
				{
					ucontrol.value.integer.value[0] = volume;
					ucontrol.value.integer.value[1] = volume;
					ucontrol.value.integer.value[2] = volume;
					ucontrol.value.integer.value[3] = volume;
					ucontrol.value.integer.value[4] = volume;
					ucontrol.value.integer.value[5] = volume;
				#if defined(CONFIG_SH_QBOXHD_1_0) || defined(CONFIG_SH_QBOXHD_MINI_1_0)
				/* For 'Side Left' and 'Side Right' in pseudo_card 'Master' */
					ucontrol.value.integer.value[6] = volume;
					ucontrol.value.integer.value[7] = volume;
				#endif

					snd_pseudo_integer_put(single_control, &ucontrol);
				}
	
			} else {
				printk("Pseudo Mixer does not deliver controls\n");
			}
			spdif_out_mute(AVS_UNMUTE);
			avs_command_kernel(AVSIOSMUTE, AVS_UNMUTE);
		}

		kfree(myString);
		//result = sscanf(page, "%3s %3s %3s %3s %3s", s1, s2, s3, s4, s5);
	}
	
	ret = count;
out:
	free_page((unsigned long)page);
	mutex_unlock (&(ProcDeviceContext->DvbContext->Lock));
	return ret;
}


int proc_audio_j1_mute_read (char *page, char **start, off_t off, int count,
			  int *eof, void *data_unused)
{
	int len = 0;
	
	int number = 0;
	struct snd_kcontrol *single_control = NULL;
	struct snd_kcontrol ** kcontrol = pseudoGetControls(&number);
	int vLoop;
	
	for (vLoop = 0; vLoop < number; vLoop++)
	{
		if (kcontrol[vLoop]->private_value == PSEUDO_ADDR(master_volume))
		{
			single_control = kcontrol[vLoop];
			break;
		}		
	}	
	if ((kcontrol != NULL) && (single_control != NULL))
	{
		struct snd_ctl_elem_value ucontrol;
		snd_pseudo_integer_get(single_control, &ucontrol);
		if (ucontrol.value.integer.value[0] > -63)
			len = sprintf(page, "0\n");
		else
			len = sprintf(page, "1\n");
	} else
	{
		printk("Pseudo Mixer does not deliver controls\n");
	}

        return len;
}

static int passthrough = 1;

int e2_proc_audio_getPassthrough(void) {
	return passthrough;
}

int proc_audio_ac3_write(struct file *file, const char __user *buf,
                           unsigned long count, void *data)
{
	char 		*page;
	char		*myString;
	ssize_t 	ret = -ENOMEM;
	
	printk("%s %d - ", __FUNCTION__, (int) count);

	page = (char *)__get_free_page(GFP_KERNEL);
	if (page) 
	{
		ret = -EFAULT;
		if (copy_from_user(page, buf, count))
			goto out;

		myString = (char *) kmalloc(count + 1, GFP_KERNEL);
		strncpy(myString, page, count);
		myString[count] = '\0';

		printk("%s\n", myString);
		
		if (strncmp("passthrough", page, count - 1) == 0)
		{
			if(passthrough == 0)
			{
				passthrough = 1;
				if (ProcDeviceContext != NULL)
				{
			   	int last_state = ProcDeviceContext->AudioState.play_state;
			   
			   	/* avoid ugly sound */
					if (last_state == AUDIO_PLAYING)
					{
			      AudioIoctlPause (ProcDeviceContext);
			      AudioIoctlClearBuffer (ProcDeviceContext);

			      VideoIoctlFreeze (ProcDeviceContext);
			      VideoIoctlClearBuffer (ProcDeviceContext);
					}
			   
					AudioIoctlSetBypassMode (ProcDeviceContext, ((ProcDeviceContext->AudioEncoding == AUDIO_ENCODING_AC3) ? 0 : 1));

					if (last_state == AUDIO_PLAYING)
					{
						AudioIoctlContinue (ProcDeviceContext);
			      VideoIoctlContinue (ProcDeviceContext);
					}
				}
			}
		} else { //downmix
			if(passthrough == 1)
			{
				passthrough = 0;

				if (ProcDeviceContext != NULL)
                        {
					int last_state = ProcDeviceContext->AudioState.play_state;
			   
			   /* avoid ugly sound */
					if (last_state == AUDIO_PLAYING)
					{
						AudioIoctlPause (ProcDeviceContext);
			      AudioIoctlClearBuffer (ProcDeviceContext);
			      VideoIoctlFreeze (ProcDeviceContext);
			      VideoIoctlClearBuffer (ProcDeviceContext);
					}

					AudioIoctlSetBypassMode (ProcDeviceContext, ((ProcDeviceContext->AudioEncoding == AUDIO_ENCODING_AC3) ? 0 : 1));

					if (last_state == AUDIO_PLAYING)
					{
						AudioIoctlContinue (ProcDeviceContext);
			      VideoIoctlContinue (ProcDeviceContext);
					}
				}
			}
		}

		kfree(myString);
		//result = sscanf(page, "%3s %3s %3s %3s %3s", s1, s2, s3, s4, s5);
	}
	
	ret = count;
out:
	
	free_page((unsigned long)page);
	return ret;
}


int proc_audio_ac3_read (char *page, char **start, off_t off, int count,
			  int *eof, void *data_unused)
{
	int len = 0;
	printk("%s %d\n", __FUNCTION__, count);

	if (passthrough == 1)
	{
		len = sprintf(page, "passthrough\n");
	} else { //downmix
		len = sprintf(page, "downmix\n");
	}

        return len;
}

int proc_audio_ac3_choices_read (char *page, char **start, off_t off, int count,
			  int *eof, void *data_unused)
{
	int len = 0;
	printk("%s %d\n", __FUNCTION__, count);

	len = sprintf(page, "downmix passthrough\n");

        return len;
}
