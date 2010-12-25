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
#include <sys/ioctl.h>

#include "lcd.h"

#define LCD_SIZE	320 * 240 * 2

int main(int argc, char * argv[])
{
	int 			i, 
					lcd;
	FILE 			*fd = NULL;
	char			*progname;
	unsigned char 	ptr, 
					buff[LCD_SIZE];

	if (argc < 2)
		return -1;
	progname = argv[0];
	memset(buff, 0, LCD_SIZE);

	if ((fd = fopen((const char *)argv[1], "rb")) == NULL) {
		fprintf(stderr, "%s: %m '%s'\n", progname, argv[1]);
		return -1;
	}

	if (fread(buff, sizeof(char), LCD_SIZE, fd) != LCD_SIZE) {
		fprintf(stderr, "%s: Could not read %d bytes\n", progname, LCD_SIZE);
		fclose(fd);
		return -1;
	}
	fclose(fd);

	for (i = 0; i < LCD_SIZE; i += 2) {
		ptr = buff[i];
		buff[i] = buff[i + 1];
		buff[i + 1] = ptr;
	}

	if ((lcd = open("/dev/lcd", O_RDWR | O_NONBLOCK)) == -1) {
		fprintf(stderr, "%s: %m\n", progname);
		return -1;
	}

	/* Removed the backlight */
	ioctl(lcd,LCD_IOCTL_REG_BRIGHTNESS,0);

	if (write(lcd, buff, LCD_SIZE) == -1) {
		fprintf(stderr, "%s: %m\n", progname);
		close(lcd);
		return -1;
	}

	/* Set the backlight */
	ioctl(lcd,LCD_IOCTL_REG_BRIGHTNESS,31);

	close(lcd);
	return 0;
}

