// x86_boot.S
// Written by Marcel Kilgus
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#define __ASM__	1
#include <multiboot.h>
#include <x86_kernel.h>
#include <x86_mem.h>

        .text

        .globl  start, _start
start:
_start:
        jmp     multiboot_entry

        .align  4

        // Multiboot header
multiboot_header:
        .long   MULTIBOOT_HEADER_MAGIC	// magic
        .long   MULTIBOOT_HEADER_FLAGS	// flags
        .long   -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)	// Checksum
	.long	0,0,0,0,0		// grub can load ELF natively
	.long	1			// text mode
	.long	80			// width
	.long	50			// height	

multiboot_entry:
// Here paging is not yet set up, but out kernel uses virtual addresses!
// Therefore we MUST NOT use any position-dependand code or virtual addresses
// Also, we must not use the stack (there is none yet!)
// Preserve %eax and %ebx for later
	movl	%eax,%ebp	
	movl	$BOOT_PAGE_DIR,%esi
	movl	%esi,%edi
	movl	$PTRS_PER_PAGE,%ecx
	xorl	%eax,%eax
	rep	stosl			// First clear page directory

	movl	%esi,%edi
	movl	%edi,%eax
	addl	$PAGE_SIZE,%eax		// First page table entry 
	leal	PAGE_TABLE(%eax),%ecx	// Include flags
	movl	%ecx,(%edi)		// Link into page directory at address 0
	movl	%ecx,KERNEL_VIRT_PDOFF(%edi) // AND address 0xc0000000

	// Identity map the first 4MB to virtual kernel space
	movl	%eax,%edi	    
	movl	$PAGE_SHARED,%eax
	movl	$PTRS_PER_PAGE,%ecx
1:	stosl
	addl	$PAGE_SIZE,%eax
	loop	1b

	mov 	%esi,%cr3		// Load Page Directory Base Register

    	mov 	%cr0,%ecx
    	or 	$0x80000000,%ecx       	// Enable paging!
    	mov 	%ecx,%cr0

    	// Now jump to virtual kernel space.
	movl	%ebp,%eax
    	lea 	virtual_entry,%ecx	// LEA/JMP because we don't want a relative jump
    	jmp 	*%ecx

// ---- Well done, we've moved to KERNEL_VIRT_BASE!
virtual_entry:        
        movl    $(stack + STACK_SIZE),%esp // Initialize the stack pointer

        pushl   $0
        popf				// Reset EFLAGS

        pushl   %ebx			// The Multiboot information structure
        pushl   %eax			// The magic value

        call    EXT_C(cmain)		// Enter C code

        pushl   $halt_message		// Actually, this code should not be reached
        call    EXT_C(printf)

loop:   hlt
        jmp     loop

halt_message:
        .asciz  "Halted."

        // Our stack area
        .comm   stack, STACK_SIZE
