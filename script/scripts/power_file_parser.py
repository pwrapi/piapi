
import csv

class PowerFile:
    """ Interable class that does a very stupid parse of the power
    files output from teller """
    def __init__(self, datafile, header_first_row=True):
        self.datafile = datafile
        self.filehandle = None
        self.header_first_row = header_first_row

        self.host        = None
        self.pid         = None
        self.second      = None
        self.microsecond = None
        self.port        = None
        self.a           = None
        self.v           = None
        self.ma          = None
        self.mv          = None
        self.mw          = None

    def __iter__(self):
        return self

    def next(self):
        line = []
        self._openFile()
        bad_line = True
        try:
            while len(line) < 10 or bad_line:
                try:
                    line = self.reader.next()
                except csv.Error:
                    bad_line = True
                try:
                    self._parseRow(line)
                    bad_line = False
                except ValueError as e:
#                    print e
                    bad_line = True
                except IndexError as e:
#                    print e
                    bad_line = True
                
        except StopIteration:
            # close the file, if you re-iterate it will start over
            self.close()
            raise StopIteration
        if self.second is None or bad_line:
            raise StopIteration
        return self

    def close(self):
        self.filehandle.close()
        self.filehandle = None

    def __del__(self):
        try:
            self.close()
        except:
            pass

    def _parseRow(self, line):
        if len(line) == 0:
            return
        self.host = line[0]
        self.pid = line[1]
        self.second      = int(line[2])
        self.microsecond = int(line[3])
        self.port        = int(line[4])
        self.a           = float(line[5])
        self.v           = float(line[6])
        self.ma          = float(line[7])
        self.mv          = float(line[8])
        self.mw          = float(line[9])

        if not self.host.startswith('teller'):
            raise ValueError("All hosts should begin with teller, found %s" % self.host)
        if self.port < 0 or self.port > 7:
            raise ValueError("Ports should be between 1 and 7, found %s" % self.port)

            
    def _openFile(self):
        if self.filehandle is not None:
            return
        self.filehandle = file(self.datafile, 'r')
        self.reader = csv.reader(self.filehandle, delimiter=":")
        if self.header_first_row:
            self.reader.next()


class PowerFileRow:
    def __init__(self, line=None):
        self._parseRow(line)

    def addLine(self, line):
        self.samples += 1
        self.mw_sum += line.mw
        self.mw = self.mw_sum*1.0  / self.samples
        

    def _parseRow(self, line=None):
        if line is not None:
            self.samples     = 1
            self.host        = line.host
            self.pid         = line.pid
            self.second      = line.second
            self.microsecond = line.microsecond
            self.port        = line.port
            self.a           = line.a
            self.v           = line.v
            self.ma          = line.ma
            self.mv          = line.mv
            self.mw          = line.mw
            self.mw_sum      = line.mw
        else:
            self.samples     = 0
            self.host        = None
            self.pid         = None
            self.second      = None
            self.microsecond = None
            self.port        = None
            self.a           = None
            self.v           = None
            self.ma          = None
            self.mv          = None
            self.mw          = None
            self.mw_sum      = None

class PowerFileGrouped:
    """ this class groups the results by calling a group_key_func with
    the underlying data """
    def __init__(self, datafile, group_key_func, header_first_row=True):
        self.powerfile = PowerFile(datafile, header_first_row)
        self.group_key_func = group_key_func

        self.data = None
        self.parseIntoData()


    def parseIntoData(self):
        self.data = {}
        for line in self.powerfile:
            key = self.group_key_func(line)
            if not self.data.has_key(key):
                self.data[key] = PowerFileRow(line)
            else:
                self.data[key].addLine(line)

        

##class PowerFileGroupedRealtime:
##    def __init__(self, datafile, group_by, header_first_row=True):
##        self.powerfile = PowerFile(datafile, header_first_row)
##        self.group_by = group_by
##
##        self.prev_line = None
##        self.samples = []
##
##        self.pid = None
##
##    def __iter__(self):
##        return self
##
##    def next(self):
##        self.reset()
##        if self.prev_line == -1:
##            raise StopIteration
##
##        if self.prev_line is not None:
##            self.agg_func(self.prev_line)
##
##        try:
##            while True:
##                self.prev_line = self.powerfile.next()
##                if not self.agg_func(self.prev_line):
##                    break
##        except StopIteration:
##            self.prev_line = -1
##            pass
##
##        return self
##
##    def reset(self):
##        self.second = None
##        self.pid = None
##        self.mw_sum = 0
##        self.mw = 0
##        self.samples = 0
##
##    def agg_func(self, line):
##        if self.second is not None and line.second != self.second:
##            self.mw = (self.mw_sum*1.0) / self.samples
##            return False
##
###        print "%s\t%s" % (line.second , line.mw)
##        self.samples += 1
##        self.second = line.second
##        self.pid = line.pid
##        self.mw_sum = self.mw_sum + line.mw
##        return True


if __name__ == "__main__":            
    pfg = PowerFileGrouped('../experiments/replication/full_power/hpccg_run1_power.dat', lambda x: "%s-%s" % (x.second, x.port))
    
    for line in pfg.data.values():
        print "%s\t%s\t%s\t%s\t%s" % (line.second, line.samples, line.mw, line.mw_sum, line.port)

        
