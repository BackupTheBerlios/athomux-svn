#!/usr/bin/perl

foreach $file (@ARGV) {
    my $f2 = $file;
    $f2 =~ s/_/\//g;
    $f2 =~ s/\.eps$//;

    print <<EOF
\\begin_inset Float figure
placement H
wide false
collapsed false

\\layout Standard
\\align center

\\begin_inset Graphics
        filename $file

\\end_inset

\\layout Caption

External dependencies: $f2

\\end_inset


EOF
    ;
}
