#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>

#define MAP_SIZE 	8192UL //4096UL  
#define MAP_MASK (MAP_SIZE - 1)

#define CPG_WTCSR	0x1FC0000C	// watchdog of cpu st40
#define	PW_KEY		0xA500		// the value that allow to write in WTCSR
#define	RESET		0xFF	

int main(int argc, char **argv)
{
	int fd;
	void *map_base, *virt_addr;
	off_t target;
	unsigned long addr=0;
	unsigned int value=0;
	volatile unsigned short * reg;
	int delay=0;
	
	if(argc>2)
	{
		printf("Usage: reset_st40 [delay in ms in decimal format]\n");
		printf("Example: 'reset_st40 2000' -> reset the chip after 2 seconds\n");
		return (-1);
	}

	if(argc==2)
	{
		sscanf(argv[1], "%d", &delay);
		if(delay<0)
		{
			printf("Only delay values > 0 !!\n");		
			return (-1);
		}
	}

	target = CPG_WTCSR;		

	if ( ( fd = open( "/dev/mem", O_RDWR | O_SYNC ) ) == -1 ) {
		fprintf(stderr, "open(): %m\n");
		return (-1);
	}
	// Map one page //
	map_base = mmap( 0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, target & ~MAP_MASK );
	if ( map_base == ( void * ) -1 ) {
		fprintf(stderr, "mmap(): %m\n");
		return (-1);

	}
	virt_addr = map_base + ( target & MAP_MASK );
	reg = (volatile unsigned short*)virt_addr;

	value = (PW_KEY | RESET);

	if (delay > 0)
		usleep(delay * 1000);

//	printf("Reset the chip with value 0x%04X!\n",value);
	reg[0] = (unsigned short)value;

	munmap(map_base, MAP_SIZE);
	close(fd);

	return 0;
}

