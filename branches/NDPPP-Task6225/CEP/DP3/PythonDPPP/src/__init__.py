# __init__.py: Top level .py file for python DPPP step
# Copyright (C) 2015
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
# $Id: __init__.py 23074 2012-12-03 07:51:29Z diepen $

from lofar.pythondppp._pythondppp import _DPStepBase
import numpy as np


class DPStepBase(_DPStepBase):
    """
    The base class for a python DPPP step
    """

    def __init__(self):
        _DPStepBase.__init__(self)

    def _updateInfo(self, dpinfo):
        self.itsNCorrIn = dpinfo['NCorr']
        self.itsNChanIn = dpinfo['NChan']
        self.itsNBlIn   = len(dpinfo['Ant1'])
        self.itsNCorrOut = self.itsNCorrIn
        self.itsNChanOut = self.itsNChanIn
        self.itsNBlOut   = self.itsNBlIn
        infoOut = self.updateInfo(dpinfo)
        if 'NCorr' in infoOut:
            self.itsNCorrOut = infoOut['NCorr']
        if 'NChan' in infoOut:
            self.itsNChanOut = infoOut['NChan']
        if 'Ant1' in infoOut:
            self.itsNBlOut   = len(infoOut['Ant1'])
        return infoOut

    # The following functions can be overwritten in a derived class.
    def updateInfo(self, dpinfo):
        raise ValueError("A class derived from DPStepBase must implement updateInfo")

    def needVisData(self):
        return True

    def needWrite(self):
        return False

    def show(self):
        return ''

    def showCounts(self):
        return ''

    def addToMS(self, name):
        None


    # The following functions are to be called from Python.
    def processNext(self, arrs):
        return self._processNext(arrs)

    def getData(self, nparray):
        if not nparray.flags.c_contiguous  or  nparray.size == 0:
            raise ValueError("getData argument 'nparray' has to be a contiguous numpy a\
rray")
        return self._getData (nparray)

    def getFlags(self, nparray):
        if not nparray.flags.c_contiguous  or  nparray.size == 0:
            raise ValueError("getFlags argument 'nparray' has to be a contiguous numpy a\
rray")
        return self._getFlags (nparray)

    def getWeights(self, nparray):
        if not nparray.flags.c_contiguous  or  nparray.size == 0:
            raise ValueError("getWeights argument 'nparray' has to be a contiguous numpy a\
rray")
        return self._getWeights (nparray)

    def getUVW(self, nparray):
        if not nparray.flags.c_contiguous  or  nparray.size == 0:
            raise ValueError("getUVW argument 'nparray' has to be a contiguous numpy a\
rray")
        return self._getUVW (nparray)

    def makeArrayDataIn(self):
        return np.zeros([self.itsNBlIn, self.itsNChanIn, self.itsNCorrIn], dtype='complex64')

    def makeArrayFlagsIn(self):
        return np.zeros([self.itsNBlIn, self.itsNChanIn, self.itsNCorrIn], dtype='bool')

    def makeArrayWeightsIn(self):
        return np.zeros([self.itsNBlIn, self.itsNChanIn, self.itsNCorrIn], dtype='float32')

    def makeArrayUVWIn(self):
        return np.zeros([self.itsNBlIn, 3], dtype='float64')

