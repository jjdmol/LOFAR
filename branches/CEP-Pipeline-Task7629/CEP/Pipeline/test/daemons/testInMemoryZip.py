#!/usr/bin/python
# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
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

from lofarpipe.daemons.inMemoryZip import get_zipstring_from_string, get_string_from_zipstring

# some data
data = ["data {0}".format(id) for id in range(1000)]

# zip and unzip
zip_string = get_zipstring_from_string(str(data))
data_string = get_string_from_zipstring(zip_string)

# cast to python list
data_unpacked = eval(data_string)

# check equality
if data != data_unpacked:
    raise Exception("Packed and unpacked data are not equal!!")


