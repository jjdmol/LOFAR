# tPythonStep.py: Test python DPPP class
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


from lofar.pythondppp import DPStepBase
from lofar.parameterset import parameterset

class tPythonStep(DPStepBase):
    def __init__(self, parsetName):
        DPStepBase.__init__(self)
        itsParset = parameterset(parsetName)

    def updateInfo(self, dpinfo):
        print "did update"
        self.itsInfo = dpinfo
        res = {}
        self.itsData = self.makeArrayDataIn()
        self.itsFlags = self.makeArrayFlagsIn()
        self.itsWeights = self.makeArrayWeightsIn()
        self.itsUVW = self.makeArrayUVWIn()
        return res;

    def process(self):
        self.getData (self.itsData);
        self.getFlags (self.itsFlags);
        self.getWeights (self.itsWeights);
        self.getUVW (self.itsUVW);
        print "process tPythonStep", self.itsData.sum(), self.itsFlags.sum(), self.itsWeights.sum(), self.itsUVW.sum()
        return self.processNext ({})

    def finish(self):
        print "finish tPythonStep"

    def show(self):
        return "   **showtest**"

    def addToMS(self, msname):
        print "addToMS tPythonStep", msname
