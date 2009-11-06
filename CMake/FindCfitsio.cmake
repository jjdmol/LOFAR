# $Id: FindCfitsio.cmake 13814 2009-08-20 11:55:06Z loose $
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

# Try to find Cfitsio.
#
# This will define:
#  
#  CFITSIO_FOUND        - system has Cfitsio
#  CFITSIO_INCLUDE_DIR  - the Cfitsio include directory (cached)
#  CFITSIO_INCLUDE_DIRS - the Cfitsio include directories
#                         (identical to CFITSIO_INCLUDE_DIR)
#  CFITSIO_LIBRARY      - the Cfitsio library (cached)
#  CFITSIO_LIBRARIES    - the Cfitsio libraries
#                         (identical to CFITSIO_LIBRARY)

if(NOT CFITSIO_FOUND)

  find_path(CFITSIO_INCLUDE_DIR fitsio.h)
  find_library(CFITSIO_LIBRARY cfitsio)
  mark_as_advanced(CFITSIO_INCLUDE_DIR CFITSIO_LIBRARY)

  include(FindPackageHandleStandardArgs)
  set(custom_msg "Could NOT find Cfitsio in ${CMAKE_PREFIX_PATH}")
  find_package_handle_standard_args(Cfitsio "${custom_msg}"
    CFITSIO_LIBRARY CFITSIO_INCLUDE_DIR)

  set(CFITSIO_INCLUDE_DIRS ${CFITSIO_INCLUDE_DIR})
  set(CFITSIO_LIBRARIES ${CFITSIO_LIBRARY})

endif(NOT CFITSIO_FOUND)
