// Author: Marcel Kilgus
//
#ifndef _X86_IRQ_H
#define _X86_IRQ_H

// Interrupt table size
#define IDT_SIZE		256

// Exception vectors
#define EXC_ZERO_DIVIDE 0x00
#define EXC_DEBUG		0x01
#define EXC_NMI			0x02
#define EXC_BREAKPOINT	0x03
#define EXC_OVERFLOW	0x04
#define EXC_BOUNDS_CHK	0x05
#define EXC_INVALID_OP	0x06
#define EXC_DEV_NA		0x07
#define EXC_DBLE_FAULT 	0x08
#define EXC_INVALID_TSS	0x0A
#define EXC_SEG_NP		0x0B
#define EXC_STACK		0x0C
#define EXC_GP			0x0D
#define EXC_PAGE_FAULT	0x0E
#define EXC_FP			0x10
#define EXC_ALIGN		0x11
#define EXC_SYSCALL		0x80

// Hardware IRQs
#define IRQ_BASE		0x20
#define IRQ_COUNT		0x10
#define IRQ_TIMER		0
#define IRQ_KEYBOARD 	1

// Exception return codes
#define EXC_FAULT		0
#define EXC_OK  		1
#define EXC_TASK_SWITCH	2

// Timer
#define TIMER_FREQ		18		// 18.2, really

#ifndef __ASM__

// Exception parameter structure
typedef struct {
	uns4	ss;
	uns4	gs;
	uns4	fs;
	uns4	es;
	uns4	ds;
	uns4	edi;
	uns4	esi;
	uns4	ebp;
	uns4	esp;
	uns4	ebx;
	uns4	edx;
	uns4	ecx;
	uns4	eax;
	uns4	number;
	uns4	err_code;
	uns4	eip;
	uns4	cs;
	uns4	flags;
	uns4	oldesp;
	uns4	oldss;
} exc_status_t;

typedef int(exception_handler_t)(exc_status_t *status);

exception_handler_t	*trap_handlers[IDT_SIZE];
#endif

#endif
