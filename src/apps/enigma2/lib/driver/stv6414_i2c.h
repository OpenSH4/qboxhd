
#ifndef _STV6414_I2C_H
#define _STV6414_I2C_H

#include <linux/kernel.h>
#include <linux/i2c.h>

#define	STV6414_DEVICE_NAME				"stv6414_0"  
#define	STV6414_NUMBER_OF_CONTROLLERS	1

typedef struct
{
	unsigned short offset_addr;			/* Module A, Module B, EXT*/
	unsigned char value;	/* CIS mode or "Normal" mode */
}Register_t;

/* Format */
#if 0
	|VCR format|TV format| ???????? |
	|  7   6   |  5   4  |	3 2 1 0 |
	|  0   0   |  0   0  |  x x x x |	--->	VCR e TV Input
	|  0   1   |  0   1  |  x x x x |	--->	VCR e TV Output < 2
	|  1   0   |  1   0  |  x x x x |	--->	VCR e TV Output 16/9
	|  1   1   |  1   1  |  x x x x |	--->	VCR e TV Output 4/3
#endif


#define	TV_16_9						0x20
#define	TV_4_3						0x30
#define	MASK_VCR_TV_FORMAT			0xF0

#define	STANDBY_OFF					0x00
#define STANDBY_ON					0x01

#define IOCTL_STV6414_MAGIC					'S'
#define	IOCTL_READ_I2C						_IOW(IOCTL_STV6414_MAGIC, 1, Register_t)
#define	IOCTL_WRITE_I2C						_IOW(IOCTL_STV6414_MAGIC, 2, Register_t)
#define IOCTL_READ_LAST_WRITE_VALUE_OF_REG	_IOW(IOCTL_STV6414_MAGIC, 3, Register_t)
#define	IOCTL_SET_169_43					_IOW(IOCTL_STV6414_MAGIC, 4, int)
#define	IOCTL_STANDBY_ON_OFF				_IOW(IOCTL_STV6414_MAGIC, 5, int)
#define	IOCTL_DISABLE_TV_OUTPUT				_IOW(IOCTL_STV6414_MAGIC, 6, int)


#endif // _STV6414_I2C_H






