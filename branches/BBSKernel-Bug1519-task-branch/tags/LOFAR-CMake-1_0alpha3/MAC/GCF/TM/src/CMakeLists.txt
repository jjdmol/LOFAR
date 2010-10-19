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

# Add current source directory to -I path. This is needed because GCFTM uses
# angle brackets for internal header files, instead of quotes.
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Add the directory where the generated C++ headers are stored to the -I path.
include_directories(${CMAKE_BINARY_DIR}/include/MAC)

lofar_add_library(gcftm
  Package__Version.cc
  GCF_Handler.cc
  GCF_Scheduler.cc
  GCF_Fsm.cc
  GCF_Protocols.cc
  Port/GCF_PortInterface.cc
  Port/GCF_Port.cc
  Port/GCF_RawPort.cc
  Timer/GTM_Timer.cc
  Timer/GTM_TimerHandler.cc
  PortImpl/GCF_DevicePort.cc
  PortImpl/GCF_ETHRawPort.cc
  PortImpl/GCF_ITCPort.cc
  PortImpl/GCF_TCPPort.cc
  PortImpl/GTM_Device.cc
  PortImpl/GTM_ETHSocket.cc
  PortImpl/GTM_File.cc
  PortImpl/GTM_FileHandler.cc
  PortImpl/GTM_TCPServerSocket.cc
  PortImpl/GTM_TCPSocket.cc
  ServiceBroker/GTM_SBTCPPort.cc
  ServiceBroker/ServiceBrokerTask.cc)

lofar_add_bin_program(versiongcftm versiongcftm.cc)

install(FILES
  mac.log_prop
  DESTINATION etc)
