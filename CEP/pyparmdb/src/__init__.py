# __init__.py: Top level .py file for python parmdb interface
# Copyright (C) 2007
# ASTRON (Netherlands Foundation for Research in Astronomy)
# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# $Id$

from _parmdb import ParmDB

class parmdb(ParmDB):
    """
    The Python interface to ParmDB (calibration parameter tables)
    """

    def __init__ (self, dbname):
        ParmDB.__init__ (self, dbname);

    def version (self, type='other'):
        return self._version (type)

    def getRange (self, parmnamepattern=''):
        return self._getRange (parmnamepattern)

    def getNames (self, parmnamepattern=''):
        return self._getNames (parmnamepattern)

    def getValues (self, parmnamepattern, 
                   sfreq=-1e30, efreq=1e30,
                   stime=-1e30, etime=1e30,
                   asStartEnd=True):
        try:
            a = len(sfreq)
            # Domain given as vectors.
            return self._getValuesVec (parmnamepattern, sfreq, efreq,
                                       stime, etime, asStartEnd)
        except:
            # Domain given as scalars.
            return self._getValues (parmnamepattern, sfreq, efreq,
                                    stime, etime, asStartEnd)

    def getValues (self, parmnamepattern,
                   sfreq, efreq, freqstep,
                   stime, etime, timestep,
                   asStartEnd=True):
        return self._getValues (parmnamepattern, sfreq, efreq, freqstep,
                                stime, etime, timestep, asStartEnd)
        
    def getValuesGrid (self, parmnamepattern,
                       sfreq=-1e30, efreq=1e30,
                       stime=-1e30, etime=1e30,
                       asStartEnd=True):
        return self._getValuesGrid (parmnamepattern, sfreq, efreq,
                                    stime, etime, asStartEnd)

    def getCoeff (self, parmnamepattern,
                  sfreq=-1e30, efreq=1e30,
                  stime=-1e30, etime=1e30,
                  asStartEnd=True):
        return self._getCoeff (parmnamepattern, sfreq, efreq,
                               stime, etime, asStartEnd)
