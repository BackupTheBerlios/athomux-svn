/* Author: Thomas Schoebel-Theuer
 * Copyright: University of Stuttgart
 */

#include <stdlib.h>

//#define BASETEST

#define PATH "/test1/test2/test3/test4"

#define BASEFILE "testbed.c"
#include "control_simple.h"

int main(int argc, char * argv[])
{
#ifdef DEBUG
  open_debug();
#endif
  init_static_control_simple();
  void ask(addr_t addr)
    {
      char str[DEFAULT_TRANSFER];
      struct args args = { .log_addr = addr, .log_len = DEFAULT_TRANSFER, .phys_addr = (unsigned long)str, .direction = direct_read };
      root_strategy->ops[0][opcode_trans]((void*)root_strategy, &args, "");
      if(args.success) {
	 printf("%s", str);
      }
    }
  bool cmd(addr_t addr, char * str, char * param)
    {
      struct args args = { .log_addr = addr, .log_len = strlen(str), .phys_addr = (unsigned long)str, .direction = direct_write };
      root_strategy->ops[0][opcode_trans]((void*)root_strategy, &args, param);
      return args.success;
    }
  addr_t boot(char * str, char * param)
    {
      struct args args = { .log_len = DEFAULT_TRANSFER, .reader = FALSE, .exclu = TRUE, .action = action_wait, .melt = TRUE, .try_len = DEFAULT_TRANSFER, .op_code = opcode_gadr };
      root_strategy->ops[0][opcode_gadr]((void*)root_strategy, &args, param);
      if(!args.success) {
        printf("!!!!! gadrcreateget failed\n");
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
  // basic level
#if 0
  addr_t dev = boot("brick:=device_dummy_linux\n output:=out", "testfile");
  sprintf(str, "brick:=buffer_dummy_linux\n \n connect dev:=%llx:out\n output:=out {\n", dev);
  addr_t buffer = boot(str, "");
  sprintf(str, "brick:=map_simple\n connect in:=%llx:out\noutput:=out", buffer);
#else
  sprintf(str, "brick:=map_dummy\n output:=out{\n");
#endif
  addr_t map = boot(str, "");
  sprintf(str, "brick:=adapt_meta\n connect in:=%llx:out\n output:=out\n", map);
  addr_t meta = boot(str, "");
#if 0
  sprintf(str, "brick:=spy_trace\n connect in:=%llx:out\n output:=out\n", meta);
  meta = boot(str, "spylog");
#endif
  // create root hook
  cmd(0, "output:=_root", "");
  cmd(meta, "connect hook:=0:_root", "");
  // strategy level
  sprintf(str, "brick:=fs_simple\n connect strat:=%llx:control\n output:=control{\n", control);
  addr_t fs = boot(str, "_root");
  // testing
  sprintf(str, "brick:=bench_fs\n connect strat:=%llx:control\n", fs);
  addr_t bench = boot(str, "");
#if 1
  ask(0);
  ask(DEFAULT_TRANSFER);
  ask(0x3000);
#endif
  printf("================== STARTING BENCH ================\n");
  cmd(bench, "output:=dummy", "1000");
  printf("================== END BENCH ================\n");
#ifdef MOVE_COUNT
  extern void print_counts(void);
  print_counts();
  printf("------------------------------------\n");
  printf("_makespace: shortcuts: %d count1: %d count2: %d\n", count_0, count_1, count_2);
  printf("creates (count_c1) = %d, triggering creates (count_c2) = %d\n", count_c1, count_c2);
#endif
  cmd(meta, "brick/=adapt_meta", "");
  cmd(map, "brick/=map_dummy", "");
#ifdef DEBUG
  close_debug();
#endif
  return 0;
}
