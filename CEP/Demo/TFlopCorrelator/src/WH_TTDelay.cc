//#  WH_TTDelay.cc: 
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
#include <WH_TTDelay.h>
#include <DH_Beamlet.h>
#include <DH_SubBand.h>

using namespace LOFAR;

WH_TTDelay::WH_TTDelay(const string& name, KeyValueMap kvm) 
  : WorkHolder(kvm.getInt("NoWH_Correlator",7), 1, name, "WH_TTDelay"),
    itsKVM (kvm)
{
  char str[128];
  
  //  int bufsize = (itsNbeamletsinpacket / itsNstations) * itsNpolarisations * itsNpacketsinframe;

  itsNinputs = itsKVM.getInt("noWH_Correlator", 7);
  itsNoutputs = 1; // there is one connection to the corresponding WH_Correlator

  for (int i = 0; i < itsNinputs; i++) {
    getDataManager().addInDataHolder(i, new DH_StationData(str, bufsize));
  }
  for (int i = 0; i < itsNoutputs; i++) {
    snprintf(str, 128, "output_%d_of _%d", i, itsNoutputs);
    getDataManager().addOutDataHolder(i, new DH_CorrCube(str, 
							 itsNstations, 
							 itsNsamples, 
							 itsNchannels, 
							 itsNpolarisations));
  }
}

WH_TTDelay::~WH_TTDelay() {
}

WorkHolder* WH_TTDelay::construct(const string& name, KeyValueMap kvm) {
  return new WH_TTDelay(name, kvm);
}

WH_TTDelay* WH_TTDelay::make(const string& name) {
  return new WH_TTDelay(name, itsKVM);
}

void WH_TTDelay::process() {
}
