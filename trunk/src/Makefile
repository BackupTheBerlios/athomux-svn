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

all: testbed

# this is very provisionary and should be replaced by a better parser in Perl!
defs.make defs.h loaders.h: Makefile
	(/bin/echo -n "bricks="; /bin/echo `ls *.ath | grep -v common | sed 's/\.ath/.c/'`) > defs.make
	(/bin/echo -n "objs="; /bin/echo `ls *.ath | grep -v common | sed 's/\.ath/.o/'`) >> defs.make
	(/bin/echo -n "headers="; /bin/echo `ls *.ath | grep -v common | sed 's/\.ath/.h/'`) >> defs.make
	ls *.ath | xargs grep '^instance' | sed 's/^\([^.]*\).ath:instance *#\([^ ]*\).*/\1.c : \2.ath/g' | sort -u >> defs.make
# currently this will not work transitively
	ls *.ath | xargs grep '^@\.include' | sed 's/^\([^.]*\).ath:[^@]*@\.include *"\([^"]*\)".*/\1.c : \2/g' | sort -u >> defs.make
	ls *.ath | grep -v common | sed 's/^/#include "/' | sed 's/\.ath/.h"/' > defs.h
	ls *.ath | grep -v common | sed 's/^/  \&loader_/' | sed 's/\.ath/,/' > loaders.h

-include defs.make

%.c : %.ath common.ath pre.pl Makefile
	$(TIME) ./pre.pl $< 

%.o : %.c common.h strat.h Makefile
	$(CC) -c $(CFLAGS) $< -o $@

common.c : common.ath
	# no real dependency

allobjs=$(objs) strat.o

testbed: $(bricks) $(allobjs) common.c testbed.c 
	$(CC) $(CFLAGS) $(LDFLAGS) $(allobjs) common.c -o testbed testbed.c

benchbed: $(bricks) $(allobjs) common.c benchbed.c 
	$(CC) $(CFLAGS) $(LDFLAGS) $(allobjs) common.c -o benchbed benchbed.c

my_cp: my_cp.c Makefile
	$(CC) $(CFLAGS) -o my_cp my_cp.c

strat.o: strat.c strat.h
control_dummy_linux.o: control_dummy_linux.c control_dummy_linux.h defs.h loaders.h

clean:
	rm -f defs.make defs.h loaders.h my_cp testbed benchbed testfile* *~ core* gmon.out *.pre
	rm -f $(bricks) $(allobjs) $(headers)

