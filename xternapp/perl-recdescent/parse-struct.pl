#!/usr/bin/perl -w 
#-I /home/to/Parser/parent/dir

use strict;
use Parse::RecDescent;

$::RD_HINT = 1;

    sub dbg1{printf("dbg1 %s\n", shift);}
    sub dbg2{printf("dbg2 %s %s\n", shift, substr(shift,0,10));}

#===========================================================
# grammar
my $gramm = q {

startrule: '$1' '=' block["top"]
    { print "$item[0] $item[3] \n"; }

block: '{' fields[$arg[0]] '}'
    { $return = $item[2]; }

fields: valuefields[$arg[0]] | arrayfields[$arg[0]]
    { print " fields: ", scalar(@{$item[1]}), " \n"; }

valuefields: valuefield[$arg[0]](s /,/)
    { { my @arrs = @{$item[1]};
        my $r = [];
        for(my $i=0; $i<=$#arrs; $i++) {
            my @a = @{$arrs[$i]};
            if ( $a[0] =~ m"field" ) {
                push @{$r}, ["field", 
                            sprintf("%s[%d].%s", $arg[0], $i, $a[1]), $a[2]];
                printf(" value %s[%d].%s\n", $arg[0], $i, $a[1]);
            }
        }
        $return = $r; }
    }

valuefield: identifier '=' value[$arg[0].".$item[1]"]
    { $return = ["field", "$arg[0].$item[1]", "$item[3]"]; }

arrayfields: arrayfield[""](s /,/)
    { { my @arrs = @{$item[1]};
        my $r = [];
        for(my $i=0; $i < scalar(@arrs); $i++) {
            my @a = @{$arrs[$i]};
            if ( $a[0] =~ m"scalar" ) {
                push @{$r}, ["field", "$arg[0].[$i]", $a[1].$a[2]];
                printf(" array %s[%d]\n", $arg[0], $i);
            } 
            if ( $a[0] =~ m"field" ) {
                push @{$r}, ["field", "$arg[0].[$i].$a[1]", $a[2]];
                printf(" array %s[%d].%s\n", $arg[0], $i, $a[1]);
            }
        }
        $return = $r; }
    }

arrayfield: value[$arg[0]]
    { $return = $item[1]; }

value: charvalue | datavalue | stringvalue | block | enumvalue
    { $return = $item[1]; }

datavalue: /[019]/
    { #print "datavalue: $item[1]\n"; 
      $return = ["scalar", "data", $item[1]]; }

charvalue: /0/ /\\'\\\\000\\'/
    { #print "charvalue: $item[1] $item[2]\n"; 
      $return = ["scalar", "char", "0"]; } 

stringvalue: stringvalue1 | stringvalue2
 
stringvalue1: /\\'\\\\000\\'/ /<repeats/ /[0-9]+/ /times>/
    { #print "stringvalue: \n"; 
      $return = ["scalar", "string", "str1"]; } 

stringvalue2: /\\"\\\\000[\\\\0]+\\"/
    { #print "stringvalue: \n"; 
      $return = ["scalar", "string", " str2 "]; } 

enumvalue: /\\w+::\\w+/
    { $return = ["scalar", "enum", $item[1]]; }

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


