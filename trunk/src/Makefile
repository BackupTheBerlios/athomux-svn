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
CC=$(TIME)  gcc-3.3

all : all_targets

defs.make debug.init: Makefile makegen.pl pconf.* cconf.* target.* *.ath
ifdef SUBMAKE
	@echo Warning: Infinite make recursion attempt detected. There might be a problem with your file timestamps!
else
	./makegen.pl
# do recursive calls to check for broken targets.
# (if you want to add checks for new targets, take pconf.klinux as example)
	@$(MAKE) SUBMAKE=1 -is check_broken >/dev/null 2>/dev/null
	@$(MAKE) SUBMAKE=1 -s show_broken
endif

# some additional dependencies
*/*.c */*.h : pre.pl common.ath pconf.*
*/*/lib.o : debug.init

-include defs.make

%/*/strat.o : strat.h %/strat.c

*/common.c : common.ath
	# no real dependency

%/*/*/*.o: %/control*.c %/control*.h %/*/*/defs.h %/*/*/loaders.h

.PHONY: clean
clean:
	rm -f defs.make debug.* testfile* *~ core* gmon.out
	rm -rf $(pconf_list)

