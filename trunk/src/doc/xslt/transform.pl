#!/usr/bin/perl -w

# Author: Alexander Herbig
# Copyright: Alexander Herbig
# License: see files SOFTWARE-LICENSE, PATENT-LICENSE
#
# transform.pl Options GroupTypes
# 
# Option -html: 
# Transforming bricks from xml to html
# Creating table of contents
#
# Option -concat:
# concat html files
#
# GroupTypes
# Create TOC which is group by GroupTyp

use strict;

my $optHtml = 0;
my $optConcat = 0;

my $html_dir = "../html";
my $xml_dir = "../xml";
my $graphviz_dir = "graphviz";

my @GROUPS;
my $group_count = 0;

if(scalar(@ARGV) eq 0) {
	print "transform.pl Options GroupTypes\n";
	print "# Options #\n";
	print "-html   :\ttransforming bricks into html\n";
	print "-concat :\tconcat html files\n";
	print "# GroupTypes #\n";
	print "Create TOC which is group by GroupTyp\n";
}
else {
	
	my @ARGS;
	
	# load empty table of contents
	system("cp empty.toc data.toc");
	
	# load empty menu
	system("cp empty.menu data.menu");
	
	foreach my $arg (@ARGV) {
		if(defined($arg)) {
			if(($arg cmp "-html") == 0) {
				$optHtml = 1;	
			}
			elsif(($arg cmp "-concat") == 0) {
				$optConcat = 1;
			}
			else {
				$GROUPS[$group_count] = $arg;
				$group_count++;
			}
		}	
	}
		
	if($optHtml) {
		transHtml();
	}
	
	if($optConcat) {
		concatHtml();
	}
}

sub transHtml {
	opendir(DIR, "$xml_dir") or die("couldn't open xml directory");
	my @files = grep(/\.xml$/,readdir(DIR));
	closedir(DIR);
		
	print "Transforming...\n";
	my $counter = 0;
	my $max = scalar(@files);
	
	# create navigation menu
	foreach my $group (@GROUPS) {
		if(defined($group)) {
			system("xsltproc --stringparam p_title $group -o temp.menu menu.xsl data.menu");
			system("mv temp.menu data.menu");
			print "--> Menu: $group\n";
		}	
	}
	
	foreach my $file (@files) {
		$counter++;
	
		print "--> $counter / $max : $file\n";
		my $filename = substr($file, 0, -4);
		
		# create graphviz image
		system("xsltproc -o $graphviz_dir/$filename.dot $graphviz_dir/graphviz.xsl $xml_dir/$filename.xml");		system("dot -Tpng -o$html_dir/$filename.png -Tcmapx -o$graphviz_dir/$filename.cmapx $graphviz_dir/$filename.dot");
		
		# transform bricks
		system("xsltproc --stringparam title $filename -o $html_dir/$filename.html brick_html.xsl $xml_dir/$filename.xml");
			
		# create table of contents
   		system("xsltproc --stringparam title $file -o temp.toc toc.xsl data.toc");
   		system("mv temp.toc data.toc");
	}
			
	# transform table of contents
	print "--> data.toc\n";
	
	# create TOC index
	system("xsltproc -o ".$html_dir."/__toc.html toc_html.xsl data.toc");
	print "--> TOC: index\n";
	
	# cretae TOC group by
	foreach my $group (@GROUPS) {
		if(defined($group)) {
			system("xsltproc --stringparam p_type $group -o ".$html_dir."/__toc_$group.html toc_html_group.xsl data.toc");
			print "--> TOC: Group by $group\n";
		}	
	}
	
	print "...finished\n";
}

sub concatHtml {
	# remove old concated html file, if exists
	if(-e "$html_dir/all.html") {
		system("rm $html_dir/all.html");
	}
	
	# concate all html files ofr print version
	print "concat html files...\n";
	system("cd $html_dir; ./htmlcat/htmlcat -s -d *.html > ./all.html");
	print "...finished\n";
}
