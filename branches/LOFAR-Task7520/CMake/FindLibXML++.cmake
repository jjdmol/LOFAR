# - Try to find libxml++-2.6: A library to parse XML DOM trees
# Variables used by this module:
#  LIBXML_CPP_ROOT_DIR     - QPID root directory
# Variables defined by this module:
#  LIBXML_CPP_FOUND        - system has LIBXML_CPP
#  LIBXML_CPP_INCLUDE_DIR  - the LIBXML_CPP include directory (cached)
#  LIBXML_CPP_INCLUDE_DIRS - the LIBXML_CPP include directories
#                        (identical to LIBXML_CPP_INCLUDE_DIR)
#  LIBXML_CPP_LIBRARY      - the LIBXML_CPP library (cached)
#  LIBXML_CPP_LIBRARIES    - the LIBXML_CPP libraries
#                        (identical to LIBXML_CPP_LIBRARY)

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
# $Id$

if(NOT LIBXML_CPP_FOUND)

  find_path(LIBXML_CPP_INCLUDE_DIR libxml++/parsers/parser.h
    HINTS ${LIBXML_CPP_ROOT_DIR} PATH_SUFFIXES include include/libxml++-2.6)
  find_library(LIBXML_CPP_LIBRARY xml++-2.6
    HINTS ${LIBXML_CPP_ROOT_DIR} PATH_SUFFIXES lib)

  mark_as_advanced(LIBXML_CPP_INCLUDE_DIR LIBXML_CPP_LIBRARY)

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(LIBXML_CPP DEFAULT_MSG
    LIBXML_CPP_LIBRARY LIBXML_CPP_INCLUDE_DIR)

  set(LIBXML_CPP_INCLUDE_DIRS ${LIBXML_CPP_INCLUDE_DIR})
  set(LIBXML_CPP_LIBRARIES ${LIBXML_CPP_LIBRARY})

endif(NOT LIBXML_CPP_FOUND)
