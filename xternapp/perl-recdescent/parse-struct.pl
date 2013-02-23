#!/usr/bin/perl -w 
#-I /home/to/Parser/parent/dir

use strict;
use Parse::RecDescent;

$::RD_HINT = 1;

#===========================================================
# grammar
my $gramm = q {

{   sub err{ print "Error @_"; die "Error\n"; }
    sub dbg{ printf(@_) if ( 1 ); }
    sub dbg1{printf("dbg1 %s\n", shift);}
    sub dbg2{printf("dbg2 %s %s\n", shift, substr(shift,0,10));}
    sub prta {
        my ($v, $prefix, $depth) = @_;
        if ( ! defined($v) ) {
            print "Error prta: no argument\n";
            return;
        }
        my $t = ref($v);
        if ( $t eq "SCALAR" ) {
            printf("--SCALAR: ");
            for (my $k=0; $k< 4*$depth; $k++) { print " "; }
            print " ${$v}\n";
        } elsif ( $t eq "ARRAY" && 
                  scalar(@{$v}) == 3 && 
                  defined(ref(${$v}[0])) && length(ref(${$v}[0])) == 0 &&
                  defined(ref(${$v}[1])) && length(ref(${$v}[1])) == 0 &&
                  ${$v}[0] eq "field" ) {
            printf("--field:  ");
            for (my $k=0; $k< 4*$depth; $k++) { print " "; }
            my $tail = "${$v}[2]";
            $tail =~ s/ARRAY\([0-9a-fx]+\)/ARRAYtail/;
            print " ${$v}[0]  ${$v}[1]  $tail \n";
        } elsif ( $t eq "ARRAY" ) {
            printf("--ARRAY+: ");
            for (my $k=0; $k< 4*$depth; $k++) { print " "; }
            print " $t\n";
            my @a = @{$v};
            my $len = scalar(@a);
            for (my $k=0; $k < $len; $k++) {
                prta( $a[$k], " ", $depth + 1 );
            }
            printf("--ARRAY-: \n");
        } else {
            printf("--Type %s\n", $v);
        }
    }
}

startrule: '$1' '=' block["top", $item[0]]
    { print "$item[0]:\n"; 
      my $v = $item[3];
      prta($v, " ", 1);
    }

block: '{' fields[$arg[0], "$arg[1].$item[0]"] '}'
    { #print " block \n"; 
      my $v = $item[2];
      $return = $v; }

fields: valuescope[$arg[0], "$arg[1].$item[0]"] 
      | arrayscope[$arg[0], "$arg[1].$item[0]"]
    { 
        my ($v, $r) = ($item[1], []);
        if ( ref($v) ne "ARRAY" ) {
            err " fields: not array ref: prefix $arg[0]\n";
        }
        $return = $v;
    }

valuescope: valuefield[$arg[0], "$arg[1].$item[0]"](s /,/)
    {
        my ($v, $r) = ($item[1], []);
        if ( ref($v) ne "ARRAY" ) {
            err " valuescope: not array ref: prefix $arg[0]\n";
        }
        my @a = @{$v};
        for(my $i=0; $i<=$#a; $i++) {
            my @aa = @{$a[$i]};
            if ( $aa[0] ne "field" && $aa[0] ne "block" ) {
                err " valuescope: unknown aa0 $aa[0]\n";
            }
            if ( $aa[0] eq "field" ) {
                push @{$r}, ["field", sprintf("%s", $aa[1]), $aa[2]];
#                dbg(" value %s\n", $aa[1]);
            } else { # "block"
                my @b = @{$aa[1]};
                for (my $j=0; $j < scalar(@b); $j++) {
                    my @bb = @{$b[$j]};
                    if ( $bb[0] ne "field" ) {
                        err " valuescope block: unknown bb0 $bb[0]\n";
                    }
                    push @{$r}, ["field", "$bb[1]", $bb[2]];
#                    dbg(" valuescope block field: %s %s\n", $arg[0], $bb[1]);
                }
            }
        }
        $return = ["block", $r]; 
    }

valuefield: identifier '=' value[$arg[0].".$item[1]", "$arg[1].$item[0]]"]
    { #print " valuefield: $arg[0].$item[1] $arg[1]\n";
        my $v = $item[3];
        if ( ref($v) ne "ARRAY" ) {
            err " valuefield: not array ref: prefix $arg[0]\n";
        }
        my @a = @{$v};
        if ( $a[0] eq "scalar" ) {
            $return = ["field", "$arg[0].$item[1]", "$v"]; 
        } elsif ( $a[0] eq "block" ) {
            $return = $v;
        } else {
            err " valuefield: unknown a0 $a[0]\n";
        }
    }

arrayscope: value[$arg[0], "$arg[1].$item[0]"](s /,/)
    {   
        my ($v, $r) = ($item[1], []);
        if ( ref($v) ne "ARRAY" ) {
            err " arrayscope: not array ref: prefix $arg[0]\n";
        }
        my @arrs = @{$v};
        for(my $i=0; $i < scalar(@arrs); $i++) {
            my @a = @{$arrs[$i]};
            if ( $a[0] ne "scalar" && $a[0] ne "block" ) {
                err " arrayscope: unknown a0 $a[0]\n";
            }
            if ( $a[0] eq "scalar" ) {
                push @{$r}, ["field", "$arg[0]"."[$i]", $a[1].$a[2]];
#                dbg(" array %s[%d]\n", $arg[0], $i);
            } else { # "block"
                my @b = @{$a[1]};
                for (my $j=0; $j < scalar(@b); $j++) {
                    my @aa = @{$b[$j]};
                    if ( $aa[0] ne "field" ) {
                        err " arrayscope block: unknown aa0 $aa[0]\n";
                    }
                    #when pasing down "" instead of $arg[0], use the two lines.
                    #push @{$r}, ["field", "$arg[0]...[$i].$aa[1]", $aa[2]];
                    #printf(" array %s...[%d].%s\n", $arg[0], $i, $aa[1]);

                    my ($n, $m) = ($arg[0], $aa[1]);
                    my $len1 = length($n);
                    my $s = substr($m, 0, $len1);
                    if ( $s ne $n ) {
                        err " arrayscope: arg0 work-around fixed.\n";
                    }
                    my $u = substr($m, $len1);
                    push @{$r}, ["field", "$s"."[$i]"."$u", $aa[2]];
#                    dbg(" array %s...[%d].%s\n", $s, $i, $u);
                }
            }
        } #for
        $return = ["block", $r]; 
    }

value: charvalue | datavalue | stringvalue 
     | block[$arg[0], "$arg[1].$item[0]"] | enumvalue
    { $return = $item[1]; }

datavalue: /[019]/
    { #print "datavalue: $item[1]\n"; 
      $return = ["scalar", "data", $item[1]]; }

charvalue: /0/ /\\'\\\\000\\'/
    { #print "charvalue: $item[1] $item[2]\n"; 
      $return = ["scalar", "char", "0"]; } 

stringvalue: stringvalue1 | stringvalue2
 
stringvalue1: /\\'\\\\000\\'/ /<repeats/ /[0-9]+/ /times>/
    { #print "stringvalue1: $item[1]\n"; 
      $return = ["scalar", "string", "str1"]; } 

stringvalue2: /\\"\\\\000[\\\\0]+\\"/
    { #print "stringvalue2: $item[1]\n"; 
      $return = ["scalar", "string", " str2 "]; } 

enumvalue: /\\w+::\\w+/
    { #print "enumvalue: $item[1]\n";
      $return = ["scalar", "enum", $item[1]]; }

identifier: /[a-zA-Z_]\\w*/
    { #print "identifier $item[1]\n"; 
      $return = $item[1]; }

};

#===========================================================
# parser
my $parser = new Parse::RecDescent($gramm);
if ( ! defined($parser) ) {
    print "Error creating parser\n";
    die "Error creating parser\n";
}

#===========================================================
# input
my @inlines = <>;
printf("Inputed %d lines.\n", scalar(@inlines));
my $text = "@inlines";
my $rc = $parser->startrule($text);

#===========================================================
# output


exit 0;


