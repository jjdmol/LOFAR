#  LofarOptions.cmake: Parse CMake options and set associated variables
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

message(STATUS "**** ENTER: LofarOptions.cmake ****")

## ----------------------------------------------------------------------------
## For each option that is set, try to find the associated package. It is
## considered a fatal error, if the package cannot be found.
## ----------------------------------------------------------------------------

foreach(opt ${LOFAR_OPTIONS})
  message(STATUS "${opt} is ${${opt}}")
#  if(${opt})
#  endif(${opt})
endforeach(opt ${LOFAR_OPTIONS})

#set(OPTION        USE_LOG4CXX   OFF)
#set(OPTION        USE_AIPSPP    OFF)
#set(OPTION        USE_SOCKETS   OFF)
#set(OPTION        USE_ZOID      OFF)
#set(OPTION        USE_THREADS   ON)
#set(OPTION        USE_SSE       OFF)
#set(OPTION        USE_SHMEM     ON)
#set(OPTION        USE_PYTHON    OFF)
#set(OPTION        USE_BOOST     OFF)
#set(OPTION        USE_BACKTRACE OFF)

message(STATUS "**** LEAVE: LofarOptions.cmake ****")
