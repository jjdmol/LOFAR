# __init__.py: Top level .py file for python parameterset interface
# Copyright (C) 2008
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

from _pyparameterset import ParameterValue
from _pyparameterset import PyParameterSet


class parametervalue(ParameterValue):
    """
    The Python interface to ParameterValue
    """

    def __init__(self, value, trim=True):
        ParameterValue.__init__ (self, value, trim);

    def __str__(self):
        return self.get()


class parameterset(PyParameterSet):
    """
    The Python interface to ParameterSet
    """

    def __init__(self, filename=None, caseInsensitive=False):
        if filename==None:
            PyParameterSet.__init__ (self, caseInsensitive);
        elif isinstance(filename, bool):
            PyParameterSet.__init__ (self, filename);
        else:
            PyParameterSet.__init__ (self, filename, caseInsensitive);

    def __getitem__(self, key):
        return self.get (key)

    def getBoolVector(self, key, default=None, expandable=False):
        if default==None:
            return self._getBoolVector1 (key, expandable)
        if isinstance(default, bool):
            return self._getBoolVector1 (key, default)
        return self._getBoolVector2 (key, default, expandable)

    def getIntVector(self, key, default=None, expandable=False):
        if default==None:
            return self._getIntVector1 (key, expandable)
        if isinstance(default, bool):
            return self._getIntVector1 (key, default)
        return self._getIntVector2 (key, default, expandable)

    def getFloatVector(self, key, default=None, expandable=False):
        if default==None:
            return self._getFloatVector1 (key, expandable)
        if isinstance(default, bool):
            return self._getFloatVector1 (key, default)
        return self._getFloatVector2 (key, default, expandable)

    def getDoubleVector(self, key, default=None, expandable=False):
        if default==None:
            return self._getDoubleVector1 (key, expandable)
        if isinstance(default, bool):
            return self._getDoubleVector1 (key, default)
        return self._getDoubleVector2 (key, default, expandable)

    def getStringVector(self, key, default=None, expandable=False):
        if default==None:
            return self._getStringVector1 (key, expandable)
        if isinstance(default, bool):
            return self._getStringVector1 (key, default)
        return self._getStringVector2 (key, default, expandable)
