#ifndef _H8300_IRQ_H_
#define _H8300_IRQ_H_

#if defined(CONFIG_CPU_H8300H)
#define NR_IRQS 64
#endif
#if defined(CONFIG_CPU_H8S)
#define NR_IRQS 128
#endif

/*
 * "Generic" interrupt sources
 */

static __inline__ int irq_cannonicalize(int irq)
{
	return irq;
}

extern void enable_irq(unsigned int);
extern void disable_irq(unsigned int);

extern int sys_request_irq(unsigned int, 
	void (*)(int, void *, struct pt_regs *), 
	unsigned long, const char *, void *);
extern void sys_free_irq(unsigned int, void *);
extern void request_irq_boot(unsigned int, 
	void (*)(int, void *, struct pt_regs *), 
	unsigned long, const char *, void *);

typedef struct irq_node {
	void		(*handler)(int, void *, struct pt_regs *);
	unsigned long	flags;
	void		*dev_id;
	const char	*devname;
	struct irq_node *next;
} irq_node_t;

/*
 * This structure has only 4 elements for speed reasons
 */
typedef struct irq_handler {
	void (*handler)(int, void *, struct pt_regs *);
	int         flags;
	int         count;
	void	    *dev_id;
	const char  *devname;
} irq_handler_t;

/* count of spurious interrupts */
extern volatile unsigned int num_spurious;

/*
 * Some drivers want these entry points
 */
#define enable_irq_nosync(x)	enable_irq(x)
#define disable_irq_nosync(x)	disable_irq(x)

#endif /* _H8300_IRQ_H_ */
