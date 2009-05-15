# $Id$
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

# Try to find FFTW3.
#
# This will define:
#  
#  FFTW3_FOUND        - system has FFTW3
#  FFTW3_INCLUDE_DIR  - the FFTW3 include directory (cached)
#  FFTW3_INCLUDE_DIRS - the FFTW3 include directories
#                       (identical to FFTW3_INCLUDE_DIR)
#  FFTW3_LIBRARY      - the FFTW3 library (cached)
#  FFTW3_LIBRARIES    - the FFTW3 libraries
#                       (identical to FFTW3_LIBRARY)

if(NOT FFTW3_FOUND)

  find_path(FFTW3_INCLUDE_DIR fftw3.h)
  find_library(FFTW3_LIBRARY fftw3)
  find_library(FFTW3F_LIBRARY fftw3f)

  message("FFTW3_LIBRARY = ${FFTW3_LIBRARY}")
  message("FFTW3F_LIBRARY = ${FFTW3F_LIBRARY}")

  include(FindPackageHandleStandardArgs)
  set(custom_msg "Could NOT find FFTW3 in ${CMAKE_PREFIX_PATH}")
  find_package_handle_standard_args(FFTW3 "${custom_msg}"
    FFTW3_LIBRARY FFTW3F_LIBRARY FFTW3_INCLUDE_DIR)

  set(FFTW3_INCLUDE_DIRS ${FFTW3_INCLUDE_DIR})
  set(FFTW3_LIBRARIES ${FFTW3_LIBRARY} ${FFTW3F_LIBRARY})

endif(NOT FFTW3_FOUND)
