#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <linux/elf.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include "static_loader.h"

#define BUFSIZE 512

void die(char *x) {

  if (errno) {
    perror(x) ;
  } else {
    fprintf(stderr, "Could not load %s!\nAre you sure it is a statically linked binary (no ELF or script)?\n", x);
  }
  _exit(1);
}

/* function does not return */
void load_static(int argc, char ** argv, int envc, char ** envp) {

  unsigned entry;

  if (argc < 2) {
    fprintf(stderr, "No binary supplied!\n");
    exit(1);
  }

  //show_maps();
#ifdef DEBUG
  printf("=== loading [%s]\n", argv[1]);
#endif
  if (!(entry = load_elf(argv[1]))) 
    die(argv[1]);
#ifdef DEBUG
  printf("=== %-12s entry     0x%08x\n", argv[1], entry);
#endif
  //show_maps();
#ifdef DEBUG
  printf("=== restarting from        0x%08x\n", entry);
#endif
  go_to_start(argc - 1, &argv[1], envc, envp, entry);
}


void show_maps() {

  char mapsline[256];
  FILE * maps_file = fopen("/proc/self/maps", "r");
  while (NULL != fgets(mapsline, sizeof(mapsline), maps_file))
    printf("%s", mapsline);

  fclose(maps_file);
}

int load_elf(char *arg) {
  
  int fd = 0, nread = 0;
  unsigned char buf[BUFSIZE];
  Elf32_Ehdr *elf;

  if ((fd = open(arg, O_RDONLY)) < 0)
    return 0;

  if ((nread = read(fd, buf, 52)) < 52)
    return 0;

  elf = (Elf32_Ehdr *) buf;

  /* Is it really an ELF? */
  if (!((elf->e_ident[EI_MAG0] == ELFMAG0) && 
	(elf->e_ident[EI_MAG1]  == ELFMAG1) && 
	(elf->e_ident[EI_MAG2]  == ELFMAG2) && 
	(elf->e_ident[EI_MAG3]  == ELFMAG3)))
    return 0;

  if (!(load_sections(fd, elf->e_phentsize, elf->e_phnum, elf->e_phoff))) 
    return 0;

  close(fd);

  return elf->e_entry;
}

int load_sections(int fd, unsigned short psize, unsigned short pnum,
		  int poff) {

  unsigned flags = 0;
  int nread = 0, offset = 0;
  unsigned char buf[pnum * psize + 1];
  Elf32_Phdr *phdr = NULL;
  unsigned pagesize = getpagesize();

  if ((lseek(fd, poff, SEEK_SET)) < 0) return 0;

  if ((nread = read(fd, buf, psize * pnum)) < 1) return 0;

  for (nread = 1; nread <= pnum; nread++) {
    phdr = (Elf32_Phdr *) (buf + offset);
    offset += psize;

    flags = 0;
    if (phdr->p_flags & PF_R)
      flags |= PROT_READ;
    if (phdr->p_flags & PF_W)
      flags |= PROT_WRITE;
    if (phdr->p_flags & PF_X)
      flags |= PROT_EXEC;

    if (phdr->p_type == PT_INTERP) { 
      /* Oops, it turns out to be a dynamically linked program ... */
      fprintf(stderr, "You are trying to execute a dynamically linked program!\n");
      return 0;
    } else if (phdr->p_type == PT_LOAD) {

      //printf("a) %x-%x\n", 
      //phdr->p_vaddr & ~(pagesize - 1), 
      //(phdr->p_vaddr & ~(pagesize - 1)) +
      //phdr->p_filesz + (phdr->p_offset & (pagesize - 1)));

      mmap((void*)(phdr->p_vaddr & ~(pagesize - 1)), 
	   phdr->p_filesz + (phdr->p_offset & (pagesize - 1)), 
	   flags, MAP_PRIVATE | MAP_FIXED, fd,
	   phdr->p_offset & ~(pagesize - 1));

      //printf("mmaped size: %x\n", phdr->p_filesz + (phdr->p_offset & (pagesize - 1)));

      if (phdr->p_filesz != phdr->p_memsz) {
	/* partially filled .data? */
	int addr = phdr->p_vaddr + phdr->p_filesz;
	int size = pagesize - ((phdr->p_vaddr + phdr->p_filesz) & (pagesize - 1));
	//printf("b) %x-%x\n", addr, addr + size);
	
	memset((void*) addr, 0, size);
      }
      
      if (phdr->p_filesz / pagesize != phdr->p_memsz / pagesize) {
	/* .data with .bss needing another page? */
	int addr = ((phdr->p_vaddr + phdr->p_filesz) / pagesize + 1) * pagesize;
	int size = ((phdr->p_memsz-phdr->p_filesz + pagesize - 1) / pagesize) * pagesize;
	//printf("c) %x-%x\n", addr, addr + size);

	mmap((void*) addr, size, flags,
	     MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS, 0, 0);

	//printf("mmaped size: %x\n", size);

      }
    }
  }
  return 1;
}
