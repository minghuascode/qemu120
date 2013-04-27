#!/usr/bin/perl -w

BEGIN { unshift @INC, "libpm" }

use strict;
use SNMP::MIB::Compiler;
use Data::Dumper;

my $DATE = '2013/04/22';

my $outdir = 'out.dir';
my $file   = shift;

$Data::Dumper::Purity = 1;
$Data::Dumper::Indent = 1;
$Data::Dumper::Terse  = 1;

my $mib = new SNMP::MIB::Compiler;
$mib->add_path('.', 'libmibs', 'mibs/cisco', 'mibs/com21',
	       '/home/ftp/doc/mibs/ascend');
$mib->add_extension('', '.mib', '.my');

mkdir $outdir, oct 755 unless -d $outdir;
$mib->repository($outdir);

$mib->{'accept_smiv1'} = 1;
$mib->{'accept_smiv2'} = 1;

$mib->{'debug_recursive'} = 1;
$mib->{'debug_lexer'}     = 0;

$mib->{'make_dump'}  = 1;
$mib->{'use_dump'}   = 0;
$mib->{'do_imports'} = 1;

#$mib->load($file) || $mib->compile($file) ||
#  print scalar $mib->assert if $file;

#exit if $file;

$file = 'IF-MIB' if ( ! defined($file) || ! -f $file ); #default file IF-MIB
die 'Error file $file\n' if ( ! -f $file );

my $rc = $mib->compile($file);
if ( $rc ) {
    my $fh = undef;
    open $fh, ">", $file."-tree" || die "Error output file $file"."-tree";
    print $fh "OK compile file $file\n";
    print $fh $mib->tree('ifMIB');
    print $fh $mib->tree('interfaces');
    close $fh;
}

die 'Error compile file $file\n' if ( ! $rc );
exit 0;

