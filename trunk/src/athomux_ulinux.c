/* Copyright (c) 2004 Thomas Schoebel-Theuer, Florian Niebling
 * This program is free software according to
 * files SOFTWARE-LICENSE and PATENT-LICENSE
 */

#include <stdlib.h>
#include <unistd.h>
#include "types.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>

#define BASEFILE "athomux_ulinux.c"
#include "control_simple.h"

char blanks[32];

char FUSE_LOG[512];
char SYSCALL_LOG[512];
char ATHOMUX_LOG[512];

extern FILE * _debug_strategy;
extern FILE * _debug_fuse;
extern FILE * _debug_syscall;

int main(int argc, char * argv[])
{

  printf("1\n");
  ATHOMUX_ULINUX_BASE = getenv("ATHOMUX_ULINUX_BASE");
  if (! ATHOMUX_ULINUX_BASE) {
    printf("please set the environment variable ATHOMUX_ULINUX_BASE\n");
    exit(0);
  }

  printf("2\n");
  snprintf(ATHOMUX_ULINUX_ROOT, 256, "%s/root", ATHOMUX_ULINUX_BASE);
  printf("2.1\n");
  snprintf(ATHOMUX_ULINUX_USERFS, 256, "%s/fs", ATHOMUX_ULINUX_BASE);
  printf("2.2\n");
  snprintf(ATHOMUX_ULINUX_SYSCALLS, 256, "%s/syscalls", ATHOMUX_ULINUX_BASE);
  printf("2.3\n");
  snprintf(ATHOMUX_ULINUX_LOG, 256, "%s/log", ATHOMUX_ULINUX_BASE);

  printf("3\n");

  mkdir(ATHOMUX_ULINUX_USERFS, 0777);
  mkdir(ATHOMUX_ULINUX_SYSCALLS, 0777);
  mkdir(ATHOMUX_ULINUX_LOG, 0777);

  printf("4\n");
  snprintf(FUSE_LOG, 512, "%s/fuse.log", ATHOMUX_ULINUX_LOG);
  snprintf(SYSCALL_LOG, 512, "%s/syscall.log", ATHOMUX_ULINUX_LOG);
  snprintf(ATHOMUX_LOG, 512, "%s/athomux.log", ATHOMUX_ULINUX_LOG);

  _debug_strategy = fopen(ATHOMUX_LOG, "w");
  _debug_fuse     = fopen(FUSE_LOG, "w");
  _debug_syscall  = fopen(SYSCALL_LOG, "w");

  memset(blanks, ' ', 31);
  blanks[31] = '\0';
  init_static_control_simple();

  void ask(addr_t addr) {
    char str[DEFAULT_TRANSFER];
    struct args args = { .log_addr = addr, .log_len = DEFAULT_TRANSFER, .phys_addr = (unsigned long)str, .direction = direct_read };
    root_strategy->ops[0][opcode_trans]((void*)root_strategy, &args, "");
    if (args.success) {
      printf("%s", str);
    }
  }
	
  bool cmd(addr_t addr, char * str, char * param) {
    struct args args = { .log_addr = addr, .log_len = strlen(str), .phys_addr = (unsigned long)str, .direction = direct_write };
    root_strategy->ops[0][opcode_trans]((void*)root_strategy, &args, param);
    return args.success;
  }

  addr_t boot(char * str, char * param) {  
    struct args args = { .log_len = DEFAULT_TRANSFER, .where = FALSE, .exclu = TRUE, .action = action_wait, .melt = TRUE, .try_len = DEFAULT_TRANSFER, .op_code = opcode_gadr };
    root_strategy->ops[0][opcode_gadr]((void*)root_strategy, &args, param);
    if(!args.success) {
      printf("!!!!! gadr failed\n");
      exit(-1);
    }
    args.log_len = strlen(str);
    args.phys_addr = (unsigned long)str;
    args.direction = direct_write;
    args.op_code = opcode_trans;
    root_strategy->ops[0][opcode_trans]((void*)root_strategy, &args, param);
    if(!args.success) {
      printf("!!!!! trans '%s' failed\n", str);
      exit(-1);
    }
    return args.log_addr;
  }

  char str[256];
  addr_t control = DEFAULT_TRANSFER;
  cmd(control, "output:=control\n", "");
  sprintf(str, "brick:=map_dummy\n output:=out{\n");
  addr_t map = boot(str, "");
  sprintf(str, "brick:=adapt_meta\n connect in:=%llx:out\n output:=out\n", map);
  addr_t meta = boot(str, "");
  // create root hook
  cmd(0, "output:=_root", "");
  cmd(meta, "connect hook:=0:_root", "");
  // strategy level
  sprintf(str, "brick:=fs_simple\n connect strat:=%llx:control\n output:=control{\n", control);
  addr_t fs = boot(str, "_root");

  sprintf(str, "brick:=strategy_ulinux\n connect strat:=%llx:control\n connect fs:=%llx:control\n", control, fs);
  addr_t ulinux = boot(str, "");
  sprintf(str, "output:=posix\n");
  char addr[32];
  sprintf(addr, "%lld", ulinux);
  cmd(ulinux, str, addr);
  sleep(INT_MAX);

  cmd(meta, "brick/=adapt_meta", "");
  cmd(map, "brick/=map_dummy", "");
  //cmd(test, "brick/=test_thread", "");
  return 0;
}
