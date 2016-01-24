# - Try to find the CUDA driver library
# Variables used by this module:
#  CUDADRIVER_ROOT_DIR     - CUDADRIVER root directory
# Variables defined by this module:
#  CUDADRIVER_FOUND        - system has CUDADRIVER
#  CUDADRIVER_LIBRARY      - the CUDADRIVER library (cached)
#  CUDADRIVER_LIBRARIES    - the CUDADRIVER libraries
#                       (identical to CUDADRIVER_LIBRARY)

# Copyright (C) 2009
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
# $Id: FindCUDADRIVER.cmake 21886 2012-09-04 11:57:26Z mol $

if(NOT CUDADRIVER_FOUND)
  find_library(CUDADRIVER_LIBRARY cuda
    HINTS ${CUDADRIVER_ROOT_DIR} $ENV{CUDA_ROOT} /usr/lib/nvidia-current PATH_SUFFIXES lib64 lib)
  mark_as_advanced(CUDADRIVER_LIBRARY)

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(CUDADRIVER DEFAULT_MSG 
    CUDADRIVER_LIBRARY)

  set(CUDADRIVER_LIBRARIES ${CUDADRIVER_LIBRARY})

endif(NOT CUDADRIVER_FOUND)
