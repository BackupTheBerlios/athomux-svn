#ifndef TYPES_H
#define TYPES_H

#include "common.h"

#define ELF_BUFSIZE   512

#define IMAGE_MAX     120
#define DIR_NAMELEN   16
#define DIR_HASHSIZE  IMAGE_MAX / 3 * 2

#define EPROCLISTFULL 1
#define EHASHVALDUP   2
#define ENOHASHVAL    3
#define IMAGE_SIZE    1024 * 1024 * 256
#define IMAGE_START   0x8000000
#define IMAGE_OFFSET  0x0000000
#define IMAGE_HEAP    0xB000000

#define ULINUX_MAX_PROCESSES 128
#define FD_MAX               64

extern char * ATHOMUX_ULINUX_BASE;
extern char ATHOMUX_ULINUX_ROOT[256];
extern char ATHOMUX_ULINUX_USERFS[256];
extern char ATHOMUX_ULINUX_SYSCALLS[256];
extern char ATHOMUX_ULINUX_LOG[256];
extern char FUSE_LOG[512];
extern char SYSCALL_LOG[512];
extern char ATHOMUX_LOG[512];

#define _DEBUG(stream, format, args...) fprintf(stream, format, ## args); fflush(stream)

#define _DEBUGCHECK(cond, stream, format, args...) if (cond) { fprintf(stream, format, ## args); fflush(stream); return; }

enum segment_t { SEG_CODE, SEG_DATA, SEG_DATA_0, SEG_HEAP, SEG_STACK };
enum process_t { PROCESS_EXEC, PROCESS_FORK };

enum file_t { FILE_NONE, FILE_FILE, FILE_CHAR, FILE_PIPE };


// FIXME, use athomux struct (does not exist atm.)

typedef struct filedes {
  
  enum file_t type;
  int         flags;
  addr_t      addr;
  int         filepos;
  int         dirpos;
  index_t     fs_index; // needed by fork to connect to fs
  char        filename[128];
} filedes;

typedef struct segment {
  
  paddr_t        start;
  paddr_t        length;
  
  unsigned int   flags;
  enum segment_t type;

  addr_t content;
} segment;

typedef struct elf_header {
  
  paddr_t entry;
  paddr_t length;
  
  segment code_segment[64];
  segment data_segment[64];

  int     num_code_segments;
  int     num_data_segments;
} elf_header;

// include/linux/sched.h
typedef struct task_struct {

  addr_t lock;

  char cwd[256];

  addr_t file;
  addr_t elf;
  addr_t image;
  addr_t mmu;
  addr_t posix;
  addr_t cow;

  pid_t ppid;
  pid_t pid;
  uid_t uid;
  gid_t gid;
  
  pid_t children[8];

  int   exit_code;
  int   exit_signal;

  pid_t linux_pid;

  struct task_struct *parent;

  filedes fd[FD_MAX];
  //
  int    userfs_index;
  addr_t process_state;

  enum process_t type;
  paddr_t entry;
  paddr_t bp;
  paddr_t sp;

  // environment

  char   name[256];
  addr_t argv[256];
  addr_t envp[256];
  
  int argc;
  int envc;

} task_struct;

typedef struct userfs_struct {

  char   path[256];
  addr_t image[ULINUX_MAX_PROCESSES];

} userfs_struct;

typedef struct process_struct {

  segment code_segment[4];
  segment data_segment[4];
  segment heap_segment;
  segment stack_segment;

  int num_code_segments;
  int num_data_segments;

  int init;
} process_struct;


typedef struct block_hash_entry {
  
  addr_t log_addr;
  addr_t block_addr;
  addr_t next;
} block_hash_entry;

typedef struct stat_t {

  int type;
  int mode;
  int size;
  char name[64];
} stat_t;

struct entry_desc {
  index_t desc_format;
  index_t desc_unused;
  len_t   desc_len;
  index_t desc_attrlen;
  index_t desc_next;
};

typedef struct ath_stat_t {

  struct entry_desc desc;
  struct stat_t stat;
} ath_stat_t;


typedef struct linux_stat_t {
	uns2	st_dev;
	uns2	pad1;
	uns4	st_ino;
	uns2	st_mode;
	uns2	st_nlink;
	uns2	st_uid;
	uns2	st_gid;
	uns2	st_rdev;
	uns2	pad2;
	uns4	st_size;
	uns4	st_blksize;
	uns4	st_blocks;
	uns4	st_atimex;
	uns4	unused1;
	uns4	st_mtimex;
	uns4	unused2;
	uns4	st_ctimex;
	uns4	unused3;
	uns4	unused4;
	uns4	unused5;
} linux_stat_t;

typedef struct linux_utsname_t {
	char sysname[65];
	char nodename[65];
	char release[65];
	char version[65];
	char machine[65];
} linux_utsname_t;

typedef struct linux_dirent_t {
   	uns8	d_ino;
	int8	d_off;
	uns2	d_reclen;
	uns1	d_type;
	char	d_name[256];
} linux_dirent_t;

#define EPERM            1      /* Operation not permitted */
#define ENOENT           2      /* No such file or directory */
#define ESRCH            3      /* No such process */
#define EINTR            4      /* Interrupted system call */
#define EIO              5      /* I/O error */
#define ENXIO            6      /* No such device or address */
#define E2BIG            7      /* Argument list too long */
#define ENOEXEC          8      /* Exec format error */
#define EBADF            9      /* Bad file number */
#define ECHILD          10      /* No child processes */
#define EAGAIN          11      /* Try again */
#define ENOMEM          12      /* Out of memory */
#define EACCES          13      /* Permission denied */
#define EFAULT          14      /* Bad address */
#define ENOTBLK         15      /* Block device required */
#define EBUSY           16      /* Device or resource busy */
#define EEXIST          17      /* File exists */
#define EXDEV           18      /* Cross-device link */
#define ENODEV          19      /* No such device */
#define ENOTDIR         20      /* Not a directory */
#define EISDIR          21      /* Is a directory */
#define EINVAL          22      /* Invalid argument */
#define ENFILE          23      /* File table overflow */
#define EMFILE          24      /* Too many open files */
#define ENOTTY          25      /* Not a typewriter */
#define ETXTBSY         26      /* Text file busy */
#define EFBIG           27      /* File too large */
#define ENOSPC          28      /* No space left on device */
#define ESPIPE          29      /* Illegal seek */
#define EROFS           30      /* Read-only file system */
#define EMLINK          31      /* Too many links */
#define EPIPE           32      /* Broken pipe */
#define EDOM            33      /* Math argument out of domain of func */
#define ERANGE          34      /* Math result not representable */
#define EDEADLK         35      /* Resource deadlock would occur */
#define ENAMETOOLONG    36      /* File name too long */
#define ENOLCK          37      /* No record locks available */
#define ENOSYS          38      /* Function not implemented */
#define ENOTEMPTY       39      /* Directory not empty */
#define ELOOP           40      /* Too many symbolic links encountered */
#define EWOULDBLOCK     EAGAIN  /* Operation would block */
#define ENOMSG          42      /* No message of desired type */
#define EIDRM           43      /* Identifier removed */
#define ECHRNG          44      /* Channel number out of range */
#define EL2NSYNC        45      /* Level 2 not synchronized */
#define EL3HLT          46      /* Level 3 halted */
#define EL3RST          47      /* Level 3 reset */
#define ELNRNG          48      /* Link number out of range */
#define EUNATCH         49      /* Protocol driver not attached */
#define ENOCSI          50      /* No CSI structure available */
#define EL2HLT          51      /* Level 2 halted */
#define EBADE           52      /* Invalid exchange */
#define EBADR           53      /* Invalid request descriptor */
#define EXFULL          54      /* Exchange full */
#define ENOANO          55      /* No anode */
#define EBADRQC         56      /* Invalid request code */
#define EBADSLT         57      /* Invalid slot */
#define EDEADLOCK       EDEADLK
#define EBFONT          59      /* Bad font file format */
#define ENOSTR          60      /* Device not a stream */
#define ENODATA         61      /* No data available */
#define ETIME           62      /* Timer expired */
#define ENOSR           63      /* Out of streams resources */
#define ENONET          64      /* Machine is not on the network */
#define ENOPKG          65      /* Package not installed */
#define EREMOTE         66      /* Object is remote */
#define ENOLINK         67      /* Link has been severed */
#define EADV            68      /* Advertise error */
#define ESRMNT          69      /* Srmount error */
#define ECOMM           70      /* Communication error on send */
#define EPROTO          71      /* Protocol error */
#define EMULTIHOP       72      /* Multihop attempted */
#define EDOTDOT         73      /* RFS specific error */
#define EBADMSG         74      /* Not a data message */
#define EOVERFLOW       75      /* Value too large for defined data type */
#define ENOTUNIQ        76      /* Name not unique on network */
#define EBADFD          77      /* File descriptor in bad state */
#define EREMCHG         78      /* Remote address changed */
#define ELIBACC         79      /* Can not access a needed shared library */
#define ELIBBAD         80      /* Accessing a corrupted shared library */
#define ELIBSCN         81      /* .lib section in a.out corrupted */
#define ELIBMAX         82      /* Attempting to link in too many shared libraries */
#define ELIBEXEC        83      /* Cannot exec a shared library directly */
#define EILSEQ          84      /* Illegal byte sequence */
#define ERESTART        85      /* Interrupted system call should be restarted */
#define ESTRPIPE        86      /* Streams pipe error */
#define EUSERS          87      /* Too many users */
#define ENOTSOCK        88      /* Socket operation on non-socket */
#define EDESTADDRREQ    89      /* Destination address required */
#define EMSGSIZE        90      /* Message too long */
#define EPROTOTYPE      91      /* Protocol wrong type for socket */
#define ENOPROTOOPT     92      /* Protocol not available */
#define EPROTONOSUPPORT 93      /* Protocol not supported */
#define ESOCKTNOSUPPORT 94      /* Socket type not supported */
#define EOPNOTSUPP      95      /* Operation not supported on transport endpoint */
#define EPFNOSUPPORT    96      /* Protocol family not supported */
#define EAFNOSUPPORT    97      /* Address family not supported by protocol */
#define EADDRINUSE      98      /* Address already in use */
#define EADDRNOTAVAIL   99      /* Cannot assign requested address */
#define ENETDOWN        100     /* Network is down */
#define ENETUNREACH     101     /* Network is unreachable */
#define ENETRESET       102     /* Network dropped connection because of reset */
#define ECONNABORTED    103     /* Software caused connection abort */
#define ECONNRESET      104     /* Connection reset by peer */
#define ENOBUFS         105     /* No buffer space available */
#define EISCONN         106     /* Transport endpoint is already connected */
#define ENOTCONN        107     /* Transport endpoint is not connected */
#define ESHUTDOWN       108     /* Cannot send after transport endpoint shutdown */
#define ETOOMANYREFS    109     /* Too many references: cannot splice */
#define ETIMEDOUT       110     /* Connection timed out */
#define ECONNREFUSED    111     /* Connection refused */
#define EHOSTDOWN       112     /* Host is down */
#define EHOSTUNREACH    113     /* No route to host */
#define EALREADY        114     /* Operation already in progress */
#define EINPROGRESS     115     /* Operation now in progress */
#define ESTALE          116     /* Stale NFS file handle */
#define EUCLEAN         117     /* Structure needs cleaning */
#define ENOTNAM         118     /* Not a XENIX named type file */
#define ENAVAIL         119     /* No XENIX semaphores available */
#define EISNAM          120     /* Is a named type file */
#define EREMOTEIO       121     /* Remote I/O error */
#define EDQUOT          122     /* Quota exceeded */
#define ENOMEDIUM       123     /* No medium found */
#define EMEDIUMTYPE     124     /* Wrong medium type */
  
#endif

