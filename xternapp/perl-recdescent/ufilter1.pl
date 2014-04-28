#!/usr/bin/perl -w 

use strict;

# input
my @inlines = <>;

my $state = 0;
for my $line (@inlines) {
  if ( $state == 0 ) {
    if ( $line =~ m/^\$1\s+=/ ) {
      $state = 1;
    }
  }
  if ( $state == 1 ) {
    if ( $line =~ m/Todo: inside _instance/ ) {
      $state = 2;
    } else {
      print $line;
    }
  }
}

exit 0

__END__

