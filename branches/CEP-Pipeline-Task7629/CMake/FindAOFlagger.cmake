# - Try to find AOFlagger.
# Variables used by this module:
#  AOFLAGGER_ROOT_DIR     - AOFlagger root directory
# Variables defined by this module:
#  AOFLAGGER_FOUND        - system has AOFlagger
#  AOFLAGGER_INCLUDE_DIR  - the AOFlagger include directory (cached)
#  AOFLAGGER_INCLUDE_DIRS - the AOFlagger include directories
#                         (identical to AOFLAGGER_INCLUDE_DIR)
#  AOFLAGGER_LIBRARY      - the AOFlagger library (cached)
#  AOFLAGGER_LIBRARIES    - the AOFlagger libraries
#                         (identical to AOFLAGGER_LIBRARY)

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
# $Id: FindAOFlagger.cmake 15228 2010-03-16 09:27:26Z loose $

if(NOT AOFLAGGER_FOUND)

  find_path(AOFLAGGER_INCLUDE_DIR aoflagger.h
    HINTS ${AOFLAGGER_ROOT_DIR} PATH_SUFFIXES include)
  find_library(AOFLAGGER_LIBRARY aoflagger
    HINTS ${AOFLAGGER_ROOT_DIR} PATH_SUFFIXES lib)
  mark_as_advanced(AOFLAGGER_INCLUDE_DIR AOFLAGGER_LIBRARY)

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(AOFlagger DEFAULT_MSG
    AOFLAGGER_LIBRARY AOFLAGGER_INCLUDE_DIR)

  set(AOFLAGGER_INCLUDE_DIRS ${AOFLAGGER_INCLUDE_DIR})
  set(AOFLAGGER_LIBRARIES ${AOFLAGGER_LIBRARY})

endif(NOT AOFLAGGER_FOUND)
