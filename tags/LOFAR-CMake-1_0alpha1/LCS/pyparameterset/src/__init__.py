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

    def __init__(self, filename=None, caseInsensitive=False):
        """Create a parameterset object.

        filename
          If a filename is given, the object is filled from that parset file.
          If a bool is given, it is treated as argument caseInsensitive.
        caseInsensitive
          True = parameter names are case insensitive

        """
        if filename==None:
            PyParameterSet.__init__ (self, caseInsensitive);
        elif isinstance(filename, bool):
            PyParameterSet.__init__ (self, filename);
        else:
            PyParameterSet.__init__ (self, filename, caseInsensitive);

    def __len__(self):
        """Get the number of parameters."""
        return self.size()

    def __getitem__(self, key):
        """Get the parametervalue object of a parameter."""
        return self._get (key)

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

