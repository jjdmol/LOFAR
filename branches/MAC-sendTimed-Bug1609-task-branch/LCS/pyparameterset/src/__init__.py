# __init__.py: Top level .py file for python parametersetinterface
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

from _pyparameterset import PyParameterValue
from _pyparameterset import PyParameterSet


class parametervalue(PyParameterValue):
    """
    The Python interface to ParameterValue
    """

    def __init__(self, value, trim=True, _copyObj=False):
        """ Create the parametervalue object.

        value
          The parameter value as a string.
        trim
          True = remove leading/trailing whitespace from value.

          """
        if _copyObj == True:
            # Copy constructor
            PyParameterValue.__init__ (self, value)
        else:
            PyParameterValue.__init__ (self, value, trim);

    def __str__(self):
        """Get the full parameter value."""
        return self.get()

    def expand(self):
        """Expand the value."""
        return parametervalue(self._expand(), _copyObj=True)

    def getVector(self):
        """Get the value as a vector of values."""
        return [parametervalue(v, _copyObj=True) for v in self._getVector()]
        
    def getRecord(self):
        """Get the value as a record (as a parameterset object)."""
        return parameterset (self._getRecord(), _copyObj=True)




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
        return parametervalue (self._get(key), _copyObj=True)

    def makeSubset (self, baseKey, prefix=''):
        """Return a subset as a new parameterset object.
        
        baseKey
          The leading part of the parameter name denoting the subset.
          A trailing period needs to be given.
        prefix
          The baseKey parameter name part is replaced by this new prefix.
          The default new prefix is empty.

        For example::

          newps = ps.makeSubset ('p1.p2.', 'pr.')
          
        creates a subset of all keys starting with `p1.p2.` and replaces
        that prefix by `pr.`.

        """
        ps = self._makeSubset (baseKey, prefix)
        return parameterset (ps, _copyObj=True)

    def getVector(self, key):
        """Get the value as a vector of values."""
        return [parametervalue(v, _copyObj=True) for v in self._getVector(key)]
        
    def getRecord (self, key):
        """Get the value as a record."""
        ps = self._getRecord (key)
        return parameterset (ps, _copyObj=True)

    def keys(self):
        """Get the list of all parameter names."""
        return self.keywords()

    def dict(self):
        """Turn the parset into a dict"""
        d = {}
        for key in self.keys():
            d[key] = self.get(key).get()
        return d

    def get(self, key):
        """Get the parametervalue object of a parameter."""
        return parametervalue (self._get(key), _copyObj=True)

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

