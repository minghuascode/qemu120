#!/usr/bin/perl -w
# calcmort.pl: calculate mortgage 

use strict;

    my $apr = 0.027;
    my $pmt = 457.99;
    my $periods = 25*12;
    my $rv = 0;
    my $ipay = 0;

    # no adj, mfv=1.94653, pfv 195040, pmt 457.99, rv -386.95 
    # adj -386.95/periods/1.95040 -.66, pmt 457.33, rv -105.92
    # adj * (386.95-105.37)/105.37, pmt 457.08, rv .50
    # adj +.01, pmt 457.09, rv -3.90
    my $adj = 0;
    $adj = -386.95/$periods/1.95040;
    $adj *= (1 + 105.37/(386.95-105.37));
    $adj += .01;
    $pmt = $pmt  + $adj; 
    $pmt = sprintf("%.2f", $pmt);

    $rv = calc( 100 * 1000, $apr, $pmt, 0, 500); 

    #amortization pmt:
    my $pm = armpmt(100*1000, $apr, $periods);
    printf("\n amortization pmt %.2f\n", $pm);

    my $loan = armloan(457.08, $apr, $periods);
    printf("\n amortization loan %.2f\n", $loan);

  sub armloan {
    my ($pmt, $apr, $n) = @_;
    my $r = apr2mon($apr);
    my $s = exp( $n * log( 1/(1 + $r)) );
    my $loan = $pmt * (1-$s) / $r;
    return $loan;
  }

  sub armpmt {
    my ($p, $apr, $n) = @_;
    my $r = apr2mon($apr);
    #my $r = $apr/12;
    my $s = exp( $n * log(1 + $r) );
    my $a = $p * $r * $s / ($s - 1);
    return $a;
  }

  sub calc {
    my ($mort, $apr, $pmt, $fee, $sqft) = @_;
    my $r = apr2mon($apr);
    my $b = apr2mon($apr);

    my $mfv = $mort * exp( $periods * log(1 + $b) );
    printf(" mort $mort mfv %.f \n", $mfv);

    #my $ifv = $ipay * exp( $periods * log(1 + $b) );
    #printf(" ifv %.f \n", $ifv);

    my $pv = $mort;
    my $iv = $pv;
    my $ipay = $ipay;
    $pv = $pv - $ipay;

    my $pfv = $ipay * exp( $periods * log(1 + $r) );

    for (my $k=0; $k<$periods; $k++) {
      $pfv = $pfv + $pmt * exp( ($periods - $k -1) * log(1 + $b) );
      my $i = sprintf("%.2f", $pv * $r);
      my $p = $pmt - $i - $fee;
      my $newp = $pv - $p;
      printf("  iv %.2f pv %7.2f  p %.2f i %.2f f %.2f  pv %7.2f ", 
               $iv, $pv, $p, $i, $fee, $newp); #if ( $k == 0 );
      printf("\n");
      $pv = $newp;
    }
    printf(" delta %.f  sf %.f %.f  pv %.f ".
             "sf %d  pfv %.2f mfv %.2f  pmt %f adj %f\n", 
            $iv - $pv, ($pmt * $periods)/($sqft), $pfv/$sqft, $pv, 
            $sqft, $pfv, $mfv, $pmt, $adj);
    return $pv;
  }

exit 0;

  sub apr2day {
    my $apr = shift;
    #  (1+r) ** 356 = (1+apr)
    my $rday = exp( log(1 + $apr) / 365 ) - 1;
    return $rday;
  }
  sub apr2mon {
    my $apr = shift;
    #  (1 + r) ** 365 = 1 + apr
    #  365 log (1 + r) = log (1 + apr)
    #  log (1+r) = log (1+apr) / 365
    #  (1+r) = exp ( log (1+apr) / 365 )
    my $rday = exp( log(1 + $apr) / 365 ) - 1;
    my $md = 365/12;
    my $rmon = exp( $md * log(1 + $rday) ) - 1;
    return $rmon;
  }

__END__


