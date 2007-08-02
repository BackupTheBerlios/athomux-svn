#!/usr/bin/perl -w

# Option -html: 
# Transforming bricks from xml to html
# Creating table of contents
#
# Option -concat:
# concat html files

use strict;

my $optHtml = 0;
my $optConcat = 0;

my $html_dir = "../html";

if(scalar(@ARGV) eq 0) {
	print "-html\ttransforming bricks into html\n";
	print "-concat\tconcat html files";
}
else {
	
	my @ARGS;
	
	foreach my $arg (@ARGV) {
		if(defined($arg)) {
			if(($arg cmp "-html") == 0) {
				$optHtml = 1;
			}
			elsif(($arg cmp "-concat") == 0) {
				$optConcat = 1;
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
	opendir(DIR, "../") or die("couldn't open current directory");
	my @files = grep(/\.xml$/,readdir(DIR));
	closedir(DIR);
			
	# delete old table of contens
	system("rm data.toc");
			
	# init table of contents
	open(TOC, ">>data.toc") or die ("couldn't create data.toc");
	print TOC "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
	print TOC "<toc xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">\n";
		
	print "Transforming...\n";
			
	foreach my $file (@files) {
			
		print "--> $file\n";
		my $filename = substr($file, 0, -4);
			
		# transform bricks
		system("saxon ../$filename.xml brick_html.xsl > $html_dir/$filename.html");
			
		# create table of contents
		print TOC "\t<brickname>$filename</brickname>\n";
	}
			
	# transform table of contents
	print "--> data.toc\n";
	print TOC "</toc>";
	close(TOC);
	system("saxon data.toc toc_html.xsl > ".$html_dir."/__toc.html");
	
	print "...finished\n";
}

sub concatHtml {
	# remove old concated html file
	system("rm $html_dir/all.html");
	
	# concate all html files ofr print version
	print "concat html files...\n";
	system("cd $html_dir; ./htmlcat/htmlcat -s -d *.html > ./all.html");
	print "...finished\n";
}
