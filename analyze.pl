#!/usr/bin/perl -w

package Analyze;

use strict;
use Getopt::Std;

my $datafile;
my $nodename;
my $id;
my $lowerbound;
my $upperbound;
my $gpfile;
my $verbose;
my $screen;
my %raw;
my $energy;
my $timespan=0;

Analyze->config;

Analyze->load_data;
Analyze->save_data;

if (defined($energy)) {
    Analyze->energy;
} else {
    Analyze->stats;
}

Analyze->graph;

=item config

Gets command line options for filename, pid, and verbosity.
Defaults filename to power.dat, pid to 0 (indicating all),
and verbosity to off.

=cut
sub config {
    my %options=();
    getopts("f:n:i:l:u:g:esvx", \%options);

    $datafile = "power.dat";
    $id = 0;
    $nodename = "";
    $gpfile = "power.gp";
    $verbose = "no";
    $screen = "no";

    $datafile = $options{f} if defined $options{f};
    $nodename = $options{n} if defined $options{n};
    $id = $options{i} if defined $options{i};
    $lowerbound = 0;
    $upperbound = 999999999999;
    $lowerbound = $options{l} if defined $options{l};
    $upperbound = $options{u} if defined $options{u};
    $timespan = $upperbound - $lowerbound;
    $gpfile = $options{g} if defined $options{g};
    $verbose = "yes" if defined $options{v};
    $screen = "yes" if defined $options{s};
    $energy = "yes" if defined $options{e};

    if (defined $options{x}) {
        print "\n";
        print "    ./analyze.pl [-f filename]\n";
        print "                 [-i sampleid] [-n nodename]\n";
        print "                 [-g gpfile] [-s] [-v]\n";
        print "\n";
        print "    filename is the input data file (default power.dat)\n";
        print "    sampleid indicates the session (defaults to all)\n";
        print "    nodename indicates the particular node (defaults to all)\n";
        print "    the range of interest in time is given by lower and upper\n";
        print "    subsitute gnuplot by setting gpfile (default power.gp)\n";
        print "    graph output is to file unless -s (outputs to screen)\n";
        print "    verbose output is toggled by -v\n";
        print "\n";

        exit;
    }
}

sub load_data {
    open (my $data, "<", $datafile) or die "Could not open $datafile";

    my $header = <$data>;
    while(<$data>) {
        chomp($_);
        my @arg = split(':', $_);
        my $node = shift(@arg);
        my $pid = shift(@arg);
        !defined($node) and $node = "";

        if (($id == 0 or $pid == $id) and
            ($nodename eq "" or $nodename eq $node)) {
            my $sec = shift(@arg);
            my $usec = shift(@arg);
            my $port = shift(@arg);
            my $A = shift(@arg);
            my $V = shift(@arg);
            my $mA = shift(@arg);
            my $mV = shift(@arg);
            my $mW = shift(@arg);
    
            my $timestamp = ($sec+$usec/1000000); 
            if ($timestamp >= $lowerbound and $timestamp <= $upperbound) {
                $raw{$node}{$port}{"A"}{$timestamp} = ($mA/1000); 
                $raw{$node}{$port}{"V"}{$timestamp} = ($mV/1000); 
                $raw{$node}{$port}{"W"}{$timestamp} = ($mW/1000); 
    
                if ($verbose eq "yes") {
                    print " Node: " . $node . "\n";
                    print " Port: " . $port . "\n";
                    print " Time: " . $timestamp . "\n";
                    print " Amps: " . $raw{$node}{$port}{"A"}{$timestamp} . "\n";
                    print "Volts: " . $raw{$node}{$port}{"V"}{$timestamp} . "\n";
                    print "Watts: " . $raw{$node}{$port}{"W"}{$timestamp} . "\n";
                    print "\n";
                }
            }
        }
    }

    close $data;
}

sub save_data {
    foreach my $node (sort keys %raw) {
        foreach my $port (sort keys %{$raw{$node}}) {
            foreach my $type (sort keys %{$raw{$node}{$port}}) {
                open (my $data, ">", "results/" . $node . "." . $id . "_" . $port . "_" . $type . ".dat");
                foreach my $timestamp (sort keys %{$raw{$node}{$port}{$type}}) {
                    foreach my $val ($raw{$node}{$port}{$type}{$timestamp}) {
                        print $data $timestamp . " " . $val . "\n";
                    }
                }
                close $data;
            }
        }
    }
}

sub graph {
    foreach my $node (sort keys %raw) {
        foreach my $port (sort keys %{$raw{$node}}) {
            my $cmd = "gnuplot -e \"node=\\\"" . $node . "\\\"; port=" . $port . "; id=" . $id . "; ";
            $cmd .= "crt=\\\"" . $screen . "\\\";" if $screen eq "yes";
            $cmd .= "; load 'plots/" . $gpfile . "'\"";
            system($cmd);
        }
    }
}

sub stats {
    open (my $data, ">", "results/summary." . $id . ".dat");
    foreach my $node (sort keys %raw) {
        foreach my $port (sort keys %{$raw{$node}}) {
            print $data $node . " " . $port . " ";
            foreach my $type (sort keys %{$raw{$node}{$port}}) {
                my $sum = 0;
                my $count = 0;
                my $max = 0;
                my $min = 99999999;
                my $first = -1;
                my $last = -1;
                my $power = 0;

                foreach my $timestamp (sort keys %{$raw{$node}{$port}{$type}}) {
                    foreach my $val ($raw{$node}{$port}{$type}{$timestamp}) {
                        $sum = $sum + $val;
                        $count = $count + 1;
                        $min = $val if $val < $min;
                        $max = $val if $val > $max;
                        if ($type eq "W") {
                            if ($first != -1) {
                                $power += 2 * $val;
                                $last = $timestamp;
                            } else {
                                $power += $val;
                                $first = $timestamp;
                            }
                        }
                    }
                }

                print $data $sum/$count . " " . $min . " " . $max . " ";

                if ($type eq "W") {
                    $power = ($last - $first) / (2 * $count) * $power;
                    print $data ($last - $first) . " " . $first . " " . $last . " " . $power;
                }
            }
            print $data "\n";
        }
    }
    close $data;
}

sub energy {
    foreach my $node (sort keys %raw) {
        print "\n$node:$timespan:";
        foreach my $port (sort keys %{$raw{$node}}) {
            print "P$port = ";

            foreach my $type (sort keys %{$raw{$node}{$port}}) {
                my $sum = 0;
                my $count = 0;
                my $max = 0;
                my $min = 99999999;
                my $first = -1;
                my $last = -1;
                my $power = 0;

                foreach my $timestamp (sort keys %{$raw{$node}{$port}{$type}}) {
                    foreach my $val ($raw{$node}{$port}{$type}{$timestamp}) {
                        $sum = $sum + $val;
                        $count = $count + 1;
                        $min = $val if $val < $min;
                        $max = $val if $val > $max;
                        if ($type eq "W") {
                            if ($first != -1) {
                                $power += 2 * $val;
                                $last = $timestamp;
                            } else {
                                $power += $val;
                                $first = $timestamp;
                            }
                        }
                    }
                }


                if ($type eq "W") {
                    $power = ($last - $first) / (2 * $count) * $power;
                    print "$power:";
                }
            }
        }
    }
}
