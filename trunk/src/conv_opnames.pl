#!/usr/bin/perl -w

# Author: Thomas Schoebel-Theuer
# Copyright: University of Stuttgart

use diagnostics;
use strict;
use re 'eval';
use English;
use integer;
use Digest::MD5 qw(md5),qw(md5_hex);

# convert old operation names to new ones

system "mkdir -p tmp";

sub conv_file {
  my $file = shift;
  open IN, "< $file" or die "cannot open file $file";
  local $/;
  my $text = <IN>;
  close(IN);
  sub subst {
    my ($text, $name, $substit)= @_;
    $$text =~ s/(\$|[^\w]|_|^)$name([^\w]|_|$)/$1$substit$2/g;
  }
  subst (\$text, "transfer", "trans");

  subst (\$text, "tr", "transwait");
  subst (\$text, "gettr", "gettranswait");
  subst (\$text, "trput", "transwaitput");

  subst (\$text, "getaddr", "gadr");
  subst (\$text, "getaddrcreateget", "gadrcreateget");
  subst (\$text, "getaddrgettr", "gadrgettranswait");
  subst (\$text, "getaddrtrdeleteputaddr", "gadrtranswaitdeletepadr");
  subst (\$text, "getaddrcreatetrputaddr", "gadrcreatetranswaitpadr");

  subst (\$text, "putaddr", "padr");
  subst (\$text, "putputaddr", "putpadr");
  subst (\$text, "putdeleteputaddr", "putdeletepadr");
  open OUT, "> tmp/$file" or die "cannot open output file $file";
  print OUT $text;
  close(OUT);
}

foreach my $file (`ls *.ath *.c *.h`) {
  print $file;
  chomp $file;
  conv_file($file);
}
