# - Subversion CMake macros for LOFAR
# This module defines the following macros:
#
#  lofar_subversion_update([path...])
# Run 'svn update' on each of the paths supplied as arguments. Each path 
# will be converted to an absolute. This is necessary in order to determine
# its position relative to the top-level source directory CMAKE_SOURCE_DIR.
# If any intermediate directories are missing, they will be 'svn update'-d
# non-recursively as well.

#  $Id$
#
#  Copyright (C) 2010
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

find_package(Subversion)

# --------------------------------------------------------------------------
# lofar_subversion_update([path...])
# --------------------------------------------------------------------------
macro(lofar_subversion_update)
  if(NOT Subversion_FOUND)
    message(SEND_ERROR "Subversion was NOT found, but is needed.")
  else(NOT Subversion_FOUND)
    set(_svn_update ${Subversion_SVN_EXECUTABLE} --non-interactive update -q)
    foreach(_path ${ARGV})
      if(NOT IS_ABSOLUTE ${_path})
        get_filename_component(_path ${_path} ABSOLUTE)
      endif(NOT IS_ABSOLUTE ${_path})
      string(REGEX REPLACE "^${CMAKE_SOURCE_DIR}/" "" _dirs "${_path}")
      string(REPLACE "/" ";" _dirs "${_dirs}")
      set(_wc ${CMAKE_SOURCE_DIR})
      foreach(_dir ${_dirs})
        set(_wc ${_wc}/${_dir})
        if("${_wc}" STREQUAL "${_path}")
          set(_cmd ${_svn_update} "${_wc}")
        else("${_wc}" STREQUAL "${_path}")
          set(_cmd ${_svn_update} -N "${_wc}")
        endif("${_wc}" STREQUAL "${_path}")
        execute_process(
          COMMAND ${_cmd}
          RESULT_VARIABLE _result
          ERROR_VARIABLE _error)
        if(_result)
          message(SEND_ERROR "${_cmd} failed:\n${_error}")
        endif(_result)
      endforeach(_dir ${_dirs})
    endforeach(_path ${ARGV})
  endif(NOT Subversion_FOUND)
endmacro(lofar_subversion_update)

