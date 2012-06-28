from __future__ import with_statement
from lofar.parmdb import parmdb
from argparse import ArgumentTypeError


def list_stations(parmdb, pattern=''):
    """
    Get a sorted list of all the station names, without prepending polarisation
    Information,
    Can be called with a path or an instantiated parmdb     
    """
    # validate input
    if isinstance(parmdb, basestring):
        # create a WritableParmDB
        parmdb = WritableParmDB(parmdb)
    elif not isinstance(parmdb, WritableParmDB):
        raise ArgumentTypeError("list_stations can only be called with a string or an"
                            "instantiated WritableParmDB")

    raw_name_list = parmdb.getNames(parmnamepattern=pattern)
    # The names are 'complex' split at the :
    # and get the last index of the split. use set so they are unique.
    # return the sorted list
    return sorted(set(name.split(":")[-1] for name in raw_name_list))



class WritableParmDB(parmdb):
    def __init__(self, name):
        super(WritableParmDB, self).__init__(name)
        self.name = name

    def setValues(self, name, values, start_freq, freqstep, start_time, timestep):
        """
        Write values to the ParmDB.

        Note that values should be a two dimenstional array with the first
        index corresponding to time and the second to time (this is the same
        as returned by ParmDB.getValues()).

        Arguments:

        name       -- Parameter name to write.
        values     -- NumPy array of values to write.
        start_freq -- Frequency at left/start of first bin (Hz).
        freqstep   -- Bin-to-bin frequency increment (Hz).
        start_time -- Time at left/start of first bin (MJD in seconds).
        timestep   -- Bin-to-bin time increment (s).
        
        Version 1.0: usage of direct parmdb calls instead of subprocess and 
            an executable
        """

        # Get the number of steps, both in time and freq
        time_steps, freq_steps = values.shape
        # remove the old entry
        self.deleteValues(name)

        self.addValues(name, values,
                       start_freq,
                       start_freq + freqstep * freq_steps,
                       start_time,
                       start_time + timestep * time_steps,
                       True)

