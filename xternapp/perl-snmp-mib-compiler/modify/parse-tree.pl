#!/usr/bin/perl -w

use strict;

my $cfg_skip_oid_prefix = '1.3.6.1.2.1';

my $file = shift;

$file = 'IF-MIB-tree' if ( ! defined($file) || ! -f $file ); 
if ( ! -f $file ) {
    print "Error file $file\n";
    die "Error file $file\n";
}

my @lines = ();

my $rc = 0;
my $fh = undef;

$rc = open($fh, "<", $file);
if ( ! $rc ) {
    print "Error open file $file\n";
    die "Error open file $file\n";
    exit 0;
}
@lines = <$fh>;
close($fh);

#printf "OK read %d lines\n", scalar(@lines);

    sub parseblock {
        my $startline = shift;
        my $startlevel = shift;
        my $prefix = shift;

        my $blkref = [];
        my $parsedlines = 0;
        my $lastname = "";
        for ( my $i = $startline; $i <= $#lines; $i++) {
            my $theline = $lines[$i];
            chomp($theline);
            if ( $theline =~ m/^OK compile file \w+.*$/ ) {
                $parsedlines ++;
            } elsif ( $theline =~ m/^\s*\|\s*$/ ) {
                $parsedlines ++;
            } elsif ( $theline =~ m/^(\w+\S*)\s*$/ && $startlevel == 0 ) {
                $prefix = $1;
                $parsedlines ++;
            } elsif ( $theline !~ m/^\s*\+(\d+)--\s+P(\S*)\s(\s*.*)$/ ) {
                last;
            } else {
                my ($lvl, $perm, $an) = ($1, $2, $3);
                #printf("  theline %d %s\n", $lvl, $theline);
                #printf("\n  theline %d %s\n", $lvl, $an);
                if ( $lvl < $startlevel ) {
                    last;
                }
                if ( $lvl > $startlevel ) {
                    my ($result, $nl) = parseblock( $i, $lvl, 
                                                    $prefix.".".$lastname);
                    push @{$blkref}, ["blk", $i, $lvl, $result];
                    $i += $nl -1;
                    $parsedlines += $nl;
                    next;
                }
                my @elements = split(/\s+/, $an);
                if ( scalar(@elements) <= 0 ) {
                    print "Error at line $i\n";
                    die "Error at line $i\n";
                }
                my ($foundtype, $foundname, $foundoid) = ("", "", "");
                my $strsize=0;
                my ($tablerange, $tableindex) = ("", "");
                my $enumdef="";
                for (my $k=0; $k<= $#elements; $k++) {
                    #printf("        ==%s==\n", $elements[$k]);
                    my $e = $elements[$k];
                    next if ( $e =~ m/^\s*$/ ); #empty line
                    if ( $e =~ m/^\s*T(\w+)\s*$/ ) { $foundtype = $1; }
                    if ( $e =~ m/^\s*N(\w+)\(\d+\)\s*$/ ) { $foundname = $1; }
                    if ( $e =~ m/^\s*AOid\[([\.\d]+)\]\s*$/ ) {$foundoid = $1;}
                    if ( $e =~ m/^\s*AStringSize\[((\d+\-\d+)|(\w+))\]\s*$/ ) { 
                        $strsize = $1; 
                    }
                    if ( $e =~ m/^\s*ATableRange\[([\w\-,]+)\]\s*$/ ) { 
                        $tablerange = $1; 
                    }
                    if ( $e =~ m/^\s*ATableIndex\[([\w,]+)\]\s*$/ ) { 
                        $tableindex = $1; 
                    }
                    if ( $e =~ m/^\s*AEnum\[([\w\;\:]+)\]\s*$/ ) { 
                        $enumdef = $1; 
                    }
                    if ( $foundoid ) { $foundoid =~ s/$cfg_skip_oid_prefix//; }
                }
                #printf("        ==%s==%s==%s==%s==\n", $foundtype, $foundname, 
                #       $foundoid, $strsize);
                if ( $foundtype && $foundname && $foundoid ) {
                    #printf("                     tno tnos\n");
                    if ( $strsize ) {
                        push @{$blkref}, 
                            ["tnos", $i, $lvl, 
                             $foundtype, $prefix.".".$foundname, 
                                                        $foundoid, $strsize];
                    } elsif ( length($enumdef) > 0 ) {
                        push @{$blkref}, 
                            ["tnoe", $i, $lvl, 
                             $foundtype, $prefix.".".$foundname, 
                                                        $foundoid, $enumdef];
                    } else {
                        push @{$blkref}, 
                            ["tno", $i, $lvl, 
                             $foundtype, $prefix.".".$foundname, 
                                                                $foundoid];
                    }
                } elsif ($foundname && $foundoid && $tablerange) {
                    #printf("                     nor\n");
                    my @list = split(/,/, $tablerange);
                    if ( scalar(@list) ) { 
                        push @{$blkref}, ["nor", $i, $lvl, 
                                          $prefix.".".$foundname, $foundoid, 
                             "".scalar(@list)." ".$tablerange." ".$tableindex];
                    } else {
                        push @{$blkref}, ["norx", $i, $lvl, 
                                          $prefix.".".$foundname, $foundoid, 
                                             "x ".$tablerange." ".$tableindex];
                    }
                } elsif ($foundname) {
                    #printf("                     n\n");
                    push @{$blkref}, ["name", $i, $lvl, 
                                      $prefix.".".$foundname];
                }
                $lastname = $foundname if ( $foundname );
                $parsedlines ++;
            }
        }
        return ($blkref, $parsedlines);
    }

my $currentline = 0;
my $currLevel = 0;
my $currentPrefix = "";

my ($ret, $rel) = parseblock($currentline, $currLevel, $currentPrefix);

if ( $rel < scalar @lines ) { printf("\n\n lines consumed %d\n\n", $rel); }

    sub walktree {
        my $dat = shift;
        my @blk = @{$dat};
        for ( my $i=0; $i <= $#blk; $i++ ) {
            my @elm = @{$blk[$i]};
            if ( $elm[0] eq "name" ) {
                printf(" name %3d %3d  %-8s %s\n",$elm[1]+1,$elm[2],"",$elm[3]);
            } elsif ( $elm[0] eq 'nor' ) {
                printf(" nor %3d %3d  %-8s %-66s %-11s %s\n", 
                            $elm[1]+1, $elm[2], "", $elm[3], $elm[4], $elm[5]);
            } elsif ( $elm[0] eq 'norx' ) {
                printf(" norx %3d %3d  %-8s %-66s %-11s %s\n", 
                            $elm[1]+1, $elm[2], "", $elm[3], $elm[4], $elm[5]);
            } elsif ( $elm[0] eq 'tno' ) {
                printf(" tno  %3d %3d  %-8s %-66s %-11s\n", 
                                $elm[1]+1, $elm[2], $elm[3], $elm[4], $elm[5]);
            } elsif ( $elm[0] eq 'tnos' ) {
                printf(" tnos %3d %3d  %-8s %-66s %-11s %s\n", 
                       $elm[1]+1, $elm[2], $elm[3], $elm[4], $elm[5], $elm[6]);
            } elsif ( $elm[0] eq 'tnoe' ) {
                printf(" tnoe %3d %3d  %-8s %-66s %-11s %s\n", 
                       $elm[1]+1, $elm[2], $elm[3], $elm[4], $elm[5], $elm[6]);
            } elsif ( $elm[0] eq 'blk' ) {
                #printf(" --blk  %3d %3d  \n", $elm[1]+1, $elm[2]);
                walktree($elm[3]);
            } else {
                printf(" Error: Unknown element : %s\n", $elm[0]);
                die " Error: Unknown element\n";
            }
        }
    }

walktree($ret);

exit 0;

__END__

##########################################################################

