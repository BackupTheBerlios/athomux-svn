// Memory management routines
// Author: Marcel Kilgus
				  
#include "x86_lib.h"
#include "x86_kernel.h"
#include "x86_mem.h"
#include "x86_malloc.h"

paddr_t mem_base;
paddr_t	mem_end;
size_t	mem_size;
size_t	sbrk_size;
unsigned short *mem_map;

unsigned long free_page_count = 0;
paddr_t free_page_list = 0;

// Kernel page directory
// 
// This is a 1:1 identity map of linear and physical memory.
// Except that the memory up to KERNEL_MAPPED_SPACE is also mapped to 
// 0xC0000000 (kernel space)
paddr_t	kernel_pdir;	


// Allocate a page from free page list (single linked) 
// The result address is the PHYSICAL address of the page
paddr_t get_page()
{
	paddr_t tmp = free_page_list;
	int flags;

	save_flags(flags);
	cli();
	if (!free_page_count || !free_page_list) return 0;
	free_page_list = *(paddr_t*) VIRT_MEM(tmp);

	assert((free_page_list & ~PAGE_MASK) == 0);
	assert(mem_map[MAP_NR(tmp)] == 0);

//	kprintf("Allocated page 0x%x\n", (unsigned int)tmp);
	mem_map[MAP_NR(tmp)] = 1;
	restore_flags(flags);
	return tmp;
}


// Put page back into free page list if usage count is 0
// adr is the PHYSICAL address
void free_page(paddr_t adr)
{
	int flags;

	save_flags(flags);
	cli();
	unsigned short nr = mem_map[MAP_NR(adr)];
//	kprintf("Freeing page 0x%x (mem_map %i)\n", (unsigned int)adr, nr);

	assert(adr < mem_end);
	assert(nr != MAP_PAGE_RESERVED);
	assert(nr > 0);

	mem_map[MAP_NR(adr)] = --nr;
	if (!nr) {
		*(paddr_t*)VIRT_MEM(adr) = free_page_list;
		free_page_list = adr;
	}
	restore_flags(flags);
}


// This returns the PHYSICAL address of the new page
paddr_t kmalloc_page()
{
	paddr_t p = get_page();
	if (!p) {
		errprintf("Fatal, cannot allocate kernel page!\n");
		abort();
	}
	memset(VIRT_MEM(p), 0, PAGE_SIZE);
	return p;
}


// Make page visible at specified virtual address of the given page tree
// If tree is 0, then the current page directory will be used
void show_page(paddr_t tree	, paddr_t page, paddr_t virtual, int flags)
{
	assert((page & ~PAGE_MASK) == 0);
	assert((virtual & ~PAGE_MASK) == 0);
	paddr_t *p_dir = (paddr_t*)VIRT_MEM(tree);
	if (tree == 0) p_dir = (paddr_t*)VIRT_MEM(get_cr3());

	paddr_t *p_table = (paddr_t*)VIRT_MEM(p_dir[virtual >> PAGE_DIR_SHIFT] & PAGE_MASK);
	if (!PHYS_MEM(p_table)) {
		p_table = (paddr_t*)VIRT_MEM(kmalloc_page());	
		p_dir[virtual >> PAGE_DIR_SHIFT] = (paddr_t)PHYS_MEM(p_table) | PAGE_TABLE;
	}
	p_table[(virtual & PAGE_TAB_MASK) >> PAGE_SHIFT] = page | flags;
}


// Get one entry out of a page tree (0 for current tree)
paddr_t get_table_entry(paddr_t tree, paddr_t virtual) {
	paddr_t *dir = (paddr_t*)VIRT_MEM(tree);
	if (tree == 0) dir = (paddr_t*)VIRT_MEM(get_cr3());
	paddr_t *table = (paddr_t*)VIRT_MEM(dir[virtual >> PAGE_DIR_SHIFT] & PAGE_MASK);
	if (!PHYS_MEM(table)) return 0;
	return (paddr_t)table[(virtual & PAGE_TAB_MASK) >> PAGE_SHIFT];
}


// Set one entry out of a page tree (0 for current tree)
void set_table_entry(paddr_t tree, paddr_t virtual, paddr_t entry) {
	paddr_t *dir = (paddr_t*)VIRT_MEM(tree);
	if (tree == 0) dir = (paddr_t*)VIRT_MEM(get_cr3());
	paddr_t *table = (paddr_t*)VIRT_MEM(dir[virtual >> PAGE_DIR_SHIFT] & PAGE_MASK);
	if (!PHYS_MEM(table)) return;
	table[(virtual & PAGE_TAB_MASK) >> PAGE_SHIFT] = entry;
}


// Copy whole page tree, allocating new pages as needed
paddr_t copy_page_tree(paddr_t tree, enum copy_mode mode)
{
	paddr_t *s_dir;
	paddr_t *d_dir;
	paddr_t *s_table;
	paddr_t *d_table;
	int i, j;
	int flags;

	save_flags(flags);
	cli();
	s_dir = (paddr_t*)VIRT_MEM(tree);
	d_dir = (paddr_t*)VIRT_MEM(kmalloc_page());	
	for (i = 0; i < PTRS_PER_PAGE; i++) {
		s_table = (paddr_t*)VIRT_MEM(s_dir[i] & PAGE_MASK);
		if (PHYS_MEM(s_table)) {
			d_table = (paddr_t*)VIRT_MEM(kmalloc_page());
			for (j = 0; j < PTRS_PER_PAGE; j++) {
				if (!s_table[j]) continue;
				if (mode == COPY_ON_WRITE) {
					// Mark pages as read only and for our copy-on-write mechanism
					s_table[j] = (s_table[j] & ~PAGE_RW) | PAGE_COW;
				}
				d_table[j] = s_table[j];
				// Increase usage counter for page
				int index = MAP_NR(s_table[j] & PAGE_MASK);
				if (mem_map[index] != MAP_PAGE_RESERVED && i < KERNEL_VIRT_PDINDX) {
					mem_map[index]++;
				}
			}
			d_dir[i] = (paddr_t)PHYS_MEM(d_table) | (s_dir[i] & ~PAGE_MASK);
		} else {
			d_dir[i] = 0;
		}
	}
	restore_flags(flags);
	return (paddr_t)PHYS_MEM(d_dir);
}


// Free whole page tree
void free_page_tree(paddr_t tree)
{
	paddr_t *s_dir;
	paddr_t *s_table;
	int i, j;
	int flags;

	save_flags(flags);
	cli();
	s_dir = (paddr_t*)VIRT_MEM(tree);
	for (i = 0; i < PTRS_PER_PAGE; i++) {
		s_table = (paddr_t*)VIRT_MEM(s_dir[i] & PAGE_MASK);
		if (PHYS_MEM(s_table)) {
			for (j = 0; j < PTRS_PER_PAGE; j++) {
				paddr_t tmp = s_table[j] & PAGE_MASK;
				unsigned short map = mem_map[MAP_NR(tmp)];
				if (tmp && i < KERNEL_VIRT_PDINDX && map != MAP_PAGE_RESERVED) {
//					kprintf("Trying to free 0x%x, entry at 0x%x\n", 
//						(unsigned int)tmp, (unsigned int)&s_table[j]);
					free_page((paddr_t)tmp);
				}
			}
			free_page((paddr_t)PHYS_MEM(s_table));
		}
	}
	free_page((paddr_t)PHYS_MEM(s_dir));
//	kprintf("Stuff freed!\n");
	restore_flags(flags);
}


// Extend minimal page table we already created in x86_boot.S
// The first GB is identity mapped into both address 0 and KERNEL_VIRT_BASE
void init_paging() 
{
	paddr_t *pg_dir;
	paddr_t *pg_table;
	paddr_t tmp;
	paddr_t adr;
	paddr_t next_pg_table;

	// Allocate kernel page directory
	kernel_pdir = BOOT_PAGE_DIR;
	next_pg_table = kernel_pdir + PAGE_SIZE;
	pg_dir = (paddr_t*)kernel_pdir + KERNEL_VIRT_PDINDX;

	// Map all of memory to kernel space 0xC0000000
	adr = 0;
	while (adr < mem_end) {
		pg_table = (paddr_t*)next_pg_table;
		next_pg_table += PAGE_SIZE;
		*(pg_dir++) = (unsigned long)pg_table | PAGE_TABLE;

		// Fill table
		for (tmp = 0; tmp < PTRS_PER_PAGE; tmp++, pg_table++) {
			if (adr < mem_end) {
				*pg_table = adr | PAGE_SHARED;
			} else {
				*pg_table = 0;
			}
			adr += PAGE_SIZE;
		}
	}

	// Unmap the first 4 MB that was mapped during boot
	// Also unmaps the very first page which is the NULL pointer catcher
	// Unmap the first page (NULL ptr catcher)
	pg_dir = (paddr_t*)VIRT_MEM(kernel_pdir);
	pg_table[0] = 0;

	tlb_invalidate();
}

// Currently we just look for the biggest chunk available, which is usually
// all memory above the 1MB barrier. All other memory remains unused.
// Also, we only support 4GB.
void init_memory(multiboot_info_t* mbi) 
{
	memory_map_t *mmap;
	unsigned long adr;

	mem_size = 0;
	for (mmap = (memory_map_t *) mbi->mmap_addr;
		(unsigned long) mmap < mbi->mmap_addr + mbi->mmap_length;
		mmap = (memory_map_t *) ((unsigned long) mmap
                                 + mmap->size + sizeof (mmap->size)))
	{
		if (mmap->type == 1 && mmap->length_low > mem_size) {
			mem_base = mmap->base_addr_low;
			mem_size = mmap->length_low;
		}
	}
	// GRUB can only load modules above the 1MB barrier, therefore we link 
	// our code to address 0x100000. So we need to move the base of our memory
	// up if it's there
	if (mem_base >= KERNEL_PHYS_BASE && mem_base <= KERNEL_DATA) {
		paddr_t mem_overlap = KERNEL_DATA - mem_base;
		if (mem_size < mem_overlap) abort();
		mem_base += mem_overlap;
		mem_size -= mem_overlap;
	}	
	// There should be at least one more megabyte
	if (mem_size < ONE_MEGABYTE) abort();

	mem_end = mem_base + mem_size;
	if (mem_end > (unsigned long)ONE_MEGABYTE * 1024) {
		errprintf("This operating system only supports 1GB of RAM!\n");
		mem_end = (unsigned long)ONE_MEGABYTE * 1024;
		mem_size = mem_end - mem_base;
	}
	kprintf("kernel_code_base 0x%x, kernel_data_base 0x%x, kernel_data_size 0x%x\n",
		KERNEL_PHYS_BASE, KERNEL_DATA, KERNEL_DATA_SPACE);
	kprintf("unused memory space base 0x%x, size 0x%x\n", 
		KERNEL_DATA+KERNEL_DATA_SPACE, mem_size - KERNEL_DATA_SPACE);

	// Setup up complete page tables
	init_paging();
	kprintf("paging tables set up...\n");

	// Now build memory map
	mem_map = calloc(MAP_NR(mem_end), sizeof(short));

	// First MB and kernel mapped space is reserved
	for (adr = 0; adr < KERNEL_MAPPED_SPACE; adr += PAGE_SIZE) {
		mem_map[MAP_NR(adr)] = MAP_PAGE_RESERVED;
	}

	// Build free page linked list
	free_page_count = 0;
	free_page_list = 0;
	for (adr = 0; adr < mem_end; adr += PAGE_SIZE) {
		if (mem_map[MAP_NR(adr)]) continue;

		*(unsigned long *) VIRT_MEM(adr) = free_page_list;
		free_page_list = adr;
		free_page_count++;
	}
	kprintf("memory initialised...\n");
}


// Very simply sbrk() implementation so the malloc code can be used pretty
// much unchanged.
void* sbrk(size_t increment) 
{
	paddr_t base;
	if (KERNEL_DATA_SPACE - sbrk_size < increment) {
		errprintf("sbrk: out of memory (requested 0x%x)\n", increment);
		return (void*)-1;		// Out of memory
	}
	base = mem_base + sbrk_size;
//	kprintf("sbrk: increment = 0x%x, old_size = 0x%x, new_size = 0x%x, new base = 0x%x\n", 
//		increment, sbrk_size, sbrk_size + increment, (unsigned int)base);
	sbrk_size += increment;
	return VIRT_MEM(base);
}


int getpagesize(void)
{
	return PAGE_SIZE;
}


// C library memory routines
void bcopy(const void *src2, void *dest2, size_t len)
{
	char *src = (char*)src2;
	char *dest = (char*)dest2;

	if (dest < src)
		while (len--)
			*dest++ = *src++;
	else {
		char *lasts = src + (len-1);
		char *lastd = dest + (len-1);
		while (len--)
			*(char *)lastd-- = *(char *)lasts--;
	}
}


void* memcpy(void *out, const void *in, size_t length) 
{
	bcopy(in, (void*)out, length);
	return out;
}


void *memmove(void *s1, const void *s2, size_t n)
{
	bcopy(s2, s1, n);
	return s1;
}


void* memset(void *dest, register int val, register size_t len) 
{
	register unsigned char *ptr = (unsigned char*)dest;
	while (len-- > 0)
		*ptr++ = val;
	return dest;
}
