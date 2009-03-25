#  LofarSearchPath.cmake: Return search path based on pattern in variants file,
#                         package name plus optional package version.
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
#
#  $Id$

include(LofarVariants)

# Return search path to use when searching for package '_pkg' as '_path'.
# Replace placeholders in LOFAR_SEARCH_PATH with actual values. Note that 
# we need to quote the variables, because they may be undefined.
macro(lofar_search_path _path _pkg)
  set(${_path})
  string(TOLOWER ${LOFAR_COMPILER_SUITE} comp)
  foreach(_dir ${LOFAR_SEARCH_PATH})
    string(REPLACE "+prefix" "${CMAKE_INSTALL_PREFIX}" _dir ${_dir})
    string(REPLACE "+root" "${LOFAR_ROOT}" _dir ${_dir})
    string(REPLACE "+pkg" "${_pkg}" _dir ${_dir})
    string(REPLACE "+vers" "${${_pkg}-version}" _dir ${_dir})
    string(REPLACE "+comp" "${comp}" _dir ${_dir})
    list(APPEND ${_path} ${_dir})
  endforeach(_dir in ${LOFAR_SEARCH_PATH})
endmacro(lofar_search_path _path _pkg)

# Set the search path to use when searching for package '_pkg', by setting the
# CMake variable CMAKE_PREFIX_PATH.
macro(lofar_set_search_path _pkg)
  lofar_search_path(_path ${_pkg})
  set(CMAKE_PREFIX_PATH ${_path})
endmacro(lofar_set_search_path _pkg)

