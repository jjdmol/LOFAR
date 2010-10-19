# $Id: FindAskapSoft.cmake 13814 2009-08-20 11:55:06Z loose $
#
# Copyright (C) 2009
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

# Try to find AskapSoft.
#
# This will define:
#  
#  ASKAPSOFT_FOUND        - system has AskapSoft
#  ASKAPSOFT_INCLUDE_DIR  - the AskapSoft include directory (cached)
#  ASKAPSOFT_INCLUDE_DIRS - the AskapSoft include directories
#                         (identical to ASKAPSOFT_INCLUDE_DIR)
#  ASKAPSOFT_LIBRARIES        - The AskapSoft libraries (not cached)
#  ASKAPSOFT_${COMPONENT}_LIBRARY - The absolute path of AskapSoft library 
#                              "component" (cached)
#  ASKAPSOFT_LIBRARIES    - the AskapSoft libraries

if(NOT ASKAPSOFT_FOUND)

  find_path(ASKAPSOFT_INCLUDE_DIR measurementequation/ImageSolver.h)

  find_library(ASKAPSOFT_SYNTHESIS_LIBRARY askap_synthesis)
  find_library(ASKAPSOFT_ASKAPPARALLEL_LIBRARY askap_askapparallel)
  find_library(ASKAPSOFT_MWCOMMON_LIBRARY askap_mwcommon)
  find_library(ASKAPSOFT_SCIMATH_LIBRARY askap_scimath)
  find_library(ASKAPSOFT_ASKAP_LIBRARY askap_askap)
  find_library(ASKAPSOFT_GSL_LIBRARY gsl)
  find_library(ASKAPSOFT_GSLCBLAS_LIBRARY gslcblas)
  find_library(ASKAPSOFT_LOG4CXX_LIBRARY log4cxx)
  find_library(ASKAPSOFT_APRUTIL_LIBRARY aprutil-1)
  find_library(ASKAPSOFT_EXPAT_LIBRARY expat)
  find_library(ASKAPSOFT_APR_LIBRARY apr-1)
  find_library(ASKAPSOFT_FFTW_LIBRARY fftw3)
  find_library(ASKAPSOFT_FFTWF_LIBRARY fftw3f)
  find_library(ASKAPSOFT_CMDLINEPARSER_LIBRARY cmdlineparser)

  mark_as_advanced(ASKAPSOFT_INCLUDE_DIR
   ASKAPSOFT_SYNTHESIS_LIBRARY
   ASKAPSOFT_ASKAPPARALLEL_LIBRARY
   ASKAPSOFT_MWCOMMON_LIBRARY
   ASKAPSOFT_SCIMATH_LIBRARY
   ASKAPSOFT_ASKAP_LIBRARY
   ASKAPSOFT_GSL_LIBRARY
   ASKAPSOFT_GSLCBLAS_LIBRARY
   ASKAPSOFT_LOG4CXX_LIBRARY
   ASKAPSOFT_APRUTIL_LIBRARY
   ASKAPSOFT_EXPAT_LIBRARY
   ASKAPSOFT_APR_LIBRARY
   ASKAPSOFT_FFTW_LIBRARY
   ASKAPSOFT_FFTWF_LIBRARY
   ASKAPSOFT_CMDLINEPARSER_LIBRARY
   )

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(AskapSoft DEFAULT_MSG
   ASKAPSOFT_INCLUDE_DIR
   ASKAPSOFT_SYNTHESIS_LIBRARY
   ASKAPSOFT_ASKAPPARALLEL_LIBRARY
   ASKAPSOFT_MWCOMMON_LIBRARY
   ASKAPSOFT_SCIMATH_LIBRARY
   ASKAPSOFT_ASKAP_LIBRARY
   ASKAPSOFT_GSL_LIBRARY
   ASKAPSOFT_GSLCBLAS_LIBRARY
   ASKAPSOFT_LOG4CXX_LIBRARY
   ASKAPSOFT_APRUTIL_LIBRARY
   ASKAPSOFT_EXPAT_LIBRARY
   ASKAPSOFT_APR_LIBRARY
   ASKAPSOFT_FFTW_LIBRARY
   ASKAPSOFT_FFTWF_LIBRARY
   ASKAPSOFT_CMDLINEPARSER_LIBRARY
   )

  set(ASKAPSOFT_INCLUDE_DIRS ${ASKAPSOFT_INCLUDE_DIR})

  set(ASKAPSOFT_LIBRARIES
   ${ASKAPSOFT_SYNTHESIS_LIBRARY}
   ${ASKAPSOFT_ASKAPPARALLEL_LIBRARY}
   ${ASKAPSOFT_MWCOMMON_LIBRARY}
   ${ASKAPSOFT_SCIMATH_LIBRARY}
   ${ASKAPSOFT_ASKAP_LIBRARY}
   ${ASKAPSOFT_GSL_LIBRARY}
   ${ASKAPSOFT_GSLCBLAS_LIBRARY}
   ${ASKAPSOFT_LOG4CXX_LIBRARY}
   ${ASKAPSOFT_APRUTIL_LIBRARY}
   ${ASKAPSOFT_EXPAT_LIBRARY}
   ${ASKAPSOFT_APR_LIBRARY}
   ${ASKAPSOFT_FFTW_LIBRARY}
   ${ASKAPSOFT_FFTWF_LIBRARY}
   ${ASKAPSOFT_CMDLINEPARSER_LIBRARY}
   )

endif(NOT ASKAPSOFT_FOUND)
