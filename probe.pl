#!/usr/bin/perl -w

package Probe;

use strict;

use Getopt::Std;
use Sys::Hostname;
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
        my $value = shift(@arg);
        $self->log(3, "Received control command " . $command . ":" . $value);
        if ($command eq "start") {
            $self->log(4, "Starting collection on port " .  $value);

            my $samplecount = 0;
            my $hostid = hostname;
            $hostid .= ":" . $$;

            while ($samplefreq > 0 and $samplecount < 100) {
                (my $seconds, my $microseconds) = Time::HiRes::gettimeofday;
                my @power = split /\n/, `./getRawPower8 1 2 3 4 5 6 7`;
                $power[$value] =~ s/,/:/g;
                $power[$value] =~ s/\t //g;

                $self->log(4, "Sending sample " . $samplecount);
                print STDOUT $hostid . ":" . $seconds . ":" . $microseconds . ":" . $power[$value] . "\n";
		STDOUT->flush;

                $samplecount++;
                usleep(1000000/$samplefreq);
            }
        }
    }
}

