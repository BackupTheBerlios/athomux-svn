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

#endif

