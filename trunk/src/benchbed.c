/* Author: Thomas Schoebel-Theuer
 * Copyright: University of Stuttgart
 */

#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>

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
  // create root hook
  cmd(0, "output:=_root", "");
  cmd(meta, "connect hook:=0:_root", "");
  // strategy level
  sprintf(str, "brick:=fs_simple\n connect strat:=%llx:control\n output:=control{\n", control);
  addr_t fs = boot(str, "_root");
  // testing

  sprintf(str, "brick:=bench_fs\n connect strat:=%llx:control\n", fs);
  addr_t bench = boot(str, "");

  printf("================== STARTING BENCH ================\n");
  cmd(bench, "output:=dummy", "1");
  printf("================== END BENCH ================\n");

  cmd(meta, "brick/=adapt_meta", "");
  printf("------------------------------------\n");
  printf("size_input=%u\n", (unsigned) sizeof(struct input));
  printf("size_output=%u\n", (unsigned) sizeof(struct output));
  printf("size_conn=%u\n", (unsigned) sizeof(struct connector_header));
  printf("size_brick=%u\n", (unsigned) sizeof(struct brick_header));

  struct timespec t0, t1;
  printf("------------------------------------\n");

  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t0);
  int i;
  for(i = 0; i < 1000000; i++) {
	  getpid();
	  getpid();
  }
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t1);
  printf("getpid time=%11lld\n", ((long long)t1.tv_sec-(long long)t0.tv_sec)*1000LL + ((long long)t1.tv_nsec-(long long)t0.tv_nsec)/1000000LL);
  printf("------------------------------------\n");

  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t0);
  for(i = 0; i < 1000000; i++) {
	  int fd = open("/etc/passwd", O_RDONLY);
	  close(fd);
  }
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t1);
  printf("open   time=%11lld\n", ((long long)t1.tv_sec-(long long)t0.tv_sec)*1000LL + ((long long)t1.tv_nsec-(long long)t0.tv_nsec)/1000000LL);
  printf("------------------------------------\n");

  int fd = open("/etc/services", O_RDONLY);
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t0);
  for(i = 0; i < 1000000; i++) {
	  char buf[4096];
	  lseek(fd, 0, SEEK_SET);
	  read(fd, buf, 4096);
  }
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t1);
  close(fd);
  printf("read   time=%11lld\n", ((long long)t1.tv_sec-(long long)t0.tv_sec)*1000LL + ((long long)t1.tv_nsec-(long long)t0.tv_nsec)/1000000LL);
  printf("------------------------------------\n");
  sprintf(str, "brick:=adapt_meta\n connect in:=%llx:out\n", map);
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t0);
  for(i = 0; i < 1000000; i++) {
    struct conn_info conn1 = { .conn_addr = meta, .conn_name = "in" };
    struct conn_info conn2 = { .conn_addr = map, .conn_name = "out" };
    struct args args = { .log_addr = meta, .name = "adapt_meta", .conn1 = &conn1, .conn2 = &conn2 };
    root_strategy->ops[0][opcode_instbrick]((void*)root_strategy, &args, "");
    root_strategy->ops[0][opcode_connect]((void*)root_strategy, &args, "");
    root_strategy->ops[0][opcode_deinstbrick]((void*)root_strategy, &args, "");
#if 0
	  addr_t adr = cmd(meta, str, "");
	  cmd(meta, "brick/=adapt_meta\n", "");
#endif
  }
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t1);
  printf("inst   time=%11lld\n", ((long long)t1.tv_sec-(long long)t0.tv_sec)*1000LL + ((long long)t1.tv_nsec-(long long)t0.tv_nsec)/1000000LL);
  printf("------------------------------------\n");
  cmd(map, "brick/=map_dummy", "");
#ifdef DEBUG
  close_debug();
#endif
  return 0;
}
