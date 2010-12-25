COMPILER = sh4-linux
CC = $(COMPILER)-gcc
STRIP = $(COMPILER)-strip
CFLAGS = -O2 -Wall

INC = -I/opt/STM/STLinux-2.3/devkit/sh4/target/usr/include -I../duoutils

SRC = dvo_control.c
OBJS = $(patsubst %.c, %.o, $(SRC))
TARGETS = $(patsubst %.c, %, $(SRC))

all: $(SRC) $(OBJS)
	$(STRIP) $(TARGETS)
	@cp -v $(TARGETS) /opt/STM/STLinux-2.3/devkit/sh4/target/usr/local/bin
    
.c.o:
	$(CC) $(CFLAGS) $(INC) $< -o $(patsubst %.o, %, $@) -lduoutils

clean:
	@rm -rf $(TARGETS) *.o


#INC    = -I../duoutils
#LIB    = #-L/work/qboxHD/utils_lib  -lduoutils
#CFLAGS = -Wall -O2 -g0 -std=c99 $(INC) $(LIB)
#LDFLAGS = -lduoutils
#CROSS_COMPILE  := sh4-linux-
#AS		= $(CROSS_COMPILE)as
#LD		= $(CROSS_COMPILE)ld
#CC		= $(CROSS_COMPILE)gcc
#CPP		= $(CROSS_COMPILE)gcc
#STRIP	= $(CROSS_COMPILE)strip
#TARGET  = dvo_control
#SRCS    = $(addprefix ./,$(TARGET).c)
#OBJS    = $(patsubst %.c,%.o,$(SRCS))

#INCLUDES = 


#all: $(TARGET)
#	$(STRIP) $(TARGET)
#	cp $(TARGET) /opt/STM/STLinux-2.3/devkit/sh4/target/usr/local/bin

#$(TARGET): $(OBJS)

#dep:

#clean :
#	find . -name "*.o" -exec rm \{\} \;
#	find . -name $(TARGET) -exec rm \{\} \;

#$(OBJS): $(INCLUDES)
