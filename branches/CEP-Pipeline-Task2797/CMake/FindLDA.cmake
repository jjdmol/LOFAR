# - Try to find LDA.
# Variables used by this module:
#  LDA_ROOT_DIR     - LDA root directory
# Variables defined by this module:
#  LDA_FOUND        - system has LDA
#  LDA_INCLUDE_DIR  - the LDA include directory (cached)
#  LDA_INCLUDE_DIRS - the LDA include directories
#                       (identical to LDA_INCLUDE_DIR)
#  LDA_LIBRARY      - the LDA library (cached)
#  LDA_LIBRARIES    - the LDA libraries
#                       (identical to LDA_LIBRARY)

# Copyright (C) 2011
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

if(NOT LDA_FOUND)

  find_path(LDA_INCLUDE_DIR lda_config.h
    HINTS ${LDA_ROOT_DIR} PATH_SUFFIXES include/lda)
  find_library(LDA_LIBRARY lda
    HINTS ${LDA_ROOT_DIR} PATH_SUFFIXES lib)
  mark_as_advanced(LDA_INCLUDE_DIR LDA_LIBRARY)

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(LDA DEFAULT_MSG 
    LDA_LIBRARY LDA_INCLUDE_DIR)

  set(LDA_INCLUDE_DIRS ${LDA_INCLUDE_DIR})
  set(LDA_LIBRARIES ${LDA_LIBRARY})

endif(NOT LDA_FOUND)
