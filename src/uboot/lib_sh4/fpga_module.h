#ifndef _FPGA_MODULE_H
#define _FPGA_MODULE_H

#define	FPGA_BITSTREAM_ADDR			0x90000
#define FPGA_BITSTREAM_SIZE			480148
/////////////////////////////////////////////////////////////////////////////////////////////
#define FPGA_BUFFER_SIZE			512
#define FPGA_INT_MEM_REG			0x1000
#define FPGA_IO_SIZE				0x1000      /**< Size of I/O mem: 4096 */
#define FPGA_BASE_ADDRESS			0x3800000	 
/////////////////////////////////////////////////////////////////////////////////////////////
//#define PIO2_BASE_ADDRESS           0x18022000
#define PIO2_BASE_ADDRESS           0xB8022000

/* For spare */
#define	PIO0_BASE_ADDRESS			0xB8020000
#define	PIO1_BASE_ADDRESS			0xB8021000
#define	PIO3_BASE_ADDRESS			0xB8023000
#define	PIO4_BASE_ADDRESS			0xB8024000
#define	PIO5_BASE_ADDRESS			0xB8025000



/* These offset are the same for all port of GPIO */

#define PIO2_IO_SIZE				0x1000      /**< Size of I/O mem: 4096 */
#define PIO_P2C0                    0x20
#define PIO_P2C1                    0x30
#define PIO_P2C2                    0x40
/* Set individually */
#define PIO_SET_P2C0				0x24
#define PIO_SET_P2C1                0x34
#define PIO_SET_P2C2                0x44
/* Clear individually */
#define PIO_CLR_P2C0                0x28
#define PIO_CLR_P2C1                0x38
#define PIO_CLR_P2C2                0x48
#define PIO_SET_P2OUT               0x04
#define PIO_CLR_P2OUT               0x08
#define PIO_P2IN					0x10
					                                     //c2	c1	c0

#define	PIN_RPROG_B_MASK		0x20	//output:		0	1	0


#define	PIN_DONE_MASK				0x40	//input:		1	0	1
#define	PIN_INIT_B_MASK				0x80	//output:		0	1	0


///////////////////////////////////////////////////////////////////////////////////////////		
		
#define PREPARE_FOR_PROGRAM	0x01
#define TEST_DONE_PIN		0x02
#define WRITE_ADDR_FPGA		0x03



typedef struct
{
	unsigned short offset_addr;	/* Address of the memory of fpga*/
	unsigned char value;			/* Value (a byte) to write */
}Register_t;

typedef struct
{
	unsigned long fpga_base_address;
	unsigned long pio2_base_address;
}fpga_t;
		

void fpga_module_init(void);
void fpga_set_input_spare(void);
	
#endif  /* _FPGA_MODULE_H */
