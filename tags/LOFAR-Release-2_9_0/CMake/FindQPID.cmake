# - Try to find QPID: Apache's implementation of the AMPQ protocol
# Variables used by this module:
#  QPID_ROOT_DIR     - QPID root directory
# Variables defined by this module:
#  QPID_FOUND        - system has QPID
#  QPID_INCLUDE_DIR  - the QPID include directory (cached)
#  QPID_INCLUDE_DIRS - the QPID include directories
#                        (identical to QPID_INCLUDE_DIR)
#  QPID_LIBRARY      - the QPID library (cached)
#  QPID_LIBRARIES    - the QPID libraries
#                        (identical to QPID_LIBRARY)
#  QPID_RECEIVE_EXECUTABLE - the full path of qpid-receive
#  QPID_SEND_EXECUTABLE    - the full path of qpid-send
#  QPID_STAT_EXECUTABLE    - the full path of qpid-stat

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

if(NOT QPID_FOUND)

  find_path(QPID_INCLUDE_DIR qpid/messaging/Connection.h
    HINTS ${QPID_ROOT_DIR} PATH_SUFFIXES include)
  find_library(QPID_MESSAGING_LIBRARY qpidmessaging
    HINTS ${QPID_ROOT_DIR} PATH_SUFFIXES lib)
  find_library(QPID_TYPES_LIBRARY qpidtypes
    HINTS ${QPID_ROOT_DIR} PATH_SUFFIXES lib)

  find_program(QPID_RECEIVE_EXECUTABLE qpid-receive
    HINTS ${QPID_ROOT_DIR} PATH_SUFFIXES bin)
  find_program(QPID_SEND_EXECUTABLE qpid-send
    HINTS ${QPID_ROOT_DIR} PATH_SUFFIXES bin)
  find_program(QPID_STAT_EXECUTABLE qpid-stat
    HINTS ${QPID_ROOT_DIR} PATH_SUFFIXES bin)

  set(QPID_LIBRARY ${QPID_MESSAGING_LIBRARY} ${QPID_TYPES_LIBRARY})

  mark_as_advanced(QPID_INCLUDE_DIR QPID_LIBRARY)

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(QPID DEFAULT_MSG
    QPID_LIBRARY QPID_INCLUDE_DIR)

  set(QPID_INCLUDE_DIRS ${QPID_INCLUDE_DIR})
  set(QPID_LIBRARIES ${QPID_LIBRARY})

endif(NOT QPID_FOUND)
