
#include <common.h>
#include <command.h>


#define	MAC_SIZE	18

unsigned char mac_in_flash_orig[MAC_SIZE];

unsigned char read_mac_in_flash_orig(unsigned char print);
void write_mac_in_flash(void);

