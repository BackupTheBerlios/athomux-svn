#!/usr/bin/perl

# Tag list
# specifies mandatory and optional tags.
# 
# Values are
#   STRING  for a default value (doesn't make 
#           much sense for mandatory tags)
#   ARRAY   for a list of values to choose from
#   1       if non of the former

# mandatory tags
$mand_tags{"author"}    = 1;
$mand_tags{"copyright"} = 1;
$mand_tags{"purpose"}   = 1;
$mand_tags{"desc"}      = 1;
$mand_tags{"category"}  = 1;

@maturity = ("test", "alpha", "beta", "production");
$mand_tags{"maturity"}  = \@maturity;

# optional tags
$opt_tags{"license"}    = "see files SOFTWARE-LICENSE, PATENT-LICENSE";
$opt_tags{"example"}    = 1;







#if (exists($mand_tags{"author"})) {
#    print "author exists\n";
#}

#if (!exists($mand_tags{"foobar"})) {
#    print "foobar doesn't exists\n";
#}


#$maturity = $mand_tags{"maturity"};

#print ref($maturity) . "\n";


#$length = @maturity;
#print $length . "\n";

#for ($i = 0; $i < $length; $i++) {
#    print $maturity[$i] . "\n";
#}

#@list = ();

#while (@list = each(%mand_tags)) {
    
#    if (ref($list[1]) eq "ARRAY") {
#        print "ARRAY" . "\n";
#    } else {
#        if ($list[1] != 1) {
#            print "STRING ";
#        }
#        
#        print $list[0] . ": " . $list[1] . "\n";
#    }
#}
