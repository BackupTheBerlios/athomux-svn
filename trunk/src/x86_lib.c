// Support routines for x86 Athomux implementation
// Author: Marcel Kilgus

#include "x86_lib.h"
#include "common.h"

// Some screen stuff
#define COLUMNS                 80
#define LINES                   25
#define ATTRIBUTE               7
#define VIDEO                   0xB8000

static int xpos;
static int ypos;
static volatile unsigned char *video;

struct _IO_FILE *stderr;

int	ser_port = 0x3F8;


// Hardware port access functions
inline void outportb(int port, int value) 
{
	__asm__ (
		"outb %%al,%%dx"
	  : 
	  :	"a" (value), "d" (port)
	);
}

inline void outportw(int port, int value) 
{
	__asm__ (
		"outw %%ax,%%dx"
	  : 
	  :	"a" (value), "d" (port)
	);
}

inline unsigned char inportb(int port) 
{
	unsigned char result;
	__asm__ (
		"inb %%dx,%%al"
	  : "=a" (result)
	  :	"d" (port)
	);
	return result;
}

inline unsigned short inportw(int port) 
{
	unsigned short result;
	__asm__ (
		"inw %%dx,%%ax"
	  : "=a" (result)
	  :	"d" (port)
	);
	return result;
}


void ser_init(void)
{
	outportb(ser_port + 3, 3);		// 8bit, 1 stop bit, no parity
	// Enable divisor control
	outportb(ser_port + 3, inportb(ser_port + 3) | 0x80);
	// Set baud rate to 115200
	outportb(ser_port + 1, 0);
	outportb(ser_port, 1);
	// Disable divisor control
	outportb(ser_port + 3, inportb(ser_port + 3) & 0x7f);
}


// Send byte to COM1 (presumably, fixed port number so far)
void ser_putchar(int c) 
{
	inportb(ser_port + 6);
	while (!(inportb(ser_port + 5) & 32)) {}
	outportb(ser_port, c);
}


// Use bochs' internal debugger interface to break into debugger
inline void bochs_bp(void) 
{
	__asm__ __volatile__ (
		"movw $0x8a00,%%dx\n\t"
		"movw $0x8a00,%%ax\n\t"
		"outw %%ax,%%dx\n\t"
		"movw $0x8ae0,%%ax\n\t"
		"outw %%ax,%%dx\n\t"
	  :
	  :
	  : "%ax","%dx"
	);
}


inline void tracepoint(void) 
{
	__asm__ (
		"pushf	\n\t"
		"pop	%ax\n\t"
		"orw	$0x0100,%ax\n\t"
		"push	%ax\n\t"
		"popf"
	);
}

/* Clear the screen and initialize VIDEO, XPOS and YPOS. */
void cls(void) 
{
	int i;

	video = (unsigned char *) VIDEO;

	for (i = 0; i < COLUMNS * LINES * 2; i++)
	*(video + i) = 0;

	xpos = 0;
	ypos = 0;
}


// Sleep a certain amount of milliseconds
void ath_sleep(long delay) 
{
//	long time = ticker;
//	long idelay = delay / (1000 / 18);
//	while (ticker - time < idelay);
}


// Put the character C on the screen.
static void scr_putchar(int c)
{
	static int nl_pending = 0;
	static int nl_wrap = 0;

	if (c == '\r') return;

	// Only execute new line if something else gets printed (otherwise last line
	// could not be filled)
	if (nl_pending) {
		xpos = 0;
		ypos++;
		if (ypos >= LINES) {
			// Scroll screen
			memmove((void*)video, (void*)video + COLUMNS * 2, (LINES - 1) * COLUMNS * 2);
			memset((void*)video + (LINES - 1) * COLUMNS * 2, 0, COLUMNS * 2);
			ypos = LINES - 1;
		}
		nl_pending = 0;
		if (c == '\n' && nl_wrap) return;
	}

	if (c == '\n') {
		nl_pending = 1;
		nl_wrap = 0;
		return;
	}

	*(video + (xpos + ypos * COLUMNS) * 2) = c & 0xFF;
	*(video + (xpos + ypos * COLUMNS) * 2 + 1) = ATTRIBUTE;

	xpos++;
	if (xpos >= COLUMNS) {
		nl_pending = 1;
		nl_wrap = 1;
	}
}


int	console_putchar(int c) 
{
//	scr_putchar(c);
	ser_putchar(c);
	return 1;
}


int	console_putbytes(const char *s, int len)
{
	int i;
	for (i = 0; i < len; i++) {
		console_putchar(s[i]);
	}
	return i;
}


int puts(const char *s) 
{
	console_putbytes(s, strlen(s));
	console_putchar(10);
	return 1;
}


int fputs(const char *s, FILE *f) 
{
	puts(s);
	return 1;
}

// Debugging: write out data structures in the multiboot header
void print_multiboot_info(multiboot_info_t *mbi)
{
	kprintf("********************************************************************************\n");
	kprintf("* Multiboot data structure\n*\n");

	// Flags.
	kprintf("* flags = 0x%x\n", (unsigned) mbi->flags);

	// Memory constraints
	if (CHECK_FLAG(mbi->flags, 0))
		kprintf("* mem_lower = %uKB, mem_upper = %uKB\n",
			(unsigned) mbi->mem_lower, (unsigned) mbi->mem_upper);

	// Boot device
	if (CHECK_FLAG(mbi->flags, 1))
		kprintf("* boot_device = 0x%x\n", (unsigned) mbi->boot_device);

	// Command line
	if (CHECK_FLAG(mbi->flags, 2))
		kprintf("* cmdline = %s\n", (char *) mbi->cmdline);

	// Modules
	if (CHECK_FLAG(mbi->flags, 3)) {
		module_t *mod;
		int i;

		kprintf("* mods_count = %d, mods_addr = 0x%x\n",
			(int) mbi->mods_count, (int) mbi->mods_addr);
		for (i = 0, mod = (module_t *) mbi->mods_addr;
			i < mbi->mods_count;
			i++, mod += sizeof (module_t))
		kprintf("*  mod_start = 0x%x, mod_end = 0x%x, string = %s\n",
			(unsigned) mod->mod_start,
			(unsigned) mod->mod_end,
			(char *) mod->string);
	}

	// Bits 4 and 5 are mutually exclusive!
	if (CHECK_FLAG(mbi->flags, 4) && CHECK_FLAG (mbi->flags, 5)) {
		kprintf("* Both bits 4 and 5 are set.\n");
		return;
	}

	// a.out symbol table
	if (CHECK_FLAG(mbi->flags, 4)) {
		aout_symbol_table_t *aout_sym = &(mbi->u.aout_sym);

		kprintf("* aout_symbol_table: tabsize = 0x%0x, strsize = 0x%x, addr = 0x%x\n",
			(unsigned) aout_sym->tabsize,
			(unsigned) aout_sym->strsize,
			(unsigned) aout_sym->addr);
	}

	// ELF section header table
	if (CHECK_FLAG(mbi->flags, 5)) {
		elf_section_header_table_t *elf_sec = &(mbi->u.elf_sec);

		kprintf("* elf_sec: num = %u, size = 0x%x, addr = 0x%x, shndx = 0x%x\n",
			(unsigned) elf_sec->num, (unsigned) elf_sec->size,
			(unsigned) elf_sec->addr, (unsigned) elf_sec->shndx);
	}

	// Memory map
	if (CHECK_FLAG(mbi->flags, 6)) {
		memory_map_t *mmap;

		kprintf("* mmap_addr = 0x%x, mmap_length = 0x%x\n",
			(unsigned) mbi->mmap_addr, (unsigned) mbi->mmap_length);
		for (mmap = (memory_map_t *) mbi->mmap_addr;
			(unsigned long) mmap < mbi->mmap_addr + mbi->mmap_length;
			mmap = (memory_map_t *) ((unsigned long) mmap
                                  + mmap->size + sizeof (mmap->size)))
		{
			kprintf("*  size = 0x%x, base_addr = 0x%x%x, length = 0x%x%x, type = 0x%x\n",
				(unsigned) mmap->size,
				(unsigned) mmap->base_addr_high,
				(unsigned) mmap->base_addr_low,
				(unsigned) mmap->length_high,
				(unsigned) mmap->length_low,
				(unsigned) mmap->type);
		}
	}

	// Drives
	if (CHECK_FLAG(mbi->flags, 7)) {
		drives_map_t *dmap;

		kprintf("* drives_addr = 0x%x, drives_length = 0x%x\n",
			(unsigned)mbi->drives_addr, (unsigned)mbi->drives_length);
		for (dmap = (drives_map_t*)mbi->drives_addr;
			(unsigned long)dmap < mbi->drives_addr + mbi->drives_length;
			dmap += dmap->size)
		{
			kprintf("*  size = 0x%x, number = 0x%x, mode = 0x%x, cyls = 0x%x, heads = 0x%x, secs = 0x%x",
				(unsigned)dmap->size, dmap->drive_number, dmap->drive_mode, 
				dmap->drive_cylinders, dmap->drive_heads, dmap->drive_sectors);
		}
	}

	// Config table
	if (CHECK_FLAG(mbi->flags, 8)) {
		kprintf("* config_table = 0x%x\n", (unsigned)mbi->config_table);
	}

	// Boot loader name
	if (CHECK_FLAG(mbi->flags, 9)) {
		kprintf("* boot_loader_name = \"%s\"\n", (char*)mbi->boot_loader_name);
	}

	// APM stuff
	if (CHECK_FLAG(mbi->flags, 10)) {
		apm_table_t *apm_table = (apm_table_t*)mbi->apm_table;
		kprintf("* apm_table = 0x%x, version = 0x%x, flags = 0x%x\n", 
			(unsigned) mbi->apm_table, apm_table->version, apm_table->flags);
		kprintf("*  cseg = 0x%x, cseg_len = 0x%x, offset = 0x%x\n", 
			apm_table->cseg, apm_table->cseg_len, (unsigned)apm_table->offset);
		kprintf("*  cseg_16 = 0x%x, cseg_16_len = 0x%x\n",
			apm_table->cseg_16, apm_table->cseg_16_len);
		kprintf("*  dseg = 0x%x, dseg_len = 0x%x\n",
			apm_table->dseg, apm_table->dseg_len);
	}

	// Graphicsmode
	if (CHECK_FLAG(mbi->flags, 11)) {
		kprintf("* Graphics table!\n");
	}
	kprintf("********************************************************************************\n");
}

