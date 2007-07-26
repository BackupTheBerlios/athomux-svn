#!/usr/bin/perl -w

# Transforming bricks from xml to html
# Creating table of contents

opendir(DIR, ".") or die("couldn't open current directory");
@files = grep(/\.xml$/,readdir(DIR));
closedir(DIR);

# load empty table of contents
system("cp empty.toc data.toc");

print "Transforming...\n";

foreach $file (@files) {

   print "--> $file\n";
   $filename = substr($file, 0, - 4);

   # transform bricks
   system("saxon $filename.xml brick.xsl > $filename.html");

   # create table of contents
   system("saxon data.toc toc.xsl title=$file > temp.toc");
   system("mv temp.toc data.toc");
}

# transform table of contents
print "--> data.toc\n";
system("saxon data.toc toc_html.xsl > toc.html");

print "...finished\n";
