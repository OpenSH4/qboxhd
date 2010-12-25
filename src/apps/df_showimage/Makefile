CC = sh4-linux-gcc
CFLAGS = -g -O2 -Wall -ldirectfb

CFLAGS += -I/opt/STM/STLinux-2.3/devkit/sh4/target/usr/include/directfb
LDFLAGS += -L/opt/STM/STLinux-2.3/devkit/sh4/target/usr/lib

SRC = df_showimage.c
OBJS = $(patsubst %.c, %.o, $(SRC))
TARGETS = $(patsubst %.c, %, $(SRC))

all: $(SRC) $(OBJS)
	
.c.o:
	$(CC) $(CFLAGS) $< -o $(patsubst %.o, %, $@) -ldirectfb


clean:
	@rm -f $(TARGETS) *.o
