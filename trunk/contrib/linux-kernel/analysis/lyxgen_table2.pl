#!/usr/bin/perl

while(<>) {
    $_ =~ /^(\S+)\:\s(.*)$/ || die $_;

    print <<EOF
<row topline="true">
<cell alignment="left" valignment="top" topline="true" leftline="true" usebox="none">
\\begin_inset Text

\\layout Standard

$1
\\end_inset
</cell>
<cell alignment="left" valignment="top" topline="true" leftline="true" rightline="true" usebox="none">
\\begin_inset Text

\\layout Standard

$2
\\end_inset
</cell>
</row>
EOF
    ;
}
