#!/usr/bin/perl

while(<>) {
    $_ =~ /^(\S+)\s(\S+)\s(\S+)\s(\S+)$/ || die $_;

    print <<EOF
<row topline="true">
<cell alignment="center" valignment="top" topline="true" leftline="true" usebox="none">
\\begin_inset Text

\\layout Standard

$1 ($2)
\\end_inset
</cell>
<cell alignment="left" valignment="top" topline="true" leftline="true" usebox="none">
\\begin_inset Text

\\layout Standard

$3
\\end_inset
</cell>
<cell alignment="left" valignment="top" topline="true" leftline="true" rightline="true" usebox="none">
\\begin_inset Text

\\layout Standard

$4
\\end_inset
</cell>
</row>
EOF
    ;
}
