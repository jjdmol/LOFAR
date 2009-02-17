#  $Id$
#
#  Copyright (C) 2008-2009
#  ASTRON (Netherlands Foundation for Research in Astronomy)
#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

## ----------------------------------------------------------------------------
## Variables set by the global CMake settings module:
##
## ----------------------------------------------------------------------------

#if(NOT LOFAR_CMAKE_CONFIG)

  message(STATUS "**** ENTER: CMakeSettings.cmake ****")

  set(LOFAR_CMAKE_CONFIG TRUE CACHE INTERNAL "LOFAR CMake config flag")

  ## Root directory of the LOFAR source code tree
  string(REGEX REPLACE 
    "^(.*/LOFAR)/.*$" "\\1" LOFAR_ROOT ${CMAKE_SOURCE_DIR})
  set(LOFAR_ROOT ${LOFAR_ROOT} CACHE INTERNAL "LOFAR root directory")

  ## Here's where we keep our own CMake modules.
  set(CMAKE_MODULE_PATH "${LOFAR_ROOT}/CMake")

  ## Default search path used for locating external packages
  set(LOFAR_SEARCH_PATH
    ${CMAKE_INSTALL_PREFIX}
    "/opt/lofar/external/+pkg")

  message(STATUS "LOFAR_SEARCH_PATH = ${LOFAR_SEARCH_PATH}")

  ## --------------------------------------------------------------------------
  ## Several "Auto-tools variables" needed for backward compatibility
  ## --------------------------------------------------------------------------
  set(lofar_top_srcdir "${LOFAR_ROOT}" CACHE INTERNAL "lofar_top_srcdir")
  set(lofar_sharedir "${lofar_top_srcdir}/autoconf_share" CACHE INTERNAL "lofar_sharedir")
  set(prefix "${CMAKE_INSTALL_PREFIX}" CACHE INTERNAL "prefix")
  set(srcdir "${CMAKE_CURRENT_SOURCE_DIR}" CACHE INTERNAL "srcdir")

  message(STATUS "**** LEAVE: CMakeSettings.cmake ****")

#endif(NOT LOFAR_CMAKE_CONFIG)
