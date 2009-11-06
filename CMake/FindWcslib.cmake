# $Id: FindWcslib.cmake 13814 2009-08-20 11:55:06Z loose $
#
# Copyright (C) 2008-2009
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

# Try to find Wcslib.
#
# This will define:
#  
#  WCSLIB_FOUND        - system has Wcslib
#  WCSLIB_INCLUDE_DIR  - the Wcslib include directory (cached)
#  WCSLIB_INCLUDE_DIRS - the Wcslib include directories
#                         (identical to WCSLIB_INCLUDE_DIR)
#  WCSLIB_LIBRARY      - the Wcslib library (cached)
#  WCSLIB_LIBRARIES    - the Wcslib libraries
#                         (identical to WCSLIB_LIBRARY)

if(NOT WCSLIB_FOUND)

  find_path(WCSLIB_INCLUDE_DIR wcslib/wcs.h)
  find_library(WCSLIB_LIBRARY wcs)
  mark_as_advanced(WCSLIB_INCLUDE_DIR WCSLIB_LIBRARY)

  include(FindPackageHandleStandardArgs)
  set(custom_msg "Could NOT find Wcslib in ${CMAKE_PREFIX_PATH}")
  find_package_handle_standard_args(Wcslib "${custom_msg}"
    WCSLIB_LIBRARY WCSLIB_INCLUDE_DIR)

  set(WCSLIB_INCLUDE_DIRS ${WCSLIB_INCLUDE_DIR})
  set(WCSLIB_LIBRARIES ${WCSLIB_LIBRARY})

endif(NOT WCSLIB_FOUND)
