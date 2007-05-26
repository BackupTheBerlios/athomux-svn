// Low level exception handlers
// Author: Marcel Kilgus

        .text

        .globl  exc_0x00
	.globl	trap_handlers
	.globl	unhandled_exception
	.globl	task_switch_segment
	.globl	task_switch_address

#define __ASM__
#include "x86_irq.h"
#include "x86_kernel.h"


/* Stack layout on entry

	%esp	exception number
	%esp+4	error code or 0
	%esp+8	eip
	%esp+12	cs
	%esp+16	flags
	%esp+20 oldesp
	%esp+24 oldss

Layout within the routine
	%ebp	ss
	%ebp+4	gs
	%ebp+8	fs
	%ebp+12	es
	%ebp+16	ds
	%ebp+20 edi
	%ebp+24 esi
	%ebp+28 ebp
	%ebp+32 esp
	%ebp+36 ebx
	%ebp+40 edx
	%ebp+44 ecx
	%ebp+48 eax
	%ebp+52	exception number
	%ebp+56	error code or 0
	%ebp+60	eip
	%ebp+64	cs
	%ebp+68 flags
	%ebp+72 oldesp
	%ebp+76 oldss
*/

ex_trap:
	pusha
	pushl	%ds
	pushl	%es
	pushl	%fs
	pushl	%gs
	pushl	%ss			// Just for the parameter stack
	movl	%esp,%ebp
	
	movw	$KERNEL_DS,%ax
	movw	%ax,%ds
	mov	%ax,%es
	movw	$USER_DS,%ax
	movw	%ax,%fs

	cld				// GCC likes that flag cleared
	movl	52(%ebp),%eax		// Exception number
	movl	trap_handlers(,%eax,4),%eax
	testl	%eax,%eax
	je	unknown_int
	push	%ebp			// Trick! Pretend data on stack is C structure (x86_irq.h)
	call	*%eax			// Call C function
	addl	$4,%esp
	cmpb	$EXC_OK,%al
	je	ack_int
	cmpb	$EXC_TASK_SWITCH,%al
	jne	unknown_int

	call	do_ack_int
	ljmp	*task_switch_address	
	jmp	exit

task_switch_address:
	.int	0
task_switch_segment:
	.word	0

unknown_int:
	push	%ebp			// Trick! Pretend data on stack is C structure (x86_irq.h)
	call	unhandled_exception
	addl	$4,%esp

ack_int:
	call	do_ack_int
exit:
	addl	$4,%esp			// skip SS
	popl	%gs
	popl	%fs
	popl	%es
	popl	%ds
	popa
	addl	$4*2,%esp		// Remove exception number and error code
	iret

do_ack_int:
	movl	52(%ebp),%eax
	cmpl	$IRQ_BASE,%eax
	jl	ack_exit
	cmpl	$IRQ_BASE+8,%eax
	jl	ack_low
	cmpl	$IRQ_BASE+16,%eax
	jge	ack_exit
	movb	$0x20,%al
	outb	%al,$0xa0
ack_low:
	movb	$0x20,%al
	outb	%al,$0x20
ack_exit:
	ret


// No error code on stack, just use 0
#define EXCEPTION(n)				\
	.align	16				;\
exc_ ## n:					;\
	pushl	$0				;\
	pushl	$n				;\
	jmp	ex_trap

// Error code is already on stack
#define EXCEP_ERR(n)				\
	.align	16				;\
exc_ ## n:					;\
	pushl	$n				;\
	jmp	ex_trap				


// Okay, this looks ugly, but is necessary
// Every entry is (must be!) exactly 16 bytes long
// This is achieved using the .align instruction
EXCEPTION(0x00)		// Divide error
EXCEPTION(0x01)		// Debug
EXCEPTION(0x02)		// NMI
EXCEPTION(0x03)		// Breakpoint
EXCEPTION(0x04)		// INTO-detected overflow
EXCEPTION(0x05)		// BOUND range exceeded
EXCEPTION(0x06)		// Invalid opcode
EXCEPTION(0x07)		// Device not available
EXCEPTION(0x08)		// Double fault
EXCEPTION(0x09)		// CoProcessor segment overrun
EXCEP_ERR(0x0a)		// Invalid task state segment
EXCEP_ERR(0x0b)		// Segment not present
EXCEP_ERR(0x0c)		// Stack fault
EXCEP_ERR(0x0d)		// General protection fault
EXCEP_ERR(0x0e)		// Page fault
EXCEPTION(0x0f)		// reserved
EXCEPTION(0x10)		// Floating point error
EXCEPTION(0x11)		// Alignment check
EXCEPTION(0x12)		// Machine check
EXCEPTION(0x13)
EXCEPTION(0x14)
EXCEPTION(0x15)
EXCEPTION(0x16)
EXCEPTION(0x17)
EXCEPTION(0x18)
EXCEPTION(0x19)
EXCEPTION(0x1a)
EXCEPTION(0x1b)
EXCEPTION(0x1c)
EXCEPTION(0x1d)
EXCEPTION(0x1e)
EXCEPTION(0x1f)
EXCEPTION(0x20)
EXCEPTION(0x21)
EXCEPTION(0x22)
EXCEPTION(0x23)
EXCEPTION(0x24)
EXCEPTION(0x25)
EXCEPTION(0x26)
EXCEPTION(0x27)
EXCEPTION(0x28)
EXCEPTION(0x29)
EXCEPTION(0x2a)
EXCEPTION(0x2b)
EXCEPTION(0x2c)
EXCEPTION(0x2d)
EXCEPTION(0x2e)
EXCEPTION(0x2f)
EXCEPTION(0x30)
EXCEPTION(0x31)
EXCEPTION(0x32)
EXCEPTION(0x33)
EXCEPTION(0x34)
EXCEPTION(0x35)
EXCEPTION(0x36)
EXCEPTION(0x37)
EXCEPTION(0x38)
EXCEPTION(0x39)
EXCEPTION(0x3A)
EXCEPTION(0x3B)
EXCEPTION(0x3C)
EXCEPTION(0x3D)
EXCEPTION(0x3E)
EXCEPTION(0x3F)
EXCEPTION(0x40)
EXCEPTION(0x41)
EXCEPTION(0x42)
EXCEPTION(0x43)
EXCEPTION(0x44)
EXCEPTION(0x45)
EXCEPTION(0x46)
EXCEPTION(0x47)
EXCEPTION(0x48)
EXCEPTION(0x49)
EXCEPTION(0x4A)
EXCEPTION(0x4B)
EXCEPTION(0x4C)
EXCEPTION(0x4D)
EXCEPTION(0x4E)
EXCEPTION(0x4F)
EXCEPTION(0x50)
EXCEPTION(0x51)
EXCEPTION(0x52)
EXCEPTION(0x53)
EXCEPTION(0x54)
EXCEPTION(0x55)
EXCEPTION(0x56)
EXCEPTION(0x57)
EXCEPTION(0x58)
EXCEPTION(0x59)
EXCEPTION(0x5A)
EXCEPTION(0x5B)
EXCEPTION(0x5C)
EXCEPTION(0x5D)
EXCEPTION(0x5E)
EXCEPTION(0x5F)
EXCEPTION(0x60)
EXCEPTION(0x61)
EXCEPTION(0x62)
EXCEPTION(0x63)
EXCEPTION(0x64)
EXCEPTION(0x65)
EXCEPTION(0x66)
EXCEPTION(0x67)
EXCEPTION(0x68)
EXCEPTION(0x69)
EXCEPTION(0x6A)
EXCEPTION(0x6B)
EXCEPTION(0x6C)
EXCEPTION(0x6D)
EXCEPTION(0x6E)
EXCEPTION(0x6F)
EXCEPTION(0x70)
EXCEPTION(0x71)
EXCEPTION(0x72)
EXCEPTION(0x73)
EXCEPTION(0x74)
EXCEPTION(0x75)
EXCEPTION(0x76)
EXCEPTION(0x77)
EXCEPTION(0x78)
EXCEPTION(0x79)
EXCEPTION(0x7A)
EXCEPTION(0x7B)
EXCEPTION(0x7C)
EXCEPTION(0x7D)
EXCEPTION(0x7E)
EXCEPTION(0x7F)
EXCEPTION(0x80)
EXCEPTION(0x81)
EXCEPTION(0x82)
EXCEPTION(0x83)
EXCEPTION(0x84)
EXCEPTION(0x85)
EXCEPTION(0x86)
EXCEPTION(0x87)
EXCEPTION(0x88)
EXCEPTION(0x89)
EXCEPTION(0x8A)
EXCEPTION(0x8B)
EXCEPTION(0x8C)
EXCEPTION(0x8D)
EXCEPTION(0x8E)
EXCEPTION(0x8F)
EXCEPTION(0x90)
EXCEPTION(0x91)
EXCEPTION(0x92)
EXCEPTION(0x93)
EXCEPTION(0x94)
EXCEPTION(0x95)
EXCEPTION(0x96)
EXCEPTION(0x97)
EXCEPTION(0x98)
EXCEPTION(0x99)
EXCEPTION(0x9A)
EXCEPTION(0x9B)
EXCEPTION(0x9C)
EXCEPTION(0x9D)
EXCEPTION(0x9E)
EXCEPTION(0x9F)
EXCEPTION(0xA0)
EXCEPTION(0xA1)
EXCEPTION(0xA2)
EXCEPTION(0xA3)
EXCEPTION(0xA4)
EXCEPTION(0xA5)
EXCEPTION(0xA6)
EXCEPTION(0xA7)
EXCEPTION(0xA8)
EXCEPTION(0xA9)
EXCEPTION(0xAA)
EXCEPTION(0xAB)
EXCEPTION(0xAC)
EXCEPTION(0xAD)
EXCEPTION(0xAE)
EXCEPTION(0xAF)
EXCEPTION(0xB0)
EXCEPTION(0xB1)
EXCEPTION(0xB2)
EXCEPTION(0xB3)
EXCEPTION(0xB4)
EXCEPTION(0xB5)
EXCEPTION(0xB6)
EXCEPTION(0xB7)
EXCEPTION(0xB8)
EXCEPTION(0xB9)
EXCEPTION(0xBA)
EXCEPTION(0xBB)
EXCEPTION(0xBC)
EXCEPTION(0xBD)
EXCEPTION(0xBE)
EXCEPTION(0xBF)
EXCEPTION(0xC0)
EXCEPTION(0xC1)
EXCEPTION(0xC2)
EXCEPTION(0xC3)
EXCEPTION(0xC4)
EXCEPTION(0xC5)
EXCEPTION(0xC6)
EXCEPTION(0xC7)
EXCEPTION(0xC8)
EXCEPTION(0xC9)
EXCEPTION(0xCA)
EXCEPTION(0xCB)
EXCEPTION(0xCC)
EXCEPTION(0xCD)
EXCEPTION(0xCE)
EXCEPTION(0xCF)
EXCEPTION(0xD0)
EXCEPTION(0xD1)
EXCEPTION(0xD2)
EXCEPTION(0xD3)
EXCEPTION(0xD4)
EXCEPTION(0xD5)
EXCEPTION(0xD6)
EXCEPTION(0xD7)
EXCEPTION(0xD8)
EXCEPTION(0xD9)
EXCEPTION(0xDA)
EXCEPTION(0xDB)
EXCEPTION(0xDC)
EXCEPTION(0xDD)
EXCEPTION(0xDE)
EXCEPTION(0xDF)
EXCEPTION(0xE0)
EXCEPTION(0xE1)
EXCEPTION(0xE2)
EXCEPTION(0xE3)
EXCEPTION(0xE4)
EXCEPTION(0xE5)
EXCEPTION(0xE6)
EXCEPTION(0xE7)
EXCEPTION(0xE8)
EXCEPTION(0xE9)
EXCEPTION(0xEA)
EXCEPTION(0xEB)
EXCEPTION(0xEC)
EXCEPTION(0xED)
EXCEPTION(0xEE)
EXCEPTION(0xEF)
EXCEPTION(0xF0)
EXCEPTION(0xF1)
EXCEPTION(0xF2)
EXCEPTION(0xF3)
EXCEPTION(0xF4)
EXCEPTION(0xF5)
EXCEPTION(0xF6)
EXCEPTION(0xF7)
EXCEPTION(0xF8)
EXCEPTION(0xF9)
EXCEPTION(0xFA)
EXCEPTION(0xFB)
EXCEPTION(0xFC)
EXCEPTION(0xFD)
EXCEPTION(0xFE)
EXCEPTION(0xFF)

