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


def getCLIParser():
    parser = optparse.OptionParser(description='Takes raw input from teller nand generates data files sutiable for plotting stacked line charts of power consumption')
    parser.add_option('--file', dest='file', type=str,
                      help='The path to the raw data file')
    parser.add_option('--multiple', dest='multiple', type=str,
                      help='list of run numbers to average')
    return parser

if __name__ == "__main__":
    parser = getCLIParser()
    (options, args) = parser.parse_args()
    
    if not options.multiple:
        data = totalEnergy(options.file)
        print "Total execution time is %s" % formatsec(data['total_time'])
        print "Total energy is %s" % data['total_energy']

        print "Energy by port is %s" % data['energy_by_port']

    else:

        runs = map(int, options.multiple.split(","))
        data = averageEnergyAcrossRuns(options.file, runs=runs, verbose=True)

