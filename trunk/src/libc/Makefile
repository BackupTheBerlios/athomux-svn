CFLAGS=-Wall -ggdb -march=pentium3 -m32

OFILES=atoi.o \
atol.o \
ctype.o \
doprnt.o \
doscan.o \
fprintf.o \
fstubs.o \
memchr.o \
memcmp.o \
memset.o \
putchar.o \
printf.o \
random.o \
sprintf.o \
sscanf.o \
strcasecmp.o \
strcat.o \
strchr.o \
strcmp.o \
strcpy.o \
strlen.o \
strncat.o \
strncmp.o \
strncpy.o \
strtol.o \
strtoll.o \
strtoul.o \
vfprintf.o

%.o: %.c
	$(CC) $(CFLAGS) -I. -c $< -o $@

libc.a: Makefile $(OFILES)
	ar rcs libc.a $(OFILES)

all_targets: libc.a	
