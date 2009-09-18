# __init__.py: Top level .py file for python parmdb interface
# Copyright (C) 2007
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

from _parmdb import ParmDB

class parmdb(ParmDB):
    """
    The Python interface to ParmDB (calibration parameter tables)
    """

    def __init__ (self, dbname):
        """Open the given parameter database.

        The database can be a local one given by the name of the main table
        or it can be a distributed one given by the name of its global VDS
        file. In the latter case parmdbremote processes will be started on
        nodes having access to the distributed database parts. The processes
        will be ended when closing the database.

        For example::

        import lofar.parmdb
        pdb = parmdb(dbname)     # open
	pdb = 0                  # close

        """
        ParmDB.__init__ (self, dbname);

    def version (self, type='other'):
        """Show the software version."""
        return self._version (type)

    def getRange (self, parmnamepattern=''):
        """Return the overall domain of the matching parameter names.

        The pattern must be given as a shell-like filename pattern.
        An empty pattern (the default) means '*' (thus all names).

        The range is returned as [startfreq, endfreq, starttime, endtime].

        """
        return self._getRange (parmnamepattern)

    def getNames (self, parmnamepattern=''):
	"""Return the list of matching parameter names.

        The pattern must be given as a shell-like filename pattern.
        An empty pattern (the default) means '*' (thus all names).

        """
        return self._getNames (parmnamepattern)

    def getValues (self, parmnamepattern, 
                   sfreq=-1e30, efreq=1e30,
                   stime=-1e30, etime=1e30,
                   asStartEnd=True):
        """Get the parameter values for the given names and grid.

        The values are calculated on a grid defined by the given frequency
        and time domain(s). If the domain values are given as scalar values,
        they are treated as the boundary box of the grid. The grid itself is
        defined by the default frequency and time grid width in the parameter
        database which is normally equal to those in the MeasurementSet.
        If the domain values are given as vectors, they are treated as defining
        the grid.

        parmnamepattern
          Parameter name pattern given as a shell-like filename pattern.
        sfreq
          Starts or centers of frequency grid (in Hz)
        efreq
          Ends or widths of frequency grid (in Hz)
        stime
          Starts or centers of time grid (in MJD seconds)
        etime
          Ends or widths of time grid (in MJD seconds)
        asStartEnd
          True  = frequency and time given as start/end (default)
          False = given as center/width.

        The values are returned as a dict mapping each parameter name to a
        subdict. Each subdict has the fields `values`, `freqs`, `freqwidths`,
        `times`, and `timewidths`. The first field gives the values (as a 2D
        array) at the grid points. The other fields define the used grid as
        center/width.
          
        """
        try:
            a = len(sfreq)
            # Domain given as vectors.
            return self._getValuesVec (parmnamepattern, sfreq, efreq,
                                       stime, etime, asStartEnd)
        except:
            # Domain given as scalars.
            return self._getValues (parmnamepattern, sfreq, efreq,
                                    stime, etime, asStartEnd)

    def getValuesStep (self, parmnamepattern,
                       sfreq, efreq, freqstep,
                       stime, etime, timestep,
                       asStartEnd=True):
        """Get the parameter values for the given names and domain.

        The values are calculated on a grid defined by the given frequency
        and time domain and the steps in it. It is similar to function
        :func:`getValues`.

        parmnamepattern
          Parameter name pattern given as a shell-like filename pattern.
        sfreq
          Start or center of frequency domain (in Hz)
        efreq
          End or width of frequency domain (in Hz)
        freqstep
          The step size of the grid's frequency axis.
          If <=0, the default in the parameter database is used.
        stime
          Start or center of time domain (in MJD seconds)
        etime
          End or width of time domain (in MJD seconds)
        timestep
          The step size of the grid's time axis.
          If <=0, the default in the parameter database is used.
        asStartEnd
          True  = frequency and time given as start/end (default)
          False = given as center/width.

          """
        return self._getValues (parmnamepattern, sfreq, efreq, freqstep,
                                stime, etime, timestep, asStartEnd)
        
    def getValuesGrid (self, parmnamepattern,
                       sfreq=-1e30, efreq=1e30,
                       stime=-1e30, etime=1e30,
                       asStartEnd=True):
        """Get the parameter values for the given names and domain.

        It is similar to function :func:`getValues`.
        The difference is in the grid used for scalar parameter values.
        In `getValues` the given grid is used, while in `getValuesGrid`
        the grid is used with which the scalar parameter values are stored.

        """
        return self._getValuesGrid (parmnamepattern, sfreq, efreq,
                                    stime, etime, asStartEnd)

    def getCoeff (self, parmnamepattern,
                  sfreq=-1e30, efreq=1e30,
                  stime=-1e30, etime=1e30,
                  asStartEnd=True):
        """Get the parameter coefficients and errors for the given names and domain.

        For scalar parameter values it is the same as :func:`getValuesGrid`
        with the exception that each subdict also contains a field `errors`.
        For parameters represented as funklets, the returned `values` array 
        has shape `[nt,nf,nct,ncf]` and contains the funklets coefficients.
        The `errors` field is a similar array containing the coefficient errors.
        In the shape `nt` and `nf` represent the number of times and frequencies
        (as given in the returned grid), while `nct` and `ncf` are the number
        of coefficients for each funklet in time and frequencies.

        Undefined coefficient errors are returned as -1.

        """
        return self._getCoeff (parmnamepattern, sfreq, efreq,
                               stime, etime, asStartEnd)
