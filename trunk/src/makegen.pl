#!/usr/bin/perl -w

# Author: Thomas Schoebel-Theuer
# Copyright: University of Stuttgart

use diagnostics;
use strict;
use re 'eval';
use English;
use integer;
use Digest::MD5 qw(md5),qw(md5_hex);

# general regex matching constants

my $cppmatch = qr'(?:^#(?:[^\\\n]|\\.|\\\n)*\n)'m;
my $commentmatch = qr'(?://[^\n]*\n|/\*(?:[^*]|\*[^/])*\*/)'m;
my $ws = qr"(?:\s|$commentmatch|^$cppmatch)*"m;
my $singlestringmatch = qr'(?:\"(?:[^"\\]|\\.)*\"|\'(?:[^"\\]|\\.)\')';
my $stringmatch = qr"(?:$singlestringmatch(?:$ws$singlestringmatch)*)"m;

# create additional Makefile rules out of the existing sources
# also, create defs.h and loaders.h automatically

################################################################

# name lists

my %filenames = ();
my %names = ();

sub build_names {
  my ($type, $pattern) = @_;
  my @files = `ls $pattern | grep -v "~" | grep -v "common\\."`;
  map chomp, @files;
  $filenames{$type} = \@files;
  my @copy = @files;
  if($type eq "ath") {
    map { s/\.\w+\Z// } @copy;
  } else {
    map { s/\A\w+\.// } @copy;
  }
  $names{$type} = \@copy;
}

build_names("pconf", "pconf.*");
build_names("cconf", "cconf.*");
build_names("target", "target.*");
build_names("ath", "*.ath");

################################################################

my %exceptions = ();

sub check_exception {
  my ($first, $second) = sort @_;
  return $exceptions{"$first:$second"};
}

sub make_exception {
  my $code = shift;
  my ($first, $second) = sort @_;
  $exceptions{"$first:$second"} = $code;
  #print "$first:$second = $code\n";
}

my %build_rules = ();
my %debug_names = ();

sub build_exceptions {
  my $basetype = shift;
  my @copy_names = @{$names{$basetype}};
  foreach my $source (@{$filenames{$basetype}}) {
    my $shortname = shift @copy_names;
    open IN, "< $source" or die "cannot open file '$source'";
    local $/;
    my $text = <IN>;
    close IN;
    for(;;) {
      if($text =~ m/^#?\s*(context)\s+(pconf|cconf|target|ath)\s*:\s*(.*)\n/m) {
	$text = $POSTMATCH;
	my $type = $2;
	my $list = $3;
	$list =~ s/\s//g;
	foreach my $pattern (split /,/, $list) {
	  my $code = 0;
	  $code = 1 if $pattern =~ s/\A!//;
	  foreach my $candidate (@{$names{$type}}) {
	    if($candidate =~ m/\A$pattern\Z/) {
	      make_exception($code, "$basetype.$shortname", "$type.$candidate")
	    }
	  }
	}
	next;
      }
      if($text =~ m/^\s*buildrules\s+(\w+)\s*:\s*\n?((?:.*\n)*?)\s*endrules/m) {
	$build_rules{"$source:$1"} = $2;
	$text = $POSTMATCH;
	next;
      }
      if($text =~ m/@\.trace\s*\(\s*(\w+)/g) {
	$debug_names{$1} = 1;
	$text = $POSTMATCH;
	next;
      }
      last;
    }
  }
}

build_exceptions("pconf");
build_exceptions("cconf");
build_exceptions("target");
build_exceptions("ath");

################################################################

# create subdirs

foreach my $pconf (@{$names{"pconf"}}) {
  system "mkdir -p $pconf && (cd $pconf && ln -sf ../*.h ../*.c .)"
    and die "cannot create subdirs / symlinks for $pconf";
  my $pconf_part = "pconf.$pconf";
  foreach my $cconf (@{$names{"cconf"}}) {
    my $cconf_part = "cconf.$cconf";
    next if check_exception($pconf_part, $cconf_part);
    system "mkdir -p $pconf/$cconf"
      and die "cannot create cconf subdir $pconf/$cconf";
    foreach my $target (@{$names{"target"}}) {
      my $target_part = "target.$target";
      next if check_exception($pconf_part, $target_part);
      next if check_exception($cconf_part, $target_part);
      system "mkdir -p $pconf/$cconf/extra_$target"
	and die "cannot create cconf subdir $pconf/$cconf/extra_$target";
    }
  }
}

# create files

open DEFS, ">defs.make" or die "cannot create defs.make";

my $pconfs = "pconf_list=";
foreach my $pconf (@{$names{"pconf"}}) {
  $pconfs .= "$pconf ";
}
print DEFS "$pconfs\n";

my $cconfs = "cconf_list=";
foreach my $cconf (@{$names{"cconf"}}) {
  $cconfs .= "$cconf ";
}
print DEFS "$cconfs\n";

#################

sub process_makerules {
  my $fname = shift;
  my $text = shift;
  my $done = "";
  while($text =~ m/^-?include\s+([^\s]+)\s*\n/m) {
    my $subname = $1;
    $done .= $PREMATCH;
    $text = $POSTMATCH;
    $done .= add_file($subname, @_);
  }
  $text = $done . $text;
  my $sub = "";
  while(1) {
    my $search = shift or last;
    my $subst = shift or last;
    $sub .= "s/$search/$subst/gm ";
    $text =~ s/$search/$subst/gm;
  }
  return "\n\n#start include from file '$fname' substitution '$sub'\n\n$text\n#end '$fname'\n\n";
}

sub add_file {
  my $fname = shift;
  open IN, "< $fname" or die "cannot open file '$fname'";
  local $/;
  my $text = <IN>;
  close IN;
  return process_makerules($fname, $text, @_);
}

sub add_sourcelist {
  my $list = shift;
  my $type = shift;
  my $res = "";
  foreach my $source (@$list) {
    my $text = $build_rules{"$source:$type"};
    if($text) {
      $res .= process_makerules($source, $text, @_);
    }
  }
  return $res;
}

sub add_both {
  my $fname = shift;
  my $list = shift;
  my $type = shift;
  return add_file($fname, @_) . add_sourcelist($list, $type, @_);
}

#################

my $all_bricks = "bricks=";
my $all_targets = "target_list=";

my $text = "";

foreach my $pconf (@{$names{"pconf"}}) {
  my $pconf_part = "pconf.$pconf";
  $text .= "\n# pconf $pconf\n";
  $all_bricks .= "\$(${pconf}_bricks) ";
  my $text1 .= "${pconf}_bricks=";
  my $text2 .= "${pconf}_headers=";
  foreach my $name (@{$filenames{"ath"}}) {
    my $basename = $name;
    $basename =~ s/\.ath\Z//;
    my $ath_part = "ath.$basename";
    next if check_exception($pconf_part, $ath_part);
    my $name1 = $name;
    $name1 =~ s/\.ath/.c/;
    $text1 .= "$pconf/$name1 ";
    my $name2 = $name;
    $name2 =~ s/\.ath/.h/;
    $text2 .= "$pconf/$name2 ";
  }
  $text .= "$text1\n\n";
  $text .= "$text2\n\n";
  foreach my $cconf (@{$names{"cconf"}}) {
    my $cconf_part = "cconf.$cconf";
    next if check_exception($pconf_part, $cconf_part);
    foreach my $target (@{$names{"target"}}) {
      my $target_part = "target.$target";
      next if check_exception($pconf_part, $target_part);
      next if check_exception($cconf_part, $target_part);
      $all_targets .= "${pconf}/${cconf}/$target ";
      my $objs_text = "${pconf}_${cconf}_${target}_objs=";
      open H, ">$pconf/$cconf/extra_$target/defs.h" or die "cannot create defs.h";
      open LOADERS, ">$pconf/$cconf/extra_$target/loaders.h" or die "cannot create loaders.h";
      foreach my $name (@{$filenames{"ath"}}) {
	my $basename = $name;
	$basename =~ s/\.ath\Z//;
	my $ath_part = "ath.$basename";
	next if check_exception($pconf_part, $ath_part);
	next if check_exception($cconf_part, $ath_part);
	next if check_exception($target_part, $ath_part);
	# this test is provisionary!
	if($basename =~ m/\Acontrol/) {
	  $objs_text .= "${pconf}/${cconf}/extra_${target}/$basename.o ";
	} else {
	  $objs_text .= "${pconf}/${cconf}/$basename.o ";
	}
	print H "#include \"../$basename.h\"\n";
	print LOADERS "  \&loader_$basename,\n";
	my @dep = `grep '^instance' $name`;
	if(@dep) {
	  map {s/^\s*instance\s*#(\w+).*\n/$1.ath /g} @dep;
	  $text .= "$pconf/$basename.c $pconf/$basename.h : @dep \n";
	  map {s/(\w+)\.ath/$pconf\/$1.c/g} @dep;
	  $text .= "$pconf/$cconf/$basename.o $pconf/$basename.c : @dep \n";
	}
      }
      close H;
      close LOADERS;
      $text .= "\n$objs_text\n\n";
    }
  }
  $text .= "\n# end pconf $pconf\n\n";
}
print DEFS "\n$all_bricks\n";
print DEFS "\n$all_targets\n";

print DEFS "\n$text\n";

$text = add_sourcelist($filenames{"ath"}, "global");

foreach my $pconf (@{$names{"pconf"}}) {
  my $pconf_part = "pconf.$pconf";
  $text .= add_both("pconf.$pconf", $filenames{"ath"}, "pconf", qr"\$[({]pconf[)}]", "$pconf");
  foreach my $cconf (@{$names{"cconf"}}) {
    my $cconf_part = "cconf.$cconf";
    next if check_exception($pconf_part, $cconf_part);
    $text .= add_both("cconf.$cconf", $filenames{"ath"}, "cconf", qr"\$[({]pconf[)}]", "$pconf", qr"\$[({]cconf[)}]", "$cconf");
    foreach my $target (@{$names{"target"}}) {
      my $target_part = "target.$target";
      next if check_exception($pconf_part, $target_part);
      next if check_exception($cconf_part, $target_part);
      $text .= add_both("target.$target", $filenames{"ath"}, "target", qr"\$[({]pconf[)}]", "$pconf", qr"\$[({]cconf[)}]", "$cconf", qr"\$[({]target[)}]", "$target");
    }
  }
}

print DEFS "\n$text\n";

###################
# DO NOT MOVE THE FOLLOWING TARGETS TO NORMAL MAKEFILE;
# they depend on variable evaluation at the time of inclusion of defs.make

print DEFS ".IGNORE: check_broken\n";
print DEFS ".PHONY: check_broken show_broken all_targets\n";
print DEFS "show_broken:\n\t\@echo Ignoring broken targets: \$(broken_targets)|sed -e's/:\\s*\$\$/: (none)/'\n\n";
print DEFS "all_targets: \$(filter-out \$(broken_targets),\$(target_list))\n";

close DEFS;


#################################################################

# debug config

open DEBUGNAMES, "> debug.names" or die "cannot created debugging config file";
open DEBUGINIT, "> debug.init" or die "cannot created debugging config file";
open DEBUGEXIT, "> debug.exit" or die "cannot created debugging config file";
foreach my $name (keys %debug_names) {
  print DEBUGINIT "DEBUG_OPEN($name)\n";
  print DEBUGEXIT "DEBUG_CLOSE($name)\n";
  print DEBUGNAMES "FILE * _debug_$name = NULL;\n";
}
close DEBUGNAMES;
close DEBUGINIT;
close DEBUGEXIT;
