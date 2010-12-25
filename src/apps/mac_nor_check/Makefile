COMPILER = sh4-linux
CC = $(COMPILER)-gcc
STRIP = $(COMPILER)-strip
CFLAGS = -O2 -Wall

SRC = mac_nor_check.c
OBJS = $(patsubst %.c, %.o, $(SRC))
TARGETS = $(patsubst %.c, %, $(SRC))

all: $(SRC) $(OBJS)
	$(STRIP) $(TARGETS)
    
.c.o:
	$(CC) $(CFLAGS) $(INC) $< -o $(patsubst %.o, %, $@)


clean:
	@rm -rf $(TARGETS) *.o

