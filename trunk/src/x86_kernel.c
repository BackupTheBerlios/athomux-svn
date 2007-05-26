/* Copyright (C) 1999  Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. */

// Author: Marcel Kilgus

#include "common.h"
#include "x86_lib.h"
#include "x86_kernel.h"
#include "x86_mem.h"
#include "control_simple.h"
#include <stdarg.h>

descriptor_t		gdt[GDT_SIZE];
descriptor_ptr_t 	gdt_ptr;
tss_t				base_tss;
int					gdt_count;
volatile int		shutdown = 0;
uns2 				apm_call_ptr[3];			// Far call pointer to apm entry point
extern 	task_t 		tasks[MAX_TASKS];


// Wrapper to hide the ugly mess in the segment descriptor data structure
void set_descriptor(uns4 index, uns4 base, uns4 len, int flags1, int flags2) 
{
	gdt[index].limit_lo = len & 0xffff;
	gdt[index].flags2 = (len >> 16) | flags2;
	gdt[index].base_lo = base & 0xffff;
	gdt[index].base_mid = base >> 16;
	gdt[index].base_hi = base >> 24;
	gdt[index].flags1 = flags1;

	if (index >= gdt_count)
		gdt_count = index + 1;
}


// Add a new segment to GDT, return corresponding segment selector
int add_descriptor(uns4 base, uns4 len, int flags1, int flags2) 
{
	int	seg = gdt_count * 8;
	set_descriptor(gdt_count, base, len, flags1, flags2);
	return seg;
}


// Initialise global descriptor table with flat segments and all segmentes needed
// for the advanced power management system
void init_GDT(multiboot_info_t *mbi) 
{
	uns2	seg_apm;
	apm_table_t *apm;

	memset(&gdt, 0, sizeof(descriptor_t) * GDT_SIZE);

	gdt_count = 0;
	set_descriptor(0, 0, 0, 0, 0);				// NULL entry
	set_descriptor(1, 0, 0xfffff, 0x9a, 0xc0);	// kernel code segment
	set_descriptor(2, 0, 0xfffff, 0x92, 0xc0);	// kernel data segment
	set_descriptor(3, 0, 0xbffff, 0xfa, 0xc0);	// user code segment
	set_descriptor(4, 0, 0xbffff, 0xf2, 0xc0);	// user data segment

	// Build descriptors that are needed by APM system
	if (CHECK_FLAG(mbi->flags, 10)) {
		apm = (apm_table_t*)mbi->apm_table;
		seg_apm = add_descriptor(apm->cseg << 4, apm->cseg_len - 1, 0x9a, 0x40);		// apm 32-bit code
		          add_descriptor(apm->cseg_16 << 4, apm->cseg_16_len - 1, 0x9a, 0x00);	// apm 16-bit code
		          add_descriptor(apm->dseg << 4, apm->dseg_len - 1, 0x92, 0x00);		// apm 16-bit data

		// Build FAR pointer to call into APM system
		apm_call_ptr[0] = apm->offset & 0xffff;
		apm_call_ptr[1] = apm->offset >> 16;
		apm_call_ptr[2] = seg_apm;
	} else {
		apm_call_ptr[0] = 0;
		apm_call_ptr[1] = 0;
		apm_call_ptr[2] = 0;
	}

	// GDT pointer structure
	gdt_ptr.len = GDT_SIZE * sizeof(descriptor_t);
	gdt_ptr.base = (uns4)PHYS_MEM(gdt);

	// Load new GDT and reset/flush all segment registers
	__asm__ (
		"lgdt	(gdt_ptr-0xC0000000)\n\t"
		"movw	%0,%%ax\n\t"			// entry 2 (data)
		"movw	%%ax,%%ds\n\t"
		"movw	%%ax,%%es\n\t"
		"movw	%%ax,%%fs\n\t"
		"movw	%%ax,%%gs\n\t"
		"movw	%%ax,%%ss\n\t"
		"ljmp	%1,$flush\n\t"			// entry 1 (code)
		"flush:"
	  : 
	  :	"g" (KERNEL_DS),
		"g" (KERNEL_CS)
	  : "ax"
	);
}


uns2 seg_tss;

// The processor requires at least one TSS
void init_TSS(void)
{
	memset(&base_tss, 0, sizeof(tss_t));
	
	base_tss.ss0 = KERNEL_DS;
	base_tss.esp0 = get_esp(); // only temporary

	seg_tss = add_descriptor((uns4)(&base_tss - KERNEL_VIRT_BASE), sizeof(base_tss) - 1, TSS_F1_DEFAULT, TSS_F2_DEFAULT);

	ltr(seg_tss);
}


// Tell APM to turn off the machine (will make QEMU exit nicely)
void __attribute__ ((noreturn)) apm_shutdown() 
{
	if (apm_call_ptr[0] | apm_call_ptr[1] | apm_call_ptr[2]) {
		__asm__ (
			"movw	$0x5303,%ax\n\t"			// Connect to APM
			"movw	$0x0000,%bx\n\t"
			"lcall	*(apm_call_ptr)\n\t"
			"movw	$0x5307,%ax\n\t"			// Initiate shutdown
			"movw	$0x0001,%bx\n\t"
			"movw	$0x0003,%cx\n\t"
			"lcall	*(apm_call_ptr)\n\t"
		);	
	}
	while (1);
}


void athomux_shutdown()
{
	kprintf("Shutdown requested!\n");
	shutdown = 1;
	tasks[0].state = TASK_RUNNABLE;
}

bool b_cmd(addr_t addr, char *fmt, char *param, ...) 
{
	va_list	va_args;
	char str[512];
	int err;

	va_start(va_args, param);
	err = vsnprintf(str, 511, fmt, va_args);
	va_end(va_args);
	if (err == 511) {
		errprintf("Buffer overrun in b_cmd!");
		exit(-1);
	}

	struct args args = { .log_addr = addr, .log_len = strlen(str), .phys_addr = (unsigned long)str, .direction = direct_write};
	root_strategy->ops[0][opcode_trans]((void*)root_strategy, &args, param);
	return args.success;
}


addr_t b_boot(const char *fmt, const char *param, ...) 
{
	va_list	va_args;
	char str[512];
	int err;

	va_start(va_args, param);
	err = vsnprintf(str, 511, fmt, va_args);
	va_end(va_args);
	if (err == 511) {
		errprintf("Buffer overrun in b_boot!");
		exit(-1);
	}

	struct args args = { .log_len = DEFAULT_TRANSFER, .reader = FALSE, .exclu = TRUE, .action = action_wait, .melt = TRUE, .try_len = DEFAULT_TRANSFER, .op_code = opcode_gadr , .success = FALSE};
	root_strategy->ops[0][opcode_gadr]((void*)root_strategy, &args, param);
	if (!args.success) {
		errprintf("b_boot: gadr failed!\n");
		exit(-1);
	}
	
	args.success = FALSE;
	args.log_len = strlen(str);
	args.phys_addr = (unsigned long)str;
	args.direction = direct_write;
	args.op_code = opcode_trans;
	root_strategy->ops[0][opcode_trans]((void*)root_strategy, &args, param);
	if (!args.success) {
		errprintf("b_boot: trans '%s' failed!\n", str);
		exit(-1);
	}
	return args.log_addr;
}
 

// Main function of the operating system. Woooohoooo!
void cmain(unsigned long magic, unsigned long mbi_addr) 
{
	int i;
	multiboot_info_t *mbi;

	cls();
	ser_init();
	kprintf("Athomux starting up...\n\n");

	// Am I booted by a Multiboot-compliant boot loader?
	if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
		errprintf("Invalid magic number: 0x%x\n", (unsigned) magic);
		return;
	}

	mbi = (multiboot_info_t*)mbi_addr;

//	print_multiboot_info(mbi);
	init_memory(mbi);
	init_GDT(mbi);
	kprintf("GDT set up\n");
	init_IDT();
	kprintf("IDT set up\n");
//	init_TSS();

	init_static_control_simple();

	addr_t control = DEFAULT_TRANSFER;
	b_cmd(control, "output:=control\n", "");
	addr_t map = b_boot("brick:=map_dummy\n output:=out{\n", "");
	addr_t meta = b_boot("brick:=adapt_meta\n connect in:=%llx:out\n output:=out\n", "", map);
	// create root hook
	b_cmd(0, "output:=_root", "");
	b_cmd(meta, "connect hook:=0:_root", "");

	addr_t cpu = b_boot("brick+=cpu_x86\n", "");
	// Initialise all IRQ outputs
	for (i = 0; i < 16; i++) {
		b_cmd(cpu, "output:=irq[%i]\n", "", i);
	}
	b_cmd(cpu, "output:=int80\n", "", i);
#if 0
	addr_t irq = b_boot("brick:=demo_irq\n", "", cpu);
	b_cmd(irq, "connect key:=%llx:irq[1]\n", "", cpu);
	b_cmd(irq, "connect timer:=%llx:irq[0]\n", "", cpu);

	addr_t screen = b_boot("brick:=char_device_screen\n output:=screen", "");
	addr_t test = b_boot("brick:=demo_screen\n connect test:=%llx:fd\n output:=dummy\n", "", screen);
	b_cmd(screen, "connect ", "");
#endif

	// strategy level
	addr_t fs = b_boot("brick:=fs_simple\n connect strat:=%llx:control\n output:=control{\n", "_root", control);
	addr_t strategy = b_boot("brick:=strategy_native\n"\
						     " connect strat:=%llx:control\n"\
						     " connect fs:=%llx:control", "", control, fs, cpu);

	char addr[32];
	sprintf(addr, "%llx %llx", strategy, cpu);
	b_cmd(strategy, "output:=posix\n", addr);
	b_cmd(strategy, "output:=dummy\n", addr);

	tasks[0].state = TASK_WAITING;
	while (!shutdown) {};

	// Shut off PC (or QEMU!)
	kprintf("Athomux shutting down in ");
	for (i = 5; i > 0; i--) {
		kprintf("%i...", i);
		ath_sleep(1000);
	}
	kprintf("\n");
	apm_shutdown();
}


void abort(void)
{
	errprintf("FATAL: abort() called, Athomux shutting down...\n");
	apm_shutdown();
}

void exit(int code)
{
	errprintf("FATAL: exit(%i) called, Athomux shutting down...\n", code);
	apm_shutdown();
}
