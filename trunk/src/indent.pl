#!/usr/bin/perl -w

# Author: Thomas Schoebel-Theuer
# Copyright: University of Stuttgart

# KISS IDIOT Prettyprinter
# pupose: read source file with C macro definitions from stdin,
# ensure that trailing backslashes are at position 78

use diagnostics;
use strict;
use re 'eval';
use English;
use integer;

my $pos = 78;

while(my $line = <>) {
  if($line =~ s/\s*\\\s*\n$//) {
    my $len = length($line);
    if($len < $pos) {
      $line .= " " x ($pos-$len);
    }
    $line .= "\\\n";
  }
  $line =~ s/\s*\n$/\n/;
  print $line;
}
