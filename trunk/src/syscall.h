#ifndef SYSCALL_H
#define SYSCALL_H

char * SYSCALL[253] = {
	"zero",
	"exit",            // process
	"fork",            // process
	"read",            // file
	"write",
	"open",
	"close",
	"waitpid",         // process
	"creat",           // file
	"link",
	"unlink",
	"execve",          // process
	"chdir",           // file
	"time",            // system
	"mknod",           // file
	"chmod",
	"lchown",
	"break",           // -
	"oldstat",
	"lseek",           // file
	"getpid",          // process
	"mount",           // -
	"umount",
	"setuid",
	"getuid",          // process
	"stime",           // -
	"ptrace",
	"alarm",           // signal
	"oldfstat",        // -
	"pause",           // signal
	"utime",           // file
	"stty",            // -
	"gtty",
	"access",          // file
	"nice",            // -
	"ftime",
	"sync",
	"kill",            // signal
	"rename",          // file
	"mkdir",
	"rmdir",
	"dup",
	"pipe",
	"times",           // -
	"prof",
	"brk",             // process
	"setgid",          // -
	"getgid",          // process
	"signal",          // signal
	"geteuid",         // process
	"getegid",
	"acct",            // -
	"umount2",
	"lock",
	"ioctl",           // file
	"fcntl",
	"mpx",             // -
	"setpgid",
	"ulimit",
	"oldolduname",
	"umask",           // file
	"chroot",          // -
	"ustat",
	"dup2",            // file
	"getppid",         // process
	"getpgrp",
	"setsid",          // -
	"sigaction",       // signal
	"sgetmask",
	"ssetmask",
	"setreuid",        // -
	"setregid",
	"sigsuspend",      // signal
	"sigpending",
	"sethostname",     // -
	"setrlimit",
	"getrlimit",
	"getrusage",
	"gettimeofday",    // system
	"settimeofday",    // -
	"getgroups",
	"setgroups",
	"select",          // file
	"symlink",         // -
	"oldlstat",
	"readlink",
	"uselib",
	"swapon",
	"reboot",
	"readdir",         // file
	"mmap",            // file / process
	"munmap",
	"truncate",        // file
	"ftruncate",
	"fchmod",
	"fchown",
	"getpriority",     // -
	"setpriority",
	"profil",
	"statfs",
	"fstatfs",
	"ioperm",
	"socketcall",
	"syslog",
	"setitimer",
	"getitimer",
	"stat",            // file
	"lstat",
	"fstat",
	"olduname",        // -
	"iopl",
	"vhangup",
	"idle",
	"vm86old",
	"wait4",           // signal
	"swapoff",         // -
	"sysinfo",
	"ipc",
	"fsync",           // file
	"sigreturn",       // signal
	"clone",           // -
	"setdomainname",
	"uname",           // system
	"modify_ldt",      // -
	"adjtimex",
	"mprotect",
	"sigprocmask",     // signal
	"create_module",   // -
	"init_module",
	"delete_module",
	"get_kernel_syms",
	"quotactl",
	"getpgid",         // process
	"fchdir",          // file
	"bdflush",
	"sysfs",
	"personality",
	"afs_syscall",
	"setfsuid",
	"setfsgid",
	"_llseek",
	"getdents",        // file
	"_newselect",      // -
	"flock",           // file
	"msync",           // -
	"readv",
	"writev",
	"getsid",
	"fdatasync",
	"_sysctl",
	"mlock",
	"munlock",
	"mlockall",
	"munlockall",
	"sched_setparam",
	"sched_getparam",
	"sched_setscheduler",
	"sched_getscheduler",
	"sched_yield",
	"sched_get_priority_max",
	"sched_get_priority_min",
	"sched_rr_get_interval",
	"nanosleep",
	"mremap",          // process / file
	"setresuid",       // -
	"getresuid",
	"vm86",
	"query_module",
	"poll",            // file
	"nfsservctl",      // -
	"setresgid",
	"getresgid",
	"prctl",
	"rt_sigreturn",    // signal
	"rt_sigaction",
	"rt_sigprocmask",
	"rt_sigpending",
	"rt_sigtimedwait",
	"rt_sigqueueinfo",
	"rt_sigsuspend",
	"pread",           // -
	"pwrite",
	"chown",           // file
	"getcwd",
	"capget",          // -
	"capset",
	"sigaltstack",     // signal
	"sendfile",        // -
	"getpmsg",
	"putpmsg",
	"vfork",
	"ugetrlimit",
	"mmap2",
	"truncate64",
	"ftruncate64",
	"stat64",          // file
	"lstat64",
	"fstat64",
	"lchown32",
	"getuid32",        // process
	"getgid32",
	"geteuid32",
	"getegid32",
	"setreuid32",
	"setregid32",
	"getgroups32",
	"setgroups32",
	"fchown32",        // file
	"setresuid32",     // -
	"getresuid32",
	"setresgid32",
	"getresgid32",
	"chown32",         // file
	"setuid32",        // process
	"setgid32",
	"setfsuid32",
	"setfsgid32",
	"pivot_root",      // -
	"mincore",
	"madvise",
	"getdents64",      // file
	"fcntl64",
	"EMPTY",           // -
	"security",
	"gettid",
	"readahead",
	"setxattr",
	"lsetxattr",
	"fsetxattr",
	"getxattr",
	"lgetxattr",
	"fgetxattr",
	"listxattr",
	"llistxattr",
	"flistxattr",
	"removexattr",
	"lremovexattr",
	"fremovexattr",
	"tkill",
	"sendfile64",
	"futex",
	"sched_setaffinity",
	"sched_getaffinity",
	"set_thread_area",
	"get_thread_area",
	"io_setup",
	"io_destroy",
	"io_getevents",
	"io_submit",
	"io_cancel",
	"alloc_hugepages",
	"free_hugepages",
	"exit_group"
};


#endif
