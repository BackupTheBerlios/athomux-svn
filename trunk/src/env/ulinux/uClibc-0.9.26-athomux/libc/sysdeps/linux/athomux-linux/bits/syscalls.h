#ifndef _BITS_SYSCALLS_H
#define _BITS_SYSCALLS_H
#ifndef _SYSCALL_H
# error "Never use <bits/syscalls.h> directly; include <sys/syscall.h> instead."
#endif

/* This includes the `__NR_<name>' syscall numbers taken from the Linux kernel
 * header files.  It also defines the traditional `SYS_<name>' macros for older
 * programs.  */
#include <bits/sysnum.h>

#ifndef __set_errno
# define __set_errno(val) (*__errno_location ()) = (val)
#endif

/*
   Some of the sneaky macros in the code were taken from 
   glibc-2.2.5/sysdeps/unix/sysv/linux/i386/sysdep.h
*/

#ifndef __ASSEMBLER__

/* We need some help from the assembler to generate optimal code.  We
   define some macros here which later will be used.  */
asm (".L__X'%ebx = 1\n\t"
     ".L__X'%ecx = 2\n\t"
     ".L__X'%edx = 2\n\t"
     ".L__X'%eax = 3\n\t"
     ".L__X'%esi = 3\n\t"
     ".L__X'%edi = 3\n\t"
     ".L__X'%ebp = 3\n\t"
     ".L__X'%esp = 3\n\t"
     ".macro bpushl name reg\n\t"
     ".if 1 - \\name\n\t"
     ".if 2 - \\name\n\t"
     "pushl %ebx\n\t"
     ".else\n\t"
     "xchgl \\reg, %ebx\n\t"
     ".endif\n\t"
     ".endif\n\t"
     ".endm\n\t"
     ".macro bpopl name reg\n\t"
     ".if 1 - \\name\n\t"
     ".if 2 - \\name\n\t"
     "popl %ebx\n\t"
     ".else\n\t"
     "xchgl \\reg, %ebx\n\t"
     ".endif\n\t"
     ".endif\n\t"
     ".endm\n\t"
     ".macro bmovl name reg\n\t"
     ".if 1 - \\name\n\t"
     ".if 2 - \\name\n\t"
     "movl \\reg, %ebx\n\t"
     ".endif\n\t"
     ".endif\n\t"
     ".endm\n\t");


#define PRINTC(string) \
 ({ \
  int len = strlen(string); \
  INLINE_SYSCALL_LINUX(write, 3, 2, (const __ptr_t) string, len); \
  (int) len; })

#include <sys/mman.h>
#define SYNC() \
 ({ \
  INLINE_SYSCALL_LINUX(msync, 3, 0x8000000, 1024 * 1024 * 896, MS_SYNC | MS_INVALIDATE); \
  (int) 0; })


#undef _syscall0
#define _syscall0(type,name) \
type name(void) \
{ \
  return (type) (INLINE_SYSCALL(name, 0)); \
}

#undef _syscall1
#define _syscall1(type,name,type1,arg1) \
type name(type1 arg1) \
{ \
return (type) (INLINE_SYSCALL(name, 1, arg1)); \
}

#undef _syscall2
#define _syscall2(type,name,type1,arg1,type2,arg2) \
type name(type1 arg1,type2 arg2) \
{ \
return (type) (INLINE_SYSCALL(name, 2, arg1, arg2)); \
}

#undef _syscall3
#define _syscall3(type,name,type1,arg1,type2,arg2,type3,arg3) \
type name(type1 arg1,type2 arg2,type3 arg3) \
{ \
return (type) (INLINE_SYSCALL(name, 3, arg1, arg2, arg3)); \
}

#undef _syscall4
#define _syscall4(type,name,type1,arg1,type2,arg2,type3,arg3,type4,arg4) \
type name (type1 arg1, type2 arg2, type3 arg3, type4 arg4) \
{ \
return (type) (INLINE_SYSCALL(name, 4, arg1, arg2, arg3, arg4)); \
} 

#undef _syscall5
#define _syscall5(type,name,type1,arg1,type2,arg2,type3,arg3,type4,arg4, \
	  type5,arg5) \
type name (type1 arg1,type2 arg2,type3 arg3,type4 arg4,type5 arg5) \
{ \
return (type) (INLINE_SYSCALL(name, 5, arg1, arg2, arg3, arg4, arg5)); \
}

#undef _syscall1_buf
#define _syscall1_buf(type,name,type1,arg1) \
type name(type1 arg1) \
{ \
return (type) (INLINE_SYSCALL_BUF(name, 1, arg1)); \
}

#undef _syscall2_buf
#define _syscall2_buf(type,name,type1,arg1,type2,arg2) \
type name(type1 arg1,type2 arg2) \
{ \
return (type) (INLINE_SYSCALL_BUF(name, 2, arg1, arg2)); \
}

#undef _syscall3_buf
#define _syscall3_buf(type,name,type1,arg1,type2,arg2,type3,arg3) \
type name(type1 arg1,type2 arg2,type3 arg3) \
{ \
return (type) (INLINE_SYSCALL_BUF(name, 3, arg1, arg2, arg3)); \
}

#undef _syscall4_buf
#define _syscall4_buf(type,name,type1,arg1,type2,arg2,type3,arg3,type4,arg4) \
type name (type1 arg1, type2 arg2, type3 arg3, type4 arg4) \
{ \
return (type) (INLINE_SYSCALL_BUF(name, 4, arg1, arg2, arg3, arg4)); \
} 

#undef _syscall5_buf
#define _syscall5_buf(type,name,type1,arg1,type2,arg2,type3,arg3,type4,arg4, \
	  type5,arg5) \
type name (type1 arg1,type2 arg2,type3 arg3,type4 arg4,type5 arg5) \
{ \
return (type) (INLINE_SYSCALL_BUF(name, 5, arg1, arg2, arg3, arg4, arg5)); \
}

/* flo */
extern int __athomux_syscall;
extern int __athomux_return;

#include <stdarg.h>

/* original 
#define INLINE_SYSCALL(name, nr, args...) \
  ({									      \
    unsigned int resultvar;						      \
    PRINTC(#name); \
    PRINTC("\n");  \
    asm volatile (							      \
    LOADARGS_##nr							      \
    "movl %1, %%eax\n\t"						      \
    "int $0x80\n\t"							      \
    RESTOREARGS_##nr							      \
    : "=a" (resultvar)							      \
    : "i" (__NR_##name) ASMFMT_##nr(args) : "memory", "cc");		      \
    if (resultvar >= 0xfffff001)					      \
      {									      \
	__set_errno (-resultvar);					      \
	resultvar = 0xffffffff;						      \
      }									      \
    (int) resultvar; })

*/

//__athomux_syscall = INLINE_SYSCALL_LINUX(open, 3, "/tmp/athomux_syscall", 1, 0);
//__athomux_return = INLINE_SYSCALL_LINUX(open, 3, "/tmp/athomux_return", 0, 0);

#define OPEN_PIPES \
    if (__athomux_syscall == -1) { \
      __athomux_syscall = 3; \
      PRINTC("syscall pipe open\n"); \
    }\
    if (__athomux_return == -1) { \
	  __athomux_return = 4;\
	  PRINTC("return pipe open\n"); \
    }\

#define INLINE_SYSCALL(name, nr, args...) \
  ({ \
    unsigned int resultvar = 1;                                               \
    unsigned int s[2];                                                        \
    unsigned int receive_result[2]; \
    unsigned int arg[5];                                                      \
    /*PRINTC(#name);*/                                                          \
    /*PRINTC("\n");*/                                                           \
    OPEN_PIPES \
    putargs_##nr(arg, args); \
    s[0] = __NR_##name; \
    s[1] = nr;\
    INLINE_SYSCALL_LINUX(write, 3, __athomux_syscall, &s, 8); \
    INLINE_SYSCALL_LINUX(write, 3, __athomux_syscall, &arg, nr * 4); \
    INLINE_SYSCALL_LINUX(read, 3, __athomux_return, &receive_result, 8); \
    __set_errno (receive_result[0]); \
    resultvar = receive_result[1]; \
    (int) resultvar; })

#define INLINE_SYSCALL_BUF(name, nr, args...) \
  ({ \
    unsigned int s[2];                                                        \
    unsigned int arg[5];                                                      \
    /*PRINTC(#name);*/                                                          \
    /*PRINTC("\n");*/                                                           \
    putargs_##nr(arg, args); \
    OPEN_PIPES \
    s[0] = __NR_##name; \
    s[1] = nr;\
    INLINE_SYSCALL_LINUX(write, 3, __athomux_syscall, &s, 8); \
    INLINE_SYSCALL_LINUX(write, 3, __athomux_syscall, &arg, nr * 4); \
    })

#define SEND_BUF(buf, count)  \
  { \
  unsigned int length = count; \
  OPEN_PIPES \
  INLINE_SYSCALL_LINUX(write, 3, __athomux_syscall, &length, sizeof(int)); \
  INLINE_SYSCALL_LINUX(write, 3, __athomux_syscall, (char *) buf, length); \
  }


#define RECEIVE_BUF(buf)  \
  { \
  unsigned int length; \
  OPEN_PIPES \
  INLINE_SYSCALL_LINUX(read, 3, __athomux_return, &length, sizeof(int)); \
  INLINE_SYSCALL_LINUX(read, 3, __athomux_return, (char *) buf, length); \
 } 

#define RECEIVE_RESULT \
  ({ \
  unsigned int receive_result[2]; \
  OPEN_PIPES \
  INLINE_SYSCALL_LINUX(read, 3, __athomux_return, &receive_result[0], 8); \
  __set_errno(receive_result[0]); \
  (int) receive_result[1]; })

#define LOADARGS_0
#define LOADARGS_1 \
    "bpushl .L__X'%k2, %k2\n\t"						      \
    "bmovl .L__X'%k2, %k2\n\t"
#define LOADARGS_2	LOADARGS_1
#define LOADARGS_3	LOADARGS_1
#define LOADARGS_4	LOADARGS_1
#define LOADARGS_5	LOADARGS_1

#define RESTOREARGS_0
#define RESTOREARGS_1 \
    "bpopl .L__X'%k2, %k2\n\t"
#define RESTOREARGS_2	RESTOREARGS_1
#define RESTOREARGS_3	RESTOREARGS_1
#define RESTOREARGS_4	RESTOREARGS_1
#define RESTOREARGS_5	RESTOREARGS_1

#define ASMFMT_0()
#define ASMFMT_1(arg1) \
	, "acdSD" (arg1)
#define ASMFMT_2(arg1, arg2) \
	, "adCD" (arg1), "c" (arg2)
#define ASMFMT_3(arg1, arg2, arg3) \
	, "aCD" (arg1), "c" (arg2), "d" (arg3)
#define ASMFMT_4(arg1, arg2, arg3, arg4) \
	, "aD" (arg1), "c" (arg2), "d" (arg3), "S" (arg4)
#define ASMFMT_5(arg1, arg2, arg3, arg4, arg5) \
	, "a" (arg1), "c" (arg2), "d" (arg3), "S" (arg4), "D" (arg5)


#define putargs_0(arg, arg1)
#define putargs_1(arg, arg1) \
        arg[0] = (unsigned int) arg1;
#define putargs_2(arg, arg1, arg2) \
        arg[0] = (unsigned int) arg1; arg[1] = (unsigned int) arg2;
#define putargs_3(arg, arg1, arg2, arg3) \
        arg[0] = (unsigned int) arg1; arg[1] = (unsigned int) arg2; arg[2] = (unsigned int) arg3;
#define putargs_4(arg, arg1, arg2, arg3, arg4) \
        arg[0] = (unsigned int) arg1; arg[1] = (unsigned int) arg2; arg[2] = (unsigned int) arg3; arg[3] = (unsigned int) arg4;
#define putargs_5(arg, arg1, arg2, arg3, arg4, arg5) \
        arg[0] = (unsigned int) arg1; arg[1] = (unsigned int) arg2; arg[2] = (unsigned int) arg3; arg[3] = (unsigned int) arg4; arg[4] = (unsigned int) arg5;


#define INLINE_SYSCALL_LINUX(name, nr, args...) \
  ({									      \
    unsigned int resultvar;						      \
    asm volatile (							      \
    LOADARGS_##nr							      \
    "movl %1, %%eax\n\t"						      \
    "int $0x80\n\t"							      \
    RESTOREARGS_##nr							      \
    : "=a" (resultvar)							      \
    : "i" (__NR_##name) ASMFMT_##nr(args) : "memory", "cc");		      \
    if (resultvar >= 0xfffff001)					      \
      {									      \
	__set_errno (-resultvar);					      \
	resultvar = 0xffffffff;						      \
      }									      \
    (int) resultvar; })


#ifndef size_t
#define size_t unsigned int
#endif
#include <sys/ipc.h>

extern size_t strlen(const char *);


/*
#define INLINE_SYSCALL(name, nr, args...) \
  ({									      \
    unsigned int resultvar; \
    msgbuf buf = {1, __NR_##name}; \
    PRINTC("     ->");  \
    PRINTC(#name); \
    PRINTC(" ");  \
    PRINTC(__NR_##name);  \
    PRINTC("\n");  \
    __msgget(42, IPC_CREAT | 0666); \
    __msgsnd(42, &buf, sizeof(buf), 0); \
    (int) resultvar; })
*/


/*
#define INLINE_SYSCALL(name, nr, args...) \
({                               \
  unsigned int resultvar = 0;	 \
  printf("syscall %s\n", #name); \
 (int) resultvar; })
*/

// INLINE_SYSCALL_LINUX(ipc, 5, IPCOP_msgget, key, msgflg, 0, 0); 
// return __ipc(IPCOP_msgget ,key ,msgflg ,0 ,0);
#define __msgget(key, msgflg) \
  ({ \
   INLINE_SYSCALL_LINUX(ipc, 5, 13, 42, 0, 0, 0); \
   0;})

// INLINE_SYSCALL_LINUX(ipc, 5, IPCOP_msgsnd, key, msgsz, msgflg, msgp);
// return __ipc(IPCOP_msgsnd, msqid, msgsz, msgflg, (void *)msgp);
#define __msgsnd(msqid, msgp, msgsz, msgflg) \
  ({ \
  INLINE_SYSCALL_LINUX(ipc, 5, 11, 262144, msgsz, msgflg, msgp); \
  0;})


#endif /* __ASSEMBLER__ */
#endif /* _BITS_SYSCALLS_H */

