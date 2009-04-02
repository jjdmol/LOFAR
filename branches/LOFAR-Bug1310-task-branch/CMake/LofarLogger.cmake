#  LofarLogger.cmake: Check for logger (log4cplus, log4cxx or none)
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

include(LofarFindPackage)

macro(lofar_logger)

  if(USE_LOG4CPLUS AND USE_LOG4CXX)
    message(FATAL_ERROR "Cannot use Log4Cplus and Log4Cxx at the same time")
  endif(USE_LOG4CPLUS AND USE_LOG4CXX)

  if(USE_LOG4CPLUS)
    MESSAGE(STATUS "Checking for Log4Cplus ...")
    lofar_find_package(Log4Cplus 1 log4cplus/logger.h "log4cplus jpeg")
  endif(USE_LOG4CPLUS)

  if(USE_LOG4CXX)
    MESSAGE(STATUS "Checking for Log4Cxx ...")
    lofar_find_package(Log4Cxx 1 log4cxx/logger.h)
  endif(USE_LOG4CXX)

endmacro(lofar_logger)
