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
    The Python interface to the LOFAR station beam model.
    """

    def __init__ (self, msname, inverse = False, useElementBeam = True,
        useArrayFactor = True, useChannelFreq = False, conjugateAF = False):
        """Create a stationresponse object that can be used to evaluate the
        LOFAR beam for the given Measurement Set.

        The Measurement Set defines the station and dipole positions, the phase
        center, and the channel frequencies (and reference frequency) for which
        the LOFAR beam will be evaluated.

        `msname`
          Name of the Measurement Set.
        `inverse`
          Compute the inverse of the LOFAR beam (default False).
        `useElementBeam`
          Include the effect of the dual dipole (element) beam (default True).
        `useArrayFactor`
          Include the effect of the station and tile array factor (default
          True).
        `useChannelFreq`
          Compute the phase shift for the station beamformer using the channel
          frequency instead of the subband reference frequency. This option
          should be enabled for Measurement Sets that contain multiple subbands
          compressed to single channels inside a single spectral window
          (default: False).
        `conjugateAF`
          Conjugate the station and tile array factors (default False).

        For example::

        import pyrap.tables
        import lofar.stationresponse
        response = lofar.stationresponse.stationresponse('test.MS')

        # Iterate over all time stamps in the Measurement Set and compute the
        # beam Jones matrix for station 0, channel 0.
        ms = pyrap.tables.table('test.MS')
        for subtable in ms.iter('TIME'):
            time = subtable.getcell("TIME", 0)
            print time, response.evaluateChannel(time, 0, 0)
        """
        StationResponse.__init__ (self, msname, inverse, useElementBeam,
          useArrayFactor, useChannelFreq, conjugateAF)

    def version (self, type='other'):
        """Show the software version."""
        return self._version (type)

    def setRefDelay (self, ra, dec):
        """Set the reference direction used by the station beamformer. By
        default, DELAY_DIR of field 0 is used.

        `ra`
          Right ascension (in radians, J2000)
        `dec`
          Declination (in radians, J2000)
        """
        self._setRefDelay(ra, dec)

    def setRefTile (self, ra, dec):
        """Set the reference direction used by the analog tile beamformer
        (relevant for HBA observations only). By default, LOFAR_TILE_BEAM_DIR
        of field 0 is used. If not present, DELAY_DIR of field 0 is used
        instead.

        `ra`
          Right ascension (in radians, J2000)
        `dec`
          Declination (in radians, J2000)
        """
        self._setRefTile(ra, dec)

    def setDirection (self, ra, dec):
        """Set the direction of interest (can be and often will be different
        from the pointing). By default, PHASE_DIR of field 0 is used.

        `ra`
          Right ascension (in radians, J2000)
        `dec`
          Declination (in radians, J2000)
        """
        self._setDirection(ra, dec)

    def evaluate (self, time):
        """Compute the beam Jones matrix for all stations and channels at the
        given time. The result is returned as a 4-dim complex numpy array with
        shape: no. of stations x no. of channels x 2 x 2.

        `time`
          Time (MJD in seconds)
        """
        return self._evaluate0 (time)

    def evaluateStation (self, time, station):
        """Compute the beam Jones matrix for all channels at the given time for
        the given station. The result is returned as a 3-dim complex numpy array
        with shape: no. of channels x 2 x 2.

        `time`
          Time (MJD in seconds).
        `station`
          Station number (as in the ANTENNA table of the Measurement Set).
        """
        return self._evaluate1 (time, station)

    def evaluateChannel (self, time, station, channel):
        """Compute the beam Jones matrix for the given time, station, and
        channel. The result is returned as a 2-dim complex numpy array with
        shape: 2 x 2.

        `time`
          Time (MJD in seconds).
        `station`
          Station number (as defined in the ANTENNA table of the Measurement
          Set).
        `channel`
          Channel number (as in the SPECTRAL_WINDOW table of the Measurement
          Set).
        """
        return self._evaluate2 (time, station, channel)
