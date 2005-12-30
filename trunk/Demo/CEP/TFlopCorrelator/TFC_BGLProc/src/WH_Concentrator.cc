//#  WH_Concentrator.cc: receive data and concentrate into one DH
//#
//#  Copyright (C) 2002-2004
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

#include <APS/ParameterSet.h>
#include <Common/LofarLogger.h>

#include <WH_Concentrator.h>

#include <TFC_Interface/DH_Vis.h>
#include <TFC_Interface/DH_VisArray.h>

using namespace LOFAR;

WH_Concentrator::WH_Concentrator(const string& name, ACC::APS::ParameterSet myPset, int inputs) :
  WorkHolder(inputs, 1, name, "WH_Concentrator"),
  itsNinputs(inputs),
  itsPS     (myPset)
{
  char str[40];

  itsNVis      = itsPS.getInt32("BGLProc.NCorrelatorsPerComputeCell");
  itsNStations = itsPS.getInt32("FakeData.NStations");
  itsNPols     = itsPS.getInt32("Data.NPolarisations");

  ASSERTSTR(itsNVis == itsNinputs, "Configuration mismatch: itsNinputs");

  for (int in = 0; in < inputs; in++) { 
    sprintf(str, "in_%d", in);
    
    getDataManager().addInDataHolder(in, new DH_Vis(str, 1, itsPS));
  }

  getDataManager().addOutDataHolder(0, new DH_VisArray("out", itsPS));
}

WH_Concentrator::~WH_Concentrator() 
{
}

WorkHolder* WH_Concentrator::construct(const string& name, 
				       const ACC::APS::ParameterSet pset,
				       int inputs) {
  return new WH_Concentrator(name, pset, inputs);
}

WH_Concentrator* WH_Concentrator::make(const string& name) {
  return new WH_Concentrator(name, itsPS, itsNinputs);
}

void WH_Concentrator::preprocess() {
}

void WH_Concentrator::process() {

  DH_VisArray* outDH = static_cast<DH_VisArray*>(getDataManager().getOutHolder(0));
  DH_Vis*      inDH;

  for (int i = 0; i < itsNVis; i++) {
    inDH = static_cast<DH_Vis*>(getDataManager().getInHolder(i));
    memcpy(outDH->getBufferElement(i /*, 0, 0, 0*/),
	   inDH->getBuffer(), 
	   inDH->getBufSize()*sizeof(fcomplex));
    outDH->setCenterFreq(inDH->getCenterFreq(), i);
  }
}

void WH_Concentrator::dump() const {
}
