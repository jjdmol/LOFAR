# - Try to find OpenCL.
# Variables used by this module:
#  OPENCL_ROOT_DIR     - OPENCL root directory
# Variables defined by this module:
#  OPENCL_FOUND        - system has OPENCL
#  OPENCL_INCLUDE_DIR  - the OPENCL include directory (cached)
#  OPENCL_INCLUDE_DIRS - the OPENCL include directories
#                       (identical to OPENCL_INCLUDE_DIR)
#  OPENCL_LIBRARY      - the OPENCL library (cached)
#  OPENCL_LIBRARIES    - the OPENCL libraries
#                       (identical to OPENCL_LIBRARY)

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
# $Id: FindOpenCL.cmake 17486 2011-03-01 13:08:26Z mol $

if(NOT OPENCL_FOUND)

  # CentOS sets OPENCL_INCLUDE and OPENCL_LIB in the environment

  find_path(OPENCL_INCLUDE_DIR "CL/cl.h"
    HINTS ${OPENCL_ROOT_DIR} ENV OPENCL_INCLUDE PATH_SUFFIXES include)
  find_library(OPENCL_LIBRARY OpenCL
    HINTS ${OPENCL_ROOT_DIR} ENV OPENCL_LIB PATH_SUFFIXES lib)
  mark_as_advanced(OPENCL_INCLUDE_DIR OPENCL_LIBRARY)

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(OpenCL DEFAULT_MSG 
    OPENCL_LIBRARY OPENCL_INCLUDE_DIR)

  set(OPENCL_INCLUDE_DIRS ${OPENCL_INCLUDE_DIR})
  set(OPENCL_LIBRARIES ${OPENCL_LIBRARY})

endif(NOT OPENCL_FOUND)
