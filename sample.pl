#!/usr/bin/perl -w

package Sample;

use strict;

use Getopt::Std;
use Sys::Hostname;
use IO::Socket::INET;
use Time::HiRes qw(usleep nanosleep);

=item main

Main sets up all variables to be shared between threads,
gathers setup information through config, spawns the
sample thread, and starts the server for incoming control
commands.

=cut
my $saddr;
my $port;
my $output;
my $samplefreq;
my @sensorstate;

Sample->config;
Sample->sample;

=item config

Gets command line options for host server, output file,
and sample frequency.  Defaults host to localhost, output
file to blank, and sample frequency to 1 Hz.

=cut
sub config {
    my %options=();
    getopts("h:p:f:x", \%options);

    $saddr = "localhost";
    $port = "20203";
    $samplefreq = 1;
    @sensorstate = qw/off off off off off off off off/;

    $saddr = $options{h} if defined $options{h};
    $port = $options{p} if defined $options{p};
    $output = $options{f} if defined $options{f};

    if (defined $options{x}) {
        print "\n";
        print "    ./sample.pl [-h hostname] [-p port]\n";
        print "                [-f filename]\n";
        print "\n";
        print "    logs samples to hostname and port unless a\n";
        print "    filename is provided in which case logs to file\n";
        print "\n";

        exit;
    }
}

=item load_config

Opens a hidden file .config to refresh sensor states and
sample frequency parameters for processes and removes
the file once read.

=cut
sub load_config {
    open(IN, "<", "/tmp/.config" . $$);

    for (my $sensorport = 0; $sensorport < 8; $sensorport++) {
        chomp($sensorstate[$sensorport] = <IN>);
    }
    chomp($samplefreq = <IN>);

    close(IN);

    unlink("/tmp/.config" . $$) or warn "Could not unlink .config";

    print "Restoring configuration...\n";
    for (my $i = 0; $i < 8; $i++) {
        print "sensorstate[" . $i . "]  = " . $sensorstate[$i] . "\n";
    }
    print "samplefreq = " . $samplefreq . "\n";
}

=item channel

Opens a channel for communicating over.  If an output filename is
defined then output will be to this file, otherwise a socket
connection is established to the host server.

=cut
sub channel {
    my $self = shift;
    my $sock;

    if ($output) {
        open($sock, ">" . "$output");
    } else {
        $sock = IO::Socket::INET->new(
            PeerAddr => $saddr,
            PeerPort => $port,
            Proto    => 'tcp'
        );
    }
    $sock or die "Can't connect\n";

    return $sock;
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
    my $sock = Sample->channel();
    my $hostid = hostname;
    $hostid .= ":" . $$;

    while ($samplefreq >= 0) {
        (my $seconds, my $microseconds) = Time::HiRes::gettimeofday;
        my @power = split /\n/, `./getRawPower8 1 2 3 4 5 6 7`;
        for (my $sensorport = 1; $sensorport < 8; $sensorport++) {
            if ($sensorstate[$sensorport] eq "on") {
                $power[$sensorport] =~ s/,/:/g;
                $power[$sensorport] =~ s/\t //g;

                print $sock $hostid . ":" . $seconds . ":" . $microseconds . ":" . $power[$sensorport] . "\n";
                $sock->flush;
            }
        }
        usleep(1000000/$samplefreq) if $samplefreq;
        Sample->load_config if (-e "/tmp/.config" . $$);
    }

    close($sock);
}
