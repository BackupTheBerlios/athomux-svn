// Author: Marcel Kilgus

#ifndef _X86_MEM_H
#define _X86_MEM_H

#define PAGE_SIZE 		4096
#define PAGE_SHIFT 		12
#define PAGE_DIR_SHIFT	22
#define PAGE_MASK		0xfffff000
#define PAGE_TAB_MASK	0x003ff000
#define PAGE_DIR_MASK	0xfc000000
#define PTRS_PER_PAGE	(PAGE_SIZE>>2)

#define MAP_NR(addr) ((addr) >> PAGE_SHIFT)
#define MAP_PAGE_RESERVED (1<<15)

#define PAGE_PRESENT	0x001	// Page is present in RAM
#define PAGE_RW			0x002	// Page is writable
#define PAGE_USER		0x004	// User mode code can access the page
#define PAGE_PWT		0x008	// Page caching
#define PAGE_PCD		0x010	// Page caching
#define PAGE_ACCESSED	0x020	// Flag read/write access
#define PAGE_DIRTY		0x040	// Flag write access
#define PAGE_COW		0x200	// Copy on write, implemented in software

#define PAGE_PRIVATE	(PAGE_PRESENT | PAGE_RW | PAGE_USER | PAGE_ACCESSED | PAGE_COW)
#define PAGE_SHARED		(PAGE_PRESENT | PAGE_RW | PAGE_USER | PAGE_ACCESSED)
#define PAGE_COPY		(PAGE_PRESENT | PAGE_USER | PAGE_ACCESSED | PAGE_COW)
#define PAGE_READONLY	(PAGE_PRESENT | PAGE_USER | PAGE_ACCESSED)
#define PAGE_TABLE		(PAGE_PRESENT | PAGE_RW | PAGE_USER | PAGE_ACCESSED)

#define PFAULT_PROTVIOL	0x01
#define PFAULT_WRITE	0x02
#define PFAULT_USERMODE 0x04

#ifndef __ASM__

#include "x86_lib.h"

extern paddr_t	kernel_pdir;	
extern unsigned short *mem_map;

enum copy_mode {
	NORMAL,
	COPY_ON_WRITE
};

paddr_t copy_page_tree(paddr_t tree, enum copy_mode mode);
void free_page_tree(paddr_t tree);
void init_memory(multiboot_info_t* mbi);

paddr_t get_page();
void free_page(paddr_t adr);
void show_page(paddr_t tree, paddr_t page, paddr_t virtual, int flags);
paddr_t get_table_entry(paddr_t tree, paddr_t virtual);
void set_table_entry(paddr_t tree, paddr_t virtual, paddr_t entry);
#endif

#endif
