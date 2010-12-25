/*
 *   Copyright (C) 2003, 2004 STMicroelectronics
 *
 *   Author: Giuseppe Cavallaro		(peppe@st.com)
 *
 *   Defines the header layout of a ST executable image file. It's a
 *   ST proprietary format suitable to be loaded onto multiprocessor
 *   architectures.
 *
 *   The format of an image file is as follows:
 *
 *   Start Offset   End offset   Description
 *   -------------------------------------------------------------
 *    0x00000000    0x00000003   Magic number
 *    0x00000004    0x00000023   ASCIIZ description
 *    0x00000024    0x00000027   CPU number (0-7)
 *    0x00000028    0x0000002B   Stack address (if applicable)
 *    0x0000002C    0x0000002F   Entry point  (if applicable)
 *    0x00000030    0x00000033   Number of sections following (N max = 16)
 *    0x00000034    + (N*0x10)   Section descriptors (up to 16; see below)
 *    0x34+(N*010)  0x00000FFF   pad to 4K boundary    
 *    0x00001000+(N*0x10)....    Concatenated section data
 *
 *   A section descriptor looks like this:
 *
 *    0x00000000    0x00000003   Type
 *    0x00000004    0x00000007   Source address
 *    0x00000008    0x0000000B   Destination address
 *    0x0000000C    0x0000000F   Length
 *
 *   The section type is encoded as follows:
 *
 *    0xN000000n
 *   
 *   where N is the placement hint and n is the boot time copy strategy.
 *
 *   N - 0 ==> place anywhere
 *       1 ==> has no placement requirement (not held in ROM at all)
 *       2 ==> reserved
 *       3 ==> place precisely at its source address in ROM
 *
 *   n - 0 ==> do nothing with this section
 *       1 ==> copy this section from its source address to its dest address
 *       2 ==> zero this section at its dest address
 *       3 ==> runlength decode this section
 *       4 ==> inflate this section
 *
 *   NOTE - runlength encoding & deflation of sections is not yet supported
 *
 *   If the addresses in ELF files look like ST40 addresses, they are
 *   left as such. Otherwise they are mapped to ST40 P2 addresses.
 *
 * -------------------------------------------------------------------------
 *
 * NOTE: The header layout must be known by the stslave tool running on
 *       ST40 Linux. Currently this is duplicate in both packages :-( !
 *       
 */
#define MAX_SECTIONS	16
typedef struct {
    u_int	 type;		/* section type:			  */
    u_long	 source;	/* source address (file offset)		  */
    u_long	 destination;	/* destination address (in memory) 	  */
    u_int	 len;		/* length in bytes			  */
} section_t;

#define LX200_IMAGE_TAG		0x13a9f175
#define STYPE(place,copy_mode)	((place) < 16) | ((copy_mode) & 0x0000ffff)
#define PLACE_ANYWHERE		0
#define PLACE_NOTINROM		1
#define PLACE_INPLACE		3

#define MODE_SKIP		0
#define MODE_COPY		1
#define MODE_ERASE		2
#define MODE_DECODE		3
#define MODE_INFLATE		4

#define BUFF_SIZE		4096

/* ---- Debug Macros ---------------------------------------------------- */
#undef DEBUG
#ifdef DEBUG
#define DEBUG(d)        printf d
#else
#define DEBUG(d)
#endif

