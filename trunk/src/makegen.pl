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

# single filter

my %contexts = ();
my %buildrules = ();

sub build_contexts {
  foreach my $source (@_) {
    open IN, "< $source" or die "cannot open file '$source'";
    local $/;
    my $text = <IN>;
    close IN;
    $contexts{$source} = "";
    for(;;) {
      if($text =~ m/^#?\s*(?:context|defaultbuild).*\n/m) {
	$contexts{$source} .= $MATCH;
	$text = $POSTMATCH;
	next;
      }
      if($text =~ m/^\s*buildrules\s+(\w+)\s*:\s*\n?((?:.*\n)*?)\s*endrules/m) {
	$buildrules{"$source:$1"} = $2;
	$text = $POSTMATCH;
	next;
      }
      last;
    }
  }
}

sub check_context {
  sub process_list {
    my ($found, $forwhat) = @_;
    my @list = split /\s*,\s*/, $found;
    my $nr_negative = 0;
    foreach my $elem (@list) {
      $elem =~ s/\s//g;
      next unless $elem;
      my $res = not ($elem =~ s/^!//);
      $nr_negative++ unless $res;
      if($forwhat =~ m/^$elem$/) {
	return $res;
      }
    }
    return $nr_negative;
  }
  my ($src, $type, $forwhat) = @_;
  my $do_build = $type =~ s/^#//; # when prefixed by #, consider 'defaultbuild'
  my $context = $contexts{$src};
  die "internal error: source '$src' not indexed" unless defined($context);
  my $try = $context;
  while($try =~ m/^#?\s*(context|defaultbuild)\s+cmd\s*($singlestringmatch)\s*:\s*(.*)\n/m) {
    $try = $POSTMATCH;
    next if (not $do_build and $1 eq "defaultbuild");
    my $list = $3;
    my $cmd = $2;
    $cmd =~ s/^"//; $cmd =~ s/"$//;
    my $name = `$cmd` or warn "file '$src': context cmd '$cmd' returned exit status $?";
    chomp $name;
    return 0 unless process_list($list, $name);
  }
  if($context =~ m/^#?\s*(?:context|defaultbuild)\s+$type(\s*:)?\s+(.+)\n/m) {
    warn "file '$src': please insert a ':' into the context statement" unless defined($1);
    return process_list($2, $forwhat);
  }
  return 1;
}

# list filter

sub filter_all {
  my ($prefix, $type, $forwhat, $list) = @_;
  my @res = ();
  foreach my $src (@$list) {
    next unless check_context("$prefix$src", $type, $forwhat);
    push @res, $src;
  }
  return \@res;
}

# first, get all the relevant file names

my @pconf_files = `ls pconf.* | grep -v "~"`;
map chomp, @pconf_files;
build_contexts(@pconf_files);
my @pconfs = @pconf_files;
map { s/pconf\.// } @pconfs;

my @cconf_files = `ls cconf.* | grep -v "~"`;
map chomp, @cconf_files;
build_contexts(@cconf_files);
my @cconfs = @cconf_files;
map { s/cconf\.// } @cconfs;
my %cconf_pconf = ();
foreach my $pconf (@pconfs) {
  $cconf_pconf{$pconf} = filter_all("cconf.", "pconf", $pconf, \@cconfs);
}

my @target_files = `ls target.* | grep -v "~"`;
map chomp, @target_files;
build_contexts(@target_files);
my @targets = @target_files;
map { s/target\.// } @targets;
my %target_pcconf = ();
my %buildtarget_pcconf = ();
foreach my $pconf (@pconfs) {
  foreach my $cconf (@{$cconf_pconf{$pconf}}) {
    my $tmplist = filter_all("target.", "pconf", $pconf, \@targets);
    $target_pcconf{"$pconf.$cconf"} = filter_all("target.", "cconf", $cconf, $tmplist);
    # same, but consider the 'defaultbuild' statement in addition to 'context'
    $tmplist = filter_all("target.", "#pconf", $pconf, \@targets);
    $buildtarget_pcconf{"$pconf.$cconf"} = filter_all("target.", "#cconf", $cconf, $tmplist);
  }
}

my @sources = `ls *.ath | grep -v common`;
map chomp, @sources;
build_contexts(@sources);

# create subdirs

foreach my $pconf (@pconfs) {
  system "mkdir -p $pconf && (cd $pconf && ln -sf ../*.h ../*.c .)"
    and die "cannot create subdirs / symlinks for $pconf";
  foreach my $cconf (@{$cconf_pconf{$pconf}}) {
    system "mkdir -p $pconf/$cconf"
      and die "cannot create cconf subdir $pconf/$cconf";
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
    my $text = $buildrules{"$source:$type"};
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
  foreach my $cconf (@{$cconf_pconf{$pconf}}) {
    $all_cconf_objs .= "\$(${pconf}_${cconf}_objs) ";
    my $text3 = "${pconf}_${cconf}_objs=";
    open H, ">$pconf/$cconf/defs.h" or die "cannot create defs.h";
    open LOADERS, ">$pconf/$cconf/loaders.h" or die "cannot create loaders.h";
    foreach my $name (@pconf_sources) {
      next unless check_context($name, "cconf", $cconf);
      my $name3 = $name;
      $name3 =~ s/\.ath/.o/;
      $text3 .= "$pconf/$cconf/$name3 ";
      my $body = $name;
      $body =~ s/\.ath//;
      print H "#include \"../$body.h\"\n";
      print LOADERS "  \&loader_$body,\n";
      my @dep = `grep '^instance' $name`;
      if(@dep) {
	map {s/^\s*instance\s*#(\w+).*\n/$1.ath /g} @dep;
	$text .= "$pconf/$body.c $pconf/$body.h : @dep \n";
      }
    }
    $text .= "$text3\n\n";
    close H;
    close LOADERS;
    next unless check_context("cconf.$cconf", "#pconf", $pconf);
    foreach my $target (@{$buildtarget_pcconf{"$pconf.$cconf"}}) {
      $all_targets .= "${pconf}/${cconf}/$target ";
    }
  }
  $text .= "\n$all_cconf_objs\n\n";
  $text .= "\n# end pconf $pconf\n\n";
}
print DEFS "\n$all_bricks\n";
print DEFS "\n$all_headers\n";
print DEFS "\n$all_objs\n";
print DEFS "\n$all_targets\n";

print DEFS "\n$text\n";

$text = add_sourcelist(\@sources, "global");

foreach my $pconf (@pconfs) {
  $text .= add_both("pconf.$pconf", \@sources, "pconf", qr"\$[({]pconf[)}]", "$pconf");
  foreach my $cconf (@{$cconf_pconf{$pconf}}) {
    $text .= add_both("cconf.$cconf", \@sources, "cconf", qr"\$[({]pconf[)}]", "$pconf", qr"\$[({]cconf[)}]", "$cconf");
    foreach my $target (@{$target_pcconf{"$pconf.$cconf"}}) {
      $text .= add_both("target.$target", \@sources, "target", qr"\$[({]pconf[)}]", "$pconf", qr"\$[({]cconf[)}]", "$cconf", qr"\$[({]target[)}]", "$target");
    }
  }
}

print DEFS "\n$text\n";

###################

print DEFS ".IGNORE: check_broken\n";
print DEFS ".PHONY: check_broken show_broken all_targets\n";
print DEFS "show_broken:\n\t\@echo Ignoring broken targets: \$(broken_targets)|sed -e's/:\\s*\$\$/: (none)/'\n\n";
print DEFS "all_targets: \$(filter-out \$(broken_targets),\$(target_list))\n";

close DEFS;
