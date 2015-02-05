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

class DPStepBase(_DPStepBase):
    """
    The base class for a python DPPP step
    """

    def __init__(self):
        _DPStepBase.__init__(self)

    def updateInfo(self, dpinfo):
        itsInfo = dpinfo
        return {}

    def needVisData(self):
        return False

    def needWrite(self):
        return False

    def show(self):
        return ""

    def showCounts(self):
        return ""

    def addToMS(name):
        None
