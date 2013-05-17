#!/usr/bin/perl -w

package Control;

use strict;
use Getopt::Std;
use IO::Socket::INET;

my $host;
my $port;
my $mode;
my $command;
my $value;

Control->config();

Control->send() if ($command && $value);
Control->test() if ($mode);

=item config

Gets command line options for control host server and either switches
mode on to run a test suite or sets up an individual command
and its associated value.  Defaults host to localhost and mode
to off.

=cut
sub config {
    my %options=();
    getopts("h:p:s:u:d:c:v:tx", \%options);

    $host = "localhost";
    $port = 20202;
    $mode = 0;

    $host = $options{h} if defined $options{h};
    $port = $options{p} if defined $options{p};
    $command = $options{c} if defined $options{c};
    $value = $options{v} if defined $options{v};
    $mode = 1 if defined $options{t};

    if (defined $options{u}) {
        $command = "start";
        $value = $options{u};
    }
    if (defined $options{d}) {
        $command = "stop";
        $value = $options{d};
    }
    if (defined $options{s}) {
        $command = "freq";
        $value = $options{s};
    }

    if (defined $options{x}) {
        print "\n";
        print "    ./control.pl [-h hostname] [-p port]\n";
        print "                 [-s samplerate]\n";
        print "                 [-u sensorport] [-d sensorport]\n";
        print "                 [-c command] [-v value] [-t]\n";
        print "\n";
        print "    sensorport value should be 1 to 7 or 8 (all sensor ports)\n";
        print "    command = { start | stop | freq } and value = { 1 - 8 | n }\n";
        print "    system test the command set with -t\n";
        print "\n";

        exit;
    }
}

=item channel

Opens a channel for communicating over through a socket
connection established to the control host server.

=cut
sub channel {
    my $sock = IO::Socket::INET->new(
        PeerAddr => $host,
        PeerPort => $port,
        Proto    => 'tcp'
    );
    $sock or die "Can't connect\n";

    return $sock;
}

=item send

Sends a single command / value pair over a channel and
closes the connection.

=cut
sub send {
        my $sock = Control->channel();
        print $sock $command . ":" . $value . "\n";
        $sock->flush;
        close($sock);
}

=item test

Test suite for exercising control commands on individual
sensors (1-7) and on all (8) sensor ports.  First each port is
switched on one at a time (1-7), next off one at a time, then
the sample frequency is increased, followed by switching
on all (8) sensor ports then off.

=cut
sub test {
    for (my $sensor = 1; $sensor < 8; $sensor++) {
        print "Testing -c start -v " . $sensor . "\n";
        $command = "start";
        $value = $sensor;
        Control->send();
        sleep 1;
    }

    for (my $sensor = 1; $sensor < 8; $sensor++) {
        print "Testing -c stop -v " . $sensor . "\n";
        $command = "stop";
        $value = $sensor;
        Control->send();
        sleep 1;
    }

    print "Testing -c freq -v 10\n";
    $command = "freq";
    $value = 10;
    Control->send();
    sleep 1;

    print "Testing -c start -v 8\n";
    $command = "start";
    $value = 8;
    Control->send();
    sleep 1;

    print "Testing -c stop -v 8\n";
    $command = "stop";
    $value = 8;
    Control->send();
    sleep 1;
}
