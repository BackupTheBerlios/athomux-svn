/* Copyright (c) 2004 Jens-Christian Korth
 * This program is free software according to
 * files SOFTWARE-LICENSE and PATENT-LICENSE
 */

#define ATHOMUX_BOOTSTRAP
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "init.h"

char * init = "athload_help";
char * param = "";
bool wait_after_init = FALSE;

sig_t old_SIGINT;


int do_shutdown() {
  signal(SIGINT, old_SIGINT);

  if(ATHOMUX_CLEANUP(init, param)) {
    printf("%s terminated normally.\n", init);
    return 0;
  } else {
    printf("%s terminated with error!\n", init);
    return 2;
  }
}

void shutdown_handler(int signum)
{
  printf("Shutdown requested...\n");
  exit(do_shutdown());
}

int main(int argc, char * argv[])
{
#ifdef DEBUG
  open_debug();
#endif
  int arg_nr = 0;
  int i;
  for (i=1; i<argc; i++) {
    if(!arg_nr && (argv[i][0] == '-')) {
      if(!strcmp(argv[i], "--help"))
	break;
      else if(!strcmp(argv[i], "-w") || !strcmp(argv[i], "--wait"))
	wait_after_init = TRUE;
      else
	printf("unknown option '%s'\n", argv[i]);
    } else {
      if(++arg_nr == 1)
	init = argv[i];
      else if(arg_nr == 2)
	param = argv[i];
    }
  }

  ATHOMUX_INIT_INIT();

  old_SIGINT = signal(SIGINT, shutdown_handler);

  if(!ATHOMUX_INIT(init, param)) {
    printf("%s terminated initialization with error!\n", init);
    return 1;
  }

  if(wait_after_init) {
    while(1) {
      pause();
    }
  }

  int res = do_shutdown();
#ifdef DEBUG
  close_debug();
#endif
  return res;
}
