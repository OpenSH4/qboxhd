

CC = sh4-linux-gcc
CFLAGS = -O2 -Wall
#CFLAGS = -O2 -Wall -DDEBUG

SRC = hotplug_manager.c
OBJS = $(patsubst %.c, %.o, $(SRC))
TARGETS = $(patsubst %.c, %, $(SRC))

all: $(SRC) $(OBJS)
	
.c.o:
	$(CC) $(CFLAGS) $(INC) $< -o $(patsubst %.o, %, $@)


clean:
	@rm -f $(TARGETS) *.o
