# - Try to find the Python interpreter, Python header files and libraries. 
# This macro effectively wraps the FindPythonInterp and FindPythonLibs macros
# provided by CMake.
#
# In addition to the variables that are set by FindPythonInterp and
# FindPythonLibs, this will define:
#  PYTHON_FOUND        - system has Python interpreter, Python headers
#                        files and libraries
#  PYTHON_INCLUDE_DIRS - path to the Python header files
#  PYTHON_BUILD_DIR    - build directory for Python packages (cached)
#  PYTHON_INSTALL_DIR  - installation directory for Python packages (cached)

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
# $Id$

# Set options string to pass to the find_package() commands below.
set(_options)
if(Python_FIND_REQUIRED)
  set(_options REQUIRED)
endif(Python_FIND_REQUIRED)

# Search for the Python interpreter
find_package(PythonInterp ${_options})

# Search for the Python header files and libraries
find_package(PythonLibs ${_options})

# Derive the Python site-packages installation directory and build directory
if(NOT PYTHON_FOUND)
  if(PYTHON_EXECUTABLE)
    set(_cmd
      "from distutils.sysconfig import *"
      "print get_python_lib(prefix='')")
    execute_process(
      COMMAND "${PYTHON_EXECUTABLE}" "-c" "${_cmd}"
      OUTPUT_VARIABLE _pydir
      ERROR_VARIABLE _err
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(_err)
      message(FATAL_ERROR "Python command failed:\n${_err}")
    endif(_err)
    set(PYTHON_BUILD_DIR "${CMAKE_BINARY_DIR}/${_pydir}" CACHE PATH 
      "Python site-packages build directory")
    set(PYTHON_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/${_pydir}" CACHE PATH 
      "Python site-packages installation directory")
  endif(PYTHON_EXECUTABLE)
endif(NOT PYTHON_FOUND)

# Set PYTHON_INCLUDE_DIRS variable, because FindPythonLibs does not do it.
set(PYTHON_INCLUDE_DIRS "${PYTHON_INCLUDE_PATH}")

# Set PYTHON_FOUND to TRUE if both Python interpreter and libraries are found
set(PYTHON_FOUND FALSE)
if(PYTHONINTERP_FOUND AND PYTHONLIBS_FOUND)
  set(PYTHON_FOUND TRUE)
endif(PYTHONINTERP_FOUND AND PYTHONLIBS_FOUND)

