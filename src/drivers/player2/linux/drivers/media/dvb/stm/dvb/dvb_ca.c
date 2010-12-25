/* *********************************************************************
* CA Device
*  Association:
*    ca0 -> cimax
*    ca1 .. caX -> hardware descrambler via PTI session
***********************************************************************/

#include <linux/module.h>
#include <linux/dvb/audio.h>
#include <linux/dvb/ca.h>

#include "dvb_module.h"
#include "backend.h"
#include "dvb_ca.h"

//__TDT__ ->file moved in this directory

#if defined(CONFIG_SH_QBOXHD_1_0) || defined(CONFIG_SH_QBOXHD_MINI_1_0)
#include "pti_hal.h"

	int stca_debug=0;
	module_param(stca_debug, int, 0644);
	MODULE_PARM_DESC(stca_debug, "Modify ST-CA debugging level (default:0=OFF)");
	#define TAGDEBUG "[ST-CA] "
	#define dprintk(level, x...) do { if (stca_debug && (level <= stca_debug)) printk(TAGDEBUG x); } while (0)
#else
#include "st-common.h"
#endif

static int ca_open (struct inode*     Inode,
		   struct file*      File)
{
    //struct dvb_device* DvbDevice = (struct dvb_device*)File->private_data;

    return dvb_generic_open (Inode, File);
}

static int ca_release (struct inode*  Inode,
			 struct file*   File)
{
    //struct dvb_device* DvbDevice = (struct dvb_device*)File->private_data;

    return dvb_generic_release (Inode, File);
}

static void dumpArray(int *buffer,int len,const char *msg)
{
	int count=len;
	char line[500]="";
	char val[5]="";

	while (count--)
	{
		sprintf(val, "%d,", *buffer);
		strcat(line,val);
		buffer++;
	};

	dprintk(3,"%s: %s %s\n",__func__,msg,line);
}

static void dumpDescramblerIndex(int *buffer,int len)
{

	dumpArray(buffer,len,"DescramblerIndex=");
}


static int ca_ioctl(struct inode *inode, struct file *file,
		 unsigned int cmd, void *parg)
{
    struct dvb_device* DvbDevice = (struct dvb_device*)file->private_data;
    struct DeviceContext_s* pContext = (struct DeviceContext_s*)DvbDevice->priv;
    struct PtiSession *pSession = pContext->pPtiSession;

    if(pSession == NULL)
    {
      printk(KERN_INFO"%s:CA is not associated with a session\n",__func__);
      return -EINVAL;
    }

    switch (cmd)
    {
		case CA_RESET:
			printk(KERN_INFO"%s: CA_RESET\n",__func__);
		break;
		case CA_GET_CAP:
			printk(KERN_INFO"%s: CA_GET_CAP\n",__func__);
		break;
		case CA_GET_SLOT_INFO:
			printk(KERN_INFO"%s: CA_GET_SLOT_INFO\n",__func__);
		break;
		case CA_GET_DESCR_INFO:
			printk(KERN_INFO"%s: CA_GET_DESCR_INFO\n",__func__);
		break;
		case CA_GET_MSG:
			printk(KERN_INFO"%s: CA_GET_MSG\n",__func__);
		break;
		case CA_SEND_MSG:
			printk(KERN_INFO"%s: CA_SEND_MSG\n",__func__);
		break;
		case CA_SET_PID: // currently this is useless but prevents from softcams errors
		{
			ca_pid_t *service_pid = (ca_pid_t*) parg;
			int vLoop;
			unsigned short pid = service_pid->pid;
			int descramble_index = service_pid->index;
			dprintk(2,"%s: CA_SET_PID index = %d pid 0x%x\n",__func__,descramble_index,pid);

#ifdef NEW_DESCRAMBLER
			if(descramble_index >=0)
			{
				if( descramble_index >= NUMBER_OF_DESCRAMBLERS)
				{
					dprintk(1,"Error only descramblers 0 - %d supported\n",NUMBER_OF_DESCRAMBLERS-1);
					return -1;
				}

				pSession->descramblerForPid[pid]=descramble_index;
				for ( vLoop = 0; vLoop < pSession->num_pids; vLoop++ )
				{
					if (( ( unsigned short ) pSession->pidtable[vLoop] == ( unsigned short ) pid ))
					{
						if ( pSession->type[vLoop] == DMX_TYPE_TS )
						{
								/* link audio/video slot to the descrambler */
							if ((pSession->pes_type[vLoop] == DMX_TS_PES_VIDEO) ||
								(pSession->pes_type[vLoop] == DMX_TS_PES_AUDIO) ||
								(pid>50)) /*dirty hack because for some reason the pes_type is changed to DMX_TS_PES_OTHER*/
							{
								int err;

								dprintk(4,"%s: previous descrambler index was: %d, current is: %d, vLoop=%d\n",
										__func__,pSession->descramblerindex[vLoop], descramble_index, vLoop);

								dumpDescramblerIndex(pSession->descramblerindex,32);

								if(pSession->descramblerindex[vLoop]!=descramble_index)
								{
									pSession->descramblerindex[vLoop]=descramble_index;

									if ((err = pti_hal_descrambler_link(pSession->session,
																		pSession->descramblers[pSession->descramblerindex[vLoop]],
																		pSession->slots[vLoop])) != 0)
										dprintk(2,"ERROR: linking slot %d to descrambler %d, err = %d\n",
												pSession->slots[vLoop],
												pSession->descramblers[pSession->descramblerindex[vLoop]],
												err);
									else
										dprintk(3,"linking pid 0x%x slot %d to descrambler %d, session = %d pSession %p\n",
													pid,pSession->slots[vLoop],
													pSession->descramblers[pSession->descramblerindex[vLoop]],
													pSession->session, pSession);
									return 0;
								}
								else
								{
									dprintk(3,"pid 0x%x is already linked to descrambler %d\n",pid,descramble_index);
									return 0;
								}
							}
							else
							{
								dprintk(1,"pid 0x%x no audio or video pid! type=%d slot=%d not linking to descrambler\n",
											pid,pSession->pes_type[vLoop],pSession->slots[vLoop]);
								return -1;
							}
						}
						else
						{
							dprintk(1,"pid 0x%x type is not DMX_TYPE_TS! not linking to descrambler\n",pid);
							return -1;
						}
					}
				}

				dprintk(2,"pid 0x%x not found in pidtable, it might be inactive\n",pid);
			}
			else if(descramble_index == -1)
			{
				dprintk(2,"pid 0x%x descramble_index == -1\n",pid);
				
#if defined(CONFIG_SH_QBOXHD_1_0) || defined(CONFIG_SH_QBOXHD_MINI_1_0)
				for ( vLoop = 0; vLoop < pSession->num_pids; vLoop++ )
				{
					if (( ( unsigned short ) pSession->pidtable[vLoop] == ( unsigned short ) pid ))
					{
						int err;
						if ((err = pti_hal_descrambler_unlink( pSession->session, pSession->descramblers[pSession->descramblerindex[vLoop]], pSession->slots[vLoop] )) != cHALNoError )
						{
						      dprintk(2,"ERROR: unlinking slot %d to descrambler %d, err = %d\n",
							      pSession->slots[vLoop],
							      pSession->descramblers[pSession->descramblerindex[vLoop]],
							      err);
						}
						break;
					}  
				}
				pSession->descramblerindex[vLoop] = -1;
#endif
				pSession->descramblerForPid[pid]=-1;
			}

#endif
			return 0;
		}
		break;

		case CA_SET_DESCR:
		{
			ca_descr_t *descr = (ca_descr_t*) parg;

	//		DVB_DEBUG("CA_SET_DESCR\n");

			if (descr->index >= 16)
				return -EINVAL;
			if (descr->parity > 1)
				return -EINVAL;

			if (&pContext->DvbContext->Lock != NULL)
				mutex_lock (&pContext->DvbContext->Lock);

	#ifdef NEW_DESCRAMBLER
			if(descr->index < 0 || descr->index >= NUMBER_OF_DESCRAMBLERS)
			{
				dprintk(2,"CA_SET_DESCR\n");
				dprintk(3,"index = %d parity = %d\n", descr->index, descr->parity);
				dprintk(4,"cw: 0x%x 0x%x 0x%x 0x%x  0x%x 0x%x 0x%x 0x%x\n",
						descr->cw[0],descr->cw[1],descr->cw[2],descr->cw[3],
						descr->cw[4],descr->cw[5],descr->cw[6],descr->cw[7]);
				dprintk(1,"Error descrambler %d not supported! needs to be in range 0 - %d\n", descr->index, NUMBER_OF_DESCRAMBLERS-1);
				return -1;
			}

			if (pti_hal_descrambler_set(pSession->session, pSession->descramblers[descr->index], descr->cw, descr->parity) != 0)
				dprintk(1,"Error while setting descrambler keys\n");

			dprintk(2,"CA_SET_DESCR\n");
			dprintk(3,"index = %d parity = %d\n", descr->index, descr->parity);
			dprintk(4,"cw: 0x%x 0x%x 0x%x 0x%x  0x%x 0x%x 0x%x 0x%x\n",
					descr->cw[0],descr->cw[1],descr->cw[2],descr->cw[3],
					descr->cw[4],descr->cw[5],descr->cw[6],descr->cw[7]);
	#else
			if (pti_hal_descrambler_set(pSession->session, pSession->descrambler, descr->cw, descr->parity) != 0)
				printk(KERN_INFO"Error while setting descrambler keys\n");
	#endif

			if (&pContext->DvbContext->Lock != NULL)
				mutex_unlock (&pContext->DvbContext->Lock);

			return 0;
		break;
		}

		default:
			dprintk(2,"%s: Error - invalid ioctl %08x\n", __FUNCTION__, cmd);
    }

    return -ENOIOCTLCMD;
}


static struct file_operations ca_fops =
{
	owner:          THIS_MODULE,
	ioctl:          dvb_generic_ioctl, /* fixme: kann das nicht wech; sollte doch nur stoeren hier?! */
	open:           ca_open,
	release:        ca_release,
};

static struct dvb_device ca_device =
{
	priv:            NULL,
	users:           1,
	readers:         1,
	writers:         1,
	fops:            &ca_fops,
	kernel_ioctl:    ca_ioctl,
};

static int caInitialized = 0;

extern int init_ci_controller(struct dvb_adapter* dvb_adap);

struct dvb_device *CaInit(struct DeviceContext_s *DeviceContext)
{
  dprintk(2,"CaInit()\n");
  if(!caInitialized)
  {
    /* the following call creates ca0 associated with the cimax hardware */
    dprintk(2,"Initializing CI Controller\n");

    init_ci_controller(&DeviceContext->DvbContext->DvbAdapter);

    caInitialized = 1;
  }

  /* returning the device structure creates further ca devices which
     are associated with the hardware descrambler */
  return &ca_device;
}

