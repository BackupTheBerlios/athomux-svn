/****************************************************************************/
/*
 *  linux/include/asm-m68knommu/ide.h
 *
 *  Copyright (C) 2001-2003  David McCullough <davidm@snapgear.com>
 *  Copyright (C) 2001       Lineo Inc., davidm@snapgear.com
 *  Copyright (C) 2002       Greg Ungerer (gerg@snapgear.com)
 *  Copyright (C) 1994-1996  Linus Torvalds & authors
 */
 
#ifndef _M68KNOMMU_IDE_H
#define _M68KNOMMU_IDE_H

#ifdef __KERNEL__

#include <linux/config.h>

#include <asm/setup.h>
#include <asm/io.h>
#include <asm/irq.h>

#ifdef CONFIG_ATARI
#include <linux/interrupt.h>
#include <asm/atari_stdma.h>
#endif

#ifdef CONFIG_MAC
#include <asm/macints.h>
#endif

#ifndef MAX_HWIFS
#define MAX_HWIFS	4	/* same as the other archs */
#endif

#undef SUPPORT_SLOW_DATA_PORTS
#define SUPPORT_SLOW_DATA_PORTS 0

#undef SUPPORT_VLB_SYNC
#define SUPPORT_VLB_SYNC 0

/*
 *	Our list of ports/irq's for different boards.
 */

static struct m68k_ide_defaults {
	ide_ioreg_t	base;
	int		irq;
} m68k_ide_defaults[MAX_HWIFS] = {
#if defined(CONFIG_SECUREEDGEMP3)
	{ ((ide_ioreg_t) 0x30800000), 29 },
#elif defined(CONFIG_eLIA) || defined(CONFIG_DISKtel)
	{ ((ide_ioreg_t) 0x30c00000), 29 },
#elif defined(CONFIG_M5249C3)
	{ ((ide_ioreg_t) 0x50000020), 165 },
#else
	{ ((ide_ioreg_t) 0x0), 0 }
#endif
};


#ifdef CONFIG_COLDFIRE

#include <asm/coldfire.h>
#include <asm/mcfsim.h>

/*
 * this is a hack until we can move it somewhere better
 */

static __inline__ void m68k_hw_setup(int irq)
{
	mcf_autovector(irq);
#ifdef CONFIG_M5249C3
	/* Enable interrupts for GPIO5 */
	*((volatile unsigned long *) 0x800000c4) |= 0x00000020;

	/* Enable interrupt level for GPIO5 - VEC37 */
	*((volatile unsigned long *) 0x80000150) |= 0x00200000;
#endif
}

#else
# define m68k_hw_setup(x)
#endif


static __inline__ int ide_default_irq(ide_ioreg_t base)
{
	int i;

	for (i = 0; i < MAX_HWIFS; i++)
		if (m68k_ide_defaults[i].base == base) {
			m68k_hw_setup(m68k_ide_defaults[i].irq);
			return(m68k_ide_defaults[i].irq);
		}
	return 0;
}

static __inline__ ide_ioreg_t ide_default_io_base(int index)
{
	if (index >= 0 && index < MAX_HWIFS)
		return(m68k_ide_defaults[index].base);
	return 0;
}


/*
 * Set up a hw structure for a specified data port, control port and IRQ.
 * This should follow whatever the default interface uses.
 */
static __inline__ void ide_init_hwif_ports(hw_regs_t *hw,
					   ide_ioreg_t data_port,
					   ide_ioreg_t ctrl_port, int *irq)
{
	ide_ioreg_t reg = data_port;
	int i;

	for (i = IDE_DATA_OFFSET; i <= IDE_STATUS_OFFSET; i++) {
		hw->io_ports[i] = reg;
		reg += 1;
	}
	if (ctrl_port) {
		hw->io_ports[IDE_CONTROL_OFFSET] = ctrl_port;
	} else {
		hw->io_ports[IDE_CONTROL_OFFSET] = data_port + 0xe;
	}
}

/*
 * This registers the standard ports for this architecture with the IDE
 * driver.
 */
#define ide_init_default_hwifs() ( \
{ \
	hw_regs_t hw; \
	ide_ioreg_t base; \
	int index; \
 \
	for (index = 0; index < MAX_HWIFS; index++) { \
		base = ide_default_io_base(index); \
		if (!base) \
			continue; \
		memset(&hw, 0, sizeof(hw)); \
		ide_init_hwif_ports(&hw, base, 0, NULL); \
		hw.irq = ide_default_irq(base); \
		ide_hwifs[index].mmio = 2; \
		ide_register_hw(&hw, NULL); \
	} \
})
#if 0
static __inline__ void ide_init_default_hwifs(void)
{
	hw_regs_t hw;
	ide_ioreg_t base;
	int index;

	for (index = 0; index < MAX_HWIFS; index++) {
		base = ide_default_io_base(index);
		if (!base)
			continue;
		memset(&hw, 0, sizeof(hw));
		ide_init_hwif_ports(&hw, base, 0, NULL);
		hw.irq = ide_default_irq(base);
		ide_hwifs[index].mmio = 2;
		ide_register_hw(&hw, NULL);
	}
}
#endif

/****************************************************************************/
/*
 *	System specific IO requirements.
 */

#define	DBGIDE(fmt,a...)
// #define	DBGIDE(fmt,a...) printk(fmt, ##a)
#define IDE_INLINE __inline__
// #define IDE_INLINE

/*
 * Get rid of defs from io.h - ide has its private and conflicting versions
 * Since so far no single m68k platform uses ISA/PCI I/O space for IDE, we
 * always use the `raw' MMIO versions
 */

#undef inb
#undef inw
#undef insw
#undef inl
#undef insl
#undef outb
#undef outw
#undef outsw
#undef outl
#undef outsl
#undef readb
#undef readw
#undef readl
#undef writeb
#undef writew
#undef writel


/****************************************************************************/
#ifdef CONFIG_COLDFIRE

#ifdef CONFIG_SECUREEDGEMP3

#define ADDR8_PTR(addr)		(((addr) & 0x1) ? (0x8000+(addr)-1) : (addr))
#define ADDR16_PTR(addr)	(addr)
#define ADDR32_PTR(addr)	(addr)
#define SWAP8(w)		((((w) & 0xffff) << 8) | (((w) & 0xffff) >> 8))
#define SWAP16(w)		(w)
#define SWAP32(w)		(w)


static IDE_INLINE void
m68k_ide_outb(unsigned int val, unsigned int addr)
{
	volatile unsigned short	*rp;

	DBGIDE("%s(val=%x,addr=%x)\n", __FUNCTION__, val, addr);
	rp = (volatile unsigned short *) ADDR8_PTR(addr);
	*rp = SWAP8(val);
}


static IDE_INLINE int
m68k_ide_inb(unsigned int addr)
{
	volatile unsigned short	*rp, val;

	DBGIDE("%s(addr=%x)\n", __FUNCTION__, addr);
	rp = (volatile unsigned short *) ADDR8_PTR(addr);
	val = *rp;
	return(SWAP8(val));
}


static IDE_INLINE void
m68k_ide_outw(unsigned int val, unsigned int addr)
{
	volatile unsigned short	*rp;

	DBGIDE("%s(val=%x,addr=%x)\n", __FUNCTION__, val, addr);
	rp = (volatile unsigned short *) ADDR16_PTR(addr);
	*rp = SWAP16(val);
}

static IDE_INLINE void
m68k_ide_outsw(unsigned int addr, const void *vbuf, unsigned long len)
{
	volatile unsigned short	*rp, val;
	unsigned short   	*buf;

	DBGIDE("%s(addr=%x,vbuf=%p,len=%x)\n", __FUNCTION__, addr, vbuf, len);
	buf = (unsigned short *) vbuf;
	rp = (volatile unsigned short *) ADDR16_PTR(addr);
	for (; (len > 0); len--) {
		val = *buf++;
		*rp = SWAP16(val);
	}
}

static IDE_INLINE int
m68k_ide_inw(unsigned int addr)
{
	volatile unsigned short *rp, val;

	DBGIDE("%s(addr=%x)\n", __FUNCTION__, addr);
	rp = (volatile unsigned short *) ADDR16_PTR(addr);
	val = *rp;
	return(SWAP16(val));
}

static IDE_INLINE void
m68k_ide_insw(unsigned int addr, void *vbuf, unsigned long len)
{
	volatile unsigned short *rp;
	unsigned short          w, *buf;

	DBGIDE("%s(addr=%x,vbuf=%p,len=%x)\n", __FUNCTION__, addr, vbuf, len);
	buf = (unsigned short *) vbuf;
	rp = (volatile unsigned short *) ADDR16_PTR(addr);
	for (; (len > 0); len--) {
		w = *rp;
		*buf++ = SWAP16(w);
	}
}

static IDE_INLINE int
m68k_ide_inl(unsigned int addr)
{
	volatile unsigned long *rp, val;

	DBGIDE("%s(addr=%x)\n", __FUNCTION__, addr);
	rp = (volatile unsigned long *) ADDR(addr);
	val = *rp;
	return(SWAP32(val));
}

static IDE_INLINE void
m68k_ide_insl(unsigned int addr, void *vbuf, unsigned long len)
{
	volatile unsigned long *rp;
	unsigned long          w, *buf;

	DBGIDE("%s(addr=%x,vbuf=%p,len=%x)\n", __FUNCTION__, addr, vbuf, len);
	buf = (unsigned long *) vbuf;
	rp = (volatile unsigned long *) ADDR32_PTR(addr);
	for (; (len > 0); len--) {
		w = *rp;
		*buf++ = SWAP32(w);
	}
}

static IDE_INLINE void
m68k_ide_outl(unsigned int val, unsigned int addr)
{
	volatile unsigned long	*rp;

	DBGIDE("%s(val=%x,addr=%x)\n", __FUNCTION__, val, addr);
	rp = (volatile unsigned long *) ADDR(addr);
	*rp = SWAP32(val);
}

static IDE_INLINE void
m68k_ide_outsl(unsigned int addr, const void *vbuf, unsigned long len)
{
	volatile unsigned long	*rp, val;
	unsigned long   	*buf;

	DBGIDE("%s(addr=%x,vbuf=%p,len=%x)\n", __FUNCTION__, addr, vbuf, len);
	buf = (unsigned long *) vbuf;
	rp = (volatile unsigned long *) ADDR32_PTR(addr);
	for (; (len > 0); len--) {
		val = *buf++;
		*rp = SWAP32(val);
	}
}

#elif defined(CONFIG_eLIA) || defined(CONFIG_DISKtel)

/* 8/16 bit acesses are controlled by flicking bits in the CS register */
#define	ACCESS_MODE_16BIT()	\
	*((volatile unsigned short *) (MCF_MBAR + MCFSIM_LOCALCS)) = 0x0080
#define	ACCESS_MODE_8BIT()	\
	*((volatile unsigned short *) (MCF_MBAR + MCFSIM_LOCALCS)) = 0x0040


static IDE_INLINE void
m68k_ide_outw(unsigned int val, unsigned int addr)
{
	ACCESS_MODE_16BIT();
	outw(val, addr);
	ACCESS_MODE_8BIT();
}

static IDE_INLINE void
m68k_ide_outsw(unsigned int addr, const void *vbuf, unsigned long len)
{
	ACCESS_MODE_16BIT();
	outsw(addr, vbuf, len);
	ACCESS_MODE_8BIT();
}

static IDE_INLINE int
m68k_ide_inw(unsigned int addr)
{
	int ret;

	ACCESS_MODE_16BIT();
	ret = inw(addr);
	ACCESS_MODE_8BIT();
	return(ret);
}

static IDE_INLINE void
m68k_ide_insw(unsigned int addr, void *vbuf, unsigned long len)
{
	ACCESS_MODE_16BIT();
	insw(addr, vbuf, len);
	ACCESS_MODE_8BIT();
}

static IDE_INLINE int
m68k_ide_inl(unsigned int addr)
{
	int ret;

	ACCESS_MODE_16BIT();
	ret = inl(addr);
	ACCESS_MODE_8BIT();
	return(ret);
}

static IDE_INLINE void
m68k_ide_insl(unsigned int addr, void *vbuf, unsigned long len)
{
	ACCESS_MODE_16BIT();
	insl(addr, vbuf, len);
	ACCESS_MODE_8BIT();
}

static IDE_INLINE void
m68k_ide_outl(unsigned int val, unsigned int addr)
{
	ACCESS_MODE_16BIT();
	outl(val, addr);
	ACCESS_MODE_8BIT();
}

static IDE_INLINE void
m68k_ide_outsl(unsigned int addr, const void *vbuf, unsigned long len)
{
	ACCESS_MODE_16BIT();
	outsl(addr, vbuf, len);
	ACCESS_MODE_8BIT();
}

#elif defined(CONFIG_M5249C3)

#define	ADDR(a)		(((a) & 0xfffffff0) + (((a) & 0xf) << 1))
#define SWAP8(w)	(w)
#define SWAP16(w)	((((w) & 0xffff) << 8) | (((w) & 0xffff) >> 8))
#define SWAP32(w)	((((w) & 0xff) << 24) | (((w) & 0xff00) << 8) | \
			 (((w) & 0xff0000) >> 8) | (((w) & 0xff000000) >> 24))


static IDE_INLINE void
m68k_ide_outb(unsigned int val, unsigned int addr)
{
	volatile unsigned short	*rp;

	DBGIDE("%s(val=%x,addr=%x)\n", __FUNCTION__, val, addr);
	rp = (volatile unsigned short *) ADDR(addr);
	*rp = SWAP8(val);
}


static IDE_INLINE int
m68k_ide_inb(unsigned int addr)
{
	volatile unsigned short	*rp, val;

	DBGIDE("%s(addr=%x)\n", __FUNCTION__, addr);
	rp = (volatile unsigned short *) ADDR(addr);
	val = *rp;
	return(SWAP8(val));
}

static IDE_INLINE void
m68k_ide_outw(unsigned int val, unsigned int addr)
{
	volatile unsigned short	*rp;

	DBGIDE("%s(val=%x,addr=%x)\n", __FUNCTION__, val, addr);
	rp = (volatile unsigned short *) ADDR(addr);
	*rp = SWAP16(val);
}

static IDE_INLINE void
m68k_ide_outsw(unsigned int addr, const void *vbuf, unsigned long len)
{
	volatile unsigned short	*rp, val;
	unsigned short   	*buf;

	DBGIDE("%s(addr=%x,vbuf=%p,len=%x)\n", __FUNCTION__, addr, vbuf, len);
	buf = (unsigned short *) vbuf;
	rp = (volatile unsigned short *) ADDR(addr);
	for (; (len > 0); len--) {
		val = *buf++;
		*rp = SWAP16(val);
	}
}

static IDE_INLINE int
m68k_ide_inw(unsigned int addr)
{
	volatile unsigned short *rp, val;

	DBGIDE("%s(addr=%x)\n", __FUNCTION__, addr);
	rp = (volatile unsigned short *) ADDR(addr);
	val = *rp;
	return(SWAP16(val));
}

static IDE_INLINE void
m68k_ide_insw(unsigned int addr, void *vbuf, unsigned long len)
{
	volatile unsigned short *rp;
	unsigned short          w, *buf;

	DBGIDE("%s(addr=%x,vbuf=%p,len=%x)\n", __FUNCTION__, addr, vbuf, len);
	buf = (unsigned short *) vbuf;
	rp = (volatile unsigned short *) ADDR(addr);
	for (; (len > 0); len--) {
		w = *rp;
		*buf++ = SWAP16(w);
	}
}

static IDE_INLINE int
m68k_ide_inl(unsigned int addr)
{
	volatile unsigned long *rp, val;

	DBGIDE("%s(addr=%x)\n", __FUNCTION__, addr);
	rp = (volatile unsigned long *) ADDR(addr);
	val = *rp;
	return(SWAP32(val));
}

static IDE_INLINE void
m68k_ide_insl(unsigned int addr, void *vbuf, unsigned long len)
{
	volatile unsigned long *rp;
	unsigned long          w, *buf;

	DBGIDE("%s(addr=%x,vbuf=%p,len=%x)\n", __FUNCTION__, addr, vbuf, len);
	buf = (unsigned long *) vbuf;
	rp = (volatile unsigned long *) ADDR(addr);
	for (; (len > 0); len--) {
		w = *rp;
		*buf++ = SWAP32(w);
	}
}

static IDE_INLINE void
m68k_ide_outl(unsigned int val, unsigned int addr)
{
	volatile unsigned long	*rp;

	DBGIDE("%s(val=%x,addr=%x)\n", __FUNCTION__, val, addr);
	rp = (volatile unsigned long *) ADDR(addr);
	*rp = SWAP32(val);
}

static IDE_INLINE void
m68k_ide_outsl(unsigned int addr, const void *vbuf, unsigned long len)
{
	volatile unsigned long	*rp, val;
	unsigned long   	*buf;

	DBGIDE("%s(addr=%x,vbuf=%p,len=%x)\n", __FUNCTION__, addr, vbuf, len);
	buf = (unsigned long *) vbuf;
	rp = (volatile unsigned long *) ADDR(addr);
	for (; (len > 0); len--) {
		val = *buf++;
		*rp = SWAP32(val);
	}
}

#endif /* CONFIG_M5249C3 */

#endif /* CONFIG_COLDFIRE */
/****************************************************************************/

#define inb				m68k_ide_inb
#define inw				m68k_ide_inw
#define insw(port, addr, n)		m68k_ide_insw((u16 *)port, addr, n)
#define inl				m68k_ide_inl
#define insl(port, addr, n)		m68k_ide_insl((u32 *)port, addr, n)
#define outb				m68k_ide_outb
#define outw				m68k_ide_outw
#define outsw(port, addr, n)		m68k_ide_outsw((u16 *)port, addr, n)
#define outl				m68k_ide_outl
#define outsl(port, addr, n)		m68k_ide_outsl((u32 *)port, addr, n)
#define readb				m68k_ide_inb
#define readw				m68k_ide_inw
#define readl				m68k_ide_inl
#define writeb(val, port)		m68k_ide_outb(val, port)
#define writew(val, port)		m68k_ide_outw(val, port)
#define writel(val, port)		m68k_ide_outl(val, port)

#define __ide_mm_insw(port, addr, n)	m68k_ide_insw((u16 *)port, addr, n)
#define __ide_mm_insl(port, addr, n)	m68k_ide_insl((u32 *)port, addr, n)
#define __ide_mm_outsw(port, addr, n)	m68k_ide_outsw((u16 *)port, addr, n)
#define __ide_mm_outsl(port, addr, n)	m68k_ide_outsl((u32 *)port, addr, n)
#define __ide_mm_outl(val, port)	m68k_ide_outl(val, (u32 *)port)
#define __ide_mm_inl(port)		m68k_ide_inl((u32 *)port)

#if defined(CONFIG_COLDFIRE)
#define insw_swapw(port, addr, n)	m68k_ide_insw((u16 *)port, addr, n)
#define outsw_swapw(port, addr, n)	m68k_ide_outsw((u16 *)port, addr, n)
#endif


/* Coldfire have byteswapped IDE busses and since many interesting
 * values in the identification string are text, chars and words they
 * happened to be almost correct without swapping.. However *_capacity
 * is needed for drives over 8 GB. RZ */
#if defined(CONFIG_Q40) || defined(CONFIG_ATARI)
#define M68K_IDE_SWAPW  (MACH_IS_Q40 || MACH_IS_ATARI)
#endif

#define IDE_ARCH_ACK_INTR
#define ide_ack_intr(hwif)	((hwif)->hw.ack_intr ? (hwif)->hw.ack_intr(hwif) : 1)

#endif /* __KERNEL__ */
#endif /* _M68KNOMMU_IDE_H */
/****************************************************************************/
