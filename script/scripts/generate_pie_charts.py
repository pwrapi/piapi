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

import matplotlib
matplotlib.use('Agg')

import numpy as np
import matplotlib.pyplot as plt


def getCLIParser():
    parser = optparse.OptionParser(description='Takes raw data files and generates pie charts showing break down of total energy by port')
    parser.add_option('--file', dest='file', type=str,
                      help='The path to the raw data file')
    parser.add_option('--output', dest='output', type=str,
                      help='The path of where to write the output.')
    parser.add_option('--multiple', dest='multiple', type=str,
                      help='list of run numbers to average')
    return parser

def pieChartForEachFile(files, output):
    plt.figure(num=None, figsize=(11, 4))

#    colors = ['#C7F464', '#556270', '#4ECDC4', '#FF6B6B', '#C44D58', 'yellowgreen']
#    colors = ['#39b54a', '#2e3192', '#f7941d', '#fff200', '#C44D58', 'yellowgreen']
#    colors = ['yellowgreen', 'gold', 'lightskyblue', 'lightcoral', 'teal', 'red']
    colors = ['red', 'gold', 'blue', 'lightcoral', 'green', 'orange']
    colors = ['#FD0006', '#FF7100', '#009B95', '#0ACF00', '#CB0077', '#5F2580']
#    order = (7,5,4,3,2,1)
    order = (1,2,3,4,5,7)
    sp_rows = 1
    sp_cols = 3
    i = 0
    for file in args:
        i += 1
        plt.subplot(sp_rows, sp_cols, i)
        percents = []
        labels = []
        explode = []
        data = totalEnergy(file)
        for port in order:
            percents.append(data['energy_by_port'][port]/data['total_energy'])
            labels.append(port_names[port])
            if port == 1:
                explode.append(0.1)
            else:
                explode.append(0)
            

#        plt.axes(aspect=1)
        pie = plt.pie(percents, explode=explode, labels=None, labeldistance=0, colors=colors,
                      autopct='%1.0f%%', shadow=False, pctdistance=1.15)
        if i == 1:
            plt.title("Lowest CPU Power")
        else:
            plt.title("Highest CPU Power")

#        plt.legend(title="Component", loc=3)
    plt.subplot(sp_rows, sp_cols, sp_rows*sp_cols)
    pie = plt.pie(percents, labels=labels, colors=colors)
    l = plt.legend(title="Component", loc="center")
    for group in pie:
        for x in group:
            x.set_visible(False)

    if options.output:
        plt.tight_layout()
        plt.savefig(output)


if __name__ == "__main__":
    parser = getCLIParser()
    (options, args) = parser.parse_args()
    
    if not options.multiple:
        pieChartForEachFile(args, options.output)
    else:

        runs = map(int, options.multiple.split(","))
        data = averageEnergyAcrossRuns(options.file, runs=runs, verbose=True)


