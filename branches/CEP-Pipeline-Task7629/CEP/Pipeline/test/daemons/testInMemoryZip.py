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

import StringIO
import zipfile

buff = StringIO.StringIO()

zip_archive = zipfile.ZipFile(buff, mode='w')

data = ["data{0}".format(idx) for idx in range(1000)]

zip_archive.writestr(str(data))

zip_archive.close()

buff2 = StringIO.StringIO(buff.getvalue())

zip_archive = zipfile.ZipFile(buff2, mode='r')

data = eval(zip_archive.read())

print data

