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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define	BLOCK_LEN	4096
#define	MAC_ADDR_IN_FLASH	0xF000
#define	MAC_SIZE	18

#define	END_ADDR			0xFFFF
#define	LAST_BYTEs_TO_CHECK	16

/* @brief Check that the MAC addr has the rigth syntax.
 * 		  Give the mac into array 'mac_in_flash' 
 */
unsigned char check_mac(unsigned char *b)
{
	int cnt,cnt1=0;

	/* Check if alphanumeric char */
	for(cnt=0;cnt<(MAC_SIZE-1);cnt++)	//-1 for '\0'
	{
		if(strchr("0123456789ABCDEF:",b[cnt])==NULL)
		{
			cnt=1;	/* For more security */
			break;
		}
		if(b[cnt]==0)
		{
			cnt=1;	/* For more security */
			break;
		}
	}

	/* Check if there is the right ':' */
	if(	(b[0]!=':') &&
		(b[1]!=':') &&
		(b[2]==':') &&
		(b[3]!=':') &&
		(b[4]!=':') &&
		(b[5]==':') &&
		(b[6]!=':') &&
		(b[7]!=':') &&
		(b[8]==':') &&
		(b[9]!=':') &&
		(b[10]!=':') &&
		(b[11]==':') &&
		(b[12]!=':') &&
		(b[13]!=':') &&
		(b[14]==':') &&
		(b[15]!=':') &&
		(b[16]!=':') &&
		(b[17]=='\0') )
			cnt1=1;

	if (cnt1 == 1) {
		if ((strcmp((const char *)b,"00:00:00:00:00:00")==0) ||
			(strcmp((const char *)b,"FF:FF:FF:FF:FF:FF")==0))
			cnt1 = 0;
	}

	if ((cnt1 == 0) || (cnt < (MAC_SIZE - 1))) /* "mac no ':'" || "invalid mac" */
		return 0;
	else
		return 1;
}

/* @brief Check if the flash is written 
 */
unsigned char check_write_nor(unsigned char * b)
{
	int cnt = 0, cnt1 = 0;

	unsigned char cmp_byte[LAST_BYTEs_TO_CHECK];

	memset(cmp_byte,0xFF,LAST_BYTEs_TO_CHECK);
	cnt = (-1);
	cnt = memcmp(b,cmp_byte,LAST_BYTEs_TO_CHECK);

	memset(cmp_byte,0x00,LAST_BYTEs_TO_CHECK);
	cnt1 = (-1);
	cnt1 = memcmp(b,cmp_byte,LAST_BYTEs_TO_CHECK);

	if ((cnt == 0) || (cnt1 == 0) )	/* cnt==0 -> all bytes to 0xFF; cnt1==0 -> all bytes to 0x00 */
		return 0;
	else
		return 1;
}

void help(const char * progname)
{
	const char usage[] = "\nUsage: %s <filename> <length> <offset>\n"
						"\tfilename: File name where the MAC address is stored\n"
						"\tlength:   Length of file in decimal\n"
						"\toffset:   Offset inside the file\n\n";

	fprintf(stderr, usage, progname);
}

int main(int argc, char * argv[])
{
	int 			offset=0,file_size=0;
	FILE 			*fd = NULL;
	char			*progname;
	unsigned char 	*buff;
	int ret=0;

	progname = argv[0];

	if(argc != 4) {
		help(progname);
		return -1;
	}
	
	if ((fd = fopen((const char *)argv[1], "rb")) == NULL) {
		fprintf(stderr, "%s: %m '%s'\n", progname, argv[1]);
		return -1;
	}

	file_size=atoi((const char *)argv[2]);
	offset=atoi((const char *)argv[3]);

	if ((buff = (unsigned char *)malloc(file_size)) == NULL) {
		fprintf(stderr, "%s: Could not allocate memory\n", progname);
		return -1;
	}
	memset(buff, 0, file_size);

	if (fread(buff, sizeof(char), file_size, fd) != file_size) {
		fprintf(stderr, "%s: Could not read %d bytes\n", progname, file_size);
		fclose(fd);
		free(buff);
		return -1;
	}
	fclose(fd);

	if ((check_mac(buff + offset) == 0) || (check_write_nor(buff + file_size - LAST_BYTEs_TO_CHECK) == 0))
		ret = (-1);
	else
		ret = 0;

	free(buff);
	return ret;
}

