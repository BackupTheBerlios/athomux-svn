# path to source and object tree
PREFIX=/usr/src/linux/

all: stat_sym_obj_obj.txt stat_sym_mod_mod.txt stat_sym_dir_dir.txt stat_sym_sys_sys.txt stat_grp_obj_obj.txt stat_grp_mod_mod.txt stat_grp_dir_dir.txt stat_grp_sys_sys.txt stat_def_obj.txt stat_def_mod.txt

.PRECIOUS: dir_%.txt sys_%.txt %_exports.txt %_imports.txt %.dot %.ps join_obj_%.txt join_mod_%.txt join_dir_%.txt join_sys_%.txt join_stripped_%.txt stat_sym_%.txt stat_grp_%.txt stat_def_%.txt

.PHONY:	clean
clean:
	rm -f *.txt

# IMPORTANT NOTE:
# This tool uses many .txt files which stand for a set of data.
# In the following the relationships between the sets are being defined
# -> THIS IS THE BASE IDEA OF THIS TOOL! YOU NEED TO UNDERSTAND THAT!


# list of .o files
obj_files.txt:
	find $(PREFIX) -name *.o -and -not -name *.mod.o -and -not -name built-in.o > $@

# list of .ko files
mod_files.txt:
	find $(PREFIX) -name *.ko -or -name built-in.o > $@

# valid prefixes:
# obj_	use symbols as extracted on object file level
# mod_	use symbols as extracted on module level
# dir_	group symbols on directory level
# sys_	group symbols on subsystem level (as defined in subsys.sed)

# group exports/imports by directory
dir_%.txt: obj_%.txt
	sed -e "s/^\([^ ]*\)\/[^\/\t]*/\1/" $< | sort -k2 | uniq - $@

# group exports/imports by subsystem
sys_%.txt: obj_%.txt subsys.sed
	sed -f subsys.sed $< | sort -k2 | uniq - $@

# list of exported symbols
%_exports.txt: %_files.txt
	cat $< | xargs nm -g -o --defined-only | sed -e "s|^$(PREFIX)||g; s/\\:[0-9a-f ]* . /\t/g;" | grep [[:space:]] | sort -k2 | ./bistrip.pl - $(PREFIX) > $@

# list of imported symbols
%_imports.txt: %_files.txt
	cat $< | xargs nm -g -o --undefined-only | sed -e "s|^$(PREFIX)||g; s/\\: *. /\t/;" | grep [[:space:]] | sort -k2 | ./bistrip.pl - $(PREFIX) > $@


# join combinations of export and import lists
#
# format: symbol provided_by required_by
join_obj_%.txt: obj_exports.txt %_imports.txt
	join -e"(undefined)" -o 0,1.1,2.1 -a 1 -1 2 -2 2 $^ > $@
join_mod_%.txt: mod_exports.txt %_imports.txt
	join -e"(undefined)" -o 0,1.1,2.1 -a 1 -1 2 -2 2 $^ > $@
join_dir_%.txt: dir_exports.txt %_imports.txt
	join -e"(undefined)" -o 0,1.1,2.1 -a 1 -1 2 -2 2 $^ > $@
join_sys_%.txt: sys_exports.txt %_imports.txt
	join -e"(undefined)" -o 0,1.1,2.1 -a 1 -1 2 -2 2 $^ > $@

# remove duplicate entries
join_stripped_%.txt: join_%.txt
	./dupstrip.pl $< > $@

# create dot file
%.dot: join_stripped_%.txt
	echo digraph $(patsubst %.dot,%,$@) { > $@
	cut -d" " -f2,3 $< | sort | uniq | sed -e "s/^\([^ ]*\) \([^ ]*\)$$/\"\2\" -> \"\1\";/" >> $@
	echo } >> $@

# create statistics: most used symbols on ___ level
stat_sym_%.txt: join_stripped_%.txt
	cut -d" " -f1,2 $< | uniq -c | sort -nrs -o $@
# restrict to most used symbol of a ___
stat_usym_%.txt: stat_sym_%.txt
	sort -nrs $< | sort -sk3 | uniq -f2 - $@
# table of appendix B of my thesis
stat_table.txt: stat_grp_obj_sys.txt stat_grp_obj_obj.txt stat_usym_obj_obj.txt
	sort -sk2 stat_grp_obj_sys.txt -o tmp1
	sort -sk2 stat_grp_obj_obj.txt -o tmp2
	sort -sk3 stat_usym_obj_obj.txt -o tmp3
	join -e0 -o 0,1.1,2.1 -a 1 -1 2 -2 2 tmp1 tmp2 | join -e"(undefined)" -o 1.3,1.2,0,2.2 -a 1 -2 3 - tmp3 | sort -nrsk2 | sort -nrs -o $@
	rm tmp1 tmp2 tmp3
# how many ___ use this ___
stat_grp_%.txt: join_stripped_%.txt
	cut -d" " -f2,3 $< | sort | uniq | cut -d" " -f1 | sort | uniq -c | sort -nrs -o $@

# how many ___ export a symbol?
stat_def_%.txt: %_exports.txt
	cut -f2 $< | uniq -c | grep -v '^      1 ' | sort -nrs -o $@

# create graphviz files
.PHONY: dots dots_dir dots_obj
dots: join_stripped_obj_sys.txt subsys.sed
	sed -e "s/^\([^ ]*\) \([^ ]*\) \([^ ]*\)$$/\2\t\2\t\3/" -f subsys.sed $< | sort -k3 | uniq | ./split2dot.pl - linux_dep_

dots_sys: join_dir_dir.txt subsys.sed
	grep -v "(undefined)" $< | sed -e "s/^\([^ ]*\) \([^ ]*\) \([^ ]*\)$$/\2\t\2\t\3/" -f subsys.sed -e "s/^[^\t]*\t\([^\t]*\)\t\1$$/DONT:/; s/^/DONT:/; s/^DONT:\([^\t]*\)\t\([^\t]*\)\t\(\1[^\t]*\)$$/\1\t\2\t\3/;" | grep -v "DONT:" | sort -k2 | uniq | ./split2dot2.pl - linux_subsys_

dots_dir: join_stripped_dir_dir.txt subsys.sed
	sed -e "s/^\([^ ]*\) \([^ ]*\) \([^ ]*\)$$/\2\t\2\t\3/" -f subsys.sed -e "s/^/DONT:/; s/^DONT:\([^\t]*\)\t\([^\t]*\)\t\(\1[^\t]*\)$$/\1\t\2\t\3/;" $< | grep -v "DONT:" | sort -k2 | uniq | ./split2dot2.pl - linux_subsys_dir_

dots_obj: join_stripped_obj_obj.txt subsys.sed
	sed -e "s/^\([^ ]*\) \([^ ]*\) \([^ ]*\)$$/\2\t\2\t\3/" -f subsys.sed -e "s/^/DONT:/; s/^DONT:\([^\t]*\)\t\([^\t]*\)\t\(\1[^\t]*\)$$/\1\t\2\t\3/;" $< | grep -v "DONT:" | sort -k2 | uniq | ./split2dot2.pl - linux_subsys_obj_
# NOTE: The following error is harmeless:
# Can't open linux_subsys_: Datei oder Verzeichnis nicht gefunden at ./split2dot2.pl line 18,

stat_subsystems.txt: join_stripped_obj_sys.txt sys_exports.txt
	join -e"(undefined)" -o 1.2,1.3,2.1 -a 1 -2 2 $^ | sort -k2 | uniq | cut -d" " -f2,3 | uniq -c | sort -nr > $@
linux_subsystems.dot: stat_subsystems.txt
	sed -e "s/\([^ ]*\) \([^ ]*\) \([^ ]*\)$$/\\\t\"\2\" -> \"\3\";\\\n/;" $< | echo -e 'digraph "Linux Kernel Subsystems" {\n\tsize = "5,5";\n\tpage = "5,5";\n\tmargin = "0";\n'`cat -`} > $@
linux_subsystems.ps: linux_subsystems.dot
	circo -Tps $< -o $@

linux_subsys: linux_subsys_arch_i386.eps linux_subsys_drivers.eps linux_subsys_fs.eps linux_subsys_kernel.eps linux_subsys_net.eps linux_subsys_sound.eps

linux_subsys_%.ps: linux_subsys_%.dot dots_sys
	neato -Tps $< -o $@

%.ps: %.dot
	fdp -Tps $< -o $@
%.eps: %.ps
	ps2epsi $< $@
%.pdf: %.eps
	epstopdf $<
