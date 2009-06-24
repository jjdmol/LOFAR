#  $Id$
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

include(LofarPackageVersion)

set(amcimpl_LIB_SRCS
  Package__Version.cc
  ConverterImpl.cc
  ConverterProcess.cc
  ConverterServer.cc
)

set(amcimpl_PROGRAMS
  versionamcimpl
  AMCServer
  WGS84toITRF)

## Suppress warnings from the Casacore Measures component.
#set_source_files_properties(${amcimpl_LIB_SRCS} ${amcimpl_PROGRAMS}
#  PROPERTIES COMPILE_FLAGS "-Wno-ignored-qualifiers")

lofar_add_library(amcimpl ${amcimpl_LIB_SRCS})

foreach(prog ${amcimpl_PROGRAMS})
  lofar_add_bin_program(${prog} ${prog}.cc)
endforeach(prog ${amcimpl_PROGRAMS})
