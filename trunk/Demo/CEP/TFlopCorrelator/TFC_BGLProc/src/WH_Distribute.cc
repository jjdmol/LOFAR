//#  WH_Distribute.cc:
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

#include <Common/LofarLogger.h>
#include <WH_Distribute.h>

using namespace LOFAR;


WH_Distribute::WH_Distribute(const string& name, 
			     ACC::APS::ParameterSet myPset, 
			     int inputs,
			     int outputs) :
  WorkHolder  (inputs, outputs, name, "WH_Distribute"), 
  itsNinputs  (inputs),
  itsNoutputs (outputs),
  itsPS       (myPset)
{
  itsNinputElements  = itsPS.getInt32("Data.NStations");
  itsNoutputElements = itsPS.getInt32("FakeData.NStations");

  itsNsamples       = itsPS.getInt32("Data.NSamplesToIntegrate"); 
  itsNpolarisations = itsPS.getInt32("Data.NPolarisations");

  /// add DataHolders. Note that we may need to create a private Pset
  /// for the outDH if we need to duplicate data
  for (int i = 0; i < itsNinputs; i++) {
    getDataManager().addInDataHolder(i, new DH_Subband("in", 1, itsPS));
  }

  for (int i = 0; i < itsNoutputs; i++) {
    getDataManager().addOutDataHolder(i, new DH_PPF("out", 1, itsPS));
  }
  DBGASSERTSTR(itsNinputs <= itsNoutputs, "WH_Distribute: itsNinputs must be <= to itsNoutputs");
  DBGASSERTSTR(itsNoutputs % itsNinputs == 0, "itsNoutputs needs to be an exact multiple of itsNinputs");
  DBGASSERTSTR(itsNinputElements <= itsNoutputElements, "WH_Distribute: itsNinputElements must be <= itsNoutputElements");

}

WH_Distribute::~WH_Distribute() {
}

WorkHolder* WH_Distribute::construct(const string& name, 
				     ACC::APS::ParameterSet myPset, 
				     int inputs, 
				     int outputs) 
{
  return new WH_Distribute(name, myPset, inputs, outputs);
}

WH_Distribute* WH_Distribute::make(const string& name) {
  return new WH_Distribute(name, itsPS, itsNinputs, itsNoutputs);
}

void WH_Distribute::preprocess() {
}

void WH_Distribute::process() {

  if (itsNinputElements == itsNoutputElements) {
    /// do a straight memcpy
    for (int i = 0; i < itsNinputs; i++) {
      
      // we could still have more outputs than inputs
      for (int o = 0; o < itsNoutputs/itsNinputs; o++) {
	memcpy(static_cast<DH_PPF*>(getDataManager().getOutHolder(i+o))->getBuffer(),
	       static_cast<DH_Subband*>(getDataManager().getInHolder(i))->getBuffer(),
	       static_cast<DH_PPF*>(getDataManager().getOutHolder(i+o))->getBufferSize() * sizeof(DH_PPF::BufferElementType));
      }
    }
  } else /* (itsNinputElements < itsNoutputElements) */ {
    // The output DH contains more data (more elements) than the input DH. 
    // duplicate data by copying data from the inDH into the outDH until it's
    // buffer is full

    // we also take into account that the number of outputs may be more than
    // the number of inputs.

    for (int i = 0; i < itsNinputs; i++) {
    
      for (int o = 0; o < itsNoutputs/itsNinputs; o++) {

	/// duplicate data to fill all OutHolders, keep copying from inDH until outDH buffer is filled
	int myCounter = 0;
	int myCopiedBytes = 0;
	int myNextBlock = 0;
	
	// buffer size in elements, not in bytes, so we multiply by the size of the buffertype (i16complex)
	const int inBufSize = static_cast<DH_Subband*>(getDataManager().getInHolder(i))->getBufferSize() * sizeof(DH_Subband::BufferType);
	const int outBufSize = static_cast<DH_PPF*>(getDataManager().getOutHolder(i+o))->getBufferSize() * sizeof(DH_PPF::BufferElementType);
	
	while (myCopiedBytes < outBufSize) {
	  
	  
	  (outBufSize-myCopiedBytes) > inBufSize ? myNextBlock = inBufSize : myNextBlock = (outBufSize - myCopiedBytes);
	  
// 	  cout << "DEBUG: " << myCopiedBytes << " + " << myNextBlock <<" = " << myCopiedBytes+myNextBlock << 
// 	    " should be <= " << outBufSize << " " << (myCopiedBytes+myNextBlock <= outBufSize) << endl;
	  // duplicate data 
	  memcpy((void*)(static_cast<DH_PPF*>(getDataManager().getOutHolder(i+o))->getBufferElement() + (myCopiedBytes/sizeof(DH_PPF::BufferElementType))),
		 static_cast<DH_Subband*>(getDataManager().getInHolder(i))->getBuffer(),
		 myNextBlock);
	  myCopiedBytes += myNextBlock;
	}
      }
    }
  }
}

void WH_Distribute::dump() const {
}

			     
