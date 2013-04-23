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
my $dat;
my $plot;
my $timespan=0;

Analyze->config;

Analyze->load_data;

if (defined($dat)) {
    Analyze->save_data;
}

if (defined($energy)) {
    Analyze->energy;
} else {
    Analyze->stats;
}

if (defined($plot)) {
    my $sys_status = system("which gnuplot");
    if ($sys_status == 0) {
        Analyze->graph;
    } else {
        print "Skipping graphing step, gnuplot not found\n";
    }
}

=item config

Gets command line options for filename, pid, and verbosity.
Defaults filename to power.dat, pid to 0 (indicating all),
and verbosity to off.

=cut
sub config {
    my %options=();
    getopts("f:n:i:l:u:g:dpesvx", \%options);

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
    $dat = "yes" if defined $options{d};
    $plot = "yes" if defined $options{p};
    $energy = "yes" if defined $options{e};

    if (defined $options{x}) {
        print "\n";
        print "    ./analyze.pl [-f filename]\n";
        print "                 [-i sampleid] [-n nodename]\n";
        print "                 [-g gpfile] [-d] [-p] [-s] [-v]\n";
        print "\n";
        print "    filename is the input data file (default power.dat)\n";
        print "    sampleid indicates the session (defaults to all)\n";
        print "    nodename indicates the particular node (defaults to all)\n";
        print "    the range of interest in time is given by lower and upper\n";
        print "    subsitute gnuplot by setting gpfile (default power.gp)\n";
        print "    creates individual detail dat files with -d (default off)\n";
        print "    creates individual detail plot files with -p (default off)\n";
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
        my ($node, $pid, $sec, $usec, $port, $A, $V, $mA, $mV, $mW)  = split(':', $_);
        !defined($node) and $node = "";

        if (($id == 0 or $pid == $id) and
            ($nodename eq "" or $nodename eq $node)) {
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
            } else {
		last if($timestamp > $upperbound + 100);
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
    open (my $data, ">>", "results/summary." . $id . ".dat");
    print $data "# HOST PORT A(avg) A(min) A(max) V(avg) V(min) V(max) W(avg) W(min) W(max) t(tot) t(min) t(max) Power(joules)\n";
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
