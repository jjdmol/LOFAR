# $Id: FindBlas.cmake 13814 2009-08-20 11:55:06Z loose $
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

# Try to find Blas.
#
# This will define:
#  
#  BLAS_FOUND        - system has Blas
#  BLAS_INCLUDE_DIR  - undefined
#  BLAS_INCLUDE_DIRS - 
#                         (identical to BLAS_INCLUDE_DIR)
#  BLAS_LIBRARY      - the Blas library (cached)
#  BLAS_LIBRARIES    - the Blas libraries
#                         (identical to BLAS_LIBRARY)

if(NOT BLAS_FOUND)

  find_library(BLAS_LIBRARY blas)
  mark_as_advanced(BLAS_LIBRARY)

  include(FindPackageHandleStandardArgs)
  set(custom_msg "Could NOT find Blas in ${CMAKE_PREFIX_PATH}")
  find_package_handle_standard_args(Blas "${custom_msg}"
    BLAS_LIBRARY)

  set(BLAS_LIBRARIES ${BLAS_LIBRARY})

endif(NOT BLAS_FOUND)
