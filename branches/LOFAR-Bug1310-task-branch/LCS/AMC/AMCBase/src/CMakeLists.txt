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

set(amcbase_LIB_SRCS
  Package__Version.cc
  BlobIO.cc
  ConverterClient.cc
  ConverterCommand.cc
  ConverterStatus.cc
  Coord3D.cc
  DH_Request.cc
  DH_Result.cc
  Direction.cc
  Epoch.cc
  Position.cc)

set(amcbase_PROGRAMS
  versionamcbase
)

add_library(amcbase ${amcbase_LIB_SRCS})
target_link_libraries(amcbase transport)
install(TARGETS amcbase DESTINATION ${LOFAR_LIBDIR})

foreach(prog ${amcbase_PROGRAMS})
  add_executable(${prog} ${prog}.cc)
  target_link_libraries(${prog} amcbase ${LOFAR_LIBRARIES})
  install(TARGETS ${prog} DESTINATION bin)
endforeach(prog ${amcbase_PROGRAMS})
