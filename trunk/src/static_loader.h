#ifndef STATIC_LOADER_H
#define STATIC_LOADER_H

void die(char *x);
void show_maps();

int load_sections(int fd, unsigned short psize, unsigned short pnum, int poff);
void load_static(int argc, char ** argv, int envc, char ** envp);
int load_elf(char *arg);

extern void go_to_start();

#endif
