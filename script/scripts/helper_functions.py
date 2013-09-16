#!/usr/bin/python

import os
import math
from power_file_parser import PowerFileGrouped

from constants import port_names

def totalEnergy(datafile, destdir=None, skip_first=True, verbose=False):
    if destdir is None:
        destdir = "%s/results" % os.path.dirname(datafile)

    if not os.path.exists(destdir):
        os.mkdir(destdir)

    def group_func(line):
        return "%s-%s-%s" % (line.host, line.second, line.port)

    pfg = PowerFileGrouped(datafile, group_func)
    system_power = {}
    node_power = {}
    all_ports = []
    energy_by_port = {}
#    fh = file(datafile, 'r')
#    reader = csv.reader(fh, delimiter=":")

    min_sec = 9999999999999999999
    max_sec = 0
    for line in pfg.data.values():
        if line.second < min_sec:
            min_sec = line.second
        if line.second > max_sec:
            max_sec = line.second
    total_time = max_sec - min_sec

    if verbose:
        print "We found the sampling started at %s and ended at %s, total sample time %s" % (min_sec, max_sec, printmin(total_time))

    for port in port_names.keys():
        energy_by_port[port] = 0

    count = 0
    total = 0
    for line in pfg.data.values():
        if skip_first and count == 0:
            count += 1
            continue

        total += line.mw

        energy_by_port[line.port] += line.mw

    return {'total_energy'   : total,
            'total_time'     : total_time,
            'energy_by_port' : energy_by_port}

def determineAvailableRuns(basedir, verbose=False):
    return [1,2,]
    
def averageEnergyAcrossRuns(basedir, runs=None, verbose=False):
    """ This function averages the energy across multiple runs of the
    same experiment. It returns the averages of the execution time,
    total energy as well as the amount per port along with std dev for
    each average
    """
    time_sum = 0
    energy_sum = 0
    energy_by_port_sum = {}
    for port in port_names.keys():
        energy_by_port_sum[port] = 0
    data = {}

    if runs is None:
        runs = determineAvailableRuns(basedir, verbose)

#    runs = map(int, options.multiple.split(","))
    for x in runs:
        data[x] = totalEnergy("%s/%s" % (basedir, "run%d.power.dat" % x))
        if verbose:
            print "---- Run %d ----" % x
            print "Total execution time is %s" % formatsec(data[x]['total_time'])
            print "Total energy is %s" % data[x]['total_energy']
            print "Energy Percent By port %s" % map(lambda y: (y / data[x]['total_energy'])*100, data[x]['energy_by_port'].values())
        
        time_sum += data[x]['total_time']
        energy_sum += data[x]['total_energy']
        for port, energy in data[x]['energy_by_port'].items():
            energy_by_port_sum[port] += energy

        
    time_avg = time_sum / len(runs)
    energy_avg = energy_sum / len(runs)

    energy_by_port_avg = {}
    for port, energy in energy_by_port_sum.items():
        energy_by_port_avg[port] = energy / len(runs)


    time_var = 0
    energy_var = 0
    energy_by_port_var = {}
    for x in runs:
        time_var += math.pow((data[x]['total_time'] - time_avg), 2)
        energy_var += math.pow((data[x]['total_energy'] - energy_avg), 2)

        for port, energy in data[x]['energy_by_port'].items():
            energy_by_port_var[port] = math.pow(energy - energy_by_port_avg[port], 2)


    time_sample_stddev = math.sqrt((time_var*1)/(len(runs)-1))
    energy_sample_stddev = math.sqrt((energy_var*1)/(len(runs)-1))

    energy_by_port_stddev = {}
    for port, energy in energy_by_port_var.items():
        energy_by_port_stddev[port] = (math.sqrt(energy) / (len(runs)-1))

    energy_by_port_avg_percent = map(lambda y: (y / energy_avg)*100, energy_by_port_avg.values())
    energy_by_port_stddev_percent = map(lambda y: (y[0]/y[1])*100, zip(energy_by_port_stddev.values(), energy_by_port_avg.values()))


    if verbose:
        print "Average Time %s with Sample Standard Dev %s which is %s%%" % (formatsec(time_avg), formatsec(time_sample_stddev), (time_sample_stddev/time_avg)*100)
        print "Average Energy %s with sample Standard Dev %s which is %s%%" % (energy_avg, energy_sample_stddev, (energy_sample_stddev/energy_avg)*100)
        print "Average Energy by Port %s with Sample Standard Dev Percent %s" % (energy_by_port_avg_percent, energy_by_port_stddev_percent)

    return {'time_avg'              : time_avg,
            'time_stddev'           : time_sample_stddev,
            'energy_avg'            : energy_avg,
            'energy_stddev'         : energy_sample_stddev,
            'energy_by_port_avg'    : energy_by_port_avg,
            'energy_by_port_stddev' : energy_by_port_stddev}
                

def formatsec(sec):
    if sec > 59:
        m = math.ceil(sec/60)
    else:
        m = 0
    return "%s min %2.2f sec" % (m, (sec-m*60))

