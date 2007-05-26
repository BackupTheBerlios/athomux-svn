// Interrupt initialisation
// Author: Marcel Kilgus

#include <x86_lib.h>
#include <x86_kernel.h>
#include <x86_irq.h>

idt_t				idt[IDT_SIZE];
descriptor_ptr_t	idt_ptr;

void exc_0x00(void);


int unhandled_exception(exc_status_t *s)
{
	errprintf("Unhandled exception 0x%02x at address 0x%08x (error code 0x%08x)\n\n", s->number, s->eip, s->err_code);
	errprintf("eax=0x%08x  ebx=0x%08x  ecx=0x%08x  edx=0x%08x\n", s->eax, s->ebx, s->ecx, s->edx);
	errprintf("esi=0x%08x  edi=0x%08x  ebp=0x%08x  esp=0x%08x\n", s->esi, s->edi, s->ebp, s->oldesp);
	errprintf("cs=0x%04x  ds=0x%04x  es=0x%04x  ss=0x%04x  fs=0x%04x  gs=0x%04x\n",  s->cs, s->ds, s->oldss, s->es, s->fs, s->gs);
	errprintf("eflags=0x%08x\n\n", s->flags);
	abort();
	return 0;
}


void set_IDT(int index, void *base, short sel, int flags)
{
	idt[index].base_lo = (unsigned int)base & 0xffff;
	idt[index].base_hi = (unsigned int)base >> 16;
	idt[index].sel = sel;
	idt[index].flags = flags;
}


void init_IDT()
{
	int i;
	memset(&idt, 0, sizeof(idt_t) * IDT_SIZE);

	for (i = 0; i < IDT_SIZE; i++) {
		set_IDT(i, exc_0x00 + i * 16, KERNEL_CS, IDTF_INT_GATE);
	}
	
	// INT 0x80 is special (gate from user mode level 3)
	set_IDT(0x80, exc_0x00 + 0x80 * 16, KERNEL_CS, IDTF_INT_GATE | (IDTF_DPL * 3));

	// Reprogram PIC1 so it doesn't overlapp Intel's exception vectors anymore
	// Reprogram PIC2 so it is right after PIC1
	outportb(0x20, 0x11);
	outportb(0x21, IRQ_BASE);
	outportb(0x21, 0x04);
	outportb(0x21, 0x01);
	outportb(0xA0, 0x11);
	outportb(0xA1, IRQ_BASE+8);
	outportb(0xA1, 0x02);
	outportb(0xA1, 0x01);

	memset(trap_handlers, 0, sizeof(trap_handlers));

	// GDT pointer structure
	idt_ptr.len = IDT_SIZE * sizeof(idt_t);
	idt_ptr.base = (uns4)idt;

	__asm__ (
		"lidt	idt_ptr\n\t"
	);
}
