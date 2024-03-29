/* 
 * e2_proc_info.c
 */
 
#include <linux/proc_fs.h>  	/* proc fs */ 
#include <asm/uaccess.h>    	/* copy_from_user */


int proc_info_model_read (char *page, char **start, off_t off, int count,
			  int *eof, void *data_unused)
{
	int len = 0;
	printk("%s\n", __FUNCTION__);

#if defined(CUBEREVO)
	len = sprintf(page, "cuberevo\n");
#elif defined(CUBEREVO_MINI)
	len = sprintf(page, "cuberevo-mini\n");
#elif defined(CUBEREVO_MINI2)
	len = sprintf(page, "cuberevo-mini2\n");
#elif defined(CUBEREVO_250HD)
	len = sprintf(page, "cuberevo-250hd\n");
#elif defined(CUBEREVO_MINI_FTA)
	len = sprintf(page, "cuberevo-mini-fta\n");
#elif defined(CUBEREVO_2000HD)
	len = sprintf(page, "cuberevo-2000hd\n");
#elif defined(CUBEREVO_9500HD)
	len = sprintf(page, "cuberevo-9500hd\n");
#elif defined(TF7700)
	len = sprintf(page, "tf7700\n");
#elif defined(HL101)
	len = sprintf(page, "hl101\n");
#elif defined(UFS922)
	len = sprintf(page, "ufs922\n");
#elif defined(FORTIS_HDBOX)
	len = sprintf(page, "hdbox\n");
#else
	len = sprintf(page, "ufs910\n");
#endif

        return len;
}

