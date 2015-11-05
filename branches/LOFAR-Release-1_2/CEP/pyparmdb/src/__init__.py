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

    def __init__ (self, dbname, create=False):
        """Open or create the given parameter database.

        The database can be a local one given by the name of the main table
        or it can be a distributed one given by the name of its global VDS
        file. In the latter case parmdbremote processes will be started on
        nodes having access to the distributed database parts. The processes
        will be ended when closing the database.

        If a new parameter database is created, it will be a local one.

        For example::

        import lofar.parmdb
        pdb = parmdb(dbname)     # open existing
	pdb = 0                  # close

        Almost all functions work on a local as well as a distributed database.
        The exception is :func:`addValues`. For the time being it only works
        for a local database.
        """
        ParmDB.__init__ (self, dbname, create);

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
	"""Return the list of matching parameter names with actual values.

        The pattern must be given as a shell-like filename pattern.
        An empty pattern (the default) means '*' (thus all names).

        """
        return self._getNames (parmnamepattern)

    def getDefNames (self, parmnamepattern=''):
	"""Return the list of matching parameter names with default values.

        The pattern must be given as a shell-like filename pattern.
        An empty pattern (the default) means '*' (thus all names).

        """
        return self._getDefNames (parmnamepattern)

    def getDefValues (self, parmnamepattern=''):
	"""Return the default values of matching parameter names as a dict.

        The pattern must be given as a shell-like filename pattern.
        An empty pattern (the default) means '*' (thus all names).

        """
        return self._getDefValues (parmnamepattern)

    def getValues (self, parmnamepattern, 
                   sfreq=-1e30, efreq=1e30,
                   stime=-1e30, etime=1e30,
                   asStartEnd=True):
        """Return the parameter values for the given names and grid as a dict.

        The values are calculated on a grid defined by the given frequency
        and time domain(s). If the domain values are given as scalar values,
        they are treated as the boundary box of the grid. The grid itself is
        defined by the default frequency and time grid width in the parameter
        database which are normally equal to those in the MeasurementSet.
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
        """Get the parameter values for the given names and domain as a dict.

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
        """Get the parameter values for the given names and domain as a dict.

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

    def clearTables (self):
        """Clear the tables in the ParmDB.

        All values and default values will be removed.

        """
        self._clearTables()

    def flush (self, fsync=False):
        """Flush and optionally fsync the tables in the ParmDB."""
        self._flush (fsync)

    def lock (self, lockForWrite):
        """Lock tables in the ParmDB for read or writing."""
        self._lock (lockForWrite)

    def unlock (self):
        """Unlock and flush the tables in the ParmDB."""
        self._unlock()

    def getDefaultSteps (self):
        """Get the default grid steps in freq and time."""
        return self._getDefaultSteps()

    def setDefaultSteps (self, steps):
        """Set the default grid steps in freq and time."""
        return self._setDefaultSteps (steps)

    def addDefValues (self, nameValue, value=None, check=True):
        """Put one or more default values.

        nameValue
          It can be a dict containing one or more name:value pairs where each
          value is a dict defining the parameter's default value.
          | The alternative is that it defines the parameter name and the
          value is given in the next argument.
        value
          It can be a dict defining the default value, and optional meta info.
          The function :func:`makeDefValue` should be used to create the dict.
          | The alternative is that the value is a scalar value or a 2-dim
          numpy array defining the parameter value which will be turned into
          a dict automatically using the defaults for the meta info.
        check
          True means check if the parameter does not already exist.

        """
        if isinstance(nameValue, dict):
            self._addDefValues (nameValue, check)
        elif isinstance(value, dict):
            self._addDefValues ({nameValue: value}, check)
        else:
            self._addDefValues ({nameValue: self.makeDefValue(value)}, check)

    def makeDefValue (self, value, type='', pert=1e-6, pertrel=True, mask=[]):
        """Create a dict containing the default value of a parameter

        value
          A single scalar value or a 2-dim numpy array containing polynomial
          coefficients.
        type
          A string defining the parameter type (Scalar or Polc).
          If empty, the type is Scalar if the value contains 1 value,
          otherwise it is Polc.
        pert
          Perturbation to use for numerical differentation.
        pertrel
          Is the perturbation is relative or absolute.
        mask
          A boolean array telling which polynomial coefficients are
          solvable. Default c[i,j] with i+j>max(shape) is not solvable.

        """
        d = {'value': value, 'pert':pert, 'pertrel':pertrel}
        if len(type) > 0:
            d['type'] = type
        if len(mask) > 0:
            d['mask'] = mask
        return d

    def deleteDefValues (self, parmNamePattern):
        """Remove the default value of parameters matching the pattern

        """
        self._deleteDefValues (parmNamePattern)

    def deleteValues (self, parmNamePattern, sfreq=-1e30, efreq=1e30,
                      stime=-1e30, etime=1e30, asStartEnd=True):
        """Remove the default value of parameters matching the pattern and domain

        """
        self._deleteValues (parmNamePattern, sfreq, efreq, stime, etime,
                            asStartEnd)

    def toCenterWidth (self, st, end):
        import numpy
        sv = numpy.array(st)
        ev = numpy.array(end)
        # Result might get a float, so make a numpy.array again.
        c = numpy.array(0.5 * (ev + sv))
        w = numpy.array(ev - sv)
        return (c.tolist(), w.tolist())

    def makeValue (self, values, sfreq, efreq,
                   stime, etime, asStartEnd,
                   type='', pert=1e-6, pertrel=True, mask=[]):
        """Create a dict containing the value and domain of a parameter

        This function is a helper for function :func:`addValues`, but it
        can also be used in itself to create a dict containing multiple
        parameter/value pairs.

        values
          The scalar value(s) or polynomial coefficients.
        sfreq
          Starts or centers of frequency grid (in Hz)
        efreq
          Ends or widths of frequency grid (in Hz)
        stime
          Starts or centers of time grid (in MJD seconds)
        etime
          Ends or widths of time grid (in MJD seconds)
        asStartEnd
          True  = frequency and time are given as start/end (default)
          False = given as center/width.
        type
          A string defining the parameter type (Scalar or Polc).
          If empty, the type defaults to Scalar.
        pert
          Perturbation to use for numerical differentation.
        pertrel
          Is the perturbation is relative or absolute.
        mask
          A boolean array telling which polynomial coefficients are
          solvable. Default c[i,j] with i+j>max(shape) is not solvable.

        The domain is defined by the fields s/efreq and s/etime.
        They can define a domain consisting of a single cell or a domain
        consisting of multiple cells.
        If the parameter is a polynomial, only one cell can be used.
        If the parameter is a scalar, the number of cells per axis should
        be 1 or should match the number of values for that axis. If it is 1,
        the cell will be divided uniformly into smaller cells, one for each
        value for that axis.

        """
        if asStartEnd:
            # Convert start/end to center/width
            (freqc,freqw) = self.toCenterWidth (sfreq, efreq)
            (timec,timew) = self.toCenterWidth (stime, etime)
            d = {'values': values,
                 'freqs': freqc, 'freqwidths':freqw,
                 'times': timec, 'timewidths':timew,
                 'pert':pert, 'pertrel':pertrel}
        else:
            d = {'values': values,
                 'freqs': sfreq, 'freqwidths':efreq,
                 'times': stime, 'timewidths':etime,
                 'pert':pert, 'pertrel':pertrel}
        if len(type) > 0:
            d['type'] = type
        if len(mask) > 0:
            d['mask'] = mask
        return d

    def addValues (self, nameValue, value=None, sfreq=None, efreq=None,
                   stime=None, etime=None, asStartEnd=True, type=''):
        """Add one or more values for parameters/domains.

        The parameter values are added to the database for the given domain(s).
        An exception is raised if values already exist for a parameter and
        (part of) its domain.
        Also an exception is raised if a parameter already exists and the
        given type or shape mismatches the existing one.

        It is possible to give a single parameter and value, but it
        is also possible to give a dict containing multiple parameter/value
        pairs. In general writing multiple values is better, because it
        can reduce the automatic table lock/unlock behaviour. However, it
        is possible to explicitly lock/unlock the tables (see :func:`lock`).

        nameValue
          It can be a dict containing one or more name/value pairs where each
          value is a dict defining the parameter's value and domain.
          | The alternative is that it defines the parameter name and the
          value/domain is given in the next argument(s).
        value
          It can be a dict defining the value, domain, and optional meta info.
          The function :func:`makeValue` should be used to create the dict.
          | The alternative is that the value is a scalar value or a 2-dim
          numpy array defining the parameter value. The subsequent arguments
          define the domain of the value.
        sfreq,efreq,stime,etime,asStartEnd,type
          See function :func:`makeValue` for a description of these arguments.

        """
        if isinstance(nameValue, dict):
            self._addValues (nameValue)
        elif isinstance(value, dict):
            self._addValues ({nameValue: value})
        else:
            self._addValues ({nameValue: self.makeValue(value,
                                                        sfreq, efreq,
                                                        stime, etime,
                                                        asStartEnd, type)})

