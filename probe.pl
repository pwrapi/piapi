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
my $saddr;
my $output;

my $port;
my $logfile;
my $loglevel;

my @sensorstate;
my $samplefreq;

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
    getopts("h:p:f:l:s:qx", \%options);

    $saddr = "localhost";
    $port = 20202;
    $logfile = "/tmp/probe_" . $$ . ".log";
    $loglevel = 2;
    @sensorstate = qw/off off off off off off off off/;
    $samplefreq = 1;

    $saddr = $options{h} if defined $options{h};
    $output = $options{f} if defined $options{f};

    $samplefreq = $options{s} if defined $options{s};
    $port = $options{p} if defined $options{p};
    $loglevel = $options{l} if defined $options{l};
    undef $logfile if defined $options{q};

    if (defined $options{x}) {
        print "\n";
        print "    ./probe.pl [-h hostname] [-p localport]\n";
        print "                [-f filename] [-l loglevel] [-s samplerate] [-q]\n";
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
            if ($value == 8) {
                @sensorstate = qw/on on on on on on on on/;
                $self->log(4, "Sensor state for all are now on");
            } else {
                $sensorstate[$value] = "on";
                $self->log(4, "Sensor state for " . $value .
                    " is now " . $sensorstate[$value]);
            }
        } elsif ($command eq "stop") {
            if ($value == 8) {
                @sensorstate = qw/off off off off off off off off/;
                $self->log(4, "Sensor state for all are now off");
            } else {
                $sensorstate[$value] = "off";
                $self->log(4, "Sensor state for " . $value .
                    " is now " . $sensorstate[$value]);
            }
        } elsif ($command eq "freq") {
            $samplefreq = $value;
            $self->log(4, "Sample frequency is now " .  $samplefreq);
        }
        Probe->sample;
    }
}

=item sample

Attains a channel to communicate over and samples all ports that have
a state of on, retrieving their power value and timestamp, and sends
a message with this infomation along with its hostid and port number
over the channel (either socket of local file) on a period based on
the sample frequency.  Between samples, if a .config file exists,
the sample thread will refresh its sensor states and sample frequency
parameters.

=cut
sub sample {
    my $hostid = hostname;
    $hostid .= ":" . $$;

    while ($samplefreq > 0) {
        (my $seconds, my $microseconds) = Time::HiRes::gettimeofday;
        my @power = split /\n/, `./getRawPower8 1 2 3 4 5 6 7`;
        for (my $sensorport = 1; $sensorport < 8; $sensorport++) {
            if ($sensorstate[$sensorport] eq "on") {
                $power[$sensorport] =~ s/,/:/g;
                $power[$sensorport] =~ s/\t //g;

                print STDOUT $hostid . ":" . $seconds . ":" . $microseconds . ":" . $power[$sensorport] . "\n";
                STDOUT->flush;
            }
        }
        usleep(1000000/$samplefreq);
    }
}

