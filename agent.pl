#!/usr/bin/perl -w

package Agent;

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
pipe my $load, my $save;
my $saddr;
my $rport;
my $output;

my $pid;
my $delay;
my $args;

my $port;
my $logfile;
my $loglevel;

my $pipe = "no";
my @sensorstate;
my $samplefreq;

Agent->config;
Agent->sampler if ($delay eq "no");
Agent->run(
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
    getopts("h:r:p:f:l:s:qwx", \%options);

    $saddr = "localhost";
    $rport = "20203";
    $args = "";

    $port = 20202;
    $logfile = "/tmp/agent_" . $$ . ".log";
    $loglevel = 2;
    $delay = "no";
    @sensorstate = qw/off off off off off off off off/;
    $samplefreq = 1;

    $saddr = $options{h} if defined $options{h};
    $rport = $options{r} if defined $options{r};
    $output = $options{f} if defined $options{f};

    $samplefreq = $options{s} if defined $options{s};
    $port = $options{p} if defined $options{p};
    $loglevel = $options{l} if defined $options{l};
    undef $logfile if defined $options{q};
    $wait = "yes" if defined $options{w};

    if (defined $options{x}) {
        print "\n";
        print "    ./agent.pl [-h hostname] [-r remoteport] [-p localport]\n";
        print "                [-f filename] [-l loglevel] [-s samplerate] [-q] [-w]\n";
        print "\n";
        print "    loglevel can be from 1 to 4 (default 2)\n";
        print "    defaults log to /tmp or -q to console\n";
        print "\n";

        exit;
    }
}

=item sample

Forks a child process to start sample.pl with command line
arguments passed.

=cut
sub sampler {
    defined($pid = fork) or die "Cannot fork: $!";

    unless ($pid) {
        if ($pipe eq "yes") {
            Agent->sample;
        } else {
            $args = " -h " . $saddr . " -p " . $rport;
            $args .= " -f " . $output if defined $output;
            exec "./sample.pl $args";
            die "cannot exec sample.pl: $!";
        }
    }
    print "Sampling identifier is " . $pid . "\n";
}

=item post_child_cleanup

Hooks into base event flow to kill the child process for sample.pl
prior to server exiting.

=cut
sub post_child_cleanup {
    if ($pipe eq "yes") {
        close $load;
        close $save;
    }

    kill $pid;
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
        } elsif ($command eq "attach") {
            $saddr = $value;
            $self->log(4, "Attaching to proxy at " .  $saddr);
            Agent->sampler if ($wait eq "yes");
            $wait = "no";
        } elsif ($command eq "detach") {
            $saddr = $value;
            $self->log(4, "Detaching from proxy at " .  $saddr);
            kill $pid if $wait = "no";
            $wait = "yes";
        }
    }
}

=item post_client_connection_hook

Overrides base function for saving configuration data on client
request completion

=cut
sub post_client_connection_hook() {
    my $self = shift;

    $self->log(4, "Saving configuration...");
    for (my $i = 0; $i < 8; $i++) {
        $self->log(4, "sensorstate[" . $i . "] = " . $sensorstate[$i]);
    }
    $self->log(4, "samplefreq = " .  $samplefreq);
    Agent->save_config if ($wait eq "no");
}

=item save_config

Creates a hidden file .config to pass sensor states and sample
frequency parameters between threads (due to buggy perl thread
sharing).

=cut
sub save_config {
    if ($pipe eq "yes") {
        for (my $sensorport = 0; $sensorport < 8; $sensorport++) {
            print $save $sensorstate[$sensorport] . "\n";
        }
        print $save $samplefreq . "\n";
    } else {
        open(OUT, ">", "/tmp/.config" . $pid);

        for (my $sensorport = 0; $sensorport < 8; $sensorport++) {
            print OUT $sensorstate[$sensorport] . "\n";
        }
        print OUT $samplefreq . "\n";

        close(OUT);
    }
}

=item load_config

Opens a hidden file .config to refresh sensor states and
sample frequency parameters for processes and removes
the file once read.

=cut
sub load_config {
    if ($pipe eq "yes") {
        for (my $sensorport = 0; $sensorport < 8; $sensorport++) {
            chomp($sensorstate[$sensorport] = <$load>);
        }
        chomp($samplefreq = <$load>);
    } else {
        open(IN, "<", "/tmp/.config" . $$);

        for (my $sensorport = 0; $sensorport < 8; $sensorport++) {
            chomp($sensorstate[$sensorport] = <IN>);
        }
        chomp($samplefreq = <IN>);

        close(IN);

        unlink("/tmp/.config" . $$) or warn "Could not unlink .config";
    }
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
            PeerPort => $rport,
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
    my $sock = Agent->channel;
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
        if ($pipe eq "yes") {
            Agent->load_config if select $load;
        } else {
            Agent->load_config if (-e "/tmp/.config" . $$);
        }
    }

    close($sock);
}

