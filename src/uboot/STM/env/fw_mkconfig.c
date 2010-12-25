#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "config.h"
#include "fw_env.h"

static char *hdr = 
"# Configuration file for fw_(printenv/saveenv) utility.\n"
"# Up to two entries are valid, in this case the redundand\n"
"# environment sector is assumed present.\n"
"# MTD device name	Device offset	Env. size	Flash sector size"
;


int main(int argc, char *argv[])
{
	if (argc > 1) {
		exit(EXIT_FAILURE);
	}
	puts(hdr);
	printf("%s\t\t0x%08x\t0x%08x\t0x%0x\n", DEVICE1_NAME, DEVICE1_OFFSET, ENV1_SIZE, DEVICE1_ESIZE);
#ifdef  HAVE_REDUND
	printf("%s\t\t0x%08x\t0x%08x\t0x%0x\n", DEVICE2_NAME, DEVICE2_OFFSET, ENV2_SIZE, DEVICE2_ESIZE);
#endif        
	exit(EXIT_SUCCESS);
}
