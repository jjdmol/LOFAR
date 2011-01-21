# __init__.py: Top level .py file for python stationresponse interface
# Copyright (C) 2011
# ASTRON (Netherlands Institute for Radio Astronomy)
# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
#
# This file is part of the LOFAR software suite.
# The LOFAR software suite is free software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# The LOFAR software suite is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
#
# $Id$

from _stationresponse import StationResponse

class stationresponse(StationResponse):
    """
    The Python interface to StationResponse (LOFAR beam data)
    """

    def __init__ (self, msname, ra, dec, configname='', configpath='',
                  pahseCenterRa=-1e10, phaseCenterDec=-1e10):
        """Create the object for a given MS and beam direction.

        The MeasurementSet defines the LOFAR stations, the phase center, and
        the frequencies for which the stationreponse object is created for.
        The beam direction gives the RA and DEC for which the responses have
        to be calculated (using the function :func:`getJones`).

        `msname`
          Name of the MeasurementSet to use.
        `ra`
          RA of beam direction (in radians, J2000)
        `dec`
          DEC of beam direction (in radians, J2000)
        `configname`
          name of the Station Config files (e.g. LBA_INNER)
        `configPath`
          path to the Station config files (default is $LOFARDATA/share)
        `phaseCenterRa`
          RA of phase center direction (in radians, J2000).
          It defaults to the phase center in the MS.
        `phaseCenterDec`
          DEC of phase center direction (in radians, J2000).
          It defaults to the phase center in the MS.

        For example::

        import lofar.stationresponse
        sp = stationresponse('my.ms', 2.36, 0.854, 'LBA_INNER')
        # iterate over all times in the MS and get Jones for all stations
        # and all channels.
        t = table('my.ms')
        for t1 in t.iter('TIME'):
	    jones = sp.getJones (t1.getcell('TIME', 0))

        """
        StationResponse.__init__ (self, msname, ra, dec, configName,
                                  configPath, phaseCenterRa, phaseCenterDec);

    def version (self, type='other'):
        """Show the software version."""
        return self._version (type)

    def getJones (self, time)
        """Return Jones matrices for this time slot

        The Jones matrices for all stations and channels are returned
        as a 4-dim complex numpy array[nstation,nchannel,2,2].

        `time`
          Time as in MeasurementSet (MJD in seconds)

        """
        return self._getJones1 (time)

    def getJones (self, time, station)
        """Return Jones matrices for this time slot and station

        The Jones matrices for all channels are returned
        as a 3-dim complex numpy array[nchannel,2,2].

        `time`
          Time as in MeasurementSet (MJD in seconds)
        `station`
          Station number (as in MS)

        """
        return self._getJones2 (time, station)

    def getJones (self, time, station, channel)
        """Return Jones matrices for this time slot, station, and channel

        The Jones matrices for all channels are returned
        as a 2-dim complex numpy array[2,2].

        `time`
          Time as in MeasurementSet (MJD in seconds)
        `station`
          Station number (as in MS)
        `channel`
          Channel number

        """
        return self._getJones3 (time, station, channel)

