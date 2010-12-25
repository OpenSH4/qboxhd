
#ifndef BROADCASTBOOT_H_
#define BROADCASTBOOT_H_

#define BROADCASTBOOT_ARGS_SEPARATOR 		"*|*"
#define BROADCASTBOOT_ARGS_SEPARATOR_LEN 		strlen(BROADCASTBOOT_ARGS_SEPARATOR)


/* broadcast commands */
#define QBOX_START_NETCONSOLE_COMMAND           "QBOXHD_START_NETCONSOLE"


#ifdef CONFIG_NETCONSOLE

struct broadcast_start_netconsole_arg {
    char serverip[25];
    char netmask[25];
    char ncip[25];
    char ipaddr[25];
};


int 
BroadcastStart(void);

typedef struct broadcast_start_netconsole_arg broadcast_start_netconsole_arg_t;

#endif /* CONFIG_NETCONSOLE */

#endif /* BROADCASTBOOT_H_ */
