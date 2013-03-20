set autoscale
unset log
unset label

if (!exists("node")) node=("$0" == "") ? "teller-pm00" : "$0"
if (!exists("port")) port=("$1" == "") ? "1" : "$1"
if (!exists("id")) id=("$2" == "") ? "0" : "$2"

results="results/".node.".".id."_".port

if (!exists("crt")) set term post eps
if (!exists("crt")) set output results.".eps"

set xtic auto
set ytic auto
set xlabel "seconds (since epoch)"

set title "Amperage for from ".node." port #".port
set ylabel "amps"
plot results."_A.dat" using 1:2 with lines title 'Sensor'

if (!exists("crt")) set term x11
if (!exists("crt")) set output

if (exists("crt")) print "press <enter> to continue"
if (exists("crt")) pause -1
