#ifndef __ASM_SH_IO_H
#define __ASM_SH_IO_H

/*
 * Convention:
 *    read{b,w,l}/write{b,w,l} are for PCI,
 *    while in{b,w,l}/out{b,w,l} are for ISA
 * These may (will) be platform specific function.
 * In addition we have 'pausing' versions: in{b,w,l}_p/out{b,w,l}_p
 * and 'string' versions: ins{b,w,l}/outs{b,w,l}
 * For read{b,w,l} and write{b,w,l} there are also __raw versions, which
 * do not have a memory barrier after them.
 *
 * In addition, we have
 *   ctrl_in{b,w,l}/ctrl_out{b,w,l} for SuperH specific I/O.
 *   which are processor specific.
 */

/*
 * We follow the Alpha convention here:
 *  __inb expands to an inline function call (which calls via the mv)
 *  _inb  is a real function call (note ___raw fns are _ version of __raw)
 *  inb   by default expands to _inb, but the machine specific code may
 *        define it to __inb if it chooses.
 */
#include <asm/cache.h>
#include <asm/system.h>
#include <asm/addrspace.h>
#include <asm/machvec.h>
#include <asm/pgtable.h>
#include <asm-generic/iomap.h>

#ifdef __KERNEL__

/*
 * Depending on which platform we are running on, we need different
 * I/O functions.
 */
#define __IO_PREFIX	generic
#include <asm/io_generic.h>

#define maybebadio(port) \
  printk(KERN_ERR "bad PC-like io %s:%u for port 0x%lx at 0x%08x\n", \
	 __FUNCTION__, __LINE__, (port), (u32)__builtin_return_address(0))

/*
 * Since boards are able to define their own set of I/O routines through
 * their respective machine vector, we always wrap through the mv.
 *
 * Also, in the event that a board hasn't provided its own definition for
 * a given routine, it will be wrapped to generic code at run-time.
 */

#define __inb(p)	sh_mv.mv_inb((p))
#define __inw(p)	sh_mv.mv_inw((p))
#define __inl(p)	sh_mv.mv_inl((p))
#define __outb(x,p)	sh_mv.mv_outb((x),(p))
#define __outw(x,p)	sh_mv.mv_outw((x),(p))
#define __outl(x,p)	sh_mv.mv_outl((x),(p))

#define __inb_p(p)	sh_mv.mv_inb_p((p))
#define __inw_p(p)	sh_mv.mv_inw_p((p))
#define __inl_p(p)	sh_mv.mv_inl_p((p))
#define __outb_p(x,p)	sh_mv.mv_outb_p((x),(p))
#define __outw_p(x,p)	sh_mv.mv_outw_p((x),(p))
#define __outl_p(x,p)	sh_mv.mv_outl_p((x),(p))

#define __insb(p,b,c)	sh_mv.mv_insb((p), (b), (c))
#define __insw(p,b,c)	sh_mv.mv_insw((p), (b), (c))
#define __insl(p,b,c)	sh_mv.mv_insl((p), (b), (c))
#define __outsb(p,b,c)	sh_mv.mv_outsb((p), (b), (c))
#define __outsw(p,b,c)	sh_mv.mv_outsw((p), (b), (c))
#define __outsl(p,b,c)	sh_mv.mv_outsl((p), (b), (c))

#define __readb(a)	sh_mv.mv_readb((a))
#define __readw(a)	sh_mv.mv_readw((a))
#define __readl(a)	sh_mv.mv_readl((a))
#define __writeb(v,a)	sh_mv.mv_writeb((v),(a))
#define __writew(v,a)	sh_mv.mv_writew((v),(a))
#define __writel(v,a)	sh_mv.mv_writel((v),(a))

#define inb		__inb
#define inw		__inw
#define inl		__inl
#define outb		__outb
#define outw		__outw
#define outl		__outl

#define inb_p		__inb_p
#define inw_p		__inw_p
#define inl_p		__inl_p
#define outb_p		__outb_p
#define outw_p		__outw_p
#define outl_p		__outl_p

#define insb		__insb
#define insw		__insw
#define insl		__insl
#define outsb		__outsb
#define outsw		__outsw
#define outsl		__outsl

#define __raw_readb(a)		__readb((void __iomem *)(a))
#define __raw_readw(a)		__readw((void __iomem *)(a))
#define __raw_readl(a)		__readl((void __iomem *)(a))
#define __raw_writeb(v, a)	__writeb(v, (void __iomem *)(a))
#define __raw_writew(v, a)	__writew(v, (void __iomem *)(a))
#define __raw_writel(v, a)	__writel(v, (void __iomem *)(a))

void __raw_readsb(const void __iomem *port, void *data, int longlen);
void __raw_readsw(const void __iomem *port, void *data, int longlen);
void __raw_readsl(const void __iomem *port, void *data, int longlen);
void __raw_writesb(void __iomem *port, const void *data, int longlen);
void __raw_writesw(void __iomem *port, const void *data, int longlen);
void __raw_writesl(void __iomem *port, const void *data, int longlen);

/*
 * The platform header files may define some of these macros to use
 * the inlined versions where appropriate.  These macros may also be
 * redefined by userlevel programs.
 */
#ifdef __readb
# define readb(a)	({ unsigned int r_ = __raw_readb(a); mb(); r_; })
#endif
#ifdef __raw_readw
# define readw(a)	({ unsigned int r_ = __raw_readw(a); mb(); r_; })
#endif
#ifdef __raw_readl
# define readl(a)	({ unsigned int r_ = __raw_readl(a); mb(); r_; })
#endif

#ifdef __raw_writeb
# define writeb(v,a)	({ __raw_writeb((v),(a)); mb(); })
#endif
#ifdef __raw_writew
# define writew(v,a)	({ __raw_writew((v),(a)); mb(); })
#endif
#ifdef __raw_writel
# define writel(v,a)	({ __raw_writel((v),(a)); mb(); })
#endif

#define __BUILD_MEMORY_STRING(bwlq, type)				\
									\
static inline void writes##bwlq(volatile void __iomem *mem,		\
				const void *addr, unsigned int count)	\
{									\
	const volatile type *__addr = addr;				\
									\
	while (count--) {						\
		__raw_write##bwlq(*__addr, mem);			\
		__addr++;						\
	}								\
}									\
									\
static inline void reads##bwlq(volatile void __iomem *mem, void *addr,	\
			       unsigned int count)			\
{									\
	volatile type *__addr = addr;					\
									\
	while (count--) {						\
		*__addr = __raw_read##bwlq(mem);			\
		__addr++;						\
	}								\
}

__BUILD_MEMORY_STRING(b, u8)
__BUILD_MEMORY_STRING(w, u16)
#define writesl __raw_writesl
#define readsl  __raw_readsl

#define readb_relaxed(a) readb(a)
#define readw_relaxed(a) readw(a)
#define readl_relaxed(a) readl(a)

/* Simple MMIO */
#define ioread8(a)		readb(a)
#define ioread16(a)		readw(a)
#define ioread16be(a)		be16_to_cpu(__raw_readw((a)))
#define ioread32(a)		readl(a)
#define ioread32be(a)		be32_to_cpu(__raw_readl((a)))

#define iowrite8(v,a)		writeb((v),(a))
#define iowrite16(v,a)		writew((v),(a))
#define iowrite16be(v,a)	__raw_writew(cpu_to_be16((v)),(a))
#define iowrite32(v,a)		writel((v),(a))
#define iowrite32be(v,a)	__raw_writel(cpu_to_be32((v)),(a))

#define ioread8_rep(p,d,c)	__raw_readsb(p,d,c)
#define ioread16_rep(p,d,c)	__raw_readsw(p,d,c)
#define ioread32_rep(p,d,c)	__raw_readsl(p,d,c)

#define iowrite8_rep(p,s,c)	__raw_writesb(p,s,c)
#define iowrite16_rep(p,s,c)	__raw_writesw(p,s,c)
#define iowrite32_rep(p,s,c)	__raw_writesl(p,s,c)

#define mmiowb()	wmb()	/* synco on SH-4A, otherwise a nop */

/*
 * This function provides a method for the generic case where a board-specific
 * ioport_map simply needs to return the port + some arbitrary port base.
 *
 * We use this at board setup time to implicitly set the port base, and
 * as a result, we can use the generic ioport_map.
 */
static inline void __set_io_port_base(unsigned long pbase)
{
	extern unsigned long generic_io_base;

	generic_io_base = pbase;
}

/* We really want to try and get these to memcpy etc */
extern void memcpy_fromio(void *, volatile void __iomem *, unsigned long);
extern void memcpy_toio(volatile void __iomem *, const void *, unsigned long);
extern void memset_io(volatile void __iomem *, int, unsigned long);

extern void ctrl_fn_using_non_p3_address(void);
#define CTRL_ADDR_CHECK(addr)						\
	if (__builtin_constant_p(addr) && (PXSEG(addr) != P4SEG))	\
		ctrl_fn_using_non_p3_address();				\
	else if (PXSEG(addr) != P4SEG)					\
		pr_debug("ctrl fn using non-p4 address at %s:%u\n",	\
			__FILE__, __LINE__);

/* SuperH on-chip I/O functions */
static inline unsigned char ctrl_inb(unsigned long addr)
{
	CTRL_ADDR_CHECK(addr);
	return *(volatile unsigned char*)addr;
}

static inline unsigned short ctrl_inw(unsigned long addr)
{
	CTRL_ADDR_CHECK(addr);
	return *(volatile unsigned short*)addr;
}

static inline unsigned int ctrl_inl(unsigned long addr)
{
	CTRL_ADDR_CHECK(addr);
	return *(volatile unsigned long*)addr;
}

static inline void ctrl_outb(unsigned char b, unsigned long addr)
{
	CTRL_ADDR_CHECK(addr);
	*(volatile unsigned char*)addr = b;
}

static inline void ctrl_outw(unsigned short b, unsigned long addr)
{
	CTRL_ADDR_CHECK(addr);
	*(volatile unsigned short*)addr = b;
}

static inline void ctrl_outl(unsigned int b, unsigned long addr)
{
	CTRL_ADDR_CHECK(addr);
        *(volatile unsigned long*)addr = b;
}

static inline void ctrl_delay(void)
{
	ctrl_inw(P2SEG);
}

#define IO_SPACE_LIMIT 0xffffffff

#ifdef CONFIG_MMU
/*
 * Change virtual addresses to physical addresses and vv.
 * These are trivial on the 1:1 Linux/SuperH mapping
 */
static inline unsigned long virt_to_phys(volatile void *address)
{
	return __pa(address);
}

static inline void *phys_to_virt(unsigned long address)
{
	return __va(address);
}
#else
#define phys_to_virt(address)	((void *)(address))
#define virt_to_phys(address)	((unsigned long)(address))
#endif

/*
 * readX/writeX() are used to access memory mapped devices. On some
 * architectures the memory mapped IO stuff needs to be accessed
 * differently. On the x86 architecture, we just read/write the
 * memory location directly.
 *
 * On SH, we traditionally have the whole physical address space mapped
 * at all times (as MIPS does), so "ioremap()" and "iounmap()" do not
 * need to do anything but place the address in the proper segment. This
 * is true for P1 and P2 addresses, as well as some P3 ones. However,
 * most of the P3 addresses and newer cores using extended addressing
 * need to map through page tables, so the ioremap() implementation
 * becomes a bit more complicated. See arch/sh/mm/ioremap.c for
 * additional notes on this.
 *
 * We cheat a bit and always return uncachable areas until we've fixed
 * the drivers to handle caching properly.
 */
#ifdef CONFIG_MMU
void __iomem *__ioremap_mode(unsigned long offset, unsigned long size,
			     unsigned long flags);
void __iomem *__ioremap_prot(unsigned long offset, unsigned long size,
			     pgprot_t prot);
void __iounmap(void __iomem *addr);
#else
#define __iounmap(addr)			do { } while (0)
static inline void __iomem *
__ioremap_mode(unsigned long offset, unsigned long size, unsigned long flags)
{
	unsigned long last_addr = offset + size - 1;

	if (likely(PXSEG(offset) < P3SEG && PXSEG(last_addr) < P3SEG)) {
		if (unlikely(flags & _PAGE_CACHABLE))
			return (void __iomem *)P1SEGADDR(offset);

		return (void __iomem *)P2SEGADDR(offset);
	}

	return ((void __iomem *)(offset));
}
#endif /* CONFIG_MMU */

#define ioremap(offset, size)				\
	__ioremap_mode((offset), (size), 0)
#define ioremap_nocache(offset, size)			\
	__ioremap_mode((offset), (size), 0)
#define ioremap_cache(offset, size)			\
	__ioremap_mode((offset), (size), _PAGE_CACHABLE)
#define p3_ioremap(offset, size, prot)			\
	__ioremap_prot((offset), (size), (prot))
#define iounmap(addr)					\
	__iounmap((addr))

/*
 * The caches on some architectures aren't dma-coherent and have need to
 * handle this in software.  There are three types of operations that
 * can be applied to dma buffers.
 *
 *  - dma_cache_wback_inv(start, size) makes caches and RAM coherent by
 *    writing the content of the caches back to memory, if necessary.
 *    The function also invalidates the affected part of the caches as
 *    necessary before DMA transfers from outside to memory.
 *  - dma_cache_inv(start, size) invalidates the affected parts of the
 *    caches.  Dirty lines of the caches may be written back or simply
 *    be discarded.  This operation is necessary before dma operations
 *    to the memory.
 *  - dma_cache_wback(start, size) writes back any dirty lines but does
 *    not invalidate the cache.  This can be used before DMA reads from
 *    memory,
 */

#define dma_cache_wback_inv(_start,_size) \
    __flush_purge_region(_start,_size)
#define dma_cache_inv(_start,_size) \
    __flush_invalidate_region(_start,_size)
#define dma_cache_wback(_start,_size) \
    __flush_wback_region(_start,_size)

/*
 * Convert a physical pointer to a virtual kernel pointer for /dev/mem
 * access
 */
#define xlate_dev_mem_ptr(p)	__va(p)

/*
 * Convert a virtual cached pointer to an uncached pointer
 */
#define xlate_dev_kmem_ptr(p)	p

#endif /* __KERNEL__ */

#endif /* __ASM_SH_IO_H */
