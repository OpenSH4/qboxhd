COMPILER = sh4-linux
CC = $(COMPILER)-gcc
STRIP = $(COMPILER)-strip
CFLAGS = -O2 -Wall

INC = -I/opt/STM/STLinux-2.3/devkit/sh4/target/usr/include -I../duoutils

SRC = display_image.c
OBJS = $(patsubst %.c, %.o, $(SRC))
TARGETS = $(patsubst %.c, %, $(SRC))

all: $(SRC) $(OBJS)
	$(STRIP) $(TARGETS)
#	@cp -v $(TARGETS) /mnt/rootfs_nfs/usr/local/bin
    
.c.o:
	$(CC) $(CFLAGS) $(INC) $< -o $(patsubst %.o, %, $@) -lduoutils


clean:
	@rm -rf $(TARGETS) *.o

