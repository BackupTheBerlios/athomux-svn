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
my $xml_dir = "../xml";

if(scalar(@ARGV) eq 0) {
	print "-html   :\ttransforming bricks into html\n";
	print "-concat :\tconcat html files\n";
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
				
				# load empty table of contents
				system("cp empty.toc data.toc");
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
	
	foreach my $file (@files) {
		$counter++;
	
		print "--> $counter / $max : $file\n";
		my $filename = substr($file, 0, -4);
			
		# transform bricks
		system("saxon $xml_dir/$filename.xml brick_html.xsl > $html_dir/$filename.html");
			
		# create table of contents
   	system("saxon data.toc toc.xsl title=$file > temp.toc");
   	system("mv temp.toc data.toc");
	}
			
	# transform table of contents
	print "--> data.toc\n";
	
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
