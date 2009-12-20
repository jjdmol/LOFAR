# - Try to find jni.h: 
# Variables used by this module:
#  JNI_ROOT_DIR     - JNI root directory
# Variables defined by this module:
#  JNI_FOUND        - system has JNI
#  JNI_INCLUDE_DIR  - the JNI include directory (cached)
#  JNI_INCLUDE_DIRS - the JNI include directories 
#                      (identical to JNI_INCLUDE_DIR)
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
# $Id: $

if(NOT JNI_FOUND)

  find_path(JNI_INCLUDE_DIR jni.h
    PATHS ${JNI_ROOT_DIR} PATH_SUFFIXES include)
  mark_as_advanced(JNI_INCLUDE_DIR)

  find_path(JNI_INCLUDE_SUBDIR jni_md.h
    PATHS ${JNI_ROOT_DIR} PATH_SUFFIXES include/linux)
  mark_as_advanced(JNI_INCLUDE_SUBDIR)


  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(JNI DEFAULT_MSG JNI_INCLUDE_DIR JNI_INCLUDE_SUBDIR)

  set(JNI_INCLUDE_DIRS ${JNI_INCLUDE_DIR} ${JNI_INCLUDE_SUBDIR})

endif(NOT JNI_FOUND)
