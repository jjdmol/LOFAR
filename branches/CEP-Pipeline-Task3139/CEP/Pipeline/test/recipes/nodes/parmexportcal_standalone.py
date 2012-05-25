from lofarpipe.recipes.nodes.parmexportcal import ParmExportCal
import sys
## export PYTHONPATH=/home/klijn/build/gnu_debug/lib/python2.6/dist-packages
class logger():
    """
    muck logger object, allows logging of info, debug and error function
    new entries are appended to a list with the type of logger error and 
    the suplied input
    """
    def __init__(self):
        self._log = []

    def info(self, input_string):
        self._log.append(("info", input_string))

    def debug(self, input_string):
        self._log.append(("debug", input_string))

    def error(self, input_string):
        self._log.append(("error", input_string))

    def warn(self, input_string):
        self._log.append(("warn", input_string))

    def last(self):
        """
        return that last error
        """
        return self._log[-1]


class ParmExportCalWrapper(ParmExportCal):
    """
    Wrapper inserting logger functionality
    """
    def __init__(self):
        """
        """
        self.logger = logger()


if __name__ == "__main__":

    usage = """
    (shalow) wrapper for the parmexportcal node script allowing
    functional testing of the edit_parmdb functionality.
    Needed is a parmdb  created as folows:
    
    $ parmdbm 

    # Copy pasta:
    create table='test_blank.parmdb' 
    add Gain:0:0:Real:test ny=8, nx=2, values=[0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,10,10,100,100], freqwidths=[2], times=[2], freqs=[1], timestep=[1]
    add Gain:0:0:Imag:test ny=8, nx=2, values=[0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,10,10,100,100], freqwidths=[2], times=[2], freqs=[1], timestep=[1]
    add Gain:1:1:Real:test ny=8, nx=2, values=[0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,10,10,100,100], freqwidths=[2], times=[2], freqs=[1], timestep=[1]
    add Gain:1:1:Imag:test ny=8, nx=2, values=[0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,10,10,100,100], freqwidths=[2], times=[2], freqs=[1], timestep=[1]
    
    After running the corrected values are displayed
    The last minus 1 should be 0.1 and not 10
    """
    print sys.argv
    infile, outfile, sigma = sys.argv[1:4]

    parmdb = ParmExportCalWrapper()
    parmdb, corrected_data = parmdb._filter_stations_parmdb(infile, outfile, sigma)
    for pol, datapoint in corrected_data.iteritems():
        print datapoint.real

    sys.exit()
