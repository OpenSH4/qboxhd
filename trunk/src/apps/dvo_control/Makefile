COMPILER = sh4-linux
CC = $(COMPILER)-gcc
STRIP = $(COMPILER)-strip
CFLAGS = -O2 -Wall
LDFLAGS = -L../duoutils

INC = -I/opt/STM/STLinux-2.3/devkit/sh4/target/usr/include -I../duoutils

SRC = dvo_control.c
OBJS = $(patsubst %.c, %.o, $(SRC))
TARGETS = $(patsubst %.c, %, $(SRC))

all: $(SRC) $(OBJS)
	$(STRIP) $(TARGETS)
    
.c.o:
	$(CC) $(CFLAGS) $(INC) $< -o $(patsubst %.o, %, $@) $(LDFLAGS) -lduoutils

clean:
	@rm -rf $(TARGETS) *.o


