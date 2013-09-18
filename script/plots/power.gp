set autoscale
unset log
unset label

if (!exists("node")) node=("$0" == "") ? "teller-pm00" : "$0"
if (!exists("port")) port=("$1" == "") ? "1" : "$1"
if (!exists("id")) id=("$2" == "") ? "0" : "$2"

results="results/".node.".".id."_".port

if (!exists("crt")) set term post eps
if (!exists("crt")) set output results.".eps"

set multiplot
set size 1, 0.3;

set xtic auto
set ytic auto
set xlabel "seconds"

set title "Voltage for from ".node." port #".port
set ylabel "volts"
set origin 0.0,0.6
plot results."_V.dat" using 1:2 with lines notitle

set title "Amperage for from ".node." port #".port
set ylabel "amps"
set origin 0.0,0.3
plot results."_A.dat" using 1:2 with lines notitle

set title "Wattage for from ".node." port #".port
set ylabel "watts"
set origin 0.0,0.0
plot results."_W.dat" using 1:2 with lines notitle

unset multiplot

if (!exists("crt")) set term x11
if (!exists("crt")) set output

if (exists("crt")) print "press <enter> to continue"
if (exists("crt")) pause -1
