// Author: Marcel Kilgus

#ifndef _X86_KERNEL_H
#define _X86_KERNEL_H

#define GDT_SIZE	164
#define MAX_TASKS	128

#define KERNEL_CS	0x0008
#define KERNEL_DS	0x0010
#define USER_CS		0x0018
#define USER_DS		0x0020

#define ONE_MEGABYTE 0x100000

#define KERNEL_PHYS_BASE 	0x100000
#define KERNEL_VIRT_BASE 	0xC0000000
#define KERNEL_VIRT_PDOFF	0xC00		// KERNEL_VIRT_BASE entry in page directory
#define KERNEL_VIRT_PDINDX	(KERNEL_VIRT_PDOFF >> 2)
#define KERNEL_CODE_SPACE	(2*ONE_MEGABYTE)
#define KERNEL_DATA 		(KERNEL_PHYS_BASE+KERNEL_CODE_SPACE)
#define KERNEL_DATA_SPACE 	(10*ONE_MEGABYTE)
#define KERNEL_SPACE 		(KERNEL_DATA_SPACE+KERNEL_CODE_SPACE)
#define PAGE_OFFSET			KERNEL_VIRT_BASE

// Space occupied by kernel rounded up to the next 4MB barrier
#define KERNEL_MAPPED_SPACE ((KERNEL_PHYS_BASE+KERNEL_SPACE+4*ONE_MEGABYTE-1)&~(4*ONE_MEGABYTE))

#define VIRT_MEM(x)			((void*)((unsigned int)(x) + PAGE_OFFSET))
#define PHYS_MEM(x)			((void*)((unsigned int)(x) - PAGE_OFFSET))

#define BOOT_PAGE_DIR		0x10000

#ifndef __ASM__
int add_descriptor(uns4 base, uns4 len, int flags1, int flags2);
void athomux_shutdown();

extern uns2 seg_tss;
extern tss_t base_tss;
extern descriptor_t		gdt[GDT_SIZE];

extern uns2 task_switch_segment;

#endif

#endif
