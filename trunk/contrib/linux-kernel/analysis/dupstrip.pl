#!/usr/bin/perl
while(<>) {
    $_ =~ /^(\S+)\s(\S+)\s(\S+)$/ || die;
    $cond = $3 eq '(undefined)';
    unless($cond) {
	if(length($2) > length($3)) {
	    $cond = substr($2,0,length($3)) eq $3;
	} else {
	    $cond = $2 eq $3;
	    unless ($cond) {
		$cond = substr($3,0,length($2)) eq $2;
	    }
	}
	print $_ unless $cond;
    }
}
