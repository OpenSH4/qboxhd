#include <lib/base/ebase.h>
#include <lib/base/eerror.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <sys/stat.h>
#include <fcntl.h>

#include <lib/dvb/feeder.h>

bool stop_feeder;
bool radio_service;

static void print_feeder_cmd(FeederCmd cmd)
{
  const char *cmd_str[]={
    "FEED_BRIDGE_ON",
    "FEED_BRIDGE_OFF",
    "FEED_REC_ON",
    "FEED_REC_OFF",
    "FEED_BRIDGE_PAUSE",
    "OP_END"
    };

  eDebug("%s:\n\tcmd->op      =%s\n\tcmd->src_name='%s'\n\tcmd->dst_name='%s'",
         __FUNCTION__, cmd_str[cmd->op],cmd->src_name,cmd->dst_name);
}

void feeder_cmd(FeederCmd cmd)
{
    int                 sock;
    char                sock_name[FEEDER_NAME];
    socklen_t           addr_len;
    struct sockaddr_un  core_addr;

    FEEDER_SOCK(sock_name);

    if ((sock = socket(PF_LOCAL, SOCK_STREAM, 0)) < 0) {
        eWarning("socket(): %m");
        return;
    }

    core_addr.sun_family = PF_LOCAL;
    strncpy(core_addr.sun_path, sock_name, sizeof(core_addr.sun_path));
    addr_len = sizeof(core_addr.sun_family) + strlen(core_addr.sun_path) + 1;
    if (connect(sock, (struct sockaddr *)&core_addr, addr_len) == -1) {
        eWarning("connect(): %m");
        close(sock);
        return;
    }
//     print_feeder_cmd(cmd);
    if (write(sock, cmd, sizeof(struct feeder_cmd_st)) < 0)
        eWarning("write(): %m");

    close(sock);
}

void pause_feeder_cmd(FeederCmd cmd)
{
   int                 sock,ret;
    char                sock_name[FEEDER_NAME];
	int					cnt=0;
	unsigned char		c=0;
	fd_set              read_fd;
    socklen_t           addr_len;
    struct sockaddr_un  core_addr;
	struct timeval      timeout;

    FEEDER_SOCK(sock_name);

    if ((sock = socket(PF_LOCAL, SOCK_STREAM, 0)) < 0) {
        eWarning("socket(): %m");
        return;
    }

    core_addr.sun_family = PF_LOCAL;
    strncpy(core_addr.sun_path, sock_name, sizeof(core_addr.sun_path));
    addr_len = sizeof(core_addr.sun_family) + strlen(core_addr.sun_path) + 1;
    if (connect(sock, (struct sockaddr *)&core_addr, addr_len) == -1) {
        eWarning("connect(): %m");
        close(sock);
        return;
    }

//     print_feeder_cmd(cmd);
    if (write(sock, cmd, sizeof(struct feeder_cmd_st)) < 0)
        eWarning("write(): %m");

	while(cnt<100)
	{
		FD_ZERO(&read_fd);
        FD_SET(sock, &read_fd);

        timeout.tv_sec = 0;
        timeout.tv_usec = 10000; //10ms

        /* Block until input arrives on active socket */
        if (select(sock+1, &read_fd, NULL, NULL, &timeout) < 0) {
            eDebug("select()");
            continue;
        }
		cnt++;		

        if (FD_ISSET(sock, &read_fd))
		{
			c = 0;
			ret = read(sock, &c, sizeof(c));
			if (ret < 0) {
		    	eDebug("%s(): read(): %m\n", __FUNCTION__);
				break;
			}
			if ((ret>0) && (c=='A'))
			{
// 				eDebug("ACK received ^_^");
				cnt = 100;
				break;
			}
		}
	}

    close(sock);
}



#define TUNER_NAME           64 

#define TUNER_SOCK(X)							\
    do {                                        \
        memset(X, 0, sizeof(X));                \
        sprintf(X, "/tmp/frontend.socket");  	\
    } while (0);

void comunicate_generic_info(const char * buf, unsigned char len)
{ 
    int                 sock; 
    char                sock_name[TUNER_NAME]; 
    socklen_t           addr_len; 
    struct sockaddr_un  core_addr; 
        
    TUNER_SOCK(sock_name); 
    
    if ((sock = socket(PF_LOCAL, SOCK_STREAM, 0)) < 0) { 
        eWarning("Frontend socket() error"); 
        return; 
    } 
    
    core_addr.sun_family = PF_LOCAL; 
    strncpy(core_addr.sun_path, sock_name, sizeof(core_addr.sun_path)); 
    
    addr_len = sizeof(core_addr.sun_family) + strlen(core_addr.sun_path) + 1; 
    
    if (connect(sock, (struct sockaddr *)&core_addr, addr_len) < 0 ) { 
        eWarning("Frontend socket connect() error"); 
        close(sock); 
        return; 
    } 
	
    if ( send(sock, (void*)buf, len, 0) < 0 )
        eWarning("frontend socket write(): error"); 
    
    close(sock); 
} 
   


