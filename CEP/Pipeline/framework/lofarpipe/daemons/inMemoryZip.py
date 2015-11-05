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

TEMP_FILENAME_IN_ZIP = "temp.filename"

# TODO: The module cStringIO provides an interface similar to that of the 
# StringIO module. Heavy use of StringIO.StringIO objects can be made 
# more efficient by using the function StringIO() from this module instead.

def get_zipstring_from_string(string_data):
    """
    get_zipstring_from_string expects a string, created a compressed an 
    inmemory zip file. The contents are returned as a string
    """
    if not isinstance(string_data, basestring):
      raise TypeError("get_zipstring_from_string only accepts strings")

    # Create an in memory file like io object
    buff = StringIO.StringIO()
    # create a zip_file on this file object
    zip_archive = zipfile.ZipFile(buff, 'w', 
                                  zipfile.ZIP_DEFLATED) # Zip the data

    #Put data in the archive
    zip_archive.writestr(TEMP_FILENAME_IN_ZIP, str(string_data))
    # Close the file before getting the buffer!!
    zip_archive.close()
    packed_data = buff.getvalue()

    # Release memory   
    buff.close() 
    return packed_data

def get_string_from_zipstring(zip_string):
    """  
    get_string_from_zipstring expects a string contianing a string that
    can be interpreted as a zip-file. Data is unpacked in memory and returned 
    as a string"""

    if not isinstance(zip_string, basestring):
      raise TypeError("get_string_from_zipstring only accepts strings")

    # new file like object, init with the data of the first io object
    buff = StringIO.StringIO(zip_string)
    # open as zip_archive
    zip_archive = zipfile.ZipFile(buff, 'r')
    # unpack the archive

    data_string = zip_archive.read(TEMP_FILENAME_IN_ZIP)
    zip_archive.close()
    # Release memory
    
    buff.close() 
    return data_string

