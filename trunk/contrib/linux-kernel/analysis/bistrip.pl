#!/usr/bin/perl


# choose correct entry for multiple built-in.o entries:
#
#     drivers/acpi/built-in.o acpi_disable_event
# X   drivers/acpi/events/built-in.o  acpi_disable_event
#     drivers/built-in.o      acpi_disable_event


while(<>) {
    $_ =~ /^(\S+)\s(\S+)$/ || die;
    $file = $1;
    $tag = $2;
    if($tag eq $lasttag) {
	if(($file =~ /built-in\.o/) && ($lastline =~ /built-in\.o/)) {
	    if(length($lastline) < length($_)) {
		undef $lasttag;
	    } else {
		$cancel_next = 1;
	    }
	} else {
	    $tmpfile = $ARGV[0].$lastfile;
	    $tmpfile =~ s/.k?o$/.c/;
	    if(-e $tmpfile) {
		$tmpfile = $ARGV[0].$file;
		$tmpfile =~ s/.k?o$/.c/;
		$cancel_next = 1 unless -e $tmpfile;
	    } else {
		undef $lasttag;
	    }
	}
    }
    print $lastline if defined($lasttag);
    if($cancel_next) {
	$cancel_next = 0;
	undef $lasttag;
    } else {
	$lasttag = $tag;
    }
    $lastfile = $file;
    $lastline = $_;
}
print $lastline if defined($lasttag);
