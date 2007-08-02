#!/usr/bin/perl -w

# Transforming bricks from xml to html
# Creating table of contents

use strict;

my @ARGS;
my $html_dir = "../html/";

opendir(DIR, "../") or die("couldn't open current directory");
my @files = grep(/\.xml$/,readdir(DIR));
closedir(DIR);

# load empty table of contents
system("cp empty.toc data.toc");

# remove all html files
system("rm -r ".$html_dir."*.html");

print "Transforming...\n";

foreach my $file (@files) {

   print "--> $file\n";
   my $filename = substr($file, 0, -4);

   # transform bricks
   system("saxon ../$filename.xml brick_html.xsl > $html_dir$filename.html");

   # create table of contents
   system("saxon data.toc toc.xsl title=$file > temp.toc");
   system("mv temp.toc data.toc");
}

# transform table of contents
print "--> data.toc\n";
system("saxon data.toc toc_html.xsl > ".$html_dir."__toc.html");

foreach my $arg (@ARGV) {
	if(defined($arg) && ($arg cmp "-print") == 0) {
		
		# concate all html files ofr print version
		print "concat html files...\n";
		system("cd ../html; ./htmlcat/htmlcat -s -d *.html > ./all.html");
	}
}

print "...finished\n";
