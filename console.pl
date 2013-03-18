#!/usr/bin/perl -w

package Console;

use strict;
use IO::Handle;
use Sys::Hostname;
use Time::HiRes qw(usleep nanosleep);

=item main

=cut
my $port;
my @sensorstate = qw/off off off off off off off off/;
my $samplefreq = 1;

Console->open();

#Console->read();
Console->sample();
Console->write();

sub open {
  open $port, '+<', '/dev/ttyO5' or die $!;
}

=item process_request

Overrides base function for processing request on control connection.
STDIN is redirected from socket and messages are parsed and processed
to configure sensor states and sample frequency.  Configuration data
is saved to file which sample thread will pick up (due to buggy
shared variables in perl threads).

=cut
sub read {
    eval {
        while(<$port>) {
            chomp($_);
            my @arg = split(':', $_);
            my $command = shift(@arg);
            my $value = shift(@arg);
            if ($command eq "start") {
                if ($value == 8) {
                    @sensorstate = qw/on on on on on on on on/;
                } else {
                    $sensorstate[$value] = "on";
                }
            } elsif ($command eq "stop") {
                if ($value == 8) {
                    @sensorstate = qw/off off off off off off off off/;
                } else {
                    $sensorstate[$value] = "off";
                }
            } elsif ($command eq "freq") {
                $samplefreq = $value;
            }
        }
    };
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
                chomp($power[$sensorport]);
                $power[$sensorport] =~ s/,/:/g;
                $power[$sensorport] =~ s/\t //g;

                print $port $hostid . ":" . $seconds . ":" . $microseconds . ":" . $power[$sensorport] . "\n";
            }
        }
        usleep(1000000/$samplefreq);
    }
}
