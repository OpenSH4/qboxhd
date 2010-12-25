#ifndef __feeder_h
#define __feeder_h

#define FEEDER_NAME           256

#define FEEDER_SOCK(X)                           \
    do {                                        \
        memset(X, sizeof(X), 0);                \
        sprintf(X, "/tmp/feeder_multi.sock");   \
    } while (0);

typedef enum {
    FEED_BRIDGE_ON=0,
    FEED_BRIDGE_OFF,
    FEED_REC_ON,
    FEED_REC_OFF,
	FEED_BRIDGE_PAUSE,
    OP_END
} feeder_op_t;

typedef struct feeder_cmd_st * FeederCmd;

struct feeder_cmd_st {
    feeder_op_t op;
    char        src_name[FEEDER_NAME];
    char        dst_name[FEEDER_NAME];
};

void feeder_cmd(FeederCmd);
void pause_feeder_cmd(FeederCmd cmd);
void comunicate_generic_info(const char * buf, unsigned char len);

#endif
