/* Author: Thomas Schoebel-Theuer
 * Copyright: University of Stuttgart
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>


#define BASEFILE __FILE__
#include "common.h"

void my_copy(int in, int out)
{
  int count = 0;
  int j;
  char buf[1000][DEFAULT_TRANSFER];
  char buf2[DEFAULT_TRANSFER];
  lseek(in, 0, SEEK_SET);
  lseek(out, 0, SEEK_SET);
  for(;;) {
    int len = read(in, buf[count], DEFAULT_TRANSFER);
    int status;
    memcpy(buf2, buf[count], DEFAULT_TRANSFER);
    lseek(out, 0, SEEK_CUR);
    status = write(out, buf2, DEFAULT_TRANSFER);
    count++;
    if(len < DEFAULT_TRANSFER)
      break;
  }
  printf("count: %d\n", count);
  lseek(in, 0, SEEK_SET);
  for(j = 0; j < count; j++) {
    char buf2[DEFAULT_TRANSFER];
    int len = read(in, buf2, DEFAULT_TRANSFER);
    (void)len;
    if(memcmp(buf[j], buf2, DEFAULT_TRANSFER)) {
      printf("mismatch\n");
    }
  }
}

int main()
{
  int in1;
  int out1;
  int in2;
  int out2;
  int i;

  in1 = open("/etc/termcap", O_RDONLY);
  out1 = open("testfile1", O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
  in2 = open("/etc/sensors.conf", O_RDONLY);
  out2 = open("testfile2", O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);

  for(i = 0; i < 1000; i++) {
    my_copy(in1, out1);
    my_copy(in2, out2);
  }
  return 0;
}
