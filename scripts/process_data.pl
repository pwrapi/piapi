#!/usr/bin/perl -w
#
use strict;
use Time::Local;

my $flist = shift or die "Must pass in file list to process ($!)\n";

open (FLFD, "<$flist") or die "Could not open $flist for reading ($!)\n";


sub convert_time {
    
    my ($dow, $mon, $dom, $time, $tz, $year) = split(/ /,shift);
    my %monhash =  (
        "Jan" => 0, "Feb" => 1, "Mar" => 2, "Apr" => 3, "May" => 4,
        "Jun" => 5, "Jul" => 6, "Aug" => 7, "Sep" => 8, "Oct" => 9,
        "Nov" => 10, "Dec" => 11,
    );
    $mon = $monhash{$mon};
    my($hours, $min, $sec) = split(/:/,$time);

    return timelocal($sec, $min, $hours, $dom, $mon, $year-1900);

}


while (<FLFD>) {
    chomp;
    open (IFD, "<$_") or do {
        printf "Could not open $_ for reading ($!)\n";
        next;
    };
    chomp;
    my $hostn;
    my $gflops;
    my $sepoch;
    my $fepoch;
    while (<IFD>) {
        /^HOSTNAME\:teller(\d{0,9}+)$/ and do {
            $hostn .= "teller-pm" .$1;
        };
        /^START\:(.*)$/ and do {
            $sepoch = $1;
        };
        /^WR0\w+\s+(.*)$/ and do {
            $gflops = (split(' ',$1))[5];
        };
        /^FIN\:(.*)$/ and do {
            $fepoch = $1;
        };
    }
    (!defined($hostn) or 
    !defined($sepoch) or 
    !defined($gflops) or 
    !defined($fepoch)) and next;

    my $stime = convert_time($sepoch);
    my $ftime = convert_time($fepoch);

    print "/Users/jhlaros/kratos/analyze.pl -l $stime -u $ftime -n $hostn -e -f \$1\n";
    print "#$hostn:$stime:$sepoch:$ftime:$fepoch:$gflops\n";
    close IFD;
}

