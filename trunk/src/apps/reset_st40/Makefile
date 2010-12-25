
.PHONY:clean

all: reset_st40

reset_st40: reset_st40.c
	sh4-linux-gcc $(CFLAGS) -O3 -g0 reset_st40.c -o reset_st40

clean:
	rm -f *.o *~ reset_st40
