#!/usr/bin/perl
while(<>) {
    $_ =~ /^(\S+)\s(\S+)\s(\S+)$/ || die;
    $sys = $1; $req = $2; $prov = $3;
#    print "#  $sys  $req  $prov\n";
    $req =~ s/^$sys\///;
#    print "## $sys  $req  $prov\n";

    if($prov ne $graph) {
	print FILE "$txt\"];\n\t}\n" if defined $subgraph; undef $subgraph;
	if(defined $graph) {
	    print FILE "}\n";
	    close(FILE);
	}
	openfile($prov);
	print FILE 'digraph "'.$prov.'" {'."\n";
	print FILE "\tsize = \"5,5\";\n\tpage = \"8.2,11.6\";\n\tcenter=true;\n\tmargin=0;\n\toverlap=false;\n";
	print FILE "\t\"$prov\"\n";
	$graph = $prov;
    }
    if($sys ne $subgraph) {
	print FILE "$txt\"];\n\t}\n" if defined $subgraph; undef $subgraph;
	print FILE "\t\"$prov\" -> \"cluster_$sys\";\n";
	print FILE "\tsubgraph \"cluster_".$sys.'" {'."\n";
	print FILE "\t\tlabel=\"$sys\";\n\t\tstyle=filled; color=lightgrey;\n";
	$subgraph = $sys;
	$txt = "\t\t\"node_$sys\"[shape=plaintext,label=\""
    }
    if(defined($subgraph)) {
#	print FILE "\t\t\"$req\"\n";
	$txt .= "$req\\n";
    } else {
	print FILE "\t".'"'.$prov.'" -> "'.$req."\";\n";
    }
}
print FILE "$txt\"];\n\t}\n" if defined $subgraph; undef $subgraph;
print FILE "}\n" if defined $graph;


sub openfile {
    my $filename = "$ARGV[0]$_[0].dot";
    $filename =~ s/\//_/g;
    open(FILE,">$filename") || die;
}
