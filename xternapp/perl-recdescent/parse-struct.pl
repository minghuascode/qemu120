#!/usr/bin/perl -w 
#-I /home/to/Parser/parent/dir

use strict;
use Parse::RecDescent;

$::RD_HINT = 1;

#===========================================================
# grammar
my $gramm = q {

{
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
            print " ${$v}[0]  ${$v}[1]  ${$v}[2] \n";
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

startrule: '$1' '=' block["top"]
    { print "$item[0]:\n"; 
      my $v = $item[3];
      prta($v, " ", 1);
    }

block: '{' fields[$arg[0]] '}'
    { #print " block \n"; 
      my $v = $item[2];
      #prta($v, " ", 1);
      $return = $item[2]; }

fields: valuefields[$arg[0]] | arrayfields[$arg[0]]
    { 
        my $v = $item[1];
        my $r = [];
        if ( ref($v) ne "ARRAY" ) {
            print "Error: not array ref: prefix $arg[0]\n";
            die "Error\n";
        } else {
            #print "Ref: ", ref($v), "\n";
            #my @a = @{$v};
            #print " fields: ", scalar(@{$v}), " \n"; 
        }
        #prta($v, " ", 1);
        $return = $v;
    }

valuefields: valuefield[$arg[0]](s /,/)
    {
        my $v = $item[1];
        my $r = [];
        if ( ref($v) ne "ARRAY" ) {
            print "Error: not array ref: prefix $arg[0]\n";
            die "Error\n";
        } else {
            my @a = @{$v};
            for(my $i=0; $i<=$#a; $i++) {
                my @aa = @{$a[$i]};
                if ( $aa[0] eq "field" ) {
                    push @{$r}, ["field", sprintf("%s", $aa[1]), $aa[2]];
#                    printf(" value %s\n", $aa[1]);
                } elsif ( $aa[0] eq "block" ) {
                    my @b = @{$aa[1]};
                    for (my $j=0; $j < scalar(@b); $j++) {
                        my @bb = @{$b[$j]};
                        if ( $bb[0] eq "field" ) {
                            push @{$r}, ["field", "$bb[1]", $bb[2]];
#                            printf(" field %s\n", $bb[1]);
                        } else {
                            print "Error unknown $bb[0] in field block\n";
                            die "Error\n";
                        }
                    }
                }
            }
        }
        $return = ["block", $r]; 
    }

valuefield: identifier '=' value[$arg[0].".$item[1]"]
    {
        my $v = $item[3];
        if ( ref($v) ne "ARRAY" ) {
            print "Error: not array ref: prefix $arg[0]\n";
            die "Error\n";
        } else {
            my @a = @{$v};
            if ( $a[0] eq "scalar" ) {
                $return = ["field", "$arg[0].$item[1]", "$v"]; 
            } elsif ( $a[0] eq "block" ) {
                $return = $v;
            } else {
                print "Error unknown $a[0] in field\n";
                $return = ["field", "Error unknown $a[0] in field", ""];
            }
        }
    }

arrayfields: arrayfield[""](s /,/)
    {   
        my $v = $item[1];
        my $r = [];
        if ( ref($v) ne "ARRAY" ) {
            print "Error: not array ref: prefix $arg[0]\n";
            die "Error\n";
        } else {
          my @arrs = @{$item[1]};
          for(my $i=0; $i < scalar(@arrs); $i++) {
            my @a = @{$arrs[$i]};
            if ( $a[0] =~ m"scalar" ) {
                push @{$r}, ["field", "$arg[0].[$i]", $a[1].$a[2]];
 #               printf(" array %s[%d]\n", $arg[0], $i);
            } elsif ( $a[0] =~ m"field" ) {
                push @{$r}, ["field", "$arg[0]..[$i].$a[1]", $a[2]];
                printf(" array %s..[%d].%s\n", $arg[0], $i, $a[1]);
            } elsif ( $a[0] =~ m"block" ) {
                my @b = @{$a[1]};
                for (my $j=0; $j < scalar(@b); $j++) {
                    my @aa = @{$b[$j]};
                    if ( $aa[0] =~ m"field" ) {
                        push @{$r}, ["field", "$arg[0]...[$i].$aa[1]", $aa[2]];
                        printf(" array %s...[%d].%s\n", $arg[0], $i, $aa[1]);
                    } else {
                        print "Error unknown $aa[0] in array block\n";
                        die "Error\n";
                    }
                }
            } else {
                print "Error unknown $a[0] in array\n";
                die "Error\n";
            }
          }
        }
        $return = ["block", $r]; 
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


