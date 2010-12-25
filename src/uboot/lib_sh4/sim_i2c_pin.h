#ifndef _SIM_I2C_PIN_H
#define _SIM_I2C_PIN_H


#define PIO3_BASE_ADDRESS	0xB8023000

#define PIO3_IO_SIZE		0x1000      /**< Size of I/O mem: 4096 */
#define PIO_P3C0			0x20
#define PIO_P3C1			0x30
#define PIO_P3C2			0x40
/* Set individually */
#define PIO_SET_P3C0		0x24
#define PIO_SET_P3C1		0x34
#define PIO_SET_P3C2		0x44
/* Clear individually */
#define PIO_CLR_P3C0		0x28
#define PIO_CLR_P3C1		0x38
#define PIO_CLR_P3C2		0x48
#define PIO_SET_P3OUT		0x04
#define PIO_CLR_P3OUT		0x08
#define PIO_P3IN			0x10

#define	SCL					0x01
#define	SDA					0x02		


void lpc_init(void);
unsigned char set_brg(unsigned char b);


#endif //_SIM_I2C_PIN_H
