# __init__.py: Top level .py file for python parameterset interface
# Copyright (C) 2008
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

from _pyparameterset import ParameterValue
from _pyparameterset import PyParameterSet


class parametervalue(ParameterValue):
    """
    The Python interface to ParameterValue
    """

    def __init__(self, value, trim=True):
        """ Create the parametervalue object.

        value
          The parameter value as a string.
        trim
          True = remove leading/trailing whitespace from value.

          """
        ParameterValue.__init__ (self, value, trim);

    def __str__(self):
        """Get the full parameter value."""
        return self.get()




class parameterset(PyParameterSet):
    """
    The Python interface to ParameterSet
    """

    def __init__(self, filename=None, caseInsensitive=False, _copyObj=False):
        """Create a parameterset object.

        filename
          If a filename is given, the object is filled from that parset file.
          If a bool is given, it is treated as argument caseInsensitive.
        caseInsensitive
          True = parameter names are case insensitive

        """
        if _copyObj == True:
            # Copy constructor
            PyParameterSet.__init__ (self, filename)
        elif filename==None:
            PyParameterSet.__init__ (self, caseInsensitive, 0, 0);
        elif isinstance(filename, bool):
            # Here filename argument means caseInsensitive
            PyParameterSet.__init__ (self, filename, 0, 0);
        else:
            PyParameterSet.__init__ (self, filename, caseInsensitive);

    def __len__(self):
        """Get the number of parameters."""
        return self.size()

    def __getitem__(self, key):
        """Get the parametervalue object of a parameter."""
        return self._get (key)

    def makeSubset (self, prefix):
        """Get a subset of all keys starting with the given prefix"""
        ps = self._makeSubset (prefix)
        return parameterset (ps, _copyObj=True)

    def keys(self):
        """Get a sorted list of all parameter names."""
        return self.keywords()

    def get(self, key):
        """Get the parametervalue object of a parameter."""
        return self._get (key)

    def getBoolVector(self, key, default=None, expandable=False):
        """Get the value as a list of boolean values.

        key
          Parameter name
        default
          Default value to be used if parameter is undefined.
          If None is given, an exception is raised if undefined.
        expandable
          True = ranges and repeats (.. and *) are expanded first.

        """
        if default==None:
            return self._getBoolVector1 (key, expandable)
        if isinstance(default, bool):
            return self._getBoolVector1 (key, default)
        return self._getBoolVector2 (key, default, expandable)

    def getIntVector(self, key, default=None, expandable=False):
        """Get the value as a list of integer values.

        key
          Parameter name
        default
          Default value to be used if parameter is undefined.
          If None is given, an exception is raised if undefined.
        expandable
          True = ranges and repeats (.. and *) are expanded first.

        """
        if default==None:
            return self._getIntVector1 (key, expandable)
        if isinstance(default, bool):
            return self._getIntVector1 (key, default)
        return self._getIntVector2 (key, default, expandable)

    def getFloatVector(self, key, default=None, expandable=False):
        """Get the value as a list of floating point values.

        key
          Parameter name
        default
          Default value to be used if parameter is undefined.
          If None is given, an exception is raised if undefined.
        expandable
          True = ranges and repeats (.. and *) are expanded first.

        """
        if default==None:
            return self._getFloatVector1 (key, expandable)
        if isinstance(default, bool):
            return self._getFloatVector1 (key, default)
        return self._getFloatVector2 (key, default, expandable)

    def getDoubleVector(self, key, default=None, expandable=False):
        """Get the value as a list of floating point values.

        key
          Parameter name
        default
          Default value to be used if parameter is undefined.
          If None is given, an exception is raised if undefined.
        expandable
          True = ranges and repeats (.. and *) are expanded first.

        """
        if default==None:
            return self._getDoubleVector1 (key, expandable)
        if isinstance(default, bool):
            return self._getDoubleVector1 (key, default)
        return self._getDoubleVector2 (key, default, expandable)

    def getStringVector(self, key, default=None, expandable=False):
        """Get the value as a list of string values.

        key
          Parameter name
        default
          Default value to be used if parameter is undefined.
          If None is given, an exception is raised if undefined.
        expandable
          True = ranges and repeats (.. and *) are expanded first.

        """
        if default==None:
            return self._getStringVector1 (key, expandable)
        if isinstance(default, bool):
            return self._getStringVector1 (key, default)
        return self._getStringVector2 (key, default, expandable)

