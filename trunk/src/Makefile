# Author: Thomas Schoebel-Theuer
# Copyright: University of Stuttgart

# the follwing options produce _worse_ code: -ftsp-ordering
# use the following only if you don't want to debug!
OPTFLAGS=-fomit-frame-pointer -fmerge-all-constants -frename-registers
LDFLAGS=-lrt
#LDFLAGS=-lrt -lefence

#CFLAGS=-Wall -O -ggdb3 -pg -DDEBUG -DTRACE
#CFLAGS=-Wall -O -ggdb3 -pg -DDEBUG
#CFLAGS=-Wall -O -ggdb3 -pg
#CFLAGS=-Wall -O -ggdb3 -pg -DSILENT
#CFLAGS=-Wall -O3 -DSILENT
CFLAGS=-Wall -O3 $(OPTFLAGS)
TIME=
#TIME=/usr/bin/time --format="%E"
CC=$(TIME)  gcc

all : all_targets

defs.make defs.h loaders.h: Makefile makegen.pl
	./makegen.pl

allobjs=$(objs) strat.o

strat.o: strat.c strat.h

-include defs.make

common.c : common.ath
	# no real dependency

%/*/control_dummy_linux.o: %/control_dummy_linux.c %/control_dummy_linux.h defs.h loaders.h

clean:
	rm -f defs.make defs.h loaders.h testfile* *~ core* gmon.out *.pre
	rm -rf $(pconf_list)

