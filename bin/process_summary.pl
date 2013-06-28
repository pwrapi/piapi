#!/usr/bin/perl -w
#
use strict;

use Statistics::Basic qw(:all);

my $input = shift or die "Must provide input file\n";

open (IFD, "<$input") or die "Could not open $input for reading ($!)\n";

my %phash;
my $phref = \%phash;
my @parray;

my ($hostn, $port, 
 $avg_amps, $min_amps, $max_amps, 
 $avg_volt, $min_volt, $max_volt, 
 $avg_watt, $min_watt, $max_watt, 
 $tot_time, $min_time, $max_time, 
 $joules);
while (<IFD>) {
  chomp;
  /^#.*/ and next;
  ($hostn, $port, 
   $avg_amps, $min_amps, $max_amps, 
   $avg_volt, $min_volt, $max_volt, 
   $avg_watt, $min_watt, $max_watt, 
   $tot_time, $min_time, $max_time, 
   $joules) = split;  
  
  #
  # Don't know if we need everything but keep it just the
  # same, makes later modifications easier
  # Assume that we have multiple entries for the same host
  # so store values in array based on port
  #
  push @{$phref->{$hostn}->{$port}->{'avg_amp'}}, $avg_amps;  
  push @{$phref->{$hostn}->{$port}->{'min_amp'}}, $min_amps;  
  push @{$phref->{$hostn}->{$port}->{'max_amp'}}, $max_amps;  

  push @{$phref->{$hostn}->{$port}->{'avg_volt'}}, $avg_volt;  
  push @{$phref->{$hostn}->{$port}->{'min_volt'}}, $min_volt;  
  push @{$phref->{$hostn}->{$port}->{'max_volt'}}, $max_volt;  

  push @{$phref->{$hostn}->{$port}->{'avg_watt'}}, $avg_watt;  
  push @{$phref->{$hostn}->{$port}->{'min_watt'}}, $min_watt;  
  push @{$phref->{$hostn}->{$port}->{'max_watt'}}, $max_watt;  

  push @{$phref->{$hostn}->{$port}->{'tot_time'}}, $tot_time;  
  push @{$phref->{$hostn}->{$port}->{'min_time'}}, $min_time;  
  push @{$phref->{$hostn}->{$port}->{'max_time'}}, $max_time;  

  push @{$phref->{$hostn}->{$port}->{'normalized_p_div_t'}}, ($joules/$tot_time);  

  push @{$phref->{$hostn}->{$port}->{'joules'}}, $joules;  
}





#
# Some functions to generate statistics
#

#
# Used to calculate how much time and joules values deviate 
# across multiple runs for the PowerInsight paper
#
# Beware of the change from hostn in the store to host in these references
# cut and paste warning.
#
for my $host (keys(%phash)) {
  for my $port (sort { $a <=> $b } (keys(%{$phref->{$host}}))) {
    $port == 1 or next;
    my $min_time = 999999999999999;
    my $max_time = 0;
    my $min_pow = 999999999999999;
    my $max_pow = 0;
    map { $min_time = $_ if $_ < $min_time; } @{$phref->{$host}->{$port}->{'tot_time'}}; 
    map { $max_time = $_ if $_ > $max_time; } @{$phref->{$host}->{$port}->{'tot_time'}}; 
    
    map { $min_pow = $_ if $_ < $min_pow; } @{$phref->{$host}->{$port}->{'joules'}}; 
    map { $max_pow = $_ if $_ > $max_pow; } @{$phref->{$host}->{$port}->{'joules'}}; 

    my $time_mean = mean(\@{$phref->{$host}->{$port}->{'tot_time'}});
    my $time_stddev = stddev(\@{$phref->{$host}->{$port}->{'tot_time'}});
    my $time_cv = 100 * $time_stddev/$time_mean; 
  
    my $pow_mean = mean(\@{$phref->{$host}->{$port}->{'joules'}});
    my $pow_stddev = stddev(\@{$phref->{$host}->{$port}->{'joules'}});
    my $pow_cv = 100 * $pow_stddev/$pow_mean;

    my $normalized_mean = mean(\@{$phref->{$host}->{$port}->{'normalized_p_div_t'}});
    my $normalized_stddev = stddev(\@{$phref->{$host}->{$port}->{'normalized_p_div_t'}});
    my $normalized_cv = 100 * $normalized_stddev/$normalized_mean;

    print "\nHost: $host Port: $port\n";
    print "Samples ", scalar(@{$phref->{$host}->{$port}->{'joules'}}), "\n";
    print "Time @{$phref->{$host}->{$port}->{'tot_time'}}\n";
    print "Max Time $max_time\n";
    print "Min Time $min_time\n";
    print "Time Mean $time_mean\n";
    print "Time ST_DEV $time_stddev\n";
    print "Time CV $time_cv\n";

    print "Energy @{$phref->{$host}->{$port}->{'joules'}}\n";
    print "Max Energy $max_pow\n";
    print "Min Energy $min_pow\n";
    print "Energy Mean $pow_mean\n";
    print "Energy ST_DEV $pow_stddev\n";
    print "Energy CV $pow_cv\n";

    print "Normalized Values Joules/Time @{$phref->{$host}->{$port}->{'normalized_p_div_t'}}\n";
    print "Normalized Mean $normalized_mean\n";
    print "Normalized ST_DEV $normalized_stddev\n";
    print "Normalized CV $normalized_cv\n";

  }
}

my $done = 0;
for my $port (1..7) {
  for my $sample (0..100) { # Warning, limiting samples to 100, likely not a problem
    my @joules;
    my @time;
    for my $host (keys(%phash)) {
      if (exists($phref->{$host}->{$port}->{'joules'}[$sample])) {
        push @joules, $phref->{$host}->{$port}->{'joules'}[$sample];
        push @time, $phref->{$host}->{$port}->{'tot_time'}[$sample];
      } 
    } #host
    if (defined($time[0])) {
      print "\nPort $port Sample $sample:\n";
      my $time_mean = mean(\@time);
      my $time_stddev = stddev(\@time);
      my $time_cv = 100 * $time_stddev/$time_mean;

      my $pow_mean = mean(\@joules);
      my $pow_stddev = stddev(\@joules);
      my $pow_cv = 100 * $pow_stddev/$pow_mean;

      print "Time Mean $time_mean\n";
      print "Time ST_DEV $time_stddev\n";
      print "Time CV $time_cv\n";
      print "Energy Mean $pow_mean\n";
      print "Energy ST_DEV $pow_stddev\n";
      print "Energy CV $pow_cv\n";
    }
  } #sample
} #index




# Generate an array of energy values for each port along with 
# the statistics. 
#

#my @energy;
#my %ehash;
#my $ehref = \%ehash;
#for my $key (keys(%phash)) {
#    # key is the hostname we are processing, not important other than
#    # we need it to drill deeper
#    for my $port qw/P1 P2 P3 P4 P5 P6 P7/ {
#        defined($port) and do {
#            push @{$ehref->{$port}}, $phref->{$key}{$port};
#        };
#    }
#}
#
##
## We now have energy values by port we can generate per port statistics
##
#
#for my $port qw/P1 P2 P3 P4 P5 P6 P7/ {
#    @{$ehref->{$port}} = sort { $a <=> $b } @{$ehref->{$port}};
#    my $mean = mean(\@{$ehref->{$port}});
#    my $std_dev = stddev(\@{$ehref->{$port}});
#    my $cv = 100 * $std_dev/$mean;
#    print "Port $port MEAN:\t$mean\n" .
#          "\tSTDDEV:\t$std_dev\n" .
#          "\tCV:\t$cv\n".
#          "\tMIN:\t$ehref->{$port}[0]\n" .
#          "\tMAX:\t$ehref->{$port}[$#{$ehref->{$port}}]\n\n";
#
#}
