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


# --------------------------------------------------------------------------
# python_install(source1..sourceN DESTINATION install_dir)
#
# Install Python source files and byte-compile them in the directory
# ${PYTHON_INSTALL_DIR}/${install_dir}.
# --------------------------------------------------------------------------
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

  # Set python package install directory.
  set(_inst_dir "${PYTHON_INSTALL_DIR}/${_dest_dir}")
  set(_build_dir "${PYTHON_BUILD_DIR}/${_dest_dir}")

  # Install and byte-compile each Python file.
  foreach(_py ${_py_files})
    get_filename_component(_src_dir ${_py} ABSOLUTE)
    configure_file(${_src_dir} ${_build_dir}/${_py} COPYONLY)
    install(FILES ${_py} DESTINATION ${_inst_dir})
    get_filename_component(_py ${_py} NAME)
    set(_py_code
      "import py_compile"
      "print '-- Byte-compiling: ${_inst_dir}/${_py}'"
      "py_compile.compile('${_inst_dir}/${_py}')")
    install(CODE 
      "execute_process(COMMAND ${PYTHON_EXECUTABLE} -c \"${_py_code}\")")
  endforeach(_py ${_py_files})

  # Make sure that there's a __init__.py file in each build/install directory
  string(REGEX REPLACE "/" ";" _dir_list ${_dest_dir})
  set(_init_dir)
  foreach(_dir ${_dir_list})
    set(_init_dir "${_init_dir}/${_dir}")
    execute_process(COMMAND ${CMAKE_COMMAND} -E touch
      "${PYTHON_BUILD_DIR}${_init_dir}/__init__.py")
    install(CODE 
      "execute_process(COMMAND ${CMAKE_COMMAND} -E touch 
        \"${PYTHON_INSTALL_DIR}${_init_dir}/__init__.py\")")
  endforeach(_dir ${_dir_list})

endmacro(python_install)
