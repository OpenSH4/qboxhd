/*
 * 	Copyright (C) 2010 Duolabs Srl
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "mac_addr.h"

#define	MAC_ADDR_IN_FLASH	0xF000

extern flash_info_t flash_info[];	/* info for FLASH chips */

//give the mac into array 'mac_in_flash'
unsigned char read_mac_in_flash_orig(unsigned char print)
{
	flash_info_t *info;

	int cnt,cnt1=0;
	for(cnt=0;cnt<MAC_SIZE;cnt++)
		mac_in_flash_orig[cnt]=0;
	info = &flash_info[0];
	for(cnt=0;cnt<MAC_SIZE;cnt++)
		mac_in_flash_orig[cnt]=flash_read_char_duo(info,/*0x21000*/MAC_ADDR_IN_FLASH+cnt);//0xFFFA

	/* Check if alphanumeric char */
	for(cnt=0;cnt<(MAC_SIZE-1);cnt++)	//-1 for '\0'
		if(strchr("0123456789ABCDEF:",mac_in_flash_orig[cnt])==NULL)
			break;
	/* Check if there is the right ':' */

	if(	(mac_in_flash_orig[2]==':') &&
		(mac_in_flash_orig[5]==':') &&
		(mac_in_flash_orig[8]==':') &&
		(mac_in_flash_orig[11]==':') &&
		(mac_in_flash_orig[14]==':') )
			cnt1=1;

	if( (cnt1==0) && (cnt<(MAC_SIZE-1)) )//mac no ':' & invalid mac
	{
		if(print==0)
			return 0;
		memcpy(mac_in_flash_orig,"FF:FF:FF:FF:FF:FF",17);
		mac_in_flash_orig[17]='\0';
	}
	else
	{
		if(print==0)
			return 1;
	}
	puts(mac_in_flash_orig);
	puts("\n");
	return 1;

}
//write in flash the value into the array in 'mac_in_flash_orig'
void write_mac_in_flash_orig(void)
{
	flash_info_t *info;
	info = &flash_info[0];
	flash_protect(FLAG_PROTECT_CLEAR,0xA0000000+MAC_ADDR_IN_FLASH,0xA0000000+MAC_ADDR_IN_FLASH+MAC_SIZE,info);
	flash_write(mac_in_flash_orig,0xA0000000+MAC_ADDR_IN_FLASH,MAC_SIZE);
	flash_protect(FLAG_PROTECT_SET,0xA0000000+MAC_ADDR_IN_FLASH,0xA0000000+MAC_ADDR_IN_FLASH+MAC_SIZE,info);
}

/********************************/
/*	argv[0]: command (mac)		*/
/*	argv[1]: "set" or "get"		*/
/*	argv[2]: if SET, mac_addr	*/
/********************************/
void do_mac ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	unsigned char i;
	char *s;

	if(strcmp(argv[1],"get")==0)
		read_mac_in_flash(1);
	else if( (strcmp(argv[1],"set")==0) && (argc>2) )
	{
		s=argv[2];//mac addr
	
		for(i=0;i<MAC_SIZE;i++)
			mac_in_flash_orig[i]=s[i];

		write_mac_in_flash ();
	}
}



U_BOOT_CMD(
	mac,     3,	    0,     	do_mac,
	"mac - set/get the mac in flash\n",
	"empty\n"
);



