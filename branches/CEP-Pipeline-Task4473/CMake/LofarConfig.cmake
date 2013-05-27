# - Generate the configuration files needed for building and using the LOFAR
# software.

#  Copyright (C) 2008-2010
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
#
#  $Id$

# Only process this file when we're ate the top-level source directory.
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")

  # Configure header containing global configurations for LOFAR builds. 
  message(STATUS "Generating lofar_config.h ...")
  configure_file(
    ${CMAKE_SOURCE_DIR}/lofar_config.h.cmake
    ${CMAKE_BINARY_DIR}/include/lofar_config.h)

  # Configure the lofarinit script files and mark them for install
  message(STATUS "Generating lofarinit scripts ...")
  configure_file(
    ${CMAKE_SOURCE_DIR}/lofarinit.sh.in
    ${CMAKE_BINARY_DIR}/lofarinit.sh @ONLY)
  configure_file(
    ${CMAKE_SOURCE_DIR}/lofarinit.csh.in
    ${CMAKE_BINARY_DIR}/lofarinit.csh @ONLY)
  install(FILES
    ${CMAKE_BINARY_DIR}/lofarinit.sh
    ${CMAKE_BINARY_DIR}/lofarinit.csh
    DESTINATION .)

  # Configure the CTestCustom.cmake file, used by CTest/Dart, if it exists.
  if(EXISTS ${CMAKE_SOURCE_DIR}/CTestCustom.cmake.in)
    configure_file(
      ${CMAKE_SOURCE_DIR}/CTestCustom.cmake.in
      ${CMAKE_BINARY_DIR}/CTestCustom.cmake @ONLY)
  endif(EXISTS ${CMAKE_SOURCE_DIR}/CTestCustom.cmake.in)
  
endif("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
