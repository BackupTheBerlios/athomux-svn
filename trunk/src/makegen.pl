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

# first, get all the relevant file names

my @targets = `ls target.*`;
map chomp, @targets;
map { s/target\.// } @targets;

my @pconfs = `ls pconf.*`;
map chomp, @pconfs;
map { s/pconf\.// } @pconfs;

my @cconfs = `ls cconf.*`;
map chomp, @cconfs;
map { s/cconf\.// } @cconfs;

my @sources = `ls *.ath | grep -v common`;
map chomp, @sources;

# compute derived names

my @ctargets = @sources;
map {s/\.ath/\.c /} @ctargets;

my @otargets = @sources;
map {s/\.ath/\.o /} @otargets;

my @htargets = @sources;
map {s/\.ath/\.h /} @htargets;

# create subdirs

foreach my $pconf (@pconfs) {
  system "mkdir -p $pconf";
  system "ln -sf ../defs.h $pconf/";
  system "ln -sf ../common.h $pconf/";
  system "ln -sf ../loaders.h $pconf/";
  foreach my $target (@targets) {
    system "ln -sf ../$target.c $pconf/";
  }
  foreach my $cconf (@cconfs) {
    system "mkdir -p $pconf/$cconf";
  }
}

# create files

open DEFS, ">defs.make" or die "cannot create defs.make";
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

my $pconfs = "pconf_list=";
foreach my $pconf (@pconfs) {
  $pconfs .= "$pconf ";
}
print DEFS "$pconfs\n";

my $cconfs = "cconf_list=";
foreach my $cconf (@cconfs) {
  $cconfs .= "$cconf ";
}
print DEFS "$cconfs\n";

#################

sub add_file {
  my $fname = shift;
  open IN, "< $fname" or die "cannot open file '$fname'";
  local $/;
  my $text = <IN>;
  close IN;
  my $sub = "substitution ";
  while(1) {
    my $search = shift or last;
    my $subst = shift or last;
    $sub .= "s/$search/$subst/gm ";
    $text =~ s/$search/$subst/gm;
  }
  return "\n\n#start file $fname substitution $sub\n\n$text\n#end $fname\n\n";
}

#################

my $all_bricks = "bricks=";
my $all_headers = "headers=";
my $all_objs = "objs=";
my $all_targets = "target_list=";

my $text = "";

foreach my $pconf (@pconfs) {
  $text .= "\n# pconf $pconf\n";
  $all_bricks .= "\$(${pconf}_bricks) ";
  $text .= "${pconf}_bricks=";
  foreach my $name (@ctargets) {
    $text .= "$pconf/$name ";
  }
  $text .= "\n\n";
  $all_headers .= "\$(${pconf}_headers) ";
  $text .= "${pconf}_headers=";
  foreach my $name (@htargets) {
    $text .= "$pconf/$name ";
  }
  $text .= "\n\n";
  $all_objs .= "\$(${pconf}_objs) ";
  my $all_cconf_objs = "${pconf}_objs=";
  foreach my $cconf (@cconfs) {
    $all_cconf_objs .= "\$(${pconf}_${cconf}_objs) ";
    $text .= "${pconf}_${cconf}_objs=";
    foreach my $name (@otargets) {
      $text .= "$pconf/$cconf/$name ";
    }
    $text .= "\n";
    foreach my $target (@targets) {
      $all_targets .= "${pconf}/${cconf}/$target ";
    }
  }
  $text .= "\n$all_cconf_objs\n\n";
}
print DEFS "\n$all_bricks\n";
print DEFS "\n$all_headers\n";
print DEFS "\n$all_objs\n";
print DEFS "\n$all_targets\n";

print DEFS "\n$text\n";

$text = "";

foreach my $pconf (@pconfs) {
  $text .= add_file("pconf.$pconf", qr"\$[({]pconf[)}]", "$pconf");
  foreach my $cconf (@cconfs) {
    $text .= add_file("cconf.$cconf", qr"\$[({]pconf[)}]", "$pconf", qr"\$[({]cconf[)}]", "$cconf");
    foreach my $target (@targets) {
      $text .= add_file("target.$target", qr"\$[({]pconf[)}]", "$pconf", qr"\$[({]cconf[)}]", "$cconf");
    }
  }
}

print DEFS "\n$text\n";

###################

print DEFS "all_targets : \$(target_list)\n";

close DEFS;
close H;
close LOADERS;
