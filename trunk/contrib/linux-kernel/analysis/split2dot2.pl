#!/usr/bin/perl
while(<>) {
    $_ =~ /^(\S+)\s(\S+)\s(\S+)$/ || die;
    $sys = $1; $req = $2; $prov = $3;
    unless($req =~ s/^$sys\///) { $req = uc($req); }
    unless($prov =~ s/^$sys\///) { $prov = uc($prov); }

    if($sys ne $graph) {
	if(defined $graph) {
	    print FILE "}\n";
	    close(FILE);
	}
	openfile($sys);
	print FILE 'digraph "'.$sys.'" {'."\n";
	print FILE "\tsize = \"5,5\";\n\tpage = \"6.5,10\";\n\tcenter=true;\n\toverlap=false;\n";
	$graph = $sys;
    }
    print FILE "\t".'"'.$prov.'" -> "'.$req."\";\n";
}
print FILE "}\n" if defined $graph;
close(FILE);


sub openfile {
    my $filename = "$ARGV[0]$_[0].dot";
    $filename =~ s/\//_/g;
    open(FILE,">$filename") || die;
}
