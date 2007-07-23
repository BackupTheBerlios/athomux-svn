#!/usr/bin/perl -w

# Transforming bricks from xml to html
# Creating table of contens

use File::Copy;

opendir(DIR, ".") or die("couldn't open current directory");
@files = grep(/\.xml$/,readdir(DIR));
closedir(DIR);

# load empty table of contens
copy("empty.toc", "data.toc");

foreach $file (@files) {
   
   print "$file\n";
   $filename = substr($file, 0, - 4);
   
   # transform bricks
   system("saxon $filename.xml brick.xsl > $filename.html"); 
   
   # create table of contents
   system("saxon data.toc toc.xsl title=$file > temp.toc");
   copy("temp.toc", "data.toc"); 
}

unlink("temp.toc") or die("couldn't delete temp.toc");

print "finished\n";
