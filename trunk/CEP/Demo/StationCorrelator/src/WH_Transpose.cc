//#  WH_Transpose.cc: MPI_Alltoall transpose
//#
//#  Copyright (C) 2002-2005
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <WH_Transpose.h>
#include <DH_CorrCube.h>
#include <DH_StationData.h>

using namespace LOFAR;

WH_Transpose::WH_Transpose(const string& name, KeyValueMap kvm) 
  : WorkHolder(kvm.getInt("NoWH_Correlator",7), 1, name, "WH_Transpose"),
    itsKVM (kvm)
{
  char str[128];
  
  int stations          = itsKVM.getInt("stations", 2);
  int samples           = itsKVM.getInt("samples", 256000);
  int channels          = itsKVM.getInt("channels", 46);
  int polarisations     = itsKVM.getInt("polarisations", 2);
  int beamletsinpacket  = kvm.getInt("NoRSPbeamlets", 92);
  int packetsinframe    = kvm.getInt("NoPacketsInFrame", 8);

  int bufsize = (beamletsinpacket / stations) * polarisations * packetsinframe;
 
  itsKVM.show(cout);

  itsNinputs = itsKVM.getInt("noWH_Correlator", 7);
  itsNoutputs = 1; // there is one connection to the corresponding WH_Correlator

  for (int i = 0; i < itsNinputs; i++) {
    getDataManager().addInDataHolder(i, new DH_StationData(str, bufsize));
  }
  for (int i = 0; i < itsNoutputs; i++) {
    snprintf(str, 128, "output_%d_of _%d", i, itsNoutputs);
    getDataManager().addOutDataHolder(i, new DH_CorrCube(str, 
							 stations, 
							 samples, 
							 channels, 
							 polarisations));
  }
}

WH_Transpose::~WH_Transpose() {
}

WorkHolder* WH_Transpose::construct(const string& name, KeyValueMap kvm) {
  return new WH_Transpose(name, kvm);
}

WH_Transpose* WH_Transpose::make(const string& name) {
  return new WH_Transpose(name, itsKVM);
}

void WH_Transpose::process() {
}
