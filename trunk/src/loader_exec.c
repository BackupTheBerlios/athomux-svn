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

char name[256];
char stack_ptr[4096];

extern void go_to_start_exec(int argc, char **argv, int evnc, char **envp, unsigned stack, unsigned entry);


static int sig_catcher(int signo) {

  printf(" ! pid: [%s] signal %d caught\n", name, signo);
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
	if (munmap((void*) address, size))
	  return 0;
	if (phdr->p_filesz / pagesize != phdr->p_memsz / pagesize)
	  munmap((void*)(((phdr->p_vaddr + phdr->p_filesz) / pagesize + 1) * pagesize), 
		 ((phdr->p_memsz-phdr->p_filesz+pagesize - 1) / pagesize - 1) * pagesize);
      }
    }
  }

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
#ifdef DEBUG
  printf("=== loading image  [%s]\n", arg);
#endif
  if ((fd = open(arg, O_RDWR)) < 0) {
#ifdef DEBUG
    printf("=== loading image failed\n");
#endif
    return 0;
  }

  if (mmap((char *) IMAGE_START, IMAGE_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC,
	   MAP_SHARED | MAP_FIXED, fd, 0) == MAP_FAILED) {
#ifdef DEBUG
    printf("=== mmap image failed\n");
#endif
    return 0;
  }
  
  return 1;
} 

/* 
 * argv: [0] "loader"
 *       [1] entrypoint
 *       [2] /pathname/pid  (memory image of called process in userspace fs)
 *       [3] progname  (argv[0] of called process)
 * TODO:
 * close open filedescriptors
 * 
 */
void _start(char *arguments, ...) {

  int index;
  int argv_size = 0;
  int envp_size = 0;
  char **av = &arguments;
  int argc = * (int *) (av - 1);
  char **env = av + argc + 1;
  int envc;

  char pid_name[256];
  int entry = atoi(av[1]);

  stack_t st;
  st.ss_sp = stack_ptr;
  st.ss_flags = 0;
  st.ss_size = 4096;
  sigaltstack(&st, 0);

  struct sigaction s;
  s.sa_handler = (void *) sig_catcher;
  sigemptyset(&s.sa_mask);
  s.sa_flags = SA_ONSTACK;
  sigaction(SIGUSR1, &s, 0);

  for (envc = 0; env[envc] != 0; envc ++);

  snprintf(pid_name, 256, "%s", av[2]);
  snprintf(name, 256, pid_name);

  // put argv and envp on the local stack
  //char *argv[argc - 2];
  char **argv = alloca(sizeof (char *) * (argc - 2));
  //char *envp[envc + 1];
  char **envp = alloca(sizeof (char *) * (envc + 1));

  for (index = 3; index < argc; index ++) {

    int s = strlen(av[index]) + 1;
    argv_size += s;
    argv[index - 3] = (char *) alloca(s);
    memcpy(argv[index - 3], av[index], s);
  }

  for (index = 0; index < envc; index ++) {
		
    int s = strlen(env[index]) + 1;
    envp_size += s;
    envp[index] = (char *) alloca(s);
    memcpy(envp[index], env[index], s);
  }

  argv[argc - 2] = 0;
  envp[envc] = 0;
  argc -= 3;

  // unload the athomux kernel, load the image
  //show_maps();
  //printf("-------------------------\n");
  unload_elf();
  //show_maps();
  //printf("-------------------------\n");
  if (!load_image(pid_name))
    exit(1);
  //show_maps();
  //printf("-------------------------\n");

  // put argv and envp on top of the stack in the image
  char *stack = (char *) (IMAGE_START + IMAGE_SIZE);
  char *sp = stack;
  unsigned *argv_pointer = (unsigned *) (sp - argv_size - (argc + 1) * 4);
  unsigned *envp_pointer;
  
  for (index = 0; index < argc; index ++) {

    int length = strlen(argv[index]) + 1;
    memcpy(sp - length, argv[index], length);
    argv_pointer[index] = (unsigned) (sp - length);
    sp -= length;
  }

  // substract length of pointer array from stackpointer
  sp -= ((argc + 1) * 4);

  //envc = 0;

  envp_pointer = (unsigned *) (sp - envp_size - (envc + 1) * 4);

  for (index = 0; index < envc; index ++) {

    int length = strlen(envp[index]) + 1;
    memcpy(sp - length, envp[index], length);
    envp_pointer[index] = (unsigned) (sp - length);
    sp -= length;
  }

  // should already be 0 since the image is completely 0
  argv_pointer[argc] = 0;
  envp_pointer[envc] = 0;

#ifdef DEBUG
  printf("=== restarting from        0x%08x\n", entry);
#endif
  // start the program in the image
  go_to_start_exec(argc, (char **) argv_pointer, envc, (char **) envp_pointer, (unsigned) (envp_pointer - 4), entry);
}
