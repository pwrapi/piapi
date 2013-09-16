#!/usr/bin/python

# note argparse is the latest optparse in python 2.7+
# however all of our servers are still on python 2.6
import optparse
import csv
import os

from power_file_parser import PowerFileGrouped
from constants import port_names

#stack_order = [7,6,5,4,2,3,1]
stack_order = [7,5,4,2,3,1]
#stack_order = [3,1]

def generateData(datafile, destdir=None, skip_first=True):
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

    def printmin(sec):
        import math
        m = math.ceil(sec/60)
        return "%s min %s sec" % (m, (sec-m*60))

    print "We found the sampling started at %s and ended at %s, total sample time %s" % (min_sec, max_sec, printmin(total_time))

    count = 0
    for line in pfg.data.values():
#        HOST:PID:SECONDS:MICROSECONDS:PORT:P1(A):P2(V):P3(mA):P4(mV):P5(mW):
#        host = line[0]
#        pid  = int(line[1])
#        second = int(line[2])
#        microsecond = int(line[3])
#        port = int(line[4])
#        a = int(line[5])
#        v = int(line[6])
#        ma = int(line[7])
#        mv = int(line[8])
#        mw = int(line[9])

        second = line.second - min_sec
        if second > (max_sec - min_sec):
            raise ValueError("duh")
#        second = count
#        print "second = %s" % second

        if not node_power.has_key(line.host):
            node_power[line.host] = {}
        if not node_power[line.host].has_key(second):
            node_power[line.host][second] = {}
        if not node_power[line.host][second].has_key(line.port):
            node_power[line.host][second][line.port] = 0

        if not system_power.has_key(second):
            system_power[second] = {}
        if not system_power[second].has_key(line.port):
            system_power[second][line.port] = 0

        
        node_power[line.host][second][line.port] += line.mw
        system_power[second][line.port]     += line.mw
        
        if line.port not in all_ports:
            all_ports.append(line.port)

        count += 1

    all_ports.sort()

    for host, second_data in node_power.items():
        filepath = os.path.join(destdir, "%s-stacked_ports_mw.dat" % host)
        host_fh = file(filepath, "w")
        ordered_seconds = second_data.keys()
        ordered_seconds.sort()
        for second in ordered_seconds:
            port_data = second_data[second]
            
            da_line = "%s\t%s" % (host, second)
            running_total = 0
            missing_data = False
            for port in stack_order:
                if port not in all_ports:
                    continue

                if port in port_data:
                    running_total += port_data[port]
                    da_line += "\t%s" % running_total
                else:
                    missing_data = True
                    print "Missing data for port %s at second %s for host %s" % (port, second, host)

            if not missing_data:
                host_fh.write("%s\n" % da_line)
        host_fh.close()

    filepath = os.path.join(destdir, "system-stacked_ports_mw.dat")
    system_fh = file(filepath, "w")

    seconds_ordered = system_power.keys()
    seconds_ordered.sort()
    for second in seconds_ordered:
        port_data = system_power[second]


        da_line = "system\t%s" % (second)
        missing_data = False
        running_total = 0
        for port in stack_order:
            if port not in all_ports:
                continue

            if port in port_data:
                running_total += port_data[port]
                da_line += "\t%s" % running_total
            else:
                missing_data = True
                print "Missing data for port %s at second %s" % (port, second)
        if not missing_data:
            system_fh.write("%s\n" % da_line)
    
    system_fh.close()


    plot_dir = os.path.join(os.path.dirname(destdir), "plots")
    writeNodeGP(node_power.keys(), stack_order, plot_dir)

    executeGP(node_power.keys(), plot_dir)

def executeGP(hosts, gpdir):
    basedir = os.path.dirname(gpdir)
    cmd = "cd %s; gnuplot %s"
    hosts.append('system')
    for host in hosts:
        gpfn = os.path.join("plots", "%s-stacked-line.gp" % host)
        os.system(cmd % (basedir, gpfn))


def writeNodeGP(hosts, ports, gpdir):
    """ This function writes out gnuplot scripts to generate each per
    node plot """
    if not os.path.exists(gpdir):
        os.mkdir(gpdir)

    gp_base_file = """set term post eps
set output "results/%(host)s-stacked_ports_mw.eps"
set key top right
set style fill solid
set xtic auto
set ytic auto
set xlabel "seconds"

set ytics auto

set ylabel "Power for %(host)s (mw)"

set style line 1 linetype 1 pointtype 0 linewidth 1 linecolor 1
set style line 2 linetype 1 pointtype 0 linewidth 1 linecolor 8
set style line 3 linetype 1 pointtype 0 linewidth 1 linecolor 7
set style line 4 linetype 1 pointtype 0 linewidth 1 linecolor 9
set style line 5 linetype 1 pointtype 0 linewidth 1 linecolor 6
set style line 6 linetype 1 pointtype 0 linewidth 1 linecolor 2
set style line 7 linetype 1 pointtype 0 linewidth 1 linecolor 3
set style line 8 linetype 1 pointtype 0 linewidth 1 linecolor 5
"""
    ports.reverse()
    hosts.append('system')
    for host in hosts:
        port_str = None
        filepath = os.path.join("results", "%s-stacked_ports_mw.dat" % host)
        port_index = len(ports)+1
        for port in stack_order:
            if port not in ports:
                continue
            port_index -= 1

            if port_names.has_key(port):
                port_name = port_names[port]
            else:
                port_name = "Port %s" % port
            dic = {'host':host,
                   'fp': filepath,
                   'port': port,
                   'port_name': port_name,
                   'index': port_index+2}
            if port_str is None:
                port_str = """plot '%(fp)s'  using 2:%(index)s t "%(port_name)s" w filledcurves x1 linestyle %(port)s""" % dic
            else:
                #                pass
                port_str += """, \\\n'%(fp)s' using 2:%(index)s t "%(port_name)s" w  filledcurves x1 linestyle %(port)s """ % dic


        all_code = "%s\n%s" % (gp_base_file, port_str)
#        print all_code
        fh = file(os.path.join(gpdir, "%s-stacked-line.gp" % host), "w")
        fh.write(all_code % {'host': host })
        fh.close()
    

def getCLIParser():
    parser = optparse.OptionParser(description='Takes raw input from teller nand generates data files sutiable for plotting stacked line charts of power consumption')
    parser.add_option('--file', dest='file', type=str,
                      help='The path to the raw data file')
    parser.add_option('--destdir', dest='destdir', type=str,
                      help='The directory of where to put the output files, if not given then it will attempt to create a results folder next to the given file.')
    return parser

if __name__ == "__main__":
    parser = getCLIParser()
    (options, args) = parser.parse_args()

    generateData(options.file, options.destdir)
