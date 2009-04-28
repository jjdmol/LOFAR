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

set(transport_LIB_SRCS
  Package__Version.cc
  BGLConnection.cc
  Connection.cc
  ConnectionPool.cc
  CSConnection.cc
  DataBlobExtra.cc
  DataHolder.cc
  DH_BlobStreamable.cc
  TH_Ethernet.cc
  TH_File.cc
  TH_Mem.cc
  TH_MPI.cc
  TH_ShMem.cc
  TH_Socket.cc
  TransportHolder.cc
)

set(transport_PROGRAMS
  versiontransport
)

lofar_add_library(transport ${transport_LIB_SRCS})

foreach(prog ${transport_PROGRAMS})
  lofar_add_executable(${prog} ${prog}.cc)
endforeach(prog ${transport_PROGRAMS})

