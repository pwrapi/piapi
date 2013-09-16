#!/usr/bin/python

# note argparse is the latest optparse in python 2.7+
# however all of our servers are still on python 2.6
import optparse
import csv
import os
import math

from power_file_parser import PowerFileGrouped

from constants import port_names

from helper_functions import totalEnergy, formatsec, averageEnergyAcrossRuns

def writeGnuPlot(results_dir, number_of_expr):
    """ This function writes out gnuplot scripts to generate each per
    node plot """
    gp_dir = os.path.join(args[0],'plots')
    if not os.path.exists(gp_dir):
        os.mkdir(gp_dir)

    gp_base_file = """set term post eps
set output "results/compare_experiments.eps
#set key top right
#set style fill solid
#set xtic auto
#set ytic auto
unset key

set style data histogram
set style fill solid border
set xtics rotate out
# We need to set lw in order for error bars to actually appear.
set style histogram errorbars linewidth 2 gap 0 
# Make the bars semi-transparent so that the errorbars are easier to see.
set style fill solid 0.3
set bars front large
set boxwidth 0.5 

set yrange[ 0 : 1.10 ]
set xrange[ 0.2 : %d ]

set ylabel "Normalized Time"

plot 'results/compare_experiments.dat' using 4:5:xticlabels(1) title "Energy" lt 3

"""
    filename = os.path.join(gp_dir, "compare_experiments.gp")
    fh = file(filename, "w")
    fh.write(gp_base_file % (number_of_expr+1))
    fh.close()

    executeGP(gp_dir)

def executeGP(gpdir):
    basedir = os.path.dirname(gpdir)
    cmd = "cd %s; gnuplot %s"
    gpfn = os.path.join("plots", "compare_experiments.gp")
    os.system(cmd % (basedir, gpfn))


def getCLIParser():
    parser = optparse.OptionParser(description='Takes raw input from teller nand generates data files sutiable for plotting stacked line charts of power consumption')
    parser.add_option('--runs', dest='runs', type=str,
                      help='list of run numbers the run numbers to use')
    return parser

if __name__ == "__main__":
    parser = getCLIParser()
    (options, args) = parser.parse_args()
    
    runs = None
    if options.runs:
        runs = map(int, options.runs.split(","))

    max_time = 0
    max_energy = 0
    data = {}
    for df in args:
        data[df] = averageEnergyAcrossRuns(df, runs=runs, verbose=False)
        if data[df]['energy_avg'] > max_energy:
            max_energy = data[df]['energy_avg']
        if data[df]['time_avg'] > max_time:
            max_time = data[df]['time_avg']*1.0

        print "---- experiment %s" % df
        pct =  "%2.2f" % ((data[df]['time_stddev']/data[df]['time_avg'])*100)
        print "Average Time   %s    [stddev:%s - %s%%]" % (formatsec(data[df]['time_avg']).rjust(20), formatsec(data[df]['time_stddev']).rjust(15), pct.rjust(5))
        xx = "%20.3f" % data[df]['energy_avg']
        pct = "%2.2f"%((data[df]['energy_stddev']/data[df]['energy_avg'])*100)
        print "Average Energy %s    [stddev:%s - %s%%]" % (xx.rjust(20), str(data[df]['energy_stddev']).rjust(15), pct.rjust(5))


    ## write plot data
    results_dir = os.path.join(args[0],'results')
    if not os.path.exists(results_dir):
        os.mkdir(results_dir)
    plot_data = os.path.join(results_dir, "compare_experiments.dat")
    fh = file(plot_data, "w")
    header = "experiment\tenergy\tenergy_stddev\ttime\ttime_stddev\n"
    fh.write(header)

    for df in args:
        basedir = os.path.basename(os.path.dirname(df))[0:7]
        if basedir == 'baselin':
            basedir = '3800000'
        elif basedir == 'low_pow':
            basedir = '1400000'

        plot_line = "%s\t%s\t%s\t%s\t%s\n" % (basedir, data[df]['energy_avg']/max_energy, data[df]['energy_stddev']/max_energy, data[df]['time_avg']/max_time, data[df]['time_stddev']/max_time)
        fh.write(plot_line)

    fh.close()
    writeGnuPlot(results_dir, len(args))


