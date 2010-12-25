all: streamproxy

CROSS_COMPILE=/opt/STM/STLinux-2.3/devkit/sh4/bin/sh4-linux-

streamproxy: streamproxy.c 
	${CROSS_COMPILE}gcc -Wall streamproxy.c -o streamproxy
	${CROSS_COMPILE}strip streamproxy	

clean:
	rm streamproxy



