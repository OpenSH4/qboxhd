/*
 *  Copyright (C) 2010 Duolabs Srl
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>

#define NAME_SIZE		256
#define ACTION_SIZE 	256
#define DEVPATH_SIZE 	256
#define DEVTYPE_SIZE		256
#define DATA_SIZE		2048

#define HOTPLUG_SOCKET(X)                       \
    do {                                        \
        memset(X, sizeof(X), 0);                \
        sprintf(X, "/tmp/hotplug.socket");      \
    } while (0);

static const char *usage = 
	"\nUsage: %s \n"
	"	-a ACTION (\"add\" or \"remove\")\n"
	"	-d DEVPATH\n"
	"	-t DEVICETYPE\n"
	"\n";


int main(int argc, char **argv) {
    int                		ret, sock;
	int 					opt;
    char               		sock_name[NAME_SIZE];
    socklen_t          		addr_len;
    struct sockaddr_un 		core_addr;

	char 					action[ACTION_SIZE];
	char 					devpath[DEVPATH_SIZE];
	char 					devtype[DEVTYPE_SIZE];
	char 					data[DATA_SIZE];
	unsigned char 			action_defined = 0, devpath_defined = 0, devtype_defined = 0;

	printf("\nhotplug manager v.1.1\n");

	memset( devpath, 0, DEVPATH_SIZE );
	memset( action, 0, ACTION_SIZE );

	while ((opt = getopt(argc, argv, "h:a:d:t:")) != -1) {
		switch (opt) {
		case 'a':
			strncpy(action, optarg, ACTION_SIZE);
			action_defined = 1;
			break;
		case 'd':
			strncpy(devpath, optarg, DEVPATH_SIZE);
			devpath_defined = 1;
			break;
		case 't':
			strncpy(devtype, optarg, DEVTYPE_SIZE);
			devtype_defined = 1;
			break;
		case '?':
		case 'h':
		default:
			fprintf (stderr, usage, argv[0]);
			return -1;
		};
	}

	if ( (!action_defined) || (!devpath_defined) || (!devtype_defined) ) {
			fprintf (stderr, usage, argv[0]);
			return -1;
	}

    HOTPLUG_SOCKET(sock_name);

    if ((sock = socket(PF_LOCAL, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "socket(): %s", strerror(errno));
        return -1;
    }

    core_addr.sun_family = PF_LOCAL;
    strncpy(core_addr.sun_path, sock_name, sizeof(core_addr.sun_path));
    addr_len = sizeof(core_addr.sun_family) + strlen(core_addr.sun_path) + 1;
    if (connect(sock, (struct sockaddr *)&core_addr, addr_len) == -1) {
        fprintf(stderr, "connect(): %s", strerror(errno));
        close(sock);
        return -1;
    }

	if ( action_defined ) {
			sprintf(data, "ACTION=%s", action);			
			printf( "%s\n", data );
			ret = write(sock, data, strlen(data)+1);
			if (ret < 0) {
				fprintf(stderr, "write(): %m\n");
				close(sock);
			    return -1;
			}				
	}

	if ( devpath_defined ) {
			sprintf(data, "DEVPATH=%s", devpath);			
			printf( "%s\n", data );
			ret = write(sock, data, strlen(data)+1);
			if (ret < 0) {
				fprintf(stderr, "write(): %m\n");
				close(sock);
			    return -1;
			}		
	}

	if ( devtype_defined ) {
			sprintf(data, "DEVTYPE=%s", devtype);			
			printf( "%s\n", data );
			ret = write(sock, data, strlen(data)+1);
			if (ret < 0) {
				fprintf(stderr, "write(): %m\n");
				close(sock);
			    return -1;
			}		
	}
    close(sock);

    return 0;
}
