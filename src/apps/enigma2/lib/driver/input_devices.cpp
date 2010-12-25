#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <directfb.h>
#include <directfb_keynames.h>

#include <direct/mem.h>
#include <direct/messages.h>
#include <direct/thread.h>
#include <direct/util.h>

#include <lib/base/eerror.h>
#include "input_devices.h"

/**********/
#include <lib/driver/input_fake.h>
/**********/

#define INPUT_DEVICES	2	/* Now there are 2 input device: keyboard and lircd */
#define	LIRCD_INPUT		0
#define	KEYBOARD_INPUT	1

#define LOCAL_SOCK(X)							\
    do {                                        \
        memset(X, sizeof(X), 0);                \
        sprintf(X, "/tmp/feeder_multi.sock");  	\
    } while (0);

#define DFBCHECK(x...)                                         \
	{                                                          \
    DFBResult err = x;                                         \
                                                               \
    if (err != DFB_OK)                                         \
      {                                                        \
        fprintf( stderr, "%s <%d>:\n\t", __FILE__, __LINE__ ); \
        DirectFBErrorFatal( #x, err );                         \
      }                                                        \
  }


/********************/
/* Global variables */
/********************/
static DirectFBKeySymbolNames(keynames);

Input_Devices_t buttons[MAX_BUTTONS];
static char last_rc_identifier[50];
static bool keynames_sorted = false;

extern int fd_sock;	/* it is opened in rc.cpp */
Keysymbol_Devices_t last_symbol;

/********************************************/
/* For IR device (remote control with lirc) */
/********************************************/
static int keynames_compare (const void *key,
                             const void *base)
{
  return strcmp ((const char *) key,
                 ((const struct DFBKeySymbolName *) base)->name);
}

static int keynames_sort_compare (const void *d1,
                                  const void *d2)
{
  return strcmp (((const struct DFBKeySymbolName *) d1)->name,
                 ((const struct DFBKeySymbolName *) d2)->name);
}

/* This is the function to parse the data that we read from lircd socket 
	(it is the same of the DirectFB function) */
	
static int lirc_parse_line(const char *line, Keysymbol_Devices_t * symbol, char *keyname)
{
	struct DFBKeySymbolName *symbol_name;
	char code_str[32], name[50], rc_id[50];
	char repeat[2];
	int ret=0;
	char *str=0;

	if (!keynames_sorted)
	{
		qsort ( keynames,
		D_ARRAY_SIZE( keynames ),
				sizeof(keynames[0]),
				(__compar_fn_t) keynames_sort_compare );

		last_symbol.symbol = DIKS_NULL;
		memset(last_rc_identifier, 0, sizeof(last_rc_identifier));

		keynames_sorted = true;
	}

	memset( code_str, 0, sizeof(code_str));
	memset( name, 0, sizeof(name));
	memset( rc_id, 0, sizeof(rc_id));

	ret = sscanf( line, "%s %s %s %s", &code_str[0], &repeat[0], &name[0], &rc_id[0] );
#if 0
	printf( "ret: %d\n", ret );
	printf( "code_str: %s\n", code_str );
	printf( "repeat: %s\n", repeat );
	printf( "name: %s\n", name );
	printf( "rc_id: %s\n", rc_id );
#endif
	if (ret != 4 ) {
		memcpy( symbol->rc_identifier, last_rc_identifier, sizeof(last_rc_identifier) );
		return 0;
	}
	else {
		memcpy( last_rc_identifier, rc_id, sizeof(rc_id) );
		memcpy( symbol->rc_identifier, rc_id, sizeof(rc_id) );
		memcpy( keyname, name, sizeof(name) );

		symbol_name = (DFBKeySymbolName*)bsearch( name, keynames, D_ARRAY_SIZE( keynames ),	
												  sizeof(keynames[0]), (__compar_fn_t) keynames_compare );
		if (symbol_name) {
			symbol->symbol = symbol_name->symbol;
		}

		return 1;
	}
}

/**************************************************************************************************/
/*	The following code it is used if we want to read lircd and keyboard using 2 different threads */
/**************************************************************************************************/ 
#if 0
/********************************/
/*		Thread for lircd		*/
/********************************/
void * lircd_device(void *args)
{
	int                 fd;
	struct sockaddr_un  sa;
	fd_set                  set;
	int                     result;
	int                     readlen;
	char                    buf[128];
	

	DFBInputEvent      inputevent;
	DFBInputDeviceKeySymbol symbol;
	struct timeval tv;


	/* create socket */
	sa.sun_family = AF_UNIX;
	direct_snputs( sa.sun_path, "/dev/lircd", sizeof(sa.sun_path) );

	fd = socket( PF_UNIX, SOCK_STREAM, 0 );
	if (fd < 0)
	{
		pthread_exit(NULL);
	}

	/* initiate connection */
	if (connect( fd, (struct sockaddr*)&sa, sizeof(sa) ) < 0)
	{
		close( fd );
		pthread_exit(NULL);
	}

	while(1)
	{
		FD_ZERO(&set);
		FD_SET(fd,&set);

		tv.tv_sec  = 0;
		tv.tv_usec = 150000;
	
		result = select (fd+1, &set, NULL, NULL, &tv);
		if(result>0)
		{
			/* read data */
			readlen = read( fd, buf, 128 );
			//if (readlen < 1)
			if (readlen <= 0)
				continue;

			/* get new key */
			symbol = lirc_parse_line( buf );
			if (symbol == DIKS_NULL)
			{
				printf("NOOOOOOOOOOOOOOOOOOOO!!! >___<\n");
				continue;
			}
			printf("\n---------->symbol: 0x%08X \n",symbol);

			pthread_mutex_lock(&mutex);
			key.evt.type=DIET_KEYPRESS;
			key.evt.key_symbol=symbol;
			key.evt.device_id=DIDID_REMOTE;
			key.is_old=0;
			pthread_mutex_unlock(&mutex);

		}
		/*else
		{
			printf() // no data -> TODO: Release key
		}*/
	}
	close(fd);
}

/********************************/
/*		Thread for Keyboard		*/
/********************************/
void * keyboard_device(void *args)
{
	DFBSurfaceDescription   dsc;
	DFBInputDeviceKeySymbol symbol;
	struct timeval tv;
	DFBInputEvent event;
	static IDirectFBEventBuffer *buffer = NULL;
	int                     argc = 1;	
	char                    **argv = NULL;

	DFBCHECK (DirectFBInit (&argc, &argv));
	DFBCHECK (DirectFBCreate (&dfb));

	DFBCHECK (dfb->GetInputDevice (dfb, DIDID_KEYBOARD, &keyboard));
	DFBCHECK (keyboard->CreateEventBuffer (keyboard, &buffer));

	while(1)
	{
		usleep(50000);	//50ms
		while (buffer->GetEvent (buffer, DFB_EVENT(&event)) == DFB_OK)
		{
			if (event.type == DIET_KEYPRESS)
			{
				printf("\nDIET_KEYPRESS...");
				switch (event.key_symbol)
				{
					case DIKS_CURSOR_RIGHT:
						printf("--->CURSOR_RIGHT\n");		
						break;
					case DIKS_CURSOR_LEFT:
						printf("--->CURSOR_LEFT\n");
						break;
					case DIKS_CURSOR_UP:
						printf("--->CURSOR_UP\n");
						break;
					case DIKS_CURSOR_DOWN:
						printf("--->CURSOR_DOWN\n");
						break;
				}
			}
			if (event.type == DIET_KEYRELEASE)
			{
				printf("\nDIET_KEYRELEASE...");
				switch (event.key_symbol)
				{
					case DIKS_CURSOR_RIGHT:
						printf("--->CURSOR_RIGHT\n");		
						break;
					case DIKS_CURSOR_LEFT:
						printf("--->CURSOR_LEFT\n");
						break;
					case DIKS_CURSOR_UP:
						printf("--->CURSOR_UP\n");
						break;
					case DIKS_CURSOR_DOWN:
						printf("--->CURSOR_DOWN\n");
						break;
				}		
			}
		}
	}
}

/********************************************/
/*	Init of threads (lircd and keyboard)	*/
/********************************************/
int init_input_device(void)
{
    pthread_attr_t      t_attr;

	if (pthread_attr_init(&t_attr) != 0) {
		eDebug("pthread_attr_init(): Could not init thread attributes");
		return -1;
	}

	if (pthread_attr_setdetachstate(&t_attr, PTHREAD_CREATE_DETACHED) != 0) {
		eDebug("pthread_attr_setdetachstate(): Could not set thread attributes");
		return -1;
	}

	gettimeofday(&(key.tv),NULL);
	key.evt.key_symbol=DIKS_NULL;
	key.is_old=0;

	if (pthread_create(&th_id[LIRCD_INPUT], &t_attr, lircd_device, (void*)NULL) != 0) {
		eDebug("pthread_create(): Could not create thread");
		return -1;
	}

	if (pthread_create(&th_id[LIRCD_INPUT], &t_attr, keyboard_device, (void*)NULL) != 0) {
		eDebug("pthread_create(): Could not create thread");
		return -1;
	}
}
#endif

/*******************************************************************************************/
/* This function read data from lircd socket. Then there is a parse with 'lirc_parse_line' */
/*******************************************************************************************/

#define LIRC_BUFFER_LENGTH    128

static char lircbuffer[LIRC_BUFFER_LENGTH];
int read_idx=0;

int read_lircd(void)
{
	int                     result=0, remaing_data=0, readlen=0, startpos=0;
	unsigned char 			rp=0;
	char 					keyname[50], tmpbuff[LIRC_BUFFER_LENGTH], 
							line[LIRC_BUFFER_LENGTH], 
							*endofline=0, *posbuff=0;

 	Keysymbol_Devices_t symbol;
	struct timeval tv;

	if ( read_idx == 0 )
		memset( lircbuffer, 0, sizeof(lircbuffer) );

	readlen = read( fd_sock, lircbuffer+read_idx, sizeof(lircbuffer)-read_idx-1 );
	if (readlen <= 0)
		return -1;

	read_idx = 0;

	do
	{
		posbuff = lircbuffer+read_idx;

		/* Find find occurence of /n character.
		If there isn't but i read almost one byte, i store this to start of buffer */
		endofline = strchr(posbuff, '\n');
	
		if (!endofline)  {
			remaing_data = strlen(posbuff);

			memset( tmpbuff, 0, sizeof( tmpbuff ));
			memcpy( tmpbuff, posbuff, remaing_data);
			memset( lircbuffer, 0, sizeof( lircbuffer ));
			memcpy( lircbuffer, tmpbuff, remaing_data );
			read_idx = remaing_data;

			return rp;
		}
		else { 

		/* If I have a end of line ... */
			memset( line, 0, sizeof(line) );
			memcpy( line, posbuff, strlen(posbuff) - strlen(endofline) );

#if 0
			printf("[LIRC Socket] rp:%d line: %s\n", rp, line );
#endif

			memset( buttons[rp].rc_identifier, 0, sizeof(buttons[rp].rc_identifier));
			memset( keyname, 0, sizeof(keyname) ); 
			
			result = lirc_parse_line( line, &symbol, keyname );
			if ( !result )
			{
				buttons[rp].evt.type=DIET_KEYRELEASE;
				buttons[rp].evt.device_id=DIDID_REMOTE;
				buttons[rp].evt.key_symbol=last_symbol.symbol;
				memcpy(buttons[rp].rc_identifier, last_symbol.rc_identifier, sizeof(last_symbol.rc_identifier));
			}
			else
			{
				if ( strstr(keyname,"_RELEASE") ) {	/* only '_RELEASE' beacuase is more probably that */
													/* this release is for last press button */
					buttons[rp].evt.type=DIET_KEYRELEASE;
					buttons[rp].evt.device_id=DIDID_REMOTE;
					buttons[rp].evt.key_symbol=last_symbol.symbol;
					memcpy(buttons[rp].rc_identifier, last_symbol.rc_identifier, sizeof(last_symbol.rc_identifier));
	
				}
				else {
					memcpy((void *)&last_symbol, (void *)&symbol, sizeof(Keysymbol_Devices_t));	/* save the last pressed button */
	
					/* Remove last release if there is a press */
					/* If you use it, in 'channels list' 
					doesn't work the letters when press number buttons */
		//			if( (rp>0) && (buttons[rp-1].evt.type==DIET_KEYRELEASE)	)
		//				rp--;


					buttons[rp].evt.type=DIET_KEYPRESS;
					buttons[rp].evt.device_id=DIDID_REMOTE;
					buttons[rp].evt.key_symbol=symbol.symbol;
					memcpy(buttons[rp].rc_identifier, symbol.rc_identifier, sizeof(symbol.rc_identifier));
				}
			}

			read_idx += (strlen(line)+1);
			rp++;

		}
	} while (1);
}

