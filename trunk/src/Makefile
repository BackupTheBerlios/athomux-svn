# Author: Thomas Schoebel-Theuer
# Copyright: University of Stuttgart

# the follwing options produce _worse_ code: -ftsp-ordering
# use the following only if you don't want to debug!
OPTFLAGS=-fomit-frame-pointer -fmerge-all-constants -frename-registers
LDFLAGS=-lrt
#LDFLAGS=-lrt -lefence

#CFLAGS=-Wall -O -ggdb3 -pg -DDEBUG -DTRACE
#CFLAGS=-Wall -O -ggdb3 -pg
#CFLAGS=-Wall -O -ggdb3 -pg -DSILENT
#CFLAGS=-Wall -O3 -DSILENT
TIME=
#TIME=/usr/bin/time --format="%E"
CC=$(TIME)  gcc

all : all_targets

defs.make: Makefile makegen.pl pconf.* cconf.* target.*
	./makegen.pl

allobjs=$(objs) strat.o

-include defs.make

common.c : common.ath
	# no real dependency

%/*/control_dummy_linux.o: %/control_dummy_linux.c %/control_dummy_linux.h %/*/defs.h %/*/loaders.h

clean:
	rm -f defs.make testfile* *~ core* gmon.out
	rm -rf $(pconf_list)

