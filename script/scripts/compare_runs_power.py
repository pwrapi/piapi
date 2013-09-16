#!/usr/bin/python

# note argparse is the latest optparse in python 2.7+
# however all of our servers are still on python 2.6
import optparse
import csv
import os

from power_file_parser import PowerFileGrouped

def generateData(datafiles, destdir, skip_first=True):
    if destdir is None:
        destdir = "%s/results" % os.path.dirname(datafiles[0])

    if not os.path.exists(destdir):
        os.mkdir(destdir)

    total_for_file = {}
    longest_number_of_samples = 0
    for df in datafiles:
        count = 0

        def group_func(line):
            return "%s-%s-%s" % (line.host, line.second, line.port)

        pfg = PowerFileGrouped(df, group_func)

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

        total_for_file[df] = {}
        total_per_second = {}
        total_for_file[df]['second'] = total_per_second
        all_seconds = []
        for line in pfg.data.values():

            second = line.second - min_sec
            if second > (max_sec - min_sec):
                raise ValueError("duh")

            if line.second not in all_seconds:
                all_seconds.append(line.second)

#            second = len(all_seconds)

            if not total_per_second.has_key(second):
                total_per_second[second] = 0
            total_per_second[second] += line.mw
            print total_per_second[second]

            count += 1
        longest_number_of_samples = max(longest_number_of_samples,
                                        len(all_seconds))

        
    filepath = os.path.join(destdir, "total-power-by-second_mw.dat")
    host_fh = file(filepath, "w")
    for second in range(0,longest_number_of_samples):
        line = str(second);

        for df in datafiles:
            if total_for_file[df]['second'].has_key(second):
                total = total_for_file[df]['second'][second]
            else:
                total = 0

            line += "\t%s" % total
        host_fh.write("%s\n" % line)

    host_fh.close()

    gpdir = os.path.join(os.path.dirname(destdir),'plots')
    writeNodeGP(datafiles, gpdir)
    executeGP(gpdir)


def writeNodeGP(datafiles, gpdir):
    """ This function writes out gnuplot scripts to generate each per
    node plot """
    if not os.path.exists(gpdir):
        os.mkdir(gpdir)

    gp_base_file = """set term post eps
set output "results/total-power-by-second_mw.eps"
set key top right
#set style fill solid
#set xdata time
#set timefmt "%%s"
#set format x "%%H:%%M:%%S"
set xtic auto
set ytic auto
set xlabel "seconds"

set ytics auto

set ylabel "Total Power (mw)"

set style line 1 linetype 1 pointtype 0 linewidth 1 linecolor 1
set style line 2 linetype 1 pointtype 0 linewidth 1 linecolor 8
set style line 3 linetype 1 pointtype 0 linewidth 1 linecolor 7
set style line 4 linetype 1 pointtype 0 linewidth 1 linecolor 9
set style line 5 linetype 1 pointtype 0 linewidth 1 linecolor 6
set style line 6 linetype 1 pointtype 0 linewidth 1 linecolor 2
set style line 7 linetype 1 pointtype 0 linewidth 1 linecolor 3
set style line 8 linetype 1 pointtype 0 linewidth 1 linecolor 5
"""
    filepath = os.path.join("results", "total-power-by-second_mw.dat")
    index = 1
    port_str = None
    for df in datafiles:
        index += 1;
        dic = {'run_name':df,
               'index':index,
               'fp': filepath}

        if port_str is None:
            port_str = """plot '%(fp)s'  using 1:%(index)s t "%(run_name)s" smooth unique linestyle %(index)s""" % dic
        else:
            port_str += """, \\\n'%(fp)s' using 1:%(index)s t "%(run_name)s"  smooth unique linestyle %(index)s""" % dic


        all_code = "%s\n%s" % (gp_base_file, port_str)
#        print all_code
        fh = file(os.path.join(gpdir, "total-power-by-second.gp"), "w")
        print all_code
        fh.write(all_code)
        fh.close()
            

def executeGP(gpdir):
    basedir = os.path.dirname(gpdir)
    cmd = "cd %s; gnuplot %s"
    gpfn = os.path.join("plots", "total-power-by-second.gp")
    os.system(cmd % (basedir, gpfn))


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

    generateData(args, options.destdir)

