#!/usr/bin/perl -w

package Server;

use strict;
use base qw(Net::Server::PreFork);
use Getopt::Std;

=item main

Main simply calls config for setup and starts server.

=cut
my $port;
my $output;
my $logfile;
my $loglevel;
my $sem;

Server->config();

Server->run(
    port => $port,
    log_file => $logfile,
    log_level => $loglevel
);

=item config

Gets command line options for output filename and loglevel.
Defaults output to power.dat and log level to 2.

=cut
sub config {
    my %options=();
    getopts("p:f:l:qx", \%options);

    $port = 20203;
    $output = "power.dat";
    $logfile = "/tmp/server_" . $$ . ".log";
    $loglevel = 2;

    $port = $options{p} if defined $options{p};
    $output = $options{f} if defined $options{f};
    undef $logfile if defined $options{q};
    $loglevel = $options{l} if defined $options{l};

    unlink $output;
    open(OUT, ">" . $output);
    print OUT "HOST:PID:SECONDS:MICROSECONDS:PORT:P1(A):P2(V):P3(mA):P4(mV):P5(mW):\n";

    if (defined $options{x}) {
        print "\n";
        print "    ./server.pl [-p port] [-f filename]\n";
        print "                [-l loglevel] [-q]\n";
        print "\n";
        print "    filename redirects the power file output (default power.dat)\n";
        print "    loglevel can be from 1 to 4 (default 2)\n";
        print "    defaults log to /tmp or -q to console\n";
        print "\n";

        exit;
    }
}

=item post_child_cleanup

Hooks into base event flow to close the OUT file 
prior to server exiting.

=cut
sub post_child_cleanup {
    close(OUT);
}

=item process_request

Overrides base function for processing request on client connection.
STDIN is redirected from socket and messages are logged to output file.

=cut
sub process_request {
    my $self = shift;

    while( <STDIN> ) {
        s/\r?\n$//;
        $self->log(3, "LOG - logging data " . $_);
        print OUT "$_\r\n";
        OUT->flush;
    }
}

