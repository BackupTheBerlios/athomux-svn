/* Copyright (c) 2004 Jens-Christian Korth
 * This program is free software according to
 * files SOFTWARE-LICENSE and PATENT-LICENSE
 */

#define ATHOMUX_BOOTSTRAP
#include <stdio.h>
#include "init.h"


int main(int argc, char * argv[])
{
  ATHOMUX_INIT_INIT();

  if(argc>1) {
    if(argc>2) {
      ATHOMUX_INIT(argv[1], argv[2]);
      ATHOMUX_CLEANUP(argv[1], argv[2]);
    } else {
      ATHOMUX_INIT(argv[1], "");
      ATHOMUX_CLEANUP(argv[1], "");
    }
  } else {
    ATHOMUX_INIT("athload_help", "");
    ATHOMUX_CLEANUP("athload_help", "");
  }
  printf("%s terminated normally.\n", argv[0]);
  return 0;
}
