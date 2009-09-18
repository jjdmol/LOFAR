# $Id$
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


# python_install(source1..sourceN DESTINATION install_dir)
#
# Install Python source files and byte-compile them in the directory
# ${PYTHON_INSTALL_DIR}/${install_dir}.

macro(python_install)

  # Precondition checks.
  if(NOT DEFINED PYTHON_INSTALL_DIR)
    message(FATAL_ERROR "python_install: PYTHON_INSTALL_DIR is undefined."
      "\nMaybe you forgot to do a `find_package(Python)'?")
  endif(NOT DEFINED PYTHON_INSTALL_DIR)

  if(NOT PYTHON_EXECUTABLE)
    message(FATAL_ERROR "python_install: Python interpreter not available")
  endif(NOT PYTHON_EXECUTABLE)

  # Parse arguments.
  string(REGEX REPLACE ";?DESTINATION.*" "" _py_files "${ARGN}")
  string(REGEX MATCH "DESTINATION;.*" _dest_dir "${ARGN}")
  string(REGEX REPLACE "^DESTINATION;" "" _dest_dir "${_dest_dir}")

  if(_py_files MATCHES "^$")
    message(FATAL_ERROR "python_install: no sources files specified")
  endif(_py_files MATCHES "^$")
  if(_dest_dir MATCHES "^$" OR _dest_dir MATCHES ";")
    message(FATAL_ERROR "python_install: destination directory invalid")
  endif(_dest_dir MATCHES "^$" OR _dest_dir MATCHES ";")

  set(_dest_dir "${PYTHON_INSTALL_DIR}/${_dest_dir}")

  # Byte-compile each Python file and add both .py and .pyc to install list.
  set(_pyc_files)
  foreach(_py ${_py_files})
    set(_comment "Byte-compiling ${_py}")
    set(_pyc ${CMAKE_CURRENT_BINARY_DIR}/${_py}c)
    set(_py ${CMAKE_CURRENT_SOURCE_DIR}/${_py})
    add_custom_command(OUTPUT ${_pyc}
      COMMAND ${PYTHON_EXECUTABLE}
      ARGS -c "import py_compile; py_compile.compile('${_py}', '${_pyc}')"
      COMMENT ${_comment}
      DEPENDS ${_py}
      VERBATIM)
    list(APPEND _pyc_files ${_pyc})
    install(FILES ${_py} ${_pyc} DESTINATION ${_dest_dir})
  endforeach(_py ${_py_files})

  # Create a unique custom target that depends on all .pyc files, and let
  # that target depend on the current project.
  if(NOT TARGET ${PROJECT_NAME}_py_compile)
    add_custom_target(${PROJECT_NAME}_py_compile DEPENDS ${_pyc_files})
  endif(NOT TARGET ${PROJECT_NAME}_py_compile)
  add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_py_compile)

endmacro(python_install)
