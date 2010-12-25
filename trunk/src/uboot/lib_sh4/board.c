/*
 * (C) Copyright 2004 STMicroelectronics.
 *
 * Andy Sturges <andy.sturges@st.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <version.h>
#include <devices.h>
#include <version.h>
#include <net.h>
#include <environment.h>

/* Duolabs ... */
#include "../drivers/netstmac_eth.h"



#define	MAC_ADDR_IN_FLASH	0xF000
#define	MAC_SIZE	18

#define	END_ADDR			0xFFFF
#define	LAST_BYTEs_TO_CHECK	16

/* For FPGA program and init */
#include "fpga_module.h"

#ifdef CONFIG_QBOXHD_mini
#include "sim_i2c_pin.h"
#endif



unsigned char mac_in_flash[MAC_SIZE];
extern flash_info_t flash_info[];	/* info for FLASH chips */
extern unsigned char flash_read_char_duo(flash_info_t * info, ulong addr);

extern int prova_stmac_mii_read (int phy_addr, int reg);
unsigned char IsLinked;
/* For lcd */
extern int device_init(void);
extern void demo_print(void);
/* ....... */

/* From lcd_drv.c to paint display */
extern unsigned int last_position; // reference to hight of font


extern ulong _uboot_end_data;
extern ulong _uboot_end;

ulong monitor_flash_len;

#ifndef CONFIG_IDENT_STRING
#define CONFIG_IDENT_STRING ""
#endif

const char version_string[] =
	U_BOOT_VERSION" (" __DATE__ " - " __TIME__ ") - " CONFIG_IDENT_STRING ;

/*
 * Begin and End of memory area for malloc(), and current "brk"
 */

#define	TOTAL_MALLOC_LEN	CFG_MALLOC_LEN

static ulong mem_malloc_start;
static ulong mem_malloc_end;
static ulong mem_malloc_brk;

extern int soc_init (void); 	/* Detect/set SOC settings  */
extern int board_init (void);   /* Set up board             */
extern int timer_init (void);
extern int checkboard (void);   /* Give info about board    */

static void mem_malloc_init (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	ulong dest_addr = TEXT_BASE + gd->reloc_off;

	mem_malloc_end = dest_addr;
	mem_malloc_start = dest_addr - TOTAL_MALLOC_LEN;
	mem_malloc_brk = mem_malloc_start;

	memset ((void *) mem_malloc_start,
		0, mem_malloc_end - mem_malloc_start);
}

void *sbrk (ptrdiff_t increment)
{
	ulong old = mem_malloc_brk;
	ulong new = old + increment;

	if ((new < mem_malloc_start) || (new > mem_malloc_end)) {
		return (NULL);
	}
	mem_malloc_brk = new;
	return ((void *) old);
}

static int init_func_ram (void)
{
	DECLARE_GLOBAL_DATA_PTR;

#ifdef	CONFIG_BOARD_TYPES
	int board_type = gd->board_type;
#endif

	gd->ram_size = CFG_SDRAM_SIZE;
	puts ("DRAM:  ");
	print_size (gd->ram_size, "\n");

	return (0);
}

static int display_banner (void)
{
//	printf ("\n\n%s\n\n", version_string);
	printf ("\n%s\n", version_string);
	return (0);
}

static void display_flash_config (ulong size)
{
	puts ("Flash: ");
	print_size (size, "\n");
}


static int init_baudrate (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	uchar tmp[64];		/* long enough for environment variables */
	int i = getenv_r ("baudrate", tmp, sizeof (tmp));

	gd->baudrate = (i > 0)
		? (int) simple_strtoul (tmp, NULL, 10)
		: CONFIG_BAUDRATE;

	return (0);
}

void flashWriteEnable(void);
    
/*
 * All attempts to come up with a "common" initialization sequence
 * that works for all boards and architectures failed: some of the
 * requirements are just _too_ different. To get rid of the resulting
 * mess of board dependend #ifdef'ed code we now make the whole
 * initialization sequence configurable to the user.
 *
 * The requirements for any new initalization function is simple: it
 * receives a pointer to the "global data" structure as it's only
 * argument, and returns an integer return code, where 0 means
 * "continue" and != 0 means "fatal error, hang the system".
 */
typedef int (init_fnc_t) (void);
#if 0
init_fnc_t *init_sequence[] = {
	soc_init,
	timer_init,
	board_init,
	env_init,		/* initialize environment */
	init_baudrate,		/* initialze baudrate settings */
	serial_init,		/* serial communications setup */
	console_init_f,		/* Initial console             */
	checkboard,
	display_banner,		/* say that we are here */
	init_func_ram,
	NULL,
};
#endif
init_fnc_t *init_sequence[] = {
	soc_init,
	timer_init,
	board_init,
	env_init,		/* initialize environment */
	init_baudrate,		/* initialze baudrate settings */
	serial_init,		/* serial communications setup */
	console_init_f,		/* Initial console             */
	display_banner,		/* say that we are here */
	checkboard,
	init_func_ram,
	NULL,
};

//give the mac into array 'mac_in_flash'
static unsigned char read_mac_in_flash(unsigned char print)
{
	flash_info_t *info;

	int cnt,cnt1=0;
	for(cnt=0;cnt<MAC_SIZE;cnt++)
		mac_in_flash[cnt]=0;
	info = &flash_info[0];
	for(cnt=0;cnt<MAC_SIZE;cnt++)
		mac_in_flash[cnt]=flash_read_char_duo(info,/*0x21000*/MAC_ADDR_IN_FLASH+cnt);//0xFFFA

	/* Check if alphanumeric char */
	for(cnt=0;cnt<(MAC_SIZE-1);cnt++)	//-1 for '\0'
	{
		if(strchr("0123456789ABCDEF:",mac_in_flash[cnt])==NULL)
		{
			cnt=1;	/* For more security */
			break;
		}
		if(mac_in_flash[cnt]==0)
		{
			cnt=1;	/* For more security */
			break;
		}
	}
	/* Check if there is the right ':' */

/*
	if(	(mac_in_flash[2]==':') &&
		(mac_in_flash[5]==':') &&
		(mac_in_flash[8]==':') &&
		(mac_in_flash[11]==':') &&
		(mac_in_flash[14]==':') )
			cnt1=1;
*/
	if(	(mac_in_flash[0]!=':') &&
		(mac_in_flash[1]!=':') &&
		(mac_in_flash[2]==':') &&
		(mac_in_flash[3]!=':') &&
		(mac_in_flash[4]!=':') &&
		(mac_in_flash[5]==':') &&
		(mac_in_flash[6]!=':') &&
		(mac_in_flash[7]!=':') &&
		(mac_in_flash[8]==':') &&
		(mac_in_flash[9]!=':') &&
		(mac_in_flash[10]!=':') &&
		(mac_in_flash[11]==':') &&
		(mac_in_flash[12]!=':') &&
		(mac_in_flash[13]!=':') &&
		(mac_in_flash[14]==':') &&
		(mac_in_flash[15]!=':') &&
		(mac_in_flash[16]!=':') &&
		(mac_in_flash[17]=='\0') )
			cnt1=1;

	if(cnt1==1)
	{
		if( (strcmp(mac_in_flash,"00:00:00:00:00:00")==0) ||
			(strcmp(mac_in_flash,"FF:FF:FF:FF:FF:FF")==0) )
			cnt1=0;
	}

	if( (cnt1==0) || (cnt<(MAC_SIZE-1)) )//"mac no ':'" || "invalid mac"
	{
		if(print==0)
			return 0;
		memcpy(mac_in_flash,"FF:FF:FF:FF:FF:FF",17);
		mac_in_flash[17]='\0';
	}
	else
	{
		if(print==0)
			return 1;
	}
	puts(mac_in_flash);
	puts("\n");
	return 1;

}

void start_sh4boot (void)
{
	DECLARE_GLOBAL_DATA_PTR;

unsigned char eth[20];
char * tmp_eth;
char * use_dhcp;
char * use_default_env;

	bd_t *bd;
	ulong addr;
	init_fnc_t **init_fnc_ptr;
	ulong size;

	char *s, *e;
	int i;


#ifdef CONFIG_QBOXHD_mini
	lpc_init();
#endif


//Duolabs
for(i=0;i<20;i++)
	eth[i]='\0';
IsLinked=0x00;

	addr = TEXT_BASE;
	/* Reserve memory for malloc() arena. */
	addr -= TOTAL_MALLOC_LEN;
	/* (permanently) allocate a Board Info struct
	 * and a permanent copy of the "global" data
	 */
	addr -= sizeof (gd_t);
	gd = (gd_t *) addr;
	memset ((void *) gd, 0, sizeof (gd_t));
	addr -= sizeof (bd_t);
	bd = (bd_t *) addr;
	gd->bd = bd;

	/* Reserve memory for boot params.
	 */

	addr -= CFG_BOOTPARAMS_LEN;
	for (init_fnc_ptr = init_sequence; *init_fnc_ptr; ++init_fnc_ptr) {
		if ((*init_fnc_ptr) () != 0) {
			hang ();
		}
	}

	gd->reloc_off = 0;
	gd->flags |= GD_FLG_RELOC;	/* tell others: relocation done */

	monitor_flash_len = (ulong) & _uboot_end_data - TEXT_BASE;

	/* configure available FLASH banks */
	
	flashWriteEnable();
	size = flash_init ();
	display_flash_config (size);

	bd = gd->bd;
	bd->bi_boot_params = addr;
	bd->bi_memstart = CFG_SDRAM_BASE;	/* start of  DRAM memory */
	bd->bi_memsize = gd->ram_size;	/* size  of  DRAM memory in bytes */
	bd->bi_baudrate = gd->baudrate;	/* Console Baudrate */
	bd->bi_flashstart = CFG_FLASH_BASE;
	bd->bi_flashsize = size;
#if CFG_MONITOR_BASE == CFG_FLASH_BASE
	bd->bi_flashoffset = monitor_flash_len;	/* reserved area for U-Boot */
#else
	bd->bi_flashoffset = 0;
#endif
	
	/* initialize malloc() area */
	mem_malloc_init ();

	/* Allocate environment function pointers etc. */
	env_relocate ();
/***************************/
	/* Program FPGA module */
	fpga_module_init();

	/* Init the display */
	device_init();
/***************************/
	/* Display the logo */
	demo_print();

	/* Chech if there is a block with MAC addr */
	if( (read_mac_in_flash(0)==0) )	/* No program MAC */
	{
		tmp_eth=getenv ("ethaddr");
		memcpy(eth,tmp_eth,17);
	}
	else	/* There is program MAC */
	{
		//there is a valid mac
		memcpy(eth,mac_in_flash,17);
	}
	setenv("ethaddr",eth);

//	udelay(100000);//100ms
	udelay(1000 * 1000);//1s


	use_dhcp=getenv ("use_dhcp");
	if(strstr(use_dhcp,"yes")!=NULL)	/* Use DHCP for IPs */
	{
		/* DHCP for IPs */	
		unsigned char str[64];
		setenv("bootfile","no_boot_file.txt");
		printf("Enter in DHCP ... \n");
		NetLoop(DHCP_ONLY_IPs);

		printf("Exit to DHCP ... \n");

		/* The 4 IPs are ok */
		if( (NetOurIP!=0) && (NetServerIP!=0) &&
			(NetOurGatewayIP!=0) && (NetOurSubnetMask!=0) )
		{
			printf("\n");
			ip_to_string(NetOurIP,&str);
			setenv("ipaddr",str);
			printf("Ipaddr: '%s'\n",str);
			ip_to_string(NetServerIP,&str);
			setenv("serverip",str);
			printf("Serverip: '%s'\n",str);
			ip_to_string(NetOurGatewayIP,&str);
			setenv("gatewayip",str);
			printf("Gatewayip: '%s'\n",str);
			ip_to_string(NetOurSubnetMask,&str);
			setenv("netmask",str);
			printf("Netmask: '%s'\n",str);
			udelay(100000);//100ms
		}
		else
			printf("Some IPs are wrong ... use environment IPs\n");
	}	
	printf("\n");

	use_default_env=getenv ("use_default_env");
	if(strstr(use_default_env,"yes")!=NULL)	/* Use the default env */
	{
		/* Re-write the bootargs with "dhcp IPs" OR "env IPs" ANd with "program MAC" OR "env MAC" */
		unsigned char * b_a=malloc(300);
		char * ip_addr;
		char * server_ip;
		char * gate_way;
		char * net_mask;
		char * boot_args;
		char * target_fs;
		
		ip_addr=getenv ("ipaddr");
		server_ip=getenv ("serverip");
		gate_way=getenv ("gatewayip");
		net_mask=getenv ("netmask");
		boot_args=getenv ("bootargs");
		if(strstr(boot_args,"root=/dev/nfs")!=NULL)	/* Find root=/dev/nfs */
		{
			target_fs=getenv ("target_nfs");
			memset(b_a,'\0',300);
			sprintf(b_a,"console=ttyAS0,115200 CONSOLE=/dev/ttyAS0 coprocessor_mem=4m@0x10000000,4m@0x10400000 root=/dev/nfs nfsroot=%s,tcp rw ip=%s:%s:%s:%s:qboxhd:eth0:off nwhwconf=device:eth0,hwaddr:%s",target_fs,ip_addr,server_ip,gate_way,net_mask,eth);
			setenv("bootargs",b_a);
		}
		else
		{
			memset(b_a,'\0',300);
		//sprintf(b_a,"console=ttyAS0,115200 CONSOLE=/dev/ttyAS0 coprocessor_mem=4m@0x10000000,4m@0x10400000 root=/dev/ram ramdisk_size=8192 init=/linuxrc rw ip=%s:%s:%s:%s:qboxhd:eth0:off nwhwconf=device:eth0,hwaddr:%s",ip_addr,server_ip,gate_way,net_mask,eth);
			sprintf(b_a,"console=ttyAS0,115200 CONSOLE=/dev/ttyAS0 coprocessor_mem=4m@0x10000000,4m@0x10400000 root=/dev/ram ramdisk_size=8192 init=/init rw ip=%s:%s:%s:%s:qboxhd:eth0:off nwhwconf=device:eth0,hwaddr:%s",ip_addr,server_ip,gate_way,net_mask,eth);
			setenv("bootargs",b_a);
		}
		udelay(100000);
		free(b_a);
 
		/* Print ip on the display */
		ip_addr=getenv ("ipaddr");
		set_pen_color(0xFFE0);	/* YELLOW */
//		display_print(" IP: %s\n",ip_addr);
//		display_print("\n");
		if(IsLinked==0x01)
		{
#ifndef CONFIG_QBOXHD_mini
			display_print(" IP: %s\n",ip_addr);
#else
			display_print("ip: %s\n",ip_addr);
#endif
		}
		else
		{
#ifndef CONFIG_QBOXHD_mini
			display_print(" IP ... NO LINK\n");
#else
			display_print("ip ... no link\n");
#endif
		}
		last_position+=(8*1);
		set_pen_color(0xF800);	/* RED */
		/* board MAC address */
		s = getenv ("ethaddr");
		for (i = 0; i < 6; ++i) {
			bd->bi_enetaddr[i] = (s ? simple_strtoul (s, &e, 16) : 0)
				&& 0xff;
			if (s)
				s = (*e) ? e + 1 : e;
		}
	}
 
	/* IP Address */
	bd->bi_ip_addr = getenv_IPaddr ("ipaddr");

#if defined(CONFIG_PCI)
	/*
	 * Do pci configuration
	 */
	pci_init ();
#endif

/** leave this here (after malloc(), environment and PCI are working) **/
	/* Initialize devices */
	devices_init ();

	jumptable_init ();

	/* Initialize the console (after the relocation and devices init) */
	console_init_r ();

/** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** **/

	/* Initialize from environment */
	if ((s = getenv ("loadaddr")) != NULL) {
		load_addr = simple_strtoul (s, NULL, 16);
	}
#if (CONFIG_COMMANDS & CFG_CMD_NET)
	if ((s = getenv ("bootfile")) != NULL) {
		copy_filename (BootFile, s, sizeof (BootFile));
	}
#endif /* CFG_CMD_NET */

#if defined(CONFIG_MISC_INIT_R)
	/* miscellaneous platform dependent initialisations */
	misc_init_r ();
#endif


#if (CONFIG_COMMANDS & CFG_CMD_NET )
//	puts ("Net:   ");
//	eth_init (gd->bd);
#endif

	/* main_loop() can return to retry autoboot, if so just run it again. */
	for (;;) {
		main_loop ();
	}

	/* NOTREACHED - no way out of command loop except booting */
}


void hang (void)
{
	puts ("### ERROR ### Please RESET the board ###\n");
	for (;;);
}
