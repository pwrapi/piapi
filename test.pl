#!/usr/bin/perl -w

use strict;

my @sensorstate = qw/off off off off off off off off/;
my $samplefreq = 1;

open(OUT, ">", "/tmp/.config");

for (my $sensorport = 0; $sensorport < 8; $sensorport++) {
    print OUT $sensorstate[$sensorport] . "\n";
}
print OUT $samplefreq . "\n";

close(OUT);

open(IN, "<", "/tmp/.config");

for (my $sensorport = 0; $sensorport < 8; $sensorport++) {
        chomp($sensorstate[$sensorport] = <IN>);
}
chomp($samplefreq = <IN>);

close(IN);

for (my $i = 0; $i < 8; $i++) {
    print "sensorstate[" . $i . "]  = " . $sensorstate[$i] . "\n";
}
print "samplefreq = " . $samplefreq . "\n";
