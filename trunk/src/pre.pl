#!/usr/bin/perl -w

# Author: Thomas Schoebel-Theuer
# Copyright: University of Stuttgart

use diagnostics;
use strict;
use re 'eval';
use English;
use integer;
use Digest::MD5 qw(md5),qw(md5_hex);

# switches affecting the generated code

# generate #line directives => set to 0 for faster runs or for debugging C code
my $write_line_directives = 0;
my $debug_level = 0;
my $indent_trace = 0;

# macros

%::base_macros = ();

# parameter parsing: allow predefined macros from the command line

sub get_param {
  for(;;) {
    my $param = shift @ARGV;
    return undef unless defined($param);
    if($param =~ m/^(\w+)\s*=\s*(.*)$/) {
      create_subst($1, $2, \%::base_macros);
    } else {
      return $param;
    }
  }
}

# global vars

my $infile = get_param();
die "input file missing" unless defined($infile);
my $cfile = get_param();
$cfile = $infile unless defined($cfile);
$cfile  =~ s/\.ath/\.c/;
$cfile =~ m/\/[^\/]+$/;
my $basedir = $PREMATCH;
$basedir = "." unless $basedir;
my $hfile = $cfile;
die "wrong filename format" unless ($hfile  =~ s/\.c/\.h/);
my $prefile = $cfile;
die "wrong filename format" unless ($prefile  =~ s/\.c/\.pre/);

my $copyright = "/* generated from $infile\n*/\n";
my $typetable_max = 64;


#############################################################################

# general regex matching constants

my $cppmatch = qr'(?:^#(?:[^\\\n]|\\.|\\\n)*\n)'m;
my $commentmatch = qr'(?://[^\n]*\n|/\*(?:[^*]|\*[^/])*\*/)'m;
my $ws = qr"(?:\s|$commentmatch|^$cppmatch)*"m;
my $singlestringmatch = qr'(?:\"(?:[^"\\]|\\.)*\"|\'(?:[^"\\]|\\.)\')';
my $stringmatch = qr"(?:$singlestringmatch(?:$ws$singlestringmatch)*)"m;
my $simplematch = qr"(?:[^,;\s()[\]{}\"\'/]|/(?=[^*/])|$commentmatch|$stringmatch)"m;
my $innermatch = qr'(?:(??{$simplematch})|[,;\s])*'m;
my $idmatch = qr'\w+';

# approximate context-free Dyck-style language by a regular one.
# unfortunately, the recursive substitution example on (??{
# from "man regex" did not work interpretatively, so I compile everything
# using a fixed limit :-(
# fortunately, (??{ seems to include its argument by reference, otherwise
# we would get exponential space complexity.
my $match = qr"(?:$simplematch|[,;\s]|\((??{$innermatch})\)|\[(??{$innermatch})\]|\{(??{$innermatch})\})*"m;
for(my $i = 0; $i < 20; $i++) {
  $match = qr"(?>$simplematch|[,;\s]|\((??{$match})\)|\[(??{$match})\]|\{(??{$match})\})*"m;
}

my $parenmatch = qr'\((??{$match})\)'m;
my $brackmatch = qr'\[(??{$match})\]'m;
my $bracematch = qr'\{(??{$match})\}'m;

# match nearly anything, but no comma (exept in parens)
my $argmatch = qr"(?:$simplematch|\($match\)|\[$match\]|\{$match\})*";
my $wsargmatch = qr"(?:$simplematch|\s|\($match\)|\[$match\]|\{$match\})*";

# match specifiers
my $sectmatch = qr"\(:$wsargmatch:\)";
my $specmatch = qr"(?:(?:(?:\$|:[<>])\w+|\#\w*(?:[#.]\w+)*|$sectmatch)(?:$brackmatch)?)+";
my $macrospec = qr"(?:@(?:[.=])?)?\w+(?:\s*\w+)*";

##########################################################################

# common helper substitutions

sub indent {
  my ($text, $prespace) = @_;
  $prespace = "  " unless defined($prespace);
  $$text =~ s/^([^#])/$prespace$1/mg;
}

sub purge {
  if($write_line_directives) { # remove redundant #line directives
    my $text = shift;
    my $parsed = "";
    my $oldline = 0;
    my $oldfile = "";
    $parsed = "\n" if ($$text =~ m/\A\#/m);
    while($$text =~ m/^\#line (\w+) (.+)\n/m) {
      $parsed .= $PREMATCH;
      if($1 != $oldline+1 or $2 ne $oldfile) {
	$parsed .= $MATCH;
      }
      $$text = $POSTMATCH;
      $oldline = $1;
      $oldfile = $2;
    }
    $$text = $parsed . $$text;
  }
}

sub strip_braces {
  my $text = shift;
  $$text =~ s/\A${ws}[({]//m;
  $$text =~ s/[})]$ws\Z//m;
}

sub strip_brackets {
  my $text = shift;
  $$text =~ s/\A${ws}[\[]//m;
  $$text =~ s/[\]]$ws\Z//m;
}

sub strip_quotes {
  my $text = shift;
  $$text =~ s/\A${ws}[\'"]//m;
  $$text =~ s/[\'"]$ws\Z//m;
  $$text =~ s/(?<!\\)[\'"]${ws}[\'"]//g;
}

sub brace_replace {
  my $text = shift;
  if($$text =~ m/__label__/) {
    # kludge for avoiding compiler core dumps
    $$text =~ s/\A($ws){/$1\({/m;
    $$text =~ s/}\Z/}\)/m;
  } else {
    $$text =~ s/\A($ws){/$1do {/m;
    $$text =~ s/}\Z/} while(0)/m;
  }
}

##########################################################################

# Preprocessor Part

# mode constants
my $mode_mask = 7;
my $mode_comment = 8;

sub parse_modes {
  my ($oldmode, $oldlevels, $modes) = @_;
  my $newmode = $oldmode & $mode_mask;
  if($modes =~ s/noexp,?//) {
    $newmode = 0;
  } elsif($modes =~ s/startexp,?//) {
    $newmode = 1;
  } elsif($modes =~ s/firstexp,?//) {
    $newmode = 2;
  } elsif($modes =~ s/seqexp,?//) {
    $newmode = 3;
  } elsif($modes =~ s/allexp,?//) {
    $newmode = 4;
  }
  $newmode |= $oldmode & $mode_comment;
  if($modes =~ s/(no)comment,?//) {
    if($1) {
      $newmode &= ~$mode_comment;
    } else {
      $newmode |= $mode_comment;
    }
  }
  my $newlevels = $oldlevels - 1;
  if($modes =~ s/onelevel,?//) {
    $newlevels = 1;
  } elsif($modes =~ s/deep,?//) {
    $newlevels = 9999;
  }
  return ($newmode, $newlevels);
}

# create comma-separated list of param names and of param signatures
sub parse_params {
  my ($params, $allow_sig) = @_;
  $params =~ s/\A[(<]\s*//m;
  $params =~ s/\s*[)>]\Z//m;
  my $sig = "";
  my $mac = "";
  my $varargs = ($params =~ s/\.\.\.$//m);
  foreach my $p (split '\s*,\s*', $params) {
    if ($p =~ m/(\w+)\Z/m) {
      die "you must not use type declarations in arguments of a @.macro definition; probably did you mean @.define instead of @.macro?" unless $allow_sig or $p eq $1;
      $mac .= $1 . ",";
      $sig .= $p . ", ";
    } else {
      die "bad arg '$p'";
    }
  }
  if($varargs) {
    $mac .= '\.\.\.';
    $sig .= '\.\.\.';
  }
  $sig =~ s/, \Z//m;
  $mac =~ s/,\Z//m;
  return ($sig, $mac);
}

sub make_macroname {
  my $name = shift;
  # allow multiword identifiers
  $name =~ s/\s+/\\s\+/g;
  $name =~ s/\./\\./g;
  return $name;
}

# Syntax of definitions:
# @.define `anchor_words' <optional_args> (input_args) => (output_args) {body}
# Alternatively, (body) may be used instead of {body} for allowing
# function call syntax.
# Corresponding call syntax:
# `anchor_words' optional_params (input_params) => (output_params)
# where optional_params is a list separated by _whitespace_ (not comma
# separated as usual otherwise)

sub parse_def {
  my ($text, $levels, $suppress, $macros) = @_;
  my $search = $text;
  delete $$macros{"quick@"};
  $text = "";
  for(;;) {
    if($search =~ m/\A$ws@\.undef$ws\($ws($macrospec)$ws\)/) {
      $text .= $PREMATCH;
      $search = $POSTMATCH;
      my $name = $1;
      $name = make_macroname($name);
      delete $$macros{$name};
      next;
    }
    last unless $search =~ m/\A$ws@\.(define|func|macro)$ws($brackmatch)?$ws($macrospec)$ws($parenmatch)?$ws(?:=>$ws($parenmatch))?($ws(?:$bracematch|$parenmatch))/m;
    $text .= $PREMATCH;
    $search = $POSTMATCH;
    my $cmd = $1;
    my $options = $2;
    my $name = $3;
    my $inparams = $4;
    $inparams = "" unless defined($inparams);
    my $outparams = $5;
    my $body = $6;
    if($cmd eq "define") {
      brace_replace(\$body);
    } elsif($cmd eq "macro") {
      $body =~ s/\A($ws)\{/$1/;
      $body =~ s/}$ws\Z//m;
    }
    if(defined($options) and $options =~ s/preexpand,?//) {
      my $submacros = $macros;
      if($options =~ s/prescope,?//) {
	my %new_macros = %$macros;
	$submacros = \%new_macros;
      }
      eval_macros(\$body, $levels, $suppress, $submacros);
    }
    my $allow_sig = $cmd ne "macro";

    my $outsig = undef;
    (my $sig, $inparams) = parse_params($inparams, $allow_sig);

    my $params = $inparams;
    if(defined($outparams)) {
      ($outsig, $outparams) = parse_params($outparams, $allow_sig);
      $params .= ',' . $outparams;
    }
    $params =~ s/^,//;

    $name = make_macroname($name);
    print "// macro $name\n" if($debug_level >= 1);
    my $search = $name;
    if($params) {
      $search .= "$ws\\(";
      foreach my $param (split /,/, $inparams) {
	print"//      param $param\n" if($debug_level >= 1);
	if($param eq "\\.\\.\\.") {
	  $search =~ s/,\Z//m;
	  $search .= "(,$ws(?:$wsargmatch$ws,$ws)*$wsargmatch)?$ws,";
	} else {
	  $search .= "$ws($wsargmatch)$ws,";
	}
      }
      $search =~ s/,\Z//m;
      $search .= "$ws\\)";
      if(defined($outparams)) {
	$search .= "$ws=>$ws\\(";
	foreach my $param (split /,/, $outparams) {
	  print"//   outparam $param\n" if($debug_level >= 1);
	  $search .= "$ws($wsargmatch)$ws,";
	}
	$search =~ s/,\Z/\\\)/m;
      }
    } else {
      # make empty parens optional
      $search .= "(?:$ws\\($ws\\))?";
    }
    if($cmd eq "define" or $cmd eq "macro") {
      $$macros{$name} = [$search, $params, $sig, $body, $options];
    } elsif($cmd eq "func") {
      my $tmpname = make_tmpname($name);
      # notice: $text will be expanded later (like the following text)
      $text .= "static void ${tmpname} (const union connector * on, void * brick, struct args * _args, const char * _param";
      $text .= ", $sig" if($sig);
      my $stub = "${tmpname} (on, _brick, _args, _param";
      $stub .= ",$params" if($params);
      $stub .= ")";
      if(defined($outsig)) {
	my $refsig = $outsig;
	$refsig =~ s/(\w+$ws(?:,|$))/\*$1/mg;
	$text .= ", " . $refsig;
	foreach my $outparam (split /\s*,\s*/, $outsig) {
	  $outparam =~ m/(\w+)\s*$/;
	  my $outsearch = $1;
	  $body =~ s/(?<!\w)$outsearch(?!\w)/\(\*$outsearch\)/mg;
	  $stub =~ s/(?<!\w)$outsearch(?!\w)/&\($outsearch\)/;
	}
      }
      $text .= ")\n" . $body;
      $$macros{$name} = [$search, $params, "", $stub, $options];
    } else {
      die "this should not happen";
    }
  }
  $text .= $search;
  return $text;
}

# create a name substitution macro
sub create_subst {
  my ($name, $subst, $macros) = @_;
  delete $$macros{"quick@"};
  $$macros{$name} = ["(?<!\\w)$name(?!\\w)", "", "", $subst];
}

# our stomache is the Perl interpreter
sub eval_compute {
  my ($expr, $text) = @_;
  $text = "@.compute" unless defined($text);
  my $res = eval $expr;
  print "$text $expr\ncauses Perl eval error: $@" if $@;
  $res = "" unless defined($res);
  return $res;
}

# insert a pseudo-parameter definition
sub insert_pseudoparam {
  my ($body, $text) = @_;
  unless($$body =~ s/({(?:${ws}__label__[^;]*;)*)/$1 $text/m) {
    die "could not create pseudo-param '$text'";
  }
}

# compute pre-scanning regex (for speeding up search)
sub make_quick {
  my $macros = shift;
  my $quick = qr"@(?:@|\.(?:step|expand|protect|deep|copy|(?:exp)?shuffle|scope|macro|define|isdef|func|undef|include|string|deftype|subst|compute|cpp|if))|/[/*]";
  my $quick2 = "";
  foreach my $pat (keys %$macros) {
    my $append = "|$pat(?!\\w)(?{\$name='$pat';})";
    if($pat =~ m/\A\w/) {
      $quick2 .= $append;
    } else {
      $quick .= $append;
    }
  }
  if($quick2) {
    $quick2 =~ s/^\|//;
    $quick = qr"(?:^\s*|(?<!\w))(?:$quick2)|$quick";
  }
  $$macros{"quick@"} = $quick;
  return $quick;
}

sub treat_builtins {
  my ($candidate, $done, $levels, $suppress, $macros) = @_;
  # treat @@ operators
  if($candidate =~ m/\A\s*@@\s*/) {
    my $rest = $POSTMATCH;
    eval_macros(\$rest, 0, 1, $macros);
    $done =~ m/\w*\Z/;
    my $last_id = $MATCH;
    $done = $PREMATCH;
    my $text = $last_id . $rest;
    return ($done, $text);
  }
  # stringification
  if($candidate =~ m/\A\s*@\.string\s*($bracematch)/m) {
    my $str = $1;
    my $text = $POSTMATCH;
    strip_braces(\$str);
    eval_macros(\$str, $levels, 1, $macros);
    #TODO: convert other escape characters also
    $str =~ s/%/%%/gm;
    $str =~ s/(["\\])/\\$1/gm;
    $done .= "\"$str\"";
    return ($done, $text);
  }
  # treat macro definitions
  if($candidate =~ m/\A\s*@\.(macro|func|define|undef)/m) {
    my $text = parse_def($candidate, $levels, $suppress, $macros);
    die"bad macro definition syntax" if $text eq $candidate;
    return ($done, $text);
  }
  # treat include files
  if($candidate =~ m/\A\s*@\.(include)$ws($stringmatch)/) {
    my $name = $2;
    my $text = $POSTMATCH;
    strip_quotes(\$name);
    my $subtext = readfile($name, $write_line_directives);
    if($levels > 0) {
      eval_macros(\$subtext, $levels, $suppress, $macros);
    }
    $done .= $subtext;
    return ($done, $text);
  }
  # treat default type specifiers
  if($candidate =~ m/\A\s*@\.(deftype)\s+(\w+)$ws($bracematch)/) {
    my $cmd = $1;
    my $name = $2;
    my $str = $3;
    my $text = $POSTMATCH;
    strip_braces(\$str);
    eval_macros(\$str, $levels, $suppress, $macros);
    $str =~ s/@\*->/@\*$name->/gm;
    return ($done . $str, $text);
  }
  # treat stomache
  if($candidate =~ m/\A\s*@\.(compute|cpp)$ws($parenmatch|$bracematch)/) {
    my $cmd = $1;
    my $expr = $2;
    my $text = $POSTMATCH;
    strip_braces(\$expr);
    if($cmd eq "cpp") {
      my $tmpname = "/tmp/pre.$$";
      open OUT, "| cpp > $tmpname" or die "cannot open cpp output";
      print OUT "$expr\n";
      close OUT;
      $expr = readfile($tmpname);
      system "rm -f $tmpname";
    }
    if($levels > 0) {
      eval_macros(\$expr, $levels, 1, $macros);
    }
    $expr = eval_compute($expr) if $cmd eq "compute";
    $done .= $expr;
    return ($done, $text);
  }
  if($candidate =~ m/\A\s*@\.(subst)$ws($parenmatch)$ws($parenmatch|$bracematch)/) {
    my $cmd = $1;
    my $reg = $2;
    my $expr = $3;
    my $text = $POSTMATCH;
    strip_braces(\$expr);
    my $regex = qr"$reg" or die "bad regular expression";
    $expr =~ $regex;
    if($levels > 0) {
      eval_macros(\$expr, $levels, $suppress, $macros);
    }
    $done .= $expr;
    return ($done, $text);
  }
  if($candidate =~ m/\A\s*@\.(isdef)$ws($parenmatch|$bracematch)/) {
    my $cmd = $1;
    my $expr = $2;
    my $text = $POSTMATCH;
    strip_braces(\$expr);
    $expr = make_macroname($expr);
    $expr =~ s/\s//g;
    my $bool = defined($$macros{$expr});
    $done .= $bool ? "1" : "0";
    return ($done, $text);
  }
  # treat new scope
  if($candidate =~ m/\A\s*@\.(scope)$ws($parenmatch|$bracematch)/) {
    my $cmd = $1;
    my $expr = $2;
    my $text = $POSTMATCH;
    strip_braces(\$expr);
    my %new_macros = %$macros;
    eval_macros(\$expr, $levels, $suppress, \%new_macros);
    $done .= $expr;
    return ($done, $text);
  }
  # treat conditional expansion
  if($candidate =~ m/\A\s*@\.if/) {
    my $success = 0;
    while($candidate =~ m/\A$ws@\.((?:else?)?if$ws($parenmatch)|else)$ws($bracematch)/) {
      my $expr = $2;
      my $body = $3;
      $candidate = $POSTMATCH;
      next if $success;
      $expr = 1 if $1 eq "else" or not defined($expr);
      if($levels > 0) {
	eval_macros(\$expr, $levels, 1, $macros);
      }
      my $subst = eval_compute($expr);
      if($subst) {
	if($levels > 0) {
	  strip_braces(\$body);
	  eval_macros(\$body, $levels, $suppress, $macros);
	}
	$done .= $body;
	$success = 1;
      }
    }
    return ($done, $candidate);
  }
  # treat evaluation order directives
  if($candidate =~ m/\A\s*@\.(step|expand|protect|deep|copy)(?:\($ws($argmatch)${ws}\))?/m) {
    my $rest = $POSTMATCH;
    my $cmd = $1;
    my $count = $2;
    $count = 1 unless defined($count);
    my $text = "";
    my $sublevels = 0;
    $sublevels = 1 if $cmd eq "expand";
    $sublevels = 99999 if $cmd eq "deep";
    if($rest =~ m/\A$ws($bracematch)/) {
      $text = $POSTMATCH;
      $rest = $1;
      strip_braces(\$rest);
    }
    if($cmd eq "protect") {
      my $newcount = $count - 1;
      $rest = "@.protect($newcount){$rest}" if $newcount > 0;
    } elsif($cmd eq "copy") {
      my $str = $rest;
      eval_macros(\$str, $levels, 1, $macros);
      $rest = $str x $count;
    } else {
      for(my $i = 0; $i < $count; $i++) {
	eval_macros(\$rest, $sublevels, 1, $macros);
      }
    }
    $done .= $rest;
    return ($done, $text);
  }
  if($candidate =~ m/\A\s*@\.((exp)?shuffle)$ws($brackmatch)/m) {
    my $rest = $POSTMATCH;
    my $cmd = $1;
    my $exp_mode = $2;
    my $expr = $3;
    my $listref = eval_compute($expr);
    die "bad array ref type" unless defined(@$listref);
    my @textlist = ();
    while($rest =~ m/\A$ws($bracematch)/) {
      my $text = $1;
      $rest = $POSTMATCH;
      strip_braces(\$text);
      eval_macros(\$text, $levels, $suppress, $macros) unless defined($exp_mode);
      push @textlist, $text;
    }
    foreach my $nr (@$listref) {
      my $select = $textlist[$nr];
      die "bad shuffle index $nr" unless defined($select);
      if(defined($exp_mode)) {
	$textlist[$nr] = $select;
      } else {
	$done .= $select;
      }
    }
    if(defined($exp_mode)) {
      foreach my $text (@textlist) {
	$done .= $text;
      }
    }
    return ($done, $rest);
  }
  return (undef, undef);
}

sub eval_macros {
  # $levels is the number of expansions:
  # $levels == 0 : expand _once_ at the start
  # $levels == 1 : expand _once_ everywhere
  # $levels > 1: expand as often as $levels
  # $suppress == 0: expand with comment
  # $suppress == 1: expand without comment
  # $suppress == 2: dont expand @. directives
  my ($text, $levels, $suppress, $macros) = @_;
  my $quick = undef;
  local $::name = undef;
  my $done ="";
  my $anchor = qr"";
  $anchor = qr'\A' if $levels <= 0;
  # parameters are substituted _once_ (i.e. not re-evaluated)
  # however the total result is rescanned for further substitutions
  $levels--;
  while ((defined($quick) or $quick = $$macros{"quick@"} or $quick = make_quick($macros)) and 
	 $$text =~ m/$anchor$quick/m) {
    $done .= $PREMATCH;
    my $m = $MATCH;
    my $p = $POSTMATCH;
    my $candidate = $MATCH . $POSTMATCH;
    # skip comments
    if($candidate =~ m/\A\s*\/[\/\*]/) {
      $candidate =~ m/\A\s*$commentmatch/m;
      $done .= $MATCH;
      $$text = $POSTMATCH;
      next;
    }
    if($suppress < 2 and $candidate =~ m/\A\s*@/) { 
      # skip this for non-@ commands (performance)
      my ($subdone, $tail) = treat_builtins($candidate, $done, $levels, $suppress, $macros);
      if(defined($subdone)) {
	$quick = undef;
	$done = $subdone;
	$$text = $tail;
	next;
      }
    } 
    # treat ordinary macros
    unless(defined($::name)) {
      $done .= $m;
      $$text = $p;
      next;
    }
    my $tuple = $$macros{$::name};
    unless(defined($tuple)) {
      $done .= $m;
      $$text = $p;
      next;
    }
    # unpack the macro definition...
    my ($search, $params, $sig, $body, $options) = (@$tuple);
    unless($candidate =~ m/\A(\s+)?($search)/m) {
      $done .= $m;
      $$text = $p;
      next;
    }
    $$text = $POSTMATCH;
    my $prespace = defined($1) ? $1 : "";
    my $notice = $2;
    # don't destroy $1..$n by the final replace loop
    # thus we need a tmp list for the actual parameters.
    my $n = 3;
    my @act_list = ();
    foreach my $param (split /,/, $params) {
      my $elem = eval "\$$n";
      $elem = "" unless defined($elem);
      push @act_list, $elem;
      $n++;
    }
    my $submacros = $macros;
    my $sublevels = $levels;
    my $sub_suppress = $suppress;
    if(not defined($options) or not $options =~ s/flat,?//) {
      # disallow endless recursive substitutions
      $sublevels = 1;
      $sub_suppress = 2;
      my %newmac = ();
      if($levels > 0) {
	%newmac = %$macros;
	delete $newmac{$::name};
	$sublevels = $levels;
	$sub_suppress = $suppress;
      }
      $submacros = \%newmac;
    }
    my %later = ();
    $n = 0;
    my @siglist = split /,\s*/, $sig;
    foreach my $param (split /,/, $params) {
      my $sigparam = shift @siglist;
      my $subst = $act_list[$n];
      $n++;
      # determine expansion mode of this parameter
      my $pre_expand = 0;
      my $mode = "<-";
      #$pre_expand = 1;
      #$mode = "def";
      while(defined($sigparam) and $sigparam =~ s/\A\s*@(!)?(<-|->|def)?:\s*//) {
	$pre_expand = 1 if defined($1);
	$mode = $2 if defined($2);
      }
      # allow outside-in expansion
      if($pre_expand) {
	eval_macros(\$subst, $levels, $suppress, $macros);
	$quick = undef;
      }
      # try to evaluate actual parameter at runtime only once (if possible)
      # by assigning to $sigparam
      if(defined($sigparam) and $sigparam ne $param) {
	# create lexically scoped assignments for signature params
	if($subst =~ m/(?:^|(?<!\w))$param(?:\Z|(?!\w))/m) {
	  # This is HELL! you cannot define int x = x in C such that the new
	  # x is _truly_ _hiding_ an x from the outer scope.
	  # But in Ada you can! In C you even get a stupid assignment of
	  # an unitialized value to itself, without any warning from the
	  # compiler!
	  # Should we blame the C language definition or the implementation?
	  # We cannot just do nothing, since
	  #   (a) the types of the two x'es could be different
	  #   (b) an assignment to the inner x should not affect the outer one
	  # FIXME: replacement affects _all_ identifiers, even field
	  # names of structures, type names, etc.
	  my $newparam = "__q_${param}_";
	  $body =~ s/(?:^|(?<!\w))$param(?:\Z|(?!\w))/$newparam/mg;
	  $sigparam =~ s/$param\Z/$newparam/;
	  $param = $newparam;
	}
	insert_pseudoparam(\$body, "$sigparam = $subst; (void)$param; ");
      } elsif($param =~ m/^\\\./) {
	# handle ... open param list
	$body =~ s/$param/$subst/mg;
      } elsif($mode eq "<-") {
	# handle ordinary params
	$body =~ s/(?<!\w)$param(?!\w)/$subst/mg;
      } elsif($mode eq "->") {
	# handle late expansion
	$later{$param} = $subst;
      } else {
	if($param ne $subst) {
	  #print"$::name: $param <--- $subst\n";
	  create_subst($param, $subst, $submacros);
	}
      }
    }
    eval_macros(\$body, $sublevels, $sub_suppress, $submacros) unless defined($options) and $options =~ s/postprotect,?//;
    while(my ($param, $subst) = each %later) {
      $body =~ s/(@@)?(?<!\w)$param(?!\w)(@@)?/$subst/mg;
    }
    $body = "/* macro expansion $notice */ $body /* end $::name */" unless $suppress or not $params;
    indent(\$body, $prespace);
    $done .= $body;
  }
  $$text = $done . $$text;
}

##########################################################################

# general specifier handling

# check whether a specifier syntax is correct
sub sp_check {
  my ($spec) = @_;
  # check starting #brick
  return 0 unless $spec =~ s/\A#\w*//;
  # strip further sub-instances
  while($spec =~ s/\A#\w+//) {}
  return 1 if $spec eq "";
  # check for .varname
  return 1 if $spec =~ m/\A\.\w+\Z/;
  # check for input/output
  return 0 unless $spec =~ s/:[<>]\w+//;
  # skip array indices
  while($spec =~ s/\A\[[^\]]*\]//) {}
  return 1 if $spec eq "";
  # check for .varname
  return 1 if $spec =~ m/\A\.\w+\Z/;
  # check local sub-instances
  return sp_check($spec) if $spec =~ m/\A\#/;
  # check section code
  return 0 unless $spec =~ s/\A\(:[^:]+:\)//;
  return 1 if $spec eq "";
  return 1 if $spec =~ m/\A\$\w+\Z/;
  return 0;
}

sub sp_syntax {
  my ($spec) = @_;
  my $ok = sp_check($spec);
  warn "specifier '$spec' has bad syntax / wrong order of elements\n" if not $ok;
}

# determine the type of a specifier
# 0 = unknown, 1 = brick, 2 = input/output, 3 = section, 4 = operation
sub sp_type {
  my ($spec) = @_;
  return 4 if $spec =~ m/\$\w+\Z/;
  return 3 if $spec =~ m/\(:[^:]*:\)\Z/;
  return 2 if $spec =~ m/:[<>]\w+(?:\[[^\]]*\])*\Z/;
  return 1 if $spec =~ m/\#\w+\Z/;
  return 0;
}

# shorten a specifier to the desired type
sub sp_shorten {
  my ($spec, $type) = @_;
  $spec =~ s/\$\w+\Z// if $type < 4;
  $spec =~ s/\(:[^:]*:\)\Z// if $type < 3;
  $spec =~ s/(?:\[[^\]]*\])+\Z// if $type < 2;
  $spec =~ s/:[<>]\w+\Z// if $type < 2;
  $spec =~ s/\#\w+\Z// if $type < 1;
  die "specifier '$spec' has not type $type" if sp_type($spec) != $type;
  return $spec;
}

# return the desired part as specified by type
sub sp_part {
  my ($spec, $type, $subtype) = @_;
  if($type == 1) { # brick specifier
    $spec =~ m/\A(?:\#(\w+)(?:\#(\w+(?:\#\w+)*))?)?/;
    if(defined($subtype)) {
      return $1 if $subtype == 0; # only the base identifier
      return $2 if $subtype == 1; # only the extension part without leading #
      die "bad subtype $subtype";
    }
    return $MATCH; # the full brick part
  }
  $spec =~ m/(?:\$(\w+))?\Z/;
  if($type == 4) { # operation specifier
    if(defined($subtype)) {
      return $1 if $subtype == 0; # only the base identifier
      die "bad subtype $subtype";
    }
    return $MATCH; # the full part
  }
  $spec = $PREMATCH;
  $spec =~ m/(?:\(:([^:]+):\))?\Z/;
  if($type == 3) { # section specifier
    if(defined($subtype)) {
      return $1 if $subtype == 0; # only the contents
      die "bad subtype $subtype";
    }
    return $MATCH; # the full part
  }
  $spec = $PREMATCH;
  $spec =~ m/(?::[<>](\w+)(\[([^\]]*)\])?)?\Z/;
  if($type == 2) { # input specifier
    if(defined($subtype)) {
      return $1 if $subtype == 0; # only the base identifier
      return $2 if $subtype == 1; # the bracket part
      return $3 if $subtype == 2; # the part insinde the brackets
      die "bad subtype $subtype";
    }
    return $MATCH; # the full part
  }
  $spec = $PREMATCH;
}

# return a tuple for standard specifiers
sub sp_parts {
  my $spec = shift;
  return (sp_part($spec, 1, 0), sp_part($spec, 2, 0), sp_part($spec, 2, 1), sp_part($spec, 3), sp_part($spec, 4, 0));
}

# make a full specifier out of a short form, obeying the scope rules
sub sp_complete {
  my ($spec, $scopes) = @_;
  $scopes = $::current unless $scopes;
  # special treatment for empty #brick
  if($spec =~ m/\A#[^\w]/) {
    $scopes =~ m/\A\#\w+/;
    my $brick = $MATCH;
    $spec =~ s/\A\#/$brick/;
  }
  # special treatment for section part (complete it independently)
  if($scopes =~ m/\(:[^:]+:\)/) {
    my $sect = $MATCH;
    $spec =~ s/(:[<>]\w+(?:\[[^\]]*\])*)([\$])/$1$sect$2/;
  }
  # try whether already complete
  return $spec if ($spec =~ m/\A\#/ and sp_check($spec));
  # try all shortings of $scopes
  my $res = $scopes . $spec;
  until(sp_check($res)) {
    die "impossible to complete specifier '$spec' in scope '$scopes'"
      unless $scopes =~ s/(?:(?:(?:[#\$.]|:[<>])\w+)|\[[^\]]*\]|\(:[^:]*:\))\Z//;
    $res = $scopes . $spec;
  }
  return $res;
}

# convert a specifier to a unique C identifier
sub sp_name {
  my ($spec, $type) = @_;
  $spec = sp_shorten($spec, $type) if defined($type);
  die "specifier '$spec' has wrong syntax" unless sp_check($spec);
  $spec =~ s/\A\#//;
  if($spec =~ m/\(:([^:]+):\)/) {
    my $pre = $PREMATCH;
    my $part = $1;
    my $post = $POSTMATCH;
    if($part =~ m/\A(.*)\.\.(.*)\Z/) {
      my $start = $1;
      my $end = $2;
      $start = eval_compute($start, "start-value");
      $end = eval_compute($end, "end-value");
      $part = "${start}_TO_$end";
    }
    $spec = "${pre}_${part}$post";
  }
  $spec =~ s/\[([^\]]*)\]//g;
  $spec =~ s/[#\$.]|:[<>]/_/g;
  return $spec;
}

# convert a specifier to a variable name, starting from prefix $start
sub sp_var {
  my ($spec, $start, $correction) = @_;
  $correction = "" unless defined($correction);
  die "specifier '$spec' has wrong syntax" unless sp_check($spec);
  my $len = length($start);
  die "bad prefix '$start' for specifier '$spec'" unless substr($spec, 0, $len) eq $start;
  substr($spec, 0, $len) = "";
  # starting #
  $spec =~ s/\A\#//;
  # handle sub-instances
  $spec =~ s/\#/\._sub_/g;
  # handle operations
  if($spec =~ m/:([<>])(\w+)\(:([^:]+):\)\$(\w+)\Z/) {
    my $type = $1 eq "<" ? "_input_.connect->ops" : "_output_.ops";
    if($correction) { # we have an input operation
      # ...NYI...
    }
    $spec = "${PREMATCH}\._conn_${2}\.${type}[$3][opcode_$4$correction]";
  }
  # handle remaining inputs/outputs
  $spec =~ s/:[<>]/\._conn_/g;
  return $spec;
}

##########################################################################

sub sp_conn_instance {
  my ($spec) = @_;
  my $res = sp_part($spec, 1, 1);
  if(defined $res) {
    $res =~ s/(^|[#.])/$1_sub_/g;
    $res =~ s/\#/\./g;
    $res .= ".";
  } else {
    $res = "";
  }
  my $array = sp_part($spec, 2, 1);
  $array = "" unless defined($array);
  $res .= "_conn_" . sp_part($spec, 2, 0) . $array;
#my $stripped = sp_complete(sp_part($spec, 2));
#my $test = sp_var($stripped, sp_part($stripped, 1));
#print"'$test' != '$res'\n" if ".$res" ne $test;
  return $res;
}

##########################################################################

# some specifier helper routines (TODO: revise)

# ? this should be replaced by sp_var()
sub spec_conn_instance {
  my ($spec) = @_;
  my $res = sp_part($spec, 1, 1);
  if(defined $res) {
    $res =~ s/(^|[#.])/$1_sub_/g;
    $res =~ s/\#/\./g;
    $res .= ".";
  } else {
    $res = "";
  }
  my $array = sp_part($spec, 2, 1);
  $array = "" unless defined($array);
  $res .= "_conn_" . sp_part($spec, 2, 0);
  return ($res, $array);
}

sub make_tmpname {
  my $name = shift;
  $name =~ s/\@/at/g;
  $name =~ s/\\//g;
  $name =~ s/\./dot/g;
  return "__" . sp_part($::current, 1, 0) ."_global_${name}__";
}

# determine the type of the (sub)brick
sub spec_bricktype {
  my ($spec) = @_;;
  my $br = sp_part($spec, 1);
  my $res = $br;
  my $subst = $::inst_types{$br} or die "cannot determine brick type for $spec";
  $res = $subst if defined($subst);
  $spec =~ s/$br/$res/;
  return $spec;
}

# check whether a specifier is an alias and replace it
sub spec_alias {
  my ($spec) = @_;
  #my $search = spec_prefix($$spec, 2);
  my $search = sp_shorten($$spec, 2);
  $search =~ s/$brackmatch//;
  #print "SEARCH: $search\n";
  my $replace = $::aliases{$search};
  if(defined($replace)) {
    #print "REPLACE: $search -> $replace\n";
    $$spec =~ s/$search/$replace/;
  }
}

##########################################################################

# generic type handling

# external format:
# type name:offset,...

# internal format:
# %hash{name} = [offset, length, type]

# parse $str and convert to internal hash representation
sub parse_typedef {
  my ($str, $deny_offsets, $lastoffset) = @_;
  unless(defined($lastoffset)) {
    $lastoffset = "0";
    $str .= ",struct empty LASTFIELD";
  }
  my $newoffset = $lastoffset;
  my $sellen = "0";
  my %hash = ();
  for(;;) {
    next if $str =~ s/\A\s*,\s*//;
    last if $str eq "";
    #handle variant definitions
    if($str =~ s/\A\s*(\.\w+)\s*($bracematch)\s*(?:,|\Z)//) {
      my $label = $1;
      my $substr = $2;
      strip_braces(\$substr);
      my ($suboffset, $subhash) = parse_typedef($substr, $deny_offsets, $lastoffset);
      die "label '$label' is already defined" if exists $hash{$label};
      # flatten the namespace
      my @sublabels = grep(/\A\./, keys %$subhash);
      foreach my $sublabel (@sublabels) {
	my $subinfo = $$subhash{$sublabel};
	delete $$subhash{$sublabel};
	my $newlabel = $label . $sublabel;
	die "duplicate sublabel '$newlabel'" if exists $hash{$newlabel};
	$hash{$newlabel} = $subinfo;
      }
      $hash{$label} = [ keys %$subhash ];
      while(my ($key, $value) = each %$subhash) {
	die "variant field '$key' conflicts with outer field" if exists $hash{$key};
	$hash{$key} = $value;
      }
      # note: this could become negative, thus use signed arithmetic!
      my $sublen = "(int)($suboffset) - (int)($lastoffset)";
      $sellen = "STUPID_MAX($sellen, $sublen)";
      $newoffset = "($lastoffset) + $sellen";
      next;
    }
    $lastoffset = $newoffset;
    # handle field selectors
    my %hire = ();
    my %fire = ();
    while($str =~ s/\A\s*([=!])((?:\.\w+)+)\s*(?:,|\Z)//) {
      my $cmd = $1;
      my $label = $2;
      if($cmd eq "=") {
	#print"hire $label\n";
	$hire{$label} = 1;
      } else {
	#print"fire $label\n";
	$fire{$label} = 1;
      }
    }
    if(scalar(%hire) or scalar(%fire)) {
      my @all = grep(/\A\./, keys %hash);
      foreach my $label (@all) {
	if(exists $fire{$label} or (scalar(%hire) and not exists $hire{$label})) {
	  #print"delete label $label\n";
	  my $fields = $hash{$label} or die "label '$label' not present";
	  delete $hash{$label};
	  foreach my $field (@$fields) {
	    #print"delete field $field\n";
	    delete $hash{$field};
	  }
	}
      }
      next;
    }
    # handle ordinary fields
    $str =~ s/\A\s*([^,]+)\s+(\w+)\s*(?::\s*($argmatch)\s*)?(?:,|\Z)// or die "bad type syntax";
    my $type = $1;
    my $name = $2;
    my $offset = $3;
    die "offsets are disallowed in use statements" if(defined($offset) and $deny_offsets);
    $offset = $lastoffset unless defined($offset);
    while($offset =~ m/(OFFSET|LENGTH)\s*\(\s*(\w+)\s*\)/) {
      my $cmd = $1;
      my $othername = $2;
      my $tuple = $hash{$othername} or die "other field name $othername does not exist";
      my ($otheroffset, $otherlen) = @$tuple;
      my $subst = ($cmd eq "OFFSET") ? $otheroffset : $otherlen;
      $offset = $PREMATCH . $subst . $POSTMATCH;
    }
    my $len = "sizeof($type)";
    $newoffset = "($offset) + $len";
    $lastoffset = $newoffset;
    $type =~ s/\A\s+//;
    $type =~ s/\s+/ /g; # norm blanks
    die "field '$name' defined twice" if defined($hash{$name});
    $hash{$name} = [$offset, $len, $type];
  }
  return ($newoffset, \%hash);
}

# return mismatching field name if it is _not_ a subtype
sub isnot_subtype {
  my ($t1, $t2) = @_;
  while(my ($name1, $tuple1) = each %$t1) {
    next if $name1 =~ m/\A\./;
    my $tuple2 = $$t2{$name1} or return $name1;
    my ($offset1, $len1, $type1) = @$tuple1;
    my ($offset2, $len2, $type2) = @$tuple2;
    return $name1 if $type1 ne $type2;
  }
  return undef;
}

sub type_hash {
  my ($name, $max) = @_;
  my $index = md5($name);
  return ord($index) % $max;
}

#generate code for a type table
sub gen_type_table {
  my ($name, $max, $prototype, $hash) = @_;
  if($prototype) {
    return "extern const struct gen_tabentry ${name}\[$max];\n";
  }
  my $res = "const struct gen_tabentry ${name}\[$max]";
  $res .= " = {\n";
  while(my ($name, $tuple) = each %$hash) {
    next if $name =~ m/\A\./;
    my ($offset, $len, $type) = @$tuple;
    my $index = type_hash($name, $max);
    $res .= "  [$index] = {\"$name\", \"$type\", $offset, $len},\n";
  }
  $res .= "};\n";
  return $res;
}

sub get_type_info {
  my ($name, $hash) = @_;
  my $tuple = $$hash{$name};
  die "undefined field '$name'" unless defined($tuple);
  my ($offset, $len, $type) = @$tuple;
  return ($offset, $type);
}

sub get_type_index {
  my ($name, $max, $hash) = @_;
  my $tuple = $$hash{$name};
  die "undefined field '$name'" unless defined($tuple);
  my ($offset, $len, $type) = @$tuple;
  my $index = type_hash($name, $max);
  return ($index, $type);
}

##########################################################################

# generic typename parsing

sub spec_type_name {
  my ($name, $spec) = @_;
  unless($name =~ s/\./_/g) {
    $spec = spec_bricktype($spec);
    my $brick = sp_part($spec, 1, 0);
    $name = "${brick}_$name" unless $name =~ s/\./_/g;
  }
  return $name;
}

sub spec_type_instance {
  my ($name, $spec) = @_;
  my $brick = sp_part($spec, 1, 1);
  if(defined($brick)) {
    $brick =~ s/[#.]/\._sub_/g;
    $brick = "_sub_${brick}.";
   } else {
     $brick = "";
   }
  $name = "$brick$name" unless $name =~ s/[#.]/\._sub_/g;
  return $name;
}

sub inherit_types {
  my ($def, $from) = @_;
  foreach my $inherit (reverse split '\s*,\s*', $from) {
    $inherit = spec_type_name($inherit, $::current);
    my $first = $::type_defs{$inherit} or die "type $inherit does not exist";
    my ($fromname,$fromdef) = @$first;
    $def = "$fromdef,$def";
  }
  return $def;
}

sub define_type {
  my ($cmd, $export, $name, $from, $def) = @_;
  strip_quotes(\$def);
  $def = inherit_types($def, $from) if defined($from);
  my ($ignore, $hash) = parse_typedef($def, $cmd ne "define");
  my $typename = spec_type_name($name, $::current);
  my $subname = spec_type_instance("_type_$name", $::current);
  my $tablename = ($export and $cmd eq "define") ? "typetable_$typename" : undef;
  my $varname = ($cmd eq "use") ? $subname : undef;
  $::type_defs{$typename} = [ $tablename, $def, $varname, sp_shorten($::current, 2), $name, $hash ];
  return $typename;
}

sub parse_types {
  my ($text) = @_;
  if($$text =~ m/\A${ws}(use|define)\s+(export\s+)?TYPE\s+(\w+)$ws(?:from$ws(\w+(?:\s*,\s*\w+)*)$ws)?($stringmatch)$ws;/) {
    $$text = $POSTMATCH;
    my $typename = define_type($1, $2, $3, $4, $5);
    my $extern_name = sp_name(spec_bricktype($::current), 2);
    #print "    $extern_name .= $typename\n";
    $::extern_type_defs{$extern_name} .= ",$typename";
    return 1;
  }
  return undef;
}


##########################################################################

# generic typename code generation

sub eval_typename_code {
  my ($code) = @_;
  my $pre = "";
  while($$code =~ m/(\w+|$parenmatch)${ws}@\*(\w+)((?:\.\w+)*)->${ws}($stringmatch|\w+)/) {
    $pre .= $PREMATCH;
    my $expr = $1;
    my $name = $2;
    my $label = $3;
    my $field = $4;
    $$code = $POSTMATCH;
    $name = spec_type_name($name, $::current);
    strip_quotes(\$field);
    my $tuple = $::type_defs{$name} or die "type $name does not exist";
    my ($tablename, $def, $varname, $origspec, $origname, $type_hash) = @$tuple;
    if($label) {
      my $ignore;
      ($ignore, $type_hash) = parse_typedef("$def,=$label", 1);
    }
    my ($subst, $type, $offset);
    if(defined($varname)) {
      (my $index, $type) = get_type_index($field, $typetable_max, $type_hash);
      $offset = "@#.${varname}\[$index].gen_offset";
    } else {
      ($offset, $type) = get_type_info($field, $type_hash);
    }
    $subst = "GEN_DEREF($expr, $name, /*$field*/ $offset, $type)";
    $pre .= $subst;
  }
  $$code = $pre . $$code;
}

sub gen_typename_header {
  local *OUT = shift;
  print OUT "  // pointers to foreign typetables\n";
  my $typetable_defs = "// typetable definitions\n";
  while(my ($name, $tuple) = each %::type_defs) {
    my ($tablename, $def, $varname, $origspec, $origname, $hash) = @$tuple;
    next unless sp_shorten($origspec, 1) eq sp_shorten($::current, 1);
    print OUT "  const struct gen_tabentry * $varname;\n" if defined($varname);
    $typetable_defs .= gen_type_table($tablename, $typetable_max, 1, []) if defined($tablename);
  }
  return $typetable_defs;
}

sub gen_typename_init {
  local *OUT = shift;
  my $brick = shift;
  while(my ($name, $tuple) = each %::type_defs) {
    my ($tablename, $def, $varname, $origspec, $origname, $hash) = @$tuple;
    next unless defined($tablename);
    next unless sp_shorten($origspec, 1) eq sp_shorten($::current, 1);
    my $code = gen_type_table($tablename, $typetable_max, 0, $hash);
    print OUT "// TYPE table $name\n$code\n";
  }
  while(my ($spec, $list) = each %::extern_type_defs) {
    print OUT "// gen_type table $spec\n";
    unless($list) {
      print OUT "#define type_${spec} type_empty\n\n";
      next;
    }
    print OUT "static const struct gen_type type_${spec}\[] = {\n";
    foreach my $name (sort (split ',', $list)) {
      next unless $name;
      my $tuple = $::type_defs{$name} or die "cannot recover $name";
      my ($tablename, $def, $varname, $origspec, $origname, $hash) = @$tuple;
      $tablename = "NULL" unless defined($tablename);
      my $varoffset = (defined($varname)) ? "STATIC_OFFSET(struct brick_$brick, $varname)" : "-1";
      print OUT "  { \"$name\", \"$def\", $tablename, $varoffset},\n";
    }
    print OUT "  { }\n};\n\n";
  }
}

sub gen_typeconnect_init {
  local *OUT = shift;
  print OUT "  // connect type-tables\n";
  while(my ($name, $tuple) = each %::type_defs) {
    my ($table, $def, $varname, $origspec, $origname, $hash) = @$tuple;
    next unless $varname;
    my $innerspec = $::wires{$origspec} or next;
    my $othername = spec_type_name($origname, $innerspec);
    die "bad name identity $name" if $name eq $othername;
    my $othertuple = $::type_defs{$othername} or die "type declaration $othername does not exist (trying to wire $origspec to $innerspec)";
    my ($othertable, $otherdef, $othervarname, $otherorigspec, $otherorigname, $otherhash) = @$othertuple;
    my $error = isnot_subtype($hash, $otherhash);
    die "type '$name' is not a subtype of '$othername': field $error is missing" if $error;
    my $bricktype = sp_name(spec_bricktype($innerspec), 1);
    my $innertable = "typetable_${bricktype}_$origname";
    print OUT "  ini->$varname = $innertable;\n";
  }
}

##########################################################################

# attribute handling

$::static = "";
$::full = "^";
$::conn_count = 0;
$::conn_totalcount = "0";
$::conn_init = "";

sub add_brick {
  my ($name) = @_;
  $name =~ s/#//;
  $::static = "brick $name={\n^^}\n";
}

sub add_connector {
  my ($type, $namespec, $subbrick, $replace) = @_;
  $subbrick =~ s/#//;
  my ($brick, $name) = sp_parts($namespec);
  my $external_name = $name;
  $external_name = sp_part($replace, 2, 0) if defined($replace);
  die "input/output name '$external_name' too long" if length($external_name) > 7;
  my ($fullname, $count) = spec_conn_instance($namespec);
  $::static =~ s/\^//;
  $::static =~ s/\^/  ${brick}_${type} ${fullname}${count}={\n^  }\n^/;
  my $typecode = $type eq "input" ? 0 : 1;
  my $offset;
  if($count =~ m/\[\]/) {
    $count = 0;
    $offset = -1;
  } else {
    $count =~ s/\[(.*)\]/$1/;
    $count = 1 if $count eq "";
    $offset = "(addr_t)STATIC_OFFSET(struct brick_${brick}, $fullname)";
  }
  my $sect_count = sp_part($namespec, 3, 0);
  my $cname = sp_name(spec_bricktype($namespec), 2);
  my $gen_type = "type_$cname";
  $::conn_init =~ s/\^//;
  $::conn_init .= "  {\n    \"$external_name\",\n    \"^\",\n    $gen_type,\n    init_conn_$cname,\n    exit_conn_$cname,\n    $typecode,\n    $::conn_totalcount,\n    $count,\n    $offset,\n    $sect_count,\n    sizeof(struct local_${subbrick}_$name)\n  },\n";
  $::conn_count++;
  $::conn_totalcount .= " + $count";
}

sub add_attr {
  my ($name, $prio, $val) = @_;
  my ($brick, $conn, $count) = sp_parts($::current);
  $count = "" unless defined($count);
  $prio = "" unless defined($prio);
  my $attr_name;
  if($conn) {
    $attr_name = "    attr${prio} ${brick}.${conn}${count}.$name";
    $::conn_init =~ s/\^/${attr_name}=$val\\n^/;
  } else {
    $attr_name = "  attr${prio} ${brick}.$name";
    $::full =~ s/\^/${attr_name}=$val\\n^/;
  }
  $::static =~ s/\^/${attr_name}=$val\n^/;
}

##########################################################################

# argument passing

# triples (input_args, output_args, clobber_args)

%::op_args = (
"output" => {
  "output_init" => ["destr,constr,clear:=FALSE", "success"],
# static ops
  "trans"    => ["log_addr,log_len,phys_addr,direction,prio:=prio_normal", "success,phys_len", "name,conn1,conn2"],
  "wait"     => ["log_addr,log_len,prio:=prio_normal,action:=action_wait", "success", "phys_addr"],
  "get"      => ["log_addr,log_len,forwrite:=FALSE", "success,phys_addr,phys_len,version"],
  "put"      => ["log_addr,log_len,prio:=prio_none", "success"],
  "lock"     => ["log_addr,log_len,data_lock:=lock_write,addr_lock:=lock_read,try_addr:=_tmp_.log_addr,try_len:=_tmp_.log_len,action:=action_wait", "success,try_addr,try_len" ],
  "unlock"   => ["log_addr,log_len,try_addr:=_args->log_addr,try_len:=_tmp_.log_len", "success,try_addr,try_len"],
  "gadr"     => ["log_len,where:=FALSE,exclu:=TRUE,action:=action_wait,try_len:=_tmp_.log_len", "success,log_addr,log_len"],
  "padr"     => ["log_addr,log_len,where:=FALSE", "success"],
# dynamic ops
  "create"   => ["log_addr,log_len,clear:=FALSE,melt:=TRUE", "success"],
  "delete"   => ["log_addr,log_len", "success"],
  "move"     => ["log_addr,log_len,offset,offset_max:=_tmp_.offset", "success,offset"],
# combinations
  "transwait" => ["log_addr,log_len,phys_addr,direction,prio:=prio_normal", "success,phys_len", "action"],
  "gettranswait"    => ["log_addr,log_len,forwrite:=FALSE,prio:=prio_normal", "success,phys_addr,phys_len", "direction,action,version"],
  "transwaitput"    => ["log_addr,log_len,phys_addr,prio:=prio_normal", "success", "direction,action"],
  "putwait"  => ["log_addr,log_len,prio:=prio_normal", "success", "direction,action"],
  "createget"=> ["log_addr,log_len,clear:=FALSE,melt:=TRUE", "success,phys_addr,phys_len", "forwrite,version"],
  "gadrcreateget" => ["log_len,clear:=FALSE,exclu:=TRUE,action:=action_wait,melt:=TRUE,try_len:=_tmp_.log_len", "success,log_addr,log_len,phys_addr,phys_len", "forwrite"],
  "gadrgettranswait" => ["log_len,exclu:=TRUE,action:=action_wait,forwrite:=FALSE,prio:=prio_normal,try_len:=_tmp_.log_len", "success,log_addr,log_len,phys_addr,phys_len", "where,forwrite,direction,action,version"],
  "putpadr" => ["log_addr,log_len,prio:=prio_background", "success", "where" ],
  "putdelete" => ["log_addr,log_len", "success", "prio" ],
  "putdeletepadr" => ["log_addr,log_len", "success", "where,prio"],
  "gadrtranswaitdeletepadr" => ["phys_addr,phys_len,action:=action_wait", "success", "log_addr,log_len,direction"],
  "gadrcreatetranswaitpadr" => ["phys_addr,phys_len,action:=action_wait,melt:=TRUE", "success", "log_addr,log_len,clear,direction"],
# strategy ops
  "instbrick"    => ["log_addr,name", "success"],
  "deinstbrick"  => ["log_addr", "success"],
  "instconn"     => ["conn1,clear:=FALSE", "success"],
  "deinstconn"   => ["conn1", "success"],
  "connect"      => ["conn1,conn2", "success"],
  "disconnect"   => ["conn1", "success"],
  "getconn"      => ["conn1,res_conn,conn_len", "success,conn_len"],
  "findconn"     => ["conn1,res_conn:=NULL,conn_len:=0", "success,conn_len"],
},
"input" => {
  "input_init"   => ["destr,constr,clear:=FALSE", "success"],
  "retract"      => ["prio:=prio_normal,log_addr:=0,log_len:=(len_t)-1,try_addr:=_args->log_addr,try_len:=_tmp_.log_len", "success"],
},
"brick" => {
  "brick_init"   => ["destr,constr,clear:=FALSE", "success"],
}
);

@::always_allowed = ("mandate", "op_code", "sect_code");

sub gen_call {
  my ($target, $optype, $arg, $param, $caller_spec, $do_restore, $mandate) = @_;
  my $opset = $::op_args{$optype} or die "bad optype '$optype'";
  my $call = "";
  $call .= "call_level++; " if $indent_trace;
  my $brick = sp_part($caller_spec, 1, 0);
  spec_alias(\$target) if $optype eq "output";
  my ($targetbrick, $conn_name, $array, $section, $op) = sp_parts($target);
  $section =~ s/^\(://;
  $section =~ s/:\)$//;
  $section = "\@sect_code" if ($section eq "ALL" or $section =~ m/\.\./);
  my $caller_op = sp_part($caller_spec, 4, 0);
  my $caller_section = sp_part($caller_spec, 3, 0);
  my $assign_op = ($op ne "op" and ((not $do_restore) or (not defined($caller_op)) or $op ne $caller_op));
  my $assign_sect = ($op ne "op" and ((not $do_restore) or (not defined($caller_section)) or $section ne $caller_section));
  if($assign_op) {
    $call .= "op_t __old_op = ($arg)->op_code; " if $do_restore;
    $call .= "($arg)->op_code = opcode_$op; ";
  }
  if($assign_sect) {
    $call .= "index_t __old_sect = ($arg)->sect_code; " if $do_restore;
    $call .= "($arg)->sect_code = $section; ";
  }
  if($mandate) {
    $mandate =~ s/^\[//;
    $mandate =~ s/\]$//;
    $call .= "mand_t __old_mand = ($arg)->mandate; " if $do_restore;
    $call .= "($arg)->mandate = $mandate; ";
  }
  my $opcode = "opcode_$op";
  $opcode = "$arg->op_code" if $op eq "op";
  if($optype eq "brick") {
    die "NYI";
  } elsif($optype eq "input") {
    if($target =~ m/:</) { # call on input
      my $inst = sp_conn_instance($target);
      $call .= "const union connector * _other_ = (void*)&_brick->$inst._input_; ";
      if($op eq "op") {
	$call .= "_other_->input.ops[$section][$arg->op_code - opcode_output_max - 1](_other_, $arg, $param); ";
      } else {
	die "bad input operation \$$op" unless exists $$opset{$op};
	my $directname = sp_name(spec_bricktype($target));
	$call .= "$directname(_other_, $arg, $param); ";
      }
    } else { # call on output
      my $inst = sp_conn_instance($target);
      $call .= "const struct input * _other_; ";
      $call .= "success_t __success = TRUE; ";
      $call .= "for(_other_ = _brick->$inst._output_.rev_chain; _other_; _other_ = _other_->rev_next) { ";
      $call .= "($arg)->success = FALSE; ";
      $call .= "_other_->ops[$section][$opcode - opcode_output_max - 1]((void*)_other_, $arg, $param); ";
      $call .= "__success &= ($arg)->success; } ";
      $call .= "($arg)->success = __success; ";
    }
  # $optype eq "output"
  } elsif($target =~ m/:</) { # call on input
    #TODO: check section bounds
    my $inst = sp_conn_instance($target);
    $call .= "const union connector * _other_ = (void*)_brick->$inst._input_.connect; ";
    $call .= "_other_->output.ops[$section][$opcode](_other_, $arg, $param); ";
  } else { # call on output
    my $other = "_other_";
    if(sp_shorten($target, 3) eq sp_shorten($caller_spec, 3)) {
      # shortcut: don't do any address arithmetic, just pass on
      $other = "on";
    } elsif($target =~ m/:>/) { # output
      my $inst = sp_conn_instance($target);
      $call .= "const union connector * _other_ = (void*)&_brick->$inst._output_; ";
    } else {
      die "bad input/output specifier in '$target'";
    }
    if($op eq "op") {
      $call .= "${other}->output.ops[$section][$arg->op_code]($other, $arg, $param); ";
    } else {
      die "bad output operation \$$op" unless exists $$opset{$op};
      my $directname = sp_name(spec_bricktype($target));
      $call .= "$directname($other, $arg, $param); ";
    }
  }
  $call .= "($arg)->op_code = __old_op; " if $assign_op and $do_restore;
  $call .= "($arg)->sect_code = __old_sect; " if $assign_sect and $do_restore;
  $call .= "($arg)->mandate = __old_mand; " if $mandate and $do_restore;
  $call .= "call_level--; " if $indent_trace;
  return $call;
}

sub gen_paramcall {
  my ($target, $optype, $inargs, $param, $outargs, $caller_spec, $mandate) = @_;
  my $opset = $::op_args{$optype} or die "bad optype '$optype'";
  my $op = sp_part($target, 4, 0);
  my $tuple = $$opset{$op} or die "bad operation name '$op' on '$optype'";
  my ($inparams, $outparams) = @$tuple;
  strip_braces(\$inargs);
  my $call = "({ struct args _tmp_; _tmp_.success = FALSE; ";
  foreach my $rawfield (split ',', $inparams) {
    my ($field, $default) = split /:=/, $rawfield;
    $inargs =~ s/^$ws($wsargmatch)$ws,?//m;
    my $arg = $1;
    $arg = $default if not defined($arg) or $arg eq "";
    die "bad or missing argument '$field' in call to $op" if not defined($arg) or $arg eq "";
    if($field =~ m/^name/) {
      $call .= " strncpy(_tmp_.$field, $arg, sizeof(_tmp_.$field)); ";
    } else {
      $call .= " _tmp_.$field = $arg; ";
    }
  }
  die "bad input parameters '$inargs' in call to $op" if($inargs ne "");
  $call .= gen_call($target, $optype, "&_tmp_", $param, $caller_spec, 0, $mandate);
  strip_braces(\$outargs);
  foreach my $field (split ',', $outparams) {
    $outargs =~ s/^$ws($wsargmatch)$ws,?//m;
    my $arg = $1;
    if($arg ne "") {
      if($field =~ m/^name/) {
	$call .= "strcpy($arg, _tmp_.$field); ";
      } else {
	$call .= "$arg = _tmp_.$field; ";
      }
    }
  }
  die "bad result parameters '$outargs' in call to $op" if($outargs ne "");
  $call .= "})";
  return $call;
}

sub eval_code {
  my ($code, $caller_spec) = @_;
  my $optype = "output";
  my $opset = $::op_args{$optype} or die "bad optype '$optype'";
  warn "please use \@args instead of '$MATCH'" if $$code =~ m/(?<!struct\s)(?<!@|\w|\.)args(?!\w).*/;
  $$code =~ s/\@args/_args/gm;
  warn "please use \@param instead of '$MATCH'" if $$code =~ m/[^@\w.]param(?!\w).*/;
  $$code =~ s/\@param/_param/gm;
  # normal call syntax
  my $pre = "";
  while ($$code =~ m/\@=(input|output)?call${ws}($specmatch)${ws}($brackmatch)?$ws($parenmatch)${ws}(?::$ws($argmatch)${ws})?=>${ws}($parenmatch)/m) {
    my $optype = $1;
    my $op_spec = $2;
    my $mandate = $3;
    my $args = $4;
    my $param = $5;
    my $results = $6;
    $pre .= $PREMATCH;
    $$code = $POSTMATCH;
    $optype = "output" unless defined($optype);
    $op_spec =~ s/\$init/\$${optype}_init/ and warn "@=${optype}call to \$init is deprecated, please replace by \$${optype}_init!";
    $mandate = "@#._mand" unless defined($mandate);
    $param = "_param" unless defined($param);
    $op_spec = sp_complete($op_spec, $caller_spec);
    $pre .= gen_paramcall($op_spec, $optype, $args, $param, $results, $caller_spec, $mandate);
  }
  $$code = $pre . $$code;
  $pre = "";
  # short call syntax
  while ($$code =~ m/\@=(input|output)?call${ws}($specmatch)${ws}($brackmatch)?$ws($argmatch)(?:${ws}:$ws($argmatch))?/m) {
    my $optype = $1;
    my $op_spec = $2;
    my $mandate = $3;
    my $arg_name = $4;
    my $param = $5;
    $pre .= $PREMATCH;
    $$code = $POSTMATCH;
    $optype = "output" unless defined($optype);
    $op_spec =~ s/\$init/\$${optype}_init/ and warn "@=${optype}call to \$init is deprecated, please replace by \$${optype}_init!";
    $param = "_param" unless defined($param);
    $op_spec = sp_complete($op_spec, $caller_spec);
    $pre .= "({ " . gen_call($op_spec, $optype, $arg_name, $param, $caller_spec, 1, $mandate) . "})";
  }
  $$code = $pre . $$code;
  # check & substitute standard args
  my $op_caller = sp_part($caller_spec, 4, 0);
  if($op_caller and $op_caller ne "op") {
    my $tuple = $$opset{$op_caller};
    if(defined($tuple)) {
      my ($inargs, $outargs, $clobberargs) = @$tuple;
      $inargs =~ s/:=[^,]+//g;
      my $list = $inargs . "," . $outargs;
      $list .= "," . $clobberargs if defined($clobberargs);
      foreach my $arg (@::always_allowed, split ',', $list) {
	$$code =~ s/@($arg)/@.OK.$1/mg;
      }
      $$code =~ s/@\.OK\.(\w+)/\(_args->$1\)/mg;
      $$code =~ s/@(\w+)/\(@ NOT_OK_$1\)/mg;
    } 
    #TODO: better checking
  }
  # remaining standard args are substituted unchecked! own risk!
  $$code =~ s/@(\w+)/\(_args->$1\)/mg;
  # substitute output vars having a qualifying prefix
  my $brick = sp_part($::current, 1, 0);
  if($$code =~ m/@[#<>][^.]/m) {
    die "please update the @#varname syntax to @#.varname, @< to @\:<. and @> to @\:>. (see new syntax in the preprocessor guide)\n";
  }
  $$code =~ s/@[:]?>\.?(\w+)($brackmatch|)\.(\w+)/\(_brick->_conn_$1$2.$3\)/mg;
  # substitute normal output vars
  my $out = sp_name($caller_spec, 2);
  $$code =~ s/@[:]?>\.?(\w+)/\(_on->$1\)/mg;
  # substitute generic type accesses
  eval_typename_code($code);
  # substitute brick vars
  $$code =~ s/@\#\.?(\w+)/\(_brick->$1\)/mg;
}

##########################################################################

# parse .ath input text, store structure in global vars
# TODO: convert to a single hash result (allow multiple instances to be parsed)

$::current = "#undef_brick";
$::current_conn_spec = "";
$::strat = undef;

$::pre_header = "[]";
$::pre_brick = "[]";
$::pre_init = "[]";
$::pre_exit = "[]";
$::def_brick = "{}";
$::init_brick = "{}";
$::exit_brick = "{}";

$::warn_syntax = "";
%::conn_spec = ();

%::ops_spec = ();
%::ops_aliases = ();

@::funcs = ();
@::funcs_outputs = ();

%::instances = ();
%::wires = ();
%::aliases = ();
%::sub_conns = ();
%::inst_types = ();

%::pc_defs = ();
%::type_defs = ();
%::extern_type_defs = ();

sub make_ops {
  my $body = shift;
  my ($brick, $conn, $unused, $section, $op) = sp_parts($::current);
  $body =~ s/BRICK_NAME/$brick/gm;
  $body =~ s/CONN_NAME/$conn/gm;
  $body =~ s/SECT_NAME/$section/gm;
  $body =~ s/OP_NAME/$op/gm;
  $::ops_spec{$::current} = $body;
  print "//   operation $::current\n" if($debug_level >= 1);
}

sub parse_lit {
  my $text = shift;
  # currently this just throws away literate programming annotations.
  # we need to write an extra tool for extracting docs.
  $text =~ s/\A${ws}purp(?:ose)?${ws}(.*)\n//;
  if($text =~ s/\A${ws}desc(?:ription)?${ws}//) {
    until ($text =~ s/\A${ws}enddesc(?:ription)?${ws}//) {
      $text =~ s/\A.*\n//;
      die "description not terminated by enddesc" if($text eq "");
    }
  }
  if($text =~ s/\A${ws}example${ws}//) {
    until ($text =~ s/\A${ws}endexample${ws}//) {
      $text =~ s/\A.*\n//;
      die "example not terminated by endexample" if($text eq "");
    }
  }
  return $text;
}

sub parse_attr {
  my ($text,$macros,$do_export) = @_;
  for(;;) {
    if($text =~ m/\A${ws}attr(?:ib)?([0-9])?\s+($idmatch)\s*=\s*([^\s]*)\s*\n/) {
      $text = $POSTMATCH;
      add_attr($2, $1, $3) if $do_export;
      next;
    }
    return $text;
  }
}

sub gen_ops_aliases {
  my ($target, $source, $noforce, $opset) = @_;
  my $sect = sp_part($target, 3, 0);
  my $opname = sp_part($target, 4, 0);
  die "unknown operation \$$opname" unless exists $$opset{$opname};
  if($sect eq "ALL") {
    my $count = sp_part($::current_conn_spec, 3, 0);
    for(my $i = 0; $i < $count; $i++) {
      my $newtarget = $target;
      $newtarget =~ s/ALL/$i/;
      next if $noforce and exists $::ops_aliases{$newtarget};
      $::ops_aliases{$newtarget} = $source;
    }
  } elsif($sect =~ m/\A(.*)\.\.(.*)\Z/) {
    my $start = $1;
    my $end = $2;
    $start = eval_compute($start, "start-value");
    $end = eval_compute($end, "end-value");
    for(my $i = $start; $i < $end; $i++) {
      my $newtarget = $target;
      $newtarget =~ s/\(:.*:\)/\(:$i:\)/ or die "bad internal subst at index $i";
      next if $noforce and exists $::ops_aliases{$newtarget};
      $::ops_aliases{$newtarget} = $source;
    }
  } else {
    $::ops_aliases{$target} = $source unless $noforce and exists $::ops_aliases{$target};
  }
}

sub parse_2 {
  my ($text,$macros) = @_;
  my $remember = not defined(sp_part($::current,1,1));
  my $optype = ($::current =~ m/:</) ? "input" : "output";
  my $opset = $::op_args{$optype} or die "bad optype '$optype'";
  for(;;) {
    if($text =~ s/\A${ws}section$ws($specmatch)//) {
      my $sect = $1;
      die "incorrect section specifier '$sect'" unless sp_type($sect) == 3;
      $::current = sp_complete($sect);
      next;
    }
    if($text =~ m/\A(${ws}(static[^(]+$parenmatch)$ws$bracematch)/m) {
      $text = $POSTMATCH;
      if($remember) {
	push @::funcs, $1;
	push @::funcs_outputs, sp_shorten($::current, 3);
      }
      next;
    }
    if($text =~ m/\A${ws}operation$ws($specmatch(?:,$specmatch)*)($ws$bracematch)/m) {
      my $names = $1;
      my $body = $2;
      $text = $POSTMATCH;
      $names =~ s/\$init/\$output_init/ and warn "operation \$init is deprecated, please replace by \$output_init!";
      if($remember) {
	if($names eq "\$op") {
	  $::current = sp_complete($names);
	  make_ops($body);
	  my $secspec = sp_shorten($::current, 3);
	  my @trylist = ();
	  foreach my $ops_spec (keys %::ops_aliases) {
	    next unless sp_shorten($ops_spec, 3) eq $secspec;
	    my $tryname = sp_part($ops_spec, 4);
	    push @trylist, $tryname;
	  }
	  foreach my $opname (keys %$opset) {
	    my $allowed = 1;
	    foreach my $tryname (@trylist) {
	      $allowed = 0 if $opname =~ m/$tryname/;
	    }
	    next unless $allowed;
	    my $target_spec = sp_complete("\$$opname");
	    gen_ops_aliases($target_spec, $::current, 1, $opset);
	  }
	} elsif($body =~ m/OP_NAME/m) {
	  foreach my $level2 (split ',', $names) {
	    die "bad operation type '$level2'" if sp_type($level2) != 4;
	    $::current = sp_complete($level2);
	    make_ops($body);
	    gen_ops_aliases($::current, $::current, 0, $opset);
	  }
	} else {
	  # avoid code bloat when OP_NAME is missing (generate 1 instance)
	  my $level2 = $names;
	  $level2 =~ s/\$//g;
	  $level2 =~ s/,/X/g;
	  $level2 =~ s/^/\$/;
	  $::current = sp_complete($level2);
	  make_ops($body);
	  foreach my $name (split ',', $names) {
	    my $single = sp_complete($name);
	    gen_ops_aliases($single, $::current, 0, $opset);
	  }
	}
      }
      next;
    }
    return $text;
  }
}

sub parse_subinstances {
  my ($text, $remember) = @_;
  for(;;) {
    if($text =~ m/\A${ws}instance$ws($specmatch)${ws}as${ws}(\w+)$ws;/) {
      my $type = $1;
      my $name = $2;
      $text = $POSTMATCH;
      sp_syntax($type);
      die "bad instance brick type '$type'" unless sp_type($type) == 1;
      my $oldcurrent = $::current;
      my $filename = sp_part($type, 1, 0) . ".ath";
      my %dummy_macros = %::base_macros;
      my ($rest,) = parse_file($filename, sp_part($::current, 1), $name, \%dummy_macros);
      $rest =~ s/${ws}//mg;
      die "incomplete instance sub-parse in $filename\n$rest" if $rest;
      $::instances{$name} = $type if $remember;
      $::current = $oldcurrent;
      while($text =~ m/\A${ws}(wire|alias)${ws}($specmatch)${ws}as${ws}($specmatch)$ws;/) {
	my $cmd = $1;
	my $sub_conn = $2;
	my $loc_conn = $3;
	$text = $POSTMATCH;
	die "bad specified type '$sub_conn'" unless sp_type($sub_conn) == 2;
	die "bad specified type '$loc_conn'" unless sp_type($loc_conn) == 2;
	my $new_spec = sp_part($::current, 1) . "#$name";
	my $sub_complete = sp_complete($sub_conn, $new_spec);
	my $loc_complete = sp_complete($loc_conn);
	my $sub_core = $sub_complete;
	my $loc_core = $loc_complete;
	$sub_core =~ s/\[\]//;
	$loc_core =~ s/\[\]//;
	#print "SUB: $sub_complete LOC: $loc_complete\n";
	if(($sub_complete =~ m/:</) xor ($loc_complete =~ m/:</)) {
	  warn "WARNING: in future releases keyword 'wire' will be REQUIRED instead of '$cmd' for specifiers $sub_conn $loc_conn. Please update your sourcecode!" unless $cmd eq "wire";
	  # create internal wire
	  if($loc_complete =~ m/\[\]/) {
	    die "you can alias only a sub-instance array to a local array" unless $sub_complete =~ m/\[\]/;
	  } else {
	    die "you must alias a sub-instance array to a local array as a whole" if $sub_complete =~ m/\[\]/;
	  }
	  if($sub_complete =~ m/:</) {
	    # alias a sub-input to a local output
	    $::wires{$sub_complete} = $loc_complete if $remember;
	    $::aliases{$sub_core} = $loc_core unless $sub_complete =~ m/\[[^\]]+\]/;
	  } else {
	    # alias a local input to a sub-output
	    $::wires{$loc_complete} = $sub_complete if $remember;
	    if($sub_complete =~ m/\[[^\]]+\]/) {
	      $::aliases{$loc_core} = $sub_complete;
	    } else {
	      $::aliases{$loc_core} = $sub_core;
	    }
	  }
	} else {
	  warn "WARNING: in future releases keyword 'alias' will be required instead of '$cmd' for specifiers $sub_conn $loc_conn" unless $cmd eq "alias";
	  # create an external alias
	  die "target of external alias $loc_conn must not be a sub-instance" if defined(sp_part($loc_conn, 1, 1));
	  $::aliases{$loc_core} = $sub_core;
	  my $full_spec = $::sub_conns{$sub_core} or die "sub-specifier $sub_conn does not exist";
	  if($full_spec =~ m/$brackmatch/) {
	    die "sub-specifier $sub_conn must be written $sub_core\[]" unless $sub_conn =~ m/\[\]/;
	    die "alias-specifier $loc_complete must be written $loc_core\[]" unless $loc_complete =~ m/\[\]/;
	  }
	  my $conn_type = ($full_spec =~ m/:</) ? "input" : "output";
	  add_connector($conn_type, $full_spec, $type, $loc_complete) if $remember;
	  my $xname = sp_name(spec_bricktype($sub_complete), 2);
	  my $yname = sp_name(spec_bricktype($loc_complete), 2);
	  #print "xname=$xname yname=$yname ($::extern_type_defs{$xname})\n";
	  die "undefined type info for $xname (internal inconsistency)" unless exists $::extern_type_defs{$xname};
	  $::extern_type_defs{$yname} = $::extern_type_defs{$xname};
	}
      }
      next;
    }
    return $text;
  }
}

sub parse_1 {
  my ($text,$subbrick,$macros) = @_;
  my $remember = not defined(sp_part($::current,1,1));
  $text = parse_subinstances($text, $remember);
  # brick operations
  for(;;) {
    if($text =~ m/\A${ws}operation$ws($specmatch)($ws$bracematch)/m) {
      my $name = $1;
      my $body = $2;
      $text = $POSTMATCH;
      if($remember) {
	$::current = sp_complete($name, sp_part($::current,1) . ":<BRICK(:0:)");
	make_ops($body);
	gen_ops_aliases($::current, $::current, 0, $::op_args{"brick"});
      }
      next;
    }
    last;
  }
  # inputs and outputs
  for(;;) {
    if($text =~ m/\A${ws}(local$ws)?(input|output)$ws($specmatch)/m) {
      my $local = $1;
      my $type = $2;
      my $level1 = $3;
      $::current = sp_shorten(sp_complete($level1), 2);
      my $array = sp_part($::current, 2, 1);
      my $nr_sections = sp_part($level1, 3, 0);
      $nr_sections = "1" unless defined($nr_sections);
      $nr_sections = eval_compute($nr_sections);
      my $enhanced_spec = sp_complete("(:$nr_sections:)");
      $::current_conn_spec = $enhanced_spec;
      $::current .= "(:0:)";
      $text = $POSTMATCH;
      my $do_export = ($remember and not defined($local));
      add_connector($type, $enhanced_spec, $subbrick) if $do_export;
      my $xname = sp_name(spec_bricktype($enhanced_spec), 2);
      $::extern_type_defs{$xname} = "";
      $text = parse_lit($text, $macros);
      $text = parse_attr($text, $macros, $do_export);
      $text =~ m/\A$ws(?:(data$ws)?($bracematch))?/;
      my $xdef = $1;
      my $def = $2;
      $text = $POSTMATCH;
      $text =~ m/\A$ws(?:(init$ws)?($bracematch))?/;
      my $xinitbody = $1;
      my $initbody = $2;
      $text = $POSTMATCH;
      $text =~ m/\A$ws(?:(exit$ws)?($bracematch))?/;
      my $xexitbody = $1;
      my $exitbody = $2;
      $text = $POSTMATCH;
      $::warn_syntax .= "$::current " if ($def and not $xdef) or ($initbody and not $xinitbody) or ($exitbody and not $xexitbody);
      $def = "{\n}" if not defined($def);
      $initbody = "{\n}" if not defined($initbody);
      $exitbody = "{\n}" if not defined($exitbody);
      if($type eq "input") {
	die "bad input specifier '$level1'" unless $level1 =~ m/^:</;
	while($text =~ m/\A${ws}use\s/) {
	  die "'use' statement not allowed on arrays" if $array;
	  next if parse_types(\$text);
	  if($text =~ m/\A${ws}use\s+PC\s(\w+)$ws($sectmatch)?$ws(?:\[($argmatch)\]$ws)?(?:(aligned)$ws(?:($parenmatch)$ws)?(round$ws))?;/) {
	    my $name = $1;
	    my $sect = $2;
	    my $max = $3;
	    $max = "16" unless defined($max);
	    my $aligned = $4;
	    my $align_size = $5;
	    $align_size = "DEFAULT_TRANSFER" if $aligned and not defined($align_size);
	    $align_size = "sizeof(void*)" unless defined($align_size);
	    my $whole_size = defined($6) ? $align_size : "1";
	    $text = $POSTMATCH;
	    if($remember) {
	      $sect = sp_part($sect, 3, 0) if defined($sect);
	      my $input = sp_part($enhanced_spec, 2, 0);
	      $::pc_defs{$name} = [$input, $sect, $max, $align_size, $whole_size];
	    }
	    next;
	  }
	  if($text =~ m/\A${ws}use\s+HASH\s+(\w+)${ws}\[$ws($argmatch)$ws\]${ws}on\s+(\w+)$ws,$ws(\w+)${ws}function$ws($parenmatch)$ws;/) {
	    my $name = $1;
	    my $max = $2;
	    my $base_cache = $3;
	    my $elem_cache = $4;
	    $text = $POSTMATCH;
	    $$macros{"BASE_$name"} = ["BASE_$name", "", "", $base_cache];
	    next;
	  }
	  die "bad use syntax";
	}
      } elsif($type eq "output") {
	die "bad output specifier '$level1'" unless $level1 =~ m/^:>/;
	while($text =~ m/\A${ws}define\s/) {
	  die "'define' statement not allowed on arrays" if $array;
	  next if parse_types(\$text);
	  die "bad define syntax";
	}
      } else {
	die "this should not happen";
      }
      if($remember) {
	die "$enhanced_spec already defined" if exists $::conn_spec{$enhanced_spec};
	$::conn_spec{$enhanced_spec} = [$def, $initbody, $exitbody];
      } else {
	my $short_spec = sp_shorten($enhanced_spec, 2);
	$short_spec =~ s/$brackmatch//;
	#print "ENH: $enhanced_spec | $short_spec\n";
	$::sub_conns{$short_spec} = $enhanced_spec;
      }
      print "// $type $::current\n" if($debug_level >= 1);
      $text = parse_2($text, $macros);
      next;
    }
    return $text;
  }
}

sub parse_all {
  my ($text,$prefix,$suffix,$macros) = @_;
  if($text =~ m/\A(?:\s*Author:\s[^\n]+\n)+\s*Copyright:\s[^\n]+\n\s*License: see files ([\w-]+(?:, [\w-]+)*)\s*\n/m) {
    $copyright = "/*\n$MATCH*/\n";
    my $licenses = $1;
    $text = $POSTMATCH;
    foreach my $license(split /,\s*/, $licenses) {
      die "license file $license does not exist" unless -s "../$license";
    }
  } else {
    warn "please start your file with Author: Copyright: and License: clauses";
  }
  for(;;) {
    if($text =~ m/\A${ws}(?:context|defaultbuild).*\n/) {
      $text = $POSTMATCH;
      next;
    }
    if($text =~ m/\A${ws}buildrules(?:.*\n)*?\s*endrules/m) {
      $text = $POSTMATCH;
      next;
    }
    last;
  }
  for(;;) {
    if($text =~ m/\A${ws}(strategy)/m) {
      $::strat = $1;
      $text = $POSTMATCH;
      next;
    }
    last;
  }
  $text =~ m/\A${ws}brick$ws($specmatch)/m or die "brick statement missing";
  my $brick = $1;
  $text = $POSTMATCH;
  sp_syntax($brick);
  die "bad brick type '$brick'" unless sp_type($brick) == 1;
  if($prefix) {
    $::current = "$prefix#$suffix";
  } else {
    $::current = $brick;
    add_brick($brick);
  }
  $::inst_types{$::current} = $brick;
  $text = parse_lit($text, $macros);
  $text = parse_attr($text, $macros, not $prefix);

  # this is old syntax, will VANISH in the future!!!
  if($text =~ m/\A$ws($brackmatch)?$ws($brackmatch)?$ws($brackmatch)?$ws($brackmatch)?$ws($bracematch)$ws($bracematch)?$ws($bracematch)?/m) {
    $text = $POSTMATCH;
    unless($prefix) {
      $::pre_header = $1;
      $::pre_brick = $2;
      $::pre_init = $3;
      $::pre_exit = $4;
      $::def_brick = $5;
      $::init_brick = $6;
      $::exit_brick = $7;
      warn "please update your code syntax to the new brick init conventions (see newest version of preprocessor guide)";
    }
  } else { # only this will survive!
    for(;;) {
      if($text =~ m/\A${ws}static_header$ws($bracematch)/) {
	$text = $POSTMATCH;
	$::pre_header = $1 unless $prefix;
	next;
      }
      if($text =~ m/\A${ws}static_data$ws($bracematch)/) {
	$text = $POSTMATCH;
	$::pre_brick = $1 unless $prefix;
	next;
      }
      if($text =~ m/\A${ws}static_init$ws($bracematch)/) {
	$text = $POSTMATCH;
	$::pre_init = $1 unless $prefix;
	next;
      }
      if($text =~ m/\A${ws}static_exit$ws($bracematch)/) {
	$text = $POSTMATCH;
	$::pre_exit = $1 unless $prefix;
	next;
      }
      if($text =~ m/\A${ws}data$ws($bracematch)/) {
	$text = $POSTMATCH;
	$::def_brick = $1 unless $prefix;
	next;
      }
      if($text =~ m/\A${ws}init$ws($bracematch)/) {
	$text = $POSTMATCH;
	$::init_brick = $1 unless $prefix;
	next;
      }
      if($text =~ m/\A${ws}exit$ws($bracematch)/) {
	$text = $POSTMATCH;
	$::exit_brick = $1 unless $prefix;
	next;
      }
      last;
    }
  }
  print "// brick $::current\n" if($debug_level >= 1);
  return parse_1($text, $brick, $macros);
}

sub parse_file {
  my ($name,$prefix,$suffix,$macros) = @_;
  my $text = readfile($name, $write_line_directives);
  eval_macros(\$text, 99999, 0, $macros);
  return (parse_all($text, $prefix, $suffix, $macros), $text);
}

##########################################################################

# consistency checks

sub test_twice {
  my %names = %::aliases;
  foreach my $testspec (keys %::conn_spec) {
    my $test = sp_shorten($testspec, 2);
    my $found = $names{$test};
    if($found) {
      $test =~ m/:(<|>)/;
      my $type = $1;
      die "connector $test is defined twice" if $found =~ m/:$type/;
    } else {
      $names{$test} = "x";
    }
  }
}

##########################################################################

# generate code from parsed data

sub gen_magic {
  local *OUT = shift;
  my $str = shift;
  my $magic = md5_hex($str);
  my $m1 = substr($magic,0,16);
  my $m2 = substr($magic,16,16);
  print OUT "#undef MAGIC_$str\n#define MAGIC_$str (0x${m1}LL ^ 0x${m2}LL)\n\n";
}

sub gen_header {
  local *OUT = shift;
  my $brick = sp_name($::current, 1);
  print OUT "#include \"../common.h\"\n"; 
  print OUT "#include \"../strat.h\"\n" if defined($::strat);
  while(my ($name, $spec) = each %::instances) {
    my $br = sp_part($spec, 1, 0);
    print OUT "#include \"$br.h\"\n";
  }
  print OUT "\n\n";
  print OUT "// General defs\n\n";
  my $pre = $::pre_header;
  $pre = "// no predefinitions" unless defined($pre);
  strip_braces(\$pre);
  strip_brackets(\$pre);
  purge(\$pre);
  print OUT "$pre\n";
  print OUT "\n// Local input/output state\n\n";
  while(my ($spec, $tuple) = each %::conn_spec) {
    my ($def) = @$tuple;
    my $type = ($spec =~ m/:</) ? "input" : "output";
    my $brick_ptr = "";
    $brick_ptr = "struct brick_$brick * _brick_ptr_; " if $spec =~ m/\[/;
    $def =~ s/^($ws\{)/$1 struct $type _${type}_; $brick_ptr/;
    purge(\$def);
    my $name = sp_name($spec, 2);
    print OUT "struct local_$name $def;\n\n";
  }
  print OUT "\n";

  print OUT "\n// brick $brick static init / exit\n";
  print OUT "\nvoid init_static_$brick (void);\n";
  print OUT "\nvoid exit_static_$brick (void);\n";
  print OUT "\n// brick $brick data\n";
  # gen brick definition
  print OUT "\nstruct brick_$brick {\n";
  # automatic fields
  print OUT "  // automatically added fields\n";
  print OUT "  mand_t _mand;\n";
  print OUT "  brick_operation_set * ops;\n";
  # gen input and output instances
  print OUT "  // connector instances\n";
  foreach my $spec (keys %::conn_spec) {
    next if $spec =~ m/\[\]/; # skip dynamic arrays
    my $conn = sp_conn_instance($spec);
    my $name = sp_name($spec, 2);
    print OUT "  struct local_$name $conn;\n";
  }
  # gen PC fields
  print OUT "  // PC fields\n";
  while(my ($name, $tuple) = each %::pc_defs) {
    my ($input, $sect, $max, $align_size, $whole_size) = @$tuple;
    print OUT "#define PC_${name}_MAX ($max)\n";
    print OUT "#define PC_${name}_ALIGN ($align_size)\n";
    print OUT "#define PC_${name}_WHOLE ($whole_size)\n";
    print OUT "  struct pc _pc_$name;\n";
    print OUT "  struct pc_elem _pc_elem_$name\[$max];\n";
  }
  # gen TYPE fields
  my $typetable_defs = gen_typename_header(*OUT);
  # gen sub-instances
  print OUT "  // sub-instances\n";
  while(my ($name, $spec) = each %::instances) {
    my $br = sp_part($spec, 1, 0);
    print OUT "  struct brick_$br _sub_$name;\n";
  }
  # user-defined fields
  print OUT "  // user-defined fields\n";
  my $def = $::def_brick;
  strip_braces(\$def);
  purge(\$def);
  indent(\$def);
  print OUT "$def\n};\n\n";
  # print typetable defs
  print OUT "$typetable_defs\n";
  # print init routine prototype for each connector
  # TODO: avoid code bloat: not all of these are needed when inline expansion is preferred for non-dynamic array connectors
  print OUT "// connector init routines\n";
  foreach my $spec (keys %::conn_spec) {
    my $cname = sp_name($spec, 2);
    print OUT "void init_conn_$cname(void * _ini, void * _brick);\n";
    print OUT "void exit_conn_$cname(void * _ini, void * _brick);\n";
  }
  print OUT "\n";
  # generate prototypes for externally callable operations
  foreach my $spec (keys %::conn_spec) {
    my $optype = ($spec =~ m/:</) ? "input" : "output";
    my $opset = $::op_args{$optype} or die "bad optype '$optype'";
    my $count = sp_part($spec, 3, 0);
    my $max = eval_compute($count);
    for(my $sect = 0; $sect < $max; $sect++) {
      my $secspec = sp_complete("(:" . $sect . ":)", $spec);
      print OUT "// ops for $secspec\n";
      my %copy_ops = %$opset;
      foreach my $ops_spec (keys %::ops_aliases) {
	next unless sp_shorten($ops_spec, 3) eq sp_shorten($secspec, 3);
	my $dst_name = sp_name($ops_spec);
	print OUT "static_operation $dst_name;\n";
	$dst_name = sp_part($ops_spec, 4, 0);
	delete $copy_ops{$dst_name};
      }
      # print unimplemented names
      print OUT "// unimplemented ops\n";
      foreach my $bare (keys %copy_ops) {
	my $dst_name = sp_name($secspec . "\$" . $bare);
	my $src_name = "missing_$bare";
	print OUT "static_operation $dst_name __attribute__((alias(\"$src_name\")));\n";
      }
      print OUT "\n";
    }
  }
  print OUT "\n";
}

sub make_pointers {
  my ($code, $spec, $direct) = @_;
  my $brick = sp_name($spec, 1);
  my $conn = sp_name($spec, 2);
  if($direct) {
    insert_pseudoparam($code, "struct brick_${brick} * const _brick = brick; (void)_brick; ");
  } elsif($spec =~ m/\[/) {
    insert_pseudoparam($code, "struct brick_${brick} * const _brick = _on->_brick_ptr_; (void)_brick; ");
  } elsif($spec =~ m/:<BRICK/) {
    insert_pseudoparam($code, "struct brick_${brick} * const _brick = (void*)on; (void)_brick; ");
    return;
  } else {
    # compute static offset (this is more efficient)
    my $field = sp_conn_instance($spec);
    insert_pseudoparam($code, "struct brick_${brick} * const _brick = BASE(_on, struct brick_${brick}, $field); (void)_brick; ");
  }
  insert_pseudoparam($code, "struct local_${conn} * const _on = (void*)on; (void)_on; ");
}

sub gen_ops {
  local *OUT = shift;
  # print local functions
  my @outputs = (@::funcs_outputs);
  foreach my $proc (@::funcs) {
    my $out = shift @outputs;
    eval_code(\$proc, $out);
    make_pointers(\$proc, $out, 1);
    purge(\$proc);
    print OUT "$proc\n";
  }
  # print body of all operations
  while(my ($spec, $code) = each %::ops_spec) {
    eval_code(\$code, $spec);
    my $name = sp_name($spec);
    print OUT "#undef OPERATION\n#define OPERATION \"$name\"\n\n";
    print OUT "void $name(const union connector * on, struct args * _args, const char * _param)\n";
    make_pointers(\$code, $spec, 0);
    purge(\$code);
    print OUT "$code\n\n";
  }
  # generate alias names
  while(my ($ops_spec,$op) = each %::ops_aliases) {
    next if $op eq $ops_spec;
    my $src_name = sp_name($op);
    my $dst_name = sp_name($ops_spec);
    print OUT "static_operation $dst_name __attribute__((alias(\"$src_name\")));\n\n";
  }
}

sub gen_conn_init {
  my ($spec, $tuple, $exit_mode) = @_;
  my $cname = sp_name($spec, 2);
  my $res ="{\n  struct local_$cname * conn = _conn; (void)conn;\n";
  my ($def, $initcode, $exitcode) = @$tuple;
  my $code = $exit_mode ? $exitcode : $initcode;
  my $name = sp_name($spec, 2);
  if($spec =~ m/:</) {
    $res .= "  conn->_input_.ops = ops_$name;\n" unless $exit_mode;
    if($code =~ m/@<(\w+)/mg) {
      warn "please update the @<varname syntax to \@:<.varname (see new syntax in the preprocessor guide)\n";
    }
    $code =~ s/@[:]?<\.?(\w+)/\(conn->$1\)/mg;
  } else {
    $res .= "  conn->_output_.ops = ops_$name;\n" unless $exit_mode;
    if($code =~ m/@>(\w+)/mg) {
      warn "please update the @>varname syntax to \@:>.varname (see new syntax in the preprocessor guide)\n";
    }
    $code =~ s/@[:]?>\.?(\w+)/\(conn->$1\)/mg;
  }
  $res .= "  conn->_brick_ptr_ = _brick;\n" if $spec =~ m/\[/ and not $exit_mode;
  $code =~ s/\@param/_param/mg;
  purge(\$code);
  indent(\$code);
  return "$res$code\n}\n";
}

sub gen_routine {
  local *OUT = shift;
  my ($name, $code) = @_;
  $code = "[\n]" unless defined($code);
  $code =~ s/\A\[/{/;
  $code =~ s/\]\Z/}/;
  purge(\$code);
  print OUT "#undef OPERATION\n#define OPERATION \"$name\"\n\n";
  print OUT "void ${name}(void)\n$code\n\n";
}

sub subst_brickvars { # this is provisionary and should vanish
  my $code = shift;
  if($code =~ m/@#(\w+)/mg) {
    warn "please update the @#varname syntax to @#.varname (see new syntax in the preprocessor guide)\n";
  }
  $code =~ s/@#\.?(\w+)/\(ini->$1\)/mg;
  return $code;
}

sub gen_initexit {
  local *OUT = shift;
  my $optype = shift;
  my $brick = sp_part($::current, 1, 0);
  print OUT "#undef OPERATION\n#define OPERATION \"${optype}_$brick\"\n\n";
  if($optype eq "init") {
    print OUT "mand_t init_$brick (void * _ini_, const char * _param, mand_t _mand)\n{\n";
  } else {
    print OUT "void exit_$brick (void * _ini_, const char * _param)\n{\n";
  }
  print OUT "struct brick_$brick * ini = _ini_; (void)ini; int _i_; (void)_i_;\n";
  if($optype eq "init") {
    print OUT "ini->_mand = _mand;\n  ini->ops = ops_${brick}_BRICK;\n";
  }
  print OUT "  // $optype connectors\n";
  while(my ($spec, $tuple) = each %::conn_spec) {
    next if $spec =~ m/\[\]/; # skip dynamic arrays
    print OUT "  // $optype $spec\n";
    my ($shortname, $array) = spec_conn_instance($spec);
    $array =~ s/\[(.*)\]/$1/;
    my $index = "";
    if($array ne "") {
      print OUT "for(_i_ = 0; _i_ < $array; _i_++) {\n";
      $index = "[_i_]";
    }
    my ($def, $initcode, $exitcode) = @$tuple;
    my $code = $optype eq "init" ? $initcode : $exitcode;
    if($code =~ m/\A$ws\{$ws\}$ws\Z/) { # inline expansion only in trivial case
      if($optype eq "init") {
	my $inittext = gen_conn_init($spec, $tuple, 0);
	indent(\$inittext, "    ");
	print OUT "  { void * _conn = &ini->$shortname$index; void * _brick = ini; (void)_brick;\n$inittext  }\n";
      } else {
	print OUT "  // not called\n";
      }
    } else {
      my $funcname = "${optype}_conn_" . sp_name($spec, 2);
      print OUT "  $funcname(&ini->$shortname$index, ini);\n";
    }
    if($array ne "") {
      print OUT"}\n";
    }
  }
  # process sub-instances
  print OUT "\n  // $optype sub-instances\n";
  while(my ($name, $spec) = each %::instances) {
    my $br = sp_part($spec, 1, 0);
    if($optype eq "init") {
      print OUT "  _mand = init_$br (&ini->_sub_$name, _param, _mand);\n";
    } else {
      print OUT "  exit_$br (&ini->_sub_$name, _param);\n";
    }
  }
  print OUT "\n";
}

sub gen_init {
  local *OUT = shift;
  my $brick = sp_part($::current, 1, 0);
  # create TYPE tables
  gen_typename_init(*OUT, $brick);
  # initialize static ops structure for each connector
  foreach my $spec ("#$brick:<BRICK(:1:)", keys %::conn_spec) {
    my $index_shift = "";
    my $optype = "output";
    if($spec =~ m/:<BRICK/) {
      $index_shift = " - opcode_input_max - 1";
      $optype = "brick";
    } elsif($spec =~ m/:</) {
      $index_shift = " - opcode_output_max - 1";
      $optype = "input";
    }
    my $opset = $::op_args{$optype} or die "bad optype '$optype'";
    my $count = sp_part($spec, 3, 0);
    my $name = sp_name($spec, 2);
    print OUT "${optype}_operation_set ops_$name\[] = {\n";
    my $max = eval_compute($count);
    for(my $sect = 0; $sect < $max; $sect++) {
      print OUT "  {\n";
      my $op_defined = undef;
      my $secspec = sp_complete("(:" . $sect . ":)", $spec);
      my %copy_ops = %$opset;
      while(my ($ops_spec,$op) = each %::ops_aliases) {
	next unless sp_shorten($ops_spec, 2) eq sp_shorten($secspec, 2);
	my $op_sect = sp_part($op, 3, 0);
	if($op_sect =~ m/\A(.*)\.\.(.*)\Z/) {
	  my $start = $1;
	  my $end = $2;
	  $start = eval_compute($start, "start-value");
	  $end = eval_compute($end, "end-value");
	  next unless ($start <= $sect and $sect <= $end);
	} else {
	  next unless ($op_sect eq $sect or $op_sect eq "ALL");
	}
	my $dst_name = sp_part($ops_spec, 4, 0);
	my $src_name = sp_name($op);
	$op_defined = $src_name if $dst_name eq "op";
	print OUT "    [opcode_$dst_name$index_shift] = &${src_name},\n";
	delete $copy_ops{$dst_name};
      }
      # print unimplemented names
      print OUT "    // unimplemented ops\n";
      foreach my $bare (keys %copy_ops) {
	my $dst_name = sp_name($secspec . "\$" . $bare);
	my $src_name = defined($op_defined) ? $op_defined : "missing_${sect}_$bare";
	print OUT "    [opcode_$bare$index_shift] = &$src_name,\n";
      }
      print OUT "  },\n";
    }
    print OUT "};\n\n";
  }
  # create init and exit routines for each connector
  while(my ($spec, $tuple) = each %::conn_spec) {
    my $cname = sp_name($spec, 2);
    my $initcode = gen_conn_init($spec, $tuple, 0);
    my $exitcode = gen_conn_init($spec, $tuple, 1);
    print OUT "void init_conn_$cname(void * _conn, void * _brick)\n$initcode\n";
    print OUT "void exit_conn_$cname(void * _conn, void * _brick)\n$exitcode\n";
  }
  # create init routine for the brick instance
  gen_initexit(*OUT, "init");
  # wire sub-instances
  print OUT "  // wire sub-instances\n";
  while(my ($input, $output) = each %::wires) {
    my ($in_instance,$in_array) = spec_conn_instance($input);
    my ($out_instance,$out_array) = spec_conn_instance($output);
    my $whole_array_in = $in_array =~ s/\[\]/\[_i_\]/;
    my $whole_array_out = $out_array =~ s/\[\]/\[_i_\]/;
    if($whole_array_in or $whole_array_out) {
      my $full_spec = undef;
      foreach my $sub_core ($input, $output) {
	$sub_core =~ s/$brackmatch//;
	$full_spec = $::sub_conns{$sub_core} unless defined($full_spec);
      }
      die "cannot find connector $input / $output" unless defined($full_spec);
      my $bound = sp_part($full_spec, 2, 1);
      $bound =~ s/^\[//;
      $bound =~ s/\]$//;
      print OUT "for(_i_ = 0; _i_ < $bound; _i_++) {\n";
    }
    print OUT "  ini->$in_instance$in_array._input_.connect = &ini->$out_instance$out_array._output_;\n";
    print OUT "  ini->$in_instance$in_array._input_.rev_next = ini->$out_instance$out_array._output_.rev_chain;\n";
    print OUT "  ini->$out_instance$out_array._output_.rev_chain = &ini->$in_instance$in_array._input_;\n";
    print OUT "}\n" if($whole_array_in or $whole_array_out);
  }
  # connect type tables
  gen_typeconnect_init(*OUT);
  # init PCs
  print OUT "  // initialize PCs\n";
  while(my ($name, $tuple) = each %::pc_defs) {
    my ($input, $sect, $max) = @$tuple;
    print OUT "\n  ini->_pc_$name.pc_version = 1;";
    print OUT "\n  ini->_pc_$name.pc_input = &ini->_conn_$input._input_;\n";
    print OUT "\n  ini->_pc_$name.pc_sect = $sect;" if defined($sect);
  }
  # user-defined part
  print OUT "\n// user-defined init part\n";
  my $code = $::init_brick;
  $code = "{\n}" unless defined($code);
  $code = subst_brickvars($code);
  $code =~ s/\@param/_param/mg;
  purge(\$code);
  indent(\$code);
  print OUT "$code\nraw_exit:\n  return _mand + 1;\n  goto raw_exit;\n}\n\n";

  # --- create dynamic exit routine
  gen_initexit(*OUT, "exit");
  $code = $::exit_brick;
  $code = "{\n}" unless defined($code);
  $code = subst_brickvars($code);
  $code =~ s/\@param/_param/mg;
  purge(\$code);
  indent(\$code);
  print OUT "  // brick exit code\n$code\n}\n\n";
  # create static init/exit routines
  gen_routine(*OUT, "init_static_$brick", $::pre_init);
  gen_routine(*OUT, "exit_static_$brick", $::pre_exit);
}

##########################################################################

# main program

sub readfile {
  my ($filename, $use_directives) = @_;
  my $text = "";
  open(IN, "< $filename") or die "cannot open input file $filename";
  if(!$use_directives) {
    local $/;
    $text = <IN>;
  } else {
    my $nr = 1;
    my $normalline = 1;
    while(my $line = <IN>) {
      $text .= "#line $nr \"$filename\"\n" if $normalline;
      $text .= $line;
      $nr++;
      $normalline = ($line !~ m/\\$/);
    }
  }
  close(IN);
  return $text;
}

my $common = readfile("common.ath", 0);
eval_macros(\$common, 1, 1, \%::base_macros);
my %macros = %::base_macros;

my ($rest, $preprocessed) = parse_file($infile, "", "", \%macros);

open(OUT, "> $prefile") or die "cannot create output file";
print OUT $preprocessed;
close(OUT);

$rest =~ s/${ws}//mg;
if($rest) {
  die "incomplete parse:\n$rest";
}

test_twice();

open(OUT, "> $hfile") or die "cannot create output file";
my $brick = sp_part($::current, 1, 0);
print OUT "// brick $brick, generated automatically\n$copyright";
print OUT "#ifndef __H_$brick\n";
print OUT "#define __H_$brick\n\n";
gen_magic(*OUT, $brick);
my $prefix = $brick;
$prefix =~ s/^([a-z]+).*/$1/;
gen_magic(*OUT, $prefix);
gen_header(*OUT);
print OUT "extern char attr_${brick}\[];\n";
print OUT "extern mand_t init_$brick (void * ini, const char * _param, mand_t _mand);\n";
print OUT "extern void exit_$brick (void * ini, const char * _param);\n";
print OUT "extern const struct loader loader_$brick;\n\n";
print OUT "extern const struct load_conn conns_$brick\[];\n\n";
print OUT "#endif\n";
close(OUT);

open(OUT, "> $cfile") or die "cannot create output file";
print OUT "// brick $brick, generated automatically\n$copyright";
print OUT "#define BASEFILE \"$infile\"\n";
print OUT "\n#undef OPERATION\n#define OPERATION \"preamble $brick\"\n";
print OUT "#include \"$brick.h\"\n";
$::static =~ s/\^//g;
$::static =~ s/(.*)\n/\"$1\\n\"\n/mg;
print OUT "\nchar attr_${brick}\[] =\n$::static;\n\n";
my $pre = $::pre_brick;
$pre = "// no predefinitions" unless defined($pre);
strip_braces(\$pre);
strip_brackets(\$pre);
purge(\$pre);
print OUT "$pre\n";
gen_ops(*OUT);
gen_init(*OUT);
print OUT "\n// Loader info for provisionary static linking with control_dummy_linux";
$::conn_init =~ s/\^//;
$::full =~ s/\^//;
print OUT "\nconst struct load_conn conns_$brick\[] = {\n$::conn_init};\n";
print OUT "\nconst struct loader loader_$brick = {\n  \"${brick}\",\n  MAGIC_$brick,\n  \"$::full\",\n  attr_$brick,\n  sizeof(struct brick_$brick),\n  $::conn_count,\n  $::conn_totalcount,\n  conns_$brick,\n  &init_static_$brick,\n  &exit_static_$brick,\n  &init_$brick,\n  &exit_$brick\n};\n\n";
close(OUT);

warn "please update the connector syntax for the following connectors: $::warn_syntax" if $::warn_syntax;

exit 0;
