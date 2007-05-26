// Author: Marcel Kilgus

#ifndef _X86_LIB_H
#define _X86_LIB_H

#include <common.h>
#include <multiboot.h>

#define CHECK_FLAG(flags,bit)   ((flags) & (1 << (bit)))

// The macro for kernel error messages
#define errprintf(...) printf(__VA_ARGS__)

// The macro for kernel debugging messages
#define kprintf(...) printf(__VA_ARGS__)

// EFLAGS register
#define FLAG_CARRY	0x000001		// Carry
#define FLAG_PARITY	0x000004		// Parity
#define FLAG_AUX	0x000010		// Auxiliary
#define FLAG_ZERO	0x000040		// Zero
#define FLAG_SIGN	0x000080		// Sign
#define FLAG_TRAP	0x000100		// Trap
#define FLAG_INT	0x000200		// Interrupt enable
#define FLAG_DIR	0x000400		// Direction
#define FLAG_OVER	0x000800		// Overflow
#define FLAG_IOPL	0x001000		// I/O priviledge level
#define FLAG_NT		0x004000		// Nested task
#define FLAG_RESUME	0x010000		// Resume
#define FLAG_VM		0x020000		// Virtual 8086 mode
#define FLAG_ALIGN	0x040000		// Alignment check
#define FLAG_VIF	0x080000		// Virtual interrupt
#define FLAG_VIP	0x100000		// Virtuel interrupt pending
#define FLAG_ID		0x200000		// ID

// Segment register
#define SEG_RPL0	0
#define SEG_RPL3	3
#define SEG_LDT		4

#pragma pack(push, 1)
// Segment descriptor
typedef struct {
	uns2	limit_lo;
	uns2	base_lo;
	uns1	base_mid;
	uns1	flags1;
	uns1	flags2;
	uns1	base_hi;
} descriptor_t;

// Interrupt descriptor
#define IDTF_TASK_GATE	0x85
#define IDTF_INT_GATE	0x8E
#define IDTF_TRAP_GATE	0x8F
#define IDTF_DPL		0x20
typedef struct {
    uns2	base_lo;
    uns2	sel;
    uns1	always0;
    uns1	flags;
    uns2	base_hi;
} idt_t;

// Descriptor table pointer
typedef struct {
	uns2	len;
	uns4	base;
} descriptor_ptr_t;

// Task segment
#define TSS_F1_BUSY		0x02
#define TSS_F1_DPL		0x20
#define TSS_F1_PRESENT	0x80
#define TSS_F1_DEFAULT	0x89

#define TSS_F2_LIMIT	0x0F
#define TSS_F2_AVL		0x10
#define TSS_F2_GRAN_4K	0x80
#define TSS_F2_DEFAULT	TSS_F2_GRAN_4K

typedef struct {
	uns2	back_link, z0;		// segment number of previous task, if nested
	uns4	esp0;				// initial stack pointer ...
	uns2	ss0, z1;			// and segment for ring 0
	uns4	esp1;				// initial stack pointer ...
	uns2	ss1, z2;			// and segment for ring 1
	uns4	esp2;				// initial stack pointer ...
	uns2	ss2, z3;			// and segment for ring 2
	uns4	cr3;				// CR3 - page table directory physical address
	uns4	eip;
	uns4	eflags;
	uns4	eax;
	uns4	ecx;
	uns4	edx;
	uns4	ebx;
	uns4	esp;				// current stack pointer
	uns4	ebp;
	uns4	esi;
	uns4	edi;
	uns2	es, z4;
	uns2	cs, z5;
	uns2	ss, z6;				// current stack segment
	uns2	ds, z7;
	uns2	fs, z8;
	uns2	gs, z9;
	uns2	ldt, z10;			// local descriptor table segment
	uns2	trace_trap;			// trap on switch to this task
	uns2	io_bit_map_offset;	// offset to start of IO permission bit map
} tss_t;

typedef struct {
	uns4	cwd;
	uns4	swd;
	uns4	twd;
	uns4	fip;
	uns4	fcs;
	uns4	foo;
	uns4	fos;
	uns4	st_space[20];		// 8*10 bytes for each FP-reg = 80 bytes
} i387_t;

typedef uns4	athpid_t;

#define TASK_NOT_RUNNABLE	0
#define TASK_RUNNABLE		1
#define TASK_WAITING		2
#define TASK_STOPPED		3

// Actual task control structure
typedef struct {
	volatile long 	state;		// See TASK_ constants
	long 			priority;	

	int				next_task;	// Index in task list of next task
	int				prev_task;	// Index in task list of previous task
	int				child_task; // Index of first child
	int				sibling_task;// Index of next sibling
	athpid_t 		pid;		// Process ID. 0 = unused slot. LSW = tag, MSW = table index
	void			*mmu_brick;	// Pointer to MMU brick responsible for this task
	void			*posix_task;// POSIX task structure

	uns2  			tss_seg;	// Task state segment selector
	tss_t 			tss;		// Task state
	i387_t			i387;		// FPU state
//	int				used_math;	// Task did use the FPU once
} task_t;

#define PID_TASK_INDEX(x)	(x >> 16)

#define INIT_TASK_PID		1

// Scheduler status
typedef struct {
	int 	task_count;			// Number of tasks
	int 	next_tag;			// Tag counter to make pids more unique
	int		current_task;		// Current running task index
//	int		last_math_task;		// Last task that used the FPU
} sched_t;

#pragma pack(pop)

extern volatile unsigned int ticker;

inline void outportb(int port, int value);
inline void outportw(int port, int value);
inline unsigned char inportb(int port);
inline unsigned short inportw(int port);

void ser_init(void);
void cls(void);
void print_multiboot_info(multiboot_info_t *mbi);
void ath_sleep(long delay);

void init_IDT(void);

// Assembler utility routines
#define sti() __asm__ __volatile__ ("sti": : :"memory")
#define cli() __asm__ __volatile__ ("cli": : :"memory")
#define nop() __asm__ __volatile__ ("nop")

#define save_flags(x) \
__asm__ __volatile__("pushfl ; popl %0":"=r" (x): /* no input */ :"memory")

#define restore_flags(x) \
__asm__ __volatile__("pushl %0 ; popfl": /* no output */ :"r" (x):"memory")

#define tlb_invalidate() \
__asm__ __volatile__("movl %%cr3,%%eax\n\tmovl %%eax,%%cr3": : :"ax")

#define set_cr3(x) \
__asm__ __volatile__("movl %0,%%cr3"::"r" (x))

#define str(x) \
__asm__ ("str	%0":"=r" (x)::"memory")

#define ltr(x) \
__asm__ ("ltr	%0"::"r" (x):"memory")

#define get_cr2() \
	({ \
	register unsigned int _temp__; \
	asm("movl %%cr2, %0" : "=r" (_temp__)); \
	_temp__; \
	})

#define get_cr3() \
	({ \
	register unsigned int _temp__; \
	asm("movl %%cr3, %0" : "=r" (_temp__)); \
	_temp__; \
	})

#define get_esp() \
    ({ \
	register unsigned int _temp__; \
	asm("movl %%esp, %0" : "=r" (_temp__)); \
	_temp__; \
    })

#define assert(x) \
	if (!(x)) { \
		printf("Assert fired in file %s, line %i\n", __FILE__, __LINE__); \
		abort(); \
	}

#endif
