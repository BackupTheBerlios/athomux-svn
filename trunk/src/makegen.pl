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

my @targetfiles = `ls target.* | grep -v "~"`;
map chomp, @targetfiles;
my @targets = @targetfiles;
map { s/target\.// } @targets;

my @pconfs = `ls pconf.* | grep -v "~"`;
map chomp, @pconfs;
map { s/pconf\.// } @pconfs;

my @cconfs = `ls cconf.* | grep -v "~"`;
map chomp, @cconfs;
map { s/cconf\.// } @cconfs;

my @sources = `ls *.ath | grep -v common`;
map chomp, @sources;

# compute contexts of bricks

my %contexts = ();

foreach my $source (@sources, @targetfiles) {
  $contexts{$source} = `grep -e '^#*context' $source`;
}

sub check_context {
  my ($src, $type, $forwhat) = @_;
  my $context = $contexts{$src} or return 1;
  if($context =~ m/^#?context\s+$type\s*(.+)\n/m) {
    my $found = $1;
    my @list = split /\s+/, $found;
    my $nr_negative = 0;
    foreach my $elem (@list) {
      my $res = not ($elem =~ s/^!//);
      $nr_negative++ unless $res;
      if($forwhat =~ m/^$elem$/) {
	return $res;
      }
    }
    return $nr_negative;
  }
  return 1;
}

# create subdirs

foreach my $pconf (@pconfs) {
  system "mkdir -p $pconf";
  system "ln -sf ../common.h $pconf/";
  system "ln -sf ../strat.h $pconf/";
  system "ln -sf ../lib.c $pconf/";
  system "ln -sf ../strat.c $pconf/";
  foreach my $target (@targets) {
    system "ln -sf ../$target.c $pconf/";
  }
  foreach my $cconf (@cconfs) {
    system "mkdir -p $pconf/$cconf";
  }
}

# create files

open DEFS, ">defs.make" or die "cannot create defs.make";

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
  $all_headers .= "\$(${pconf}_headers) ";
  $all_objs .= "\$(${pconf}_objs) ";
  my $text1 .= "${pconf}_bricks=";
  my $text2 .= "${pconf}_headers=";
  my @pconf_sources = ();
  foreach my $name (@sources) {
    next unless check_context($name, "pconf", $pconf);
    push @pconf_sources, $name;
    my $name1 = $name;
    $name1 =~ s/\.ath/.c/;
    $text1 .= "$pconf/$name1 ";
    my $name2 = $name;
    $name2 =~ s/\.ath/.h/;
    $text2 .= "$pconf/$name2 ";
  }
  $text .= "$text1\n\n";
  $text .= "$text2\n\n";
  my $all_cconf_objs = "${pconf}_objs=";
  foreach my $cconf (@cconfs) {
    $all_cconf_objs .= "\$(${pconf}_${cconf}_objs) ";
    my $text3 = "${pconf}_${cconf}_objs=";
    foreach my $name (@pconf_sources) {
      next unless check_context($name, "cconf", $cconf);
      my $name3 = $name;
      $name3 =~ s/\.ath/.o/;
      $text3 .= "$pconf/$cconf/$name3 ";
      open H, ">$pconf/$cconf/defs.h" or die "cannot create defs.h";
      open LOADERS, ">$pconf/$cconf/loaders.h" or die "cannot create loaders.h";
      my $body = $name;
      $body =~ s/\.ath//;
      print H "#include \"../$body.h\"\n";
      print LOADERS "  \&loader_$body,\n";
      my @dep = `grep '^instance' $name`;
      if(@dep) {
	map {s/^\s*instance\s*#(\w+).*\n/$1.ath /g} @dep;
	$text .= "$pconf/$body.c $pconf/$body.h : @dep \n";
      }
      close H;
      close LOADERS;
    }
    $text .= "$text3\n\n";
    foreach my $target (@targets) {
      next unless check_context("target.$target", "pconf", $pconf);
      next unless check_context("target.$target", "cconf", $cconf);
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