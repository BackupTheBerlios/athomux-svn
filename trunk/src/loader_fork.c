// unload is part of diota (http://www.elis.UGent.be/diota/)

#include "types.h"

#include <fcntl.h>
#include <linux/elf.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <alloca.h>
#include <syscall.h>

extern void go_to_start_fork(unsigned base, unsigned stack, unsigned entry);


static int sig_catcher(int signo) {
  
  printf(" ! signal %d caught\n", signo);
  return signo;
}

void show_maps() {

  char mapsline[256];
  FILE * maps_file = fopen("/proc/self/maps", "r");
  while (NULL != fgets(mapsline, sizeof(mapsline), maps_file))
    printf("%s", mapsline);

  fclose(maps_file);
}

int unload_sections(unsigned load_address, unsigned short psize,
		    unsigned short pnum, int poff) {

  int nread = 0, offset = 0;
  Elf32_Phdr *phdr = NULL;
  unsigned address, size;
  unsigned pagesize = getpagesize();
	
  for (nread = 1; nread <= pnum; nread++) {
    phdr = (Elf32_Phdr *) (load_address + poff + offset);
    offset += psize;
		
    if (phdr->p_type == PT_LOAD) {
      address = phdr->p_vaddr & ~(pagesize - 1);
      size    = phdr->p_filesz + (phdr->p_offset & (pagesize - 1));
			
      if (address == load_address) {
	address += pagesize;
	size    -= (size>pagesize)?pagesize:size;
      }
			
      if (size) {
	//printf("1) %x-%x\n", address, address + size);
	if (munmap((void*) address, size)) {
	  return 0;
	}
	if (phdr->p_filesz / pagesize != phdr->p_memsz / pagesize) { 
	  /* remove .bss */
	  //printf("2) %x-%x\n", (((phdr->p_vaddr + phdr->p_filesz) / pagesize + 1) * pagesize),
	  //(((phdr->p_vaddr + phdr->p_filesz) / pagesize + 1) * pagesize) +
	  //((phdr->p_memsz-phdr->p_filesz + pagesize - 1) / pagesize - 1) * pagesize);
	  munmap((void*)(((phdr->p_vaddr + phdr->p_filesz) / pagesize + 1) * pagesize), 
		 ((phdr->p_memsz-phdr->p_filesz+pagesize - 1) / pagesize - 1) * pagesize);
	}
      }
    }
  }

  //printf("3) %x-%x\n", load_address, load_address + pagesize);

  if (munmap((void*) load_address, pagesize)) 
    return 0;
	
  return 1;
}


int unload_elf() {
  Elf32_Ehdr * elf = (Elf32_Ehdr *) 0x8048000;
  if (!(unload_sections(0x8048000, elf->e_phentsize, elf->e_phnum, elf->e_phoff))) return 0;

  return 1;
}


int load_image(char *arg) {
  
  int fd = 0;
  
  //printf("=== loading image  [%s]\n", arg);

  if ((fd = open(arg, O_RDWR)) < 0) {

    printf("=== loading image failed\n");
    return 0;
  }

  if (mmap((char *) IMAGE_START, IMAGE_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC,
	   MAP_SHARED | MAP_FIXED, fd, 0) == MAP_FAILED) {
    printf("=== mmap image failed\n");
    return 0;
  }
  
  return 1;
} 

/* 
 * argv: [0] "loader2"
 *       [1] basepointer
 *       [2] stackpointer
 *       [3] entrypoint
 *       [4] /pathname/pid  (memory image of called process in userspace fs)
 * 
 */
void _start(char *arguments, ...) {

  char **av = &arguments;
  int argc = * (int *) (av - 1);

  char pid_name[256];

  unsigned base = atoi(av[1]);
  unsigned stack = atoi(av[2]);
  unsigned entry = atoi(av[3]);

  struct sigaction s;
  s.sa_handler = (void *) sig_catcher;
  sigemptyset(&s.sa_mask);
  s.sa_flags = 0;
  sigaction(SIGUSR1, &s, 0);
	
  snprintf(pid_name, 256, "%s", av[4]);

  // unload the athomux kernel, load the image
  //show_maps();
  unload_elf();
  //show_maps();
  if (!load_image(pid_name))
    exit(1);
  //show_maps();
	
  //printf("=== restarting from        0x%08x\n", entry);

  // start the program in the image
  go_to_start_fork(base, stack, entry);
}
