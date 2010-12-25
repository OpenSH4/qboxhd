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

#include <common.h>
#include <command.h>
#include <devices.h>
#include <net.h>

//#ifdef CONFIG_BROADCAST_NETWORK_BOOT

//#include "common/front.h"
#include "broadcastBoot.h"


DECLARE_GLOBAL_DATA_PTR;


//#define DEBUG_BROADCAST_NETWORK_BOOT


#define TIMEOUT		5		/* Seconds before trying BOOTP again */
#ifndef	CONFIG_NET_RETRY_COUNT
# define TIMEOUT_COUNT	5		/* # of timeouts before giving up  */
#else
# define TIMEOUT_COUNT  (CONFIG_NET_RETRY_COUNT)
#endif



static int wait_time;

static uchar broadcast_network_message_received = 0;

static void AckSend(void);

static rxhand_f *pH;


extern void eth_halt_special (void);

extern unsigned char IsLinked;


static void 
hex_dump( uchar *buff, unsigned buff_len ) 
{
    int i;

    for ( i=0; i<buff_len; i++ ) {
	printf( "0x%02x ", buff[i] );
    }
	
     printf( "\n" );
}


/*
 *	Parse QBOX_START_NETCONSOLE packet.
 */
static int 
parse_brodcast_QBOX_START_NETCONSOLE_packet(uchar * pkt, unsigned len, broadcast_start_netconsole_arg_t *broadcast_start_netconsole_msg )
{

   uchar * pkt_str = pkt;
   uchar * sep = NULL;
   uchar * next_sep = NULL;
   int value_len = 0;
   uchar * pvalue = NULL;
   uchar serverip_present=0, ncip_present=0, netmask_present=0, ipaddr_present=0;

    memset( broadcast_start_netconsole_msg->serverip, 0, sizeof( broadcast_start_netconsole_msg->serverip ) );
    memset( broadcast_start_netconsole_msg->netmask, 0, sizeof( broadcast_start_netconsole_msg->netmask ) );
    memset( broadcast_start_netconsole_msg->ncip, 0, sizeof( broadcast_start_netconsole_msg->ncip ) );
    memset( broadcast_start_netconsole_msg->ipaddr, 0, sizeof( broadcast_start_netconsole_msg->ipaddr ) );
	
#ifdef DEBUG_BROADCAST_NETWORK_BOOT
	printf("\nData Received [len: %d]: %s\n", len, pkt);
#endif

   // find first separator of string ...
   while ( (sep = strpbrk( pkt_str, BROADCASTBOOT_ARGS_SEPARATOR)) ) {

       // check if over limit ...
	if ( ( sep + BROADCASTBOOT_ARGS_SEPARATOR_LEN ) > (pkt + len) ) {
		break;
	}

       // check if next separator exist ...
       next_sep = strpbrk( sep + BROADCASTBOOT_ARGS_SEPARATOR_LEN, BROADCASTBOOT_ARGS_SEPARATOR);

	if ( !next_sep ) break;

	if ( strncmp( sep + BROADCASTBOOT_ARGS_SEPARATOR_LEN, "serverip=", 9 ) == 0 ) 
	{
	    value_len = next_sep - ( sep + BROADCASTBOOT_ARGS_SEPARATOR_LEN + 9 );
	    pvalue = sep + BROADCASTBOOT_ARGS_SEPARATOR_LEN + 9;
           memcpy( broadcast_start_netconsole_msg->serverip, pvalue, value_len);

#ifdef DEBUG_BROADCAST_NETWORK_BOOT
	    printf("serverip found: %s\n", broadcast_start_netconsole_msg->serverip );
#endif
	    serverip_present = 1;
	}

	if ( strncmp( sep + BROADCASTBOOT_ARGS_SEPARATOR_LEN, "netmask=", 8 ) == 0 ) 
	{
	    value_len = next_sep - ( sep + BROADCASTBOOT_ARGS_SEPARATOR_LEN + 8 );
	    pvalue = sep + BROADCASTBOOT_ARGS_SEPARATOR_LEN + 8;
           memcpy( broadcast_start_netconsole_msg->netmask, pvalue, value_len);

#ifdef DEBUG_BROADCAST_NETWORK_BOOT
	    printf("netmask found: %s\n", broadcast_start_netconsole_msg->netmask );
#endif
	    netmask_present = 1;
	}

	if ( strncmp( sep + BROADCASTBOOT_ARGS_SEPARATOR_LEN, "ncip=", 5 ) == 0 ) 
	{
	    value_len = next_sep - ( sep + BROADCASTBOOT_ARGS_SEPARATOR_LEN + 5 );
	    pvalue = sep + BROADCASTBOOT_ARGS_SEPARATOR_LEN + 5;
           memcpy( broadcast_start_netconsole_msg->ncip, pvalue, value_len);

#ifdef DEBUG_BROADCAST_NETWORK_BOOT
	    printf("ncip found: %s\n", broadcast_start_netconsole_msg->ncip );
#endif
	    ncip_present = 1;
	}

	if ( strncmp( sep + BROADCASTBOOT_ARGS_SEPARATOR_LEN, "ipaddr=", 7 ) == 0 ) 
	{
	    value_len = next_sep - ( sep + BROADCASTBOOT_ARGS_SEPARATOR_LEN + 7 );
	    pvalue = sep + BROADCASTBOOT_ARGS_SEPARATOR_LEN + 7;
           memcpy( broadcast_start_netconsole_msg->ipaddr, pvalue, value_len);

#ifdef DEBUG_BROADCAST_NETWORK_BOOT
	    printf("ipaddr found: %s\n", broadcast_start_netconsole_msg->ipaddr );
#endif
	    ipaddr_present = 1;
	}

	pkt_str = next_sep;

   }

   if ( !serverip_present || !ncip_present || !netmask_present || !ipaddr_present )
	return 0;

   return 1;
}


/*
 *	Handle a BroadcastNetworkBootHandler received packet.
 */


static void
BroadcastNetworkBootHandler (uchar * pkt, unsigned dest, unsigned src, unsigned len)
{


	if ( len > 200 ) return;

	pkt[len] = '\0';

	if (strncmp(QBOX_START_NETCONSOLE_COMMAND, pkt, strlen(QBOX_START_NETCONSOLE_COMMAND)) == 0)
	{

#ifdef CONFIG_NETCONSOLE
		
          broadcast_start_netconsole_arg_t broadcast_start_netconsole_msg;

	   if ( !parse_brodcast_QBOX_START_NETCONSOLE_packet( pkt, len, &broadcast_start_netconsole_msg ) ) {
		return;
	   }


	   printf("\n\n**********************************************************\n*\n");
	   printf("* [ QBOX NETCONSOLE ACTIVATED ]\n*\n");

	   printf("*  NCIP   IP: %s\n", broadcast_start_netconsole_msg.ncip);
	   printf("*  QBOX   IP: %s\n", broadcast_start_netconsole_msg.ipaddr);
	   printf("*  SERVER IP: %s\n", broadcast_start_netconsole_msg.serverip);

	   printf("* \n**********************************************************\n\n");

	   setenv("ipaddr", broadcast_start_netconsole_msg.ipaddr);
	   setenv("serverip", broadcast_start_netconsole_msg.serverip);
	   setenv("netmask", broadcast_start_netconsole_msg.netmask);

		NetServerIP = getenv_IPaddr ("serverip");
		NetCopyIP(&NetOurIP, &gd->bd->bi_ip_addr);
		NetOurGatewayIP = getenv_IPaddr ("gatewayip");
		NetOurSubnetMask= getenv_IPaddr ("netmask");
	
//	   if ( strlen( broadcast_start_netconsole_msg.gateway ) > 0 )
//	       setenv("gateway", broadcast_start_netconsole_msg.gateway);
	   
	   setenv("ncip", broadcast_start_netconsole_msg.ncip);


		setenv("stdout","nc");
		setenv("stdin","nc");

	NetState = NETLOOP_SUCCESS;
#endif
	}
}

/*
 *	Timeout on BroadcastNetworkBoot request.
 */
static void
BroadcastNetworkBootTimeout(void)
{
	puts ("... None\n");
	eth_halt();
	NetState = NETLOOP_FAIL;
}

int 
BroadcastStart( void)
{
	printf("Listening for broadcast message...\n");
	NetSetTimeout(TIMEOUT * CFG_HZ, BroadcastNetworkBootTimeout);
	NetSetHandler(BroadcastNetworkBootHandler);
		
	memset(NetServerEther, 0, 6);

	return 0;
}

void broad_receive(void)
{
//	do_broadcastd(0,0,0,0);
	BroadcastStart();
}



// ****************************************************************************************
#if 0
U_BOOT_CMD(	broadcastd, 1/*14*/, 1,//0, 
  	       do_broadcastd,
		"broadcastd - active broadcast command \n",  // command usage
		"usage: broadcastd \n" // command help
);
#endif
//#endif /* CONFIG_BROADCAST_NETWORK_BOOT */
