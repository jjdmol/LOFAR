# factory.py: Generic object factory.
#
# Copyright (C) 2015
# ASTRON (Netherlands Institute for Radio Astronomy)
# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
#
# This file is part of the LOFAR software suite.
# The LOFAR software suite is free software: you can redistribute it
# and/or modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# The LOFAR software suite is distributed in the hope that it will be
# useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
#
# $Id: factory.py 1584 2015-10-02 12:10:14Z loose $

"""
Module for a generic object factory.
"""

class Factory(object):
    """Generic object factory.

    Factory allows you to create any object that has been registered with it, by
    using its `clsid`. The `clsid` can be any type that is usable as a
    dictionary key. The `create` method is responsible for creating the object;
    it passes `args` and `kwargs` to the constructor of the class that must be
    instantiated.
    """

    def __init__(self):
        """
        Constructor.
        """
        self.__registry = dict()

    def register(self, clsid, cls):
        """
        Register the class `cls` using `clsid` as its unique identifier.
        Registration will fail if `clsid` already exists in the registry.

        :return: True if registration was successful; otherwise False.
        """
        return self.__registry.setdefault(clsid, cls) == cls

    def deregister(self, clsid):
        """
        Deregister the class identified by `clsid`.
        The class identified by `clsid` will be removed from registry.

        :return: True if deregistration was successful; otherwise False.

        """
        return self.__registry.pop(clsid, None) is not None

    def create(self, clsid, *args, **kwargs):
        """
        Create a new class instance of the class identified by `clsid`.
        Positional arguments `args` and keyword arguments `kwargs` are passed
        to the constructor of the class.

        :return: instance of class identified by `clsid`, or None if `clsid`
        cannnot be found in the registry.

        """
        if clsid in self.__registry:
            return self.__registry.get(clsid)(*args, **kwargs)
        else:
            return None

    def registry(self):
        """
        Return a copy of the contents of the registry.
        """
        return self.__registry.copy()
