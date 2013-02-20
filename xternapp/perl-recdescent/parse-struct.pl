#!/usr/bin/perl -w 
#-I /home/to/Parser/parent/dir

use strict;
use Parse::RecDescent;

    sub dbg1{printf("dbg1 %s\n", shift);}
    sub dbg2{printf("dbg2 %s %s\n", shift, substr(shift,0,10));}

#===========================================================
# grammar
my $gramm = q {

startrule: '$1' '=' block["top"]
    { print "$item[0] $item[3] \n"; }

block: '{' fields[$arg[0]] '}'
    { $return = " block $item[2] "; }

fields: singlefield[$arg[0]] secondfield[$arg[0]](s?)

secondfield: ',' singlefield[$arg[0]]

singlefield: arrayfields[$arg[0]] | valuefield[$arg[0]]
    { print " field: $item[1] \n"; }

valuefield: identifier '=' value[$arg[0].".$item[1]"]
    { $return = " identifier = $arg[0].$item[1] $item[3] "; }

arrayfields: <rulevar: local $count = 0>
arrayfields: arrayfield[$arg[0]](s)

arrayfield: value[$arg[0].".[".$count."]"]
    { $return = "array = $arg[0].[".$count."] "; 
      $thisrule->{"local"}{"count"}++; }

value: charvalue | datavalue | stringvalue | block | enumvalue
    { $return = $item[1]; }

datavalue: /[019]/
    { #print "datavalue: $item[1]\n"; 
      $return = " datavalue $item[1] "; }

charvalue: /0/ /\\'\\\\000\\'/
    { #print "charvalue: $item[1] $item[2]\n"; 
      $return = " charvalue 0 "; } 

stringvalue: stringvalue1 | stringvalue2
 
stringvalue1: /\\'\\\\000\\'/ /<repeats/ /[0-9]+/ /times>/
    { #print "stringvalue: \n"; 
      $return = " str1 "; } 

stringvalue2: /\\"\\\\000[\\\\0]+\\"/
    { #print "stringvalue: \n"; 
      $return = " str2 "; } 

enumvalue: /\\w+::\\w+/

identifier: /[a-zA-Z_]\\w*/
    { #print "identifier $item[1]\n"; 
      $return = $item[1]; }

};

#===========================================================
# parser
my $parser = new Parse::RecDescent($gramm);
if ( ! defined($parser) ) {
    die "Error creating parser\n";
}

#===========================================================
# input

my @inlines = <>;
my $text = "@inlines";
my $rc = $parser->startrule($text);

#===========================================================
# output


exit 0;


