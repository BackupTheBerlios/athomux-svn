#!/usr/bin/perl -w

# Author: Thomas Schoebel-Theuer
# Copyright: University of Stuttgart

use diagnostics;
use strict;
use re 'eval';
use English;
use integer;
use Digest::MD5 qw(md5),qw(md5_hex);

# create additional Makefile rules out of the existing sources
# also, create defs.h and loaders.h automatically

my @sources = `ls *.ath | grep -v common`;
map chomp, @sources;

my @ctargets = @sources;
map {s/\.ath/\.c /} @ctargets;

my @otargets = @sources;
map {s/\.ath/\.o /} @otargets;

my @htargets = @sources;
map {s/\.ath/\.h /} @htargets;

open DEFS, ">defs.make" or die "cannot create defs.make";
print DEFS "bricks=", @ctargets, "\n";
print DEFS "objs=", @otargets, "\n";
print DEFS "headers=", @htargets, "\n";

open H, ">defs.h" or die "cannot create defs.h";
open LOADERS, ">loaders.h" or die "cannot create loaders.h";

foreach my $src (@sources) {
  my $body = $src;
  $body =~ s/\.ath//;
  print H "#include \"$body.h\"\n";
  print LOADERS "  \&loader_$body,\n";
  my @dep = `grep '^instance' $src`;
  if(@dep) {
    map {s/^\s*instance\s*#(\w+).*\n/$1.ath /g} @dep;
    print DEFS "$body.c $body.h : ", @dep, "\n";
  }
}

my $targets = "targets=";
foreach my $target (`ls target.*`) {
  chomp $target;
  print DEFS "-include $target\n";
  $target =~ s/target\.//;
  $targets .= "$target ";
}
print DEFS "$targets\nall_targets : \$(targets)\n";

close DEFS;
close H;
close LOADERS;
