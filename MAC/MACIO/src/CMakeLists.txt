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

## ---------------------------------------------------------------------------
## Add MAC protocols (generates a .ph and .cc file from a .prot file)
## ---------------------------------------------------------------------------
include(MACAddProtocol)
mac_add_protocol(LOG_Protocol "${PACKAGE_SOURCE_DIR}/autogen")
mac_add_protocol(KVT_Protocol "${PACKAGE_SOURCE_DIR}/autogen")
mac_add_protocol(SB_Protocol  "${PACKAGE_SOURCE_DIR}/autogen")

## ---------------------------------------------------------------------------
## Add libraries to be generated
## ---------------------------------------------------------------------------
lofar_add_library(log_protocol LOG_Protocol.cc)
lofar_add_library(kvt_protocol KVT_Protocol.cc)
lofar_add_library(macio 
  Package__Version.cc
  GCF_Event.cc
  EventPort.cc
  ProtocolDefs.cc
  SB_Protocol.cc)

## ---------------------------------------------------------------------------
## Add executables to be generated
## ---------------------------------------------------------------------------
lofar_add_bin_program(versionmacio versionmacio.cc)
