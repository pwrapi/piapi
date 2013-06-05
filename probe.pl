#!/usr/bin/perl -w

package Probe;

use strict;

use Getopt::Std;
use IO::Socket::INET;
use Time::HiRes qw(usleep nanosleep);
use base qw(Net::Server::PreFork);

=item main

Main gathers setup information through config, forks the
sample process, and starts the server for incoming control
commands.

=cut
my $samplefreq;
my $port;
my $logfile;
my $loglevel;
my %energy;

Probe->config;
Probe->run(
    port => $port,
    log_file => $logfile,
    log_level => $loglevel
);

=item config

Gets command line options for loglevel.  Defaults log level
to 2, starts all sensor state as off, and sample frequency
at 1 Hz.  Also collects args for sample.pl.

=cut
sub config {
    my %options=();
    getopts("p:l:s:qx", \%options);

    $port = 20202;
    $logfile = "/tmp/probe_" . $$ . ".log";
    $loglevel = 2;
    $samplefreq = 1;

    $samplefreq = $options{s} if defined $options{s};
    $port = $options{p} if defined $options{p};
    $loglevel = $options{l} if defined $options{l};
    undef $logfile if defined $options{q};

    if (defined $options{x}) {
        print "\n";
        print "    ./probe.pl [-p port]\n";
        print "               [-l loglevel] [-s samplerate] [-q]\n";
        print "\n";
        print "    loglevel can be from 1 to 4 (default 2)\n";
        print "    defaults log to /tmp or -q to console\n";
        print "\n";

        exit;
    }
}

=item process_request

Overrides base function for processing request on control connection.
STDIN is redirected from socket and messages are parsed and processed
to configure sensor states and sample frequency.  Configuration data
is saved to file which sample thread will pick up (due to buggy
shared variables in perl threads).

=cut
sub process_request {
    my $self = shift;

    while(<STDIN>) {
        chomp($_);
        my @arg = split(':', $_);
        my $command = shift(@arg);
        my $sensorport = shift(@arg);
        my $samples = shift(@arg);
        $self->log(3, "Received control command " . $command . ":" . $sensorport . ":" . $samples);
        if ($command eq "collect") {
            $self->log(4, "Starting collection of " . $samples . " on port " .  $sensorport);

            my $samplecount = 0;
            while ($samplefreq > 0 and $samplecount < $samples) {
                (my $sec, my $usec) = Time::HiRes::gettimeofday;
                my @power = split /\n/, `./getRawPower8 1 2 3 4 5 6 7`;
                my $timestamp = ($sec+$usec/1000000); 
                $power[$sensorport] =~ s/,/:/g;
                $power[$sensorport] =~ s/\t //g;

                $energy{$timestamp} = $power[$sensorport];
                $self->log(4, "Sending sample " . $samplecount . " of " . $samples);
                print STDOUT $samplecount . ":" . $samples . ":" . Probe->totalenergy . "\n";
		STDOUT->flush;

                $samplecount++;
                usleep(1000000/$samplefreq);
            }
        }
    }
}

sub totalenergy {
    my $count = 0;
    my $first = -1;
    my $last = -1;
    my $power = 0;

    foreach my $timestamp (sort keys %energy) {
        foreach my $val ($energy{$timestamp}) {
            $count = $count + 1;
            if ($first != -1) {
                $power += 2 * $val;
                $last = $timestamp;
            } else {
                $power += $val;
                $first = $timestamp;
            }
        }
    }

    $power = ($last - $first) / (2 * $count) * $power;
    return $power;
}
