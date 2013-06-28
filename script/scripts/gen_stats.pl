#!/usr/bin/perl -w
#
use strict;

use Statistics::Basic qw(:all);

my $input = shift or die "Must provide input file\n";

open (IFD, "<$input") or die "Could not open $input for reading ($!)\n";

my $hostn;
my $seconds;
my %phash;
my $phref = \%phash;
my @parray;
my $port;
my $val;
while (<IFD>) {
    /^(teller-pm\d\d):(\d+):(P\d\s=\s.*)$/ and do {
        $hostn = $1;
        $seconds = $2;
        #
        # Try to deal with variable number of ports and data
        # UP to 7 (P1-P7)
        # $#parray+1 is the number of ports/data that we have
        #
        @parray = split(/:/,$3);
        for my $item (0..$#parray) {
            ($port,$val) = split( / = /,$parray[$item]);
            #print "port \"$port\" val $val\n";
            $phref->{$hostn}->{'time'} = $seconds;
            $phref->{$hostn}->{$port} = $val;
        }
    };
}

#
# Generate an array of energy values for each port along with 
# the statistics. 
#

my @energy;
my %ehash;
my $ehref = \%ehash;
for my $key (keys(%phash)) {
    # key is the hostname we are processing, not important other than
    # we need it to drill deeper
    for my $port qw/P1 P2 P3 P4 P5 P6 P7/ {
        defined($port) and do {
            push @{$ehref->{$port}}, $phref->{$key}{$port};
        };
    }
}

#
# We now have energy values by port we can generate per port statistics
#

for my $port qw/P1 P2 P3 P4 P5 P6 P7/ {
    @{$ehref->{$port}} = sort { $a <=> $b } @{$ehref->{$port}};
    my $mean = mean(\@{$ehref->{$port}});
    my $std_dev = stddev(\@{$ehref->{$port}});
    my $cv = 100 * $std_dev/$mean;
    print "Port $port MEAN:\t$mean\n" .
          "\tSTDDEV:\t$std_dev\n" .
          "\tCV:\t$cv\n".
          "\tMIN:\t$ehref->{$port}[0]\n" .
          "\tMAX:\t$ehref->{$port}[$#{$ehref->{$port}}]\n\n";

}
