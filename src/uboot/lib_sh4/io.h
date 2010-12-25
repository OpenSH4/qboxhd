#ifndef __ASM_SH_IO_H
#define __ASM_SH_IO_H



/* SuperH on-chip I/O functions */
static inline unsigned char ctrl_inb(unsigned long addr)
{
	return *(volatile unsigned char*)addr;
}

static inline unsigned short ctrl_inw(unsigned long addr)
{
	return *(volatile unsigned short*)addr;
}

static inline unsigned int ctrl_inl(unsigned long addr)
{
	return *(volatile unsigned long*)addr;
}

static inline void ctrl_outb(unsigned char b, unsigned long addr)
{
	*(volatile unsigned char*)addr = b;
}

static inline void ctrl_outw(unsigned short b, unsigned long addr)
{
	*(volatile unsigned short*)addr = b;
}

static inline void ctrl_outl(unsigned int b, unsigned long addr)
{
        *(volatile unsigned long*)addr = b;
}
/*
static inline void ctrl_delay(void)
{
	ctrl_inw(P2SEG);
}
*/

#endif /* __ASM_SH_IO_H */
