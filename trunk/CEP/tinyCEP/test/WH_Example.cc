//#  WH_Example.cc: Example WorkHolder for tinyCEPFrame test programs.
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

#include <lofar_config.h>
#include <stdio.h>
#include <WH_Example.h>
#include <Blob/KeyValueMap.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{
  WH_Example::WH_Example (const string& name, 
			  unsigned int nin, unsigned int nout,
			  unsigned int nbuffer)
    : WorkHolder (nin, nout, name, "WH_Example"),
      itsBufLength (nbuffer) {
    char str[8];
    
    for (unsigned int i = 0; i < nin; i++) {
      sprintf(str, "%d", i);
      getDataManager().addInDataHolder(i, new DH_Example(string("in_") + str));
    }

    for (unsigned int i = 0; i < nout; i++) {
      sprintf(str, "%d", i);
      getDataManager().addOutDataHolder(i, new DH_Example(string("out_") + str));
    }
  }
  

  WH_Example::~WH_Example() {
  }

  WorkHolder* WH_Example::construct (const string& name, int ninput, int noutput,
				     const KeyValueMap& params) {
    return new WH_Example(name, ninput, noutput, 
			  params.getInt("nbuffer", 10));
  }
  
  WH_Example* WH_Example::make (const string& name) {
    return new WH_Example (name, getDataManager().getInputs(),
			   getDataManager().getOutputs(), itsBufLength) ;
  }

  void WH_Example::preprocess() {

    if ( getDataManager().getInputs() == getDataManager().getOutputs() ) {
      
      DH_Example* InDHptr;
      DH_Example* OutDHptr;

//       complex<float> inval = complex<float> (5, 3);
//       complex<float> outval = complex<float> (0, 0);

      for ( int i = 0; i < getDataManager().getInputs() ; i++) {
	InDHptr = (DH_Example*)getDataManager().getInHolder(i);
	OutDHptr = (DH_Example*)getDataManager().getOutHolder(i);

	InDHptr->getBuffer()[0]  = makefcomplex(0, 0);
	OutDHptr->getBuffer()[0] = makefcomplex(0, 0);
	
// 	memcpy(InDHptr->getBuffer(), 
// 	       &inval,
// 	       sizeof(DH_Example::BufferType));

// 	memcpy(OutDHptr->getBuffer(),
// 	       &outval,
// 	       sizeof(DH_Example::BufferType));


// 	InDHptr->getBuffer()  = makefcomplex(5,3);
// 	OutDHptr->getBuffer() = makefcomplex(0,0);
      }
    }	
  }
  
  void WH_Example::process() {
    DH_Example* InDHptr;
    DH_Example* OutDHptr;

    // copy input to output
    if ( getDataManager().getInputs() == getDataManager().getOutputs() ) {
      for (int i = 0; i < getDataManager().getInputs(); i++) {

	InDHptr  = (DH_Example*)getDataManager().getInHolder(i);
	OutDHptr = (DH_Example*)getDataManager().getOutHolder(i);

	memcpy(OutDHptr->getBuffer(), 
	       InDHptr->getBuffer(),
	       sizeof(DH_Example::BufferType));
	

      }
    }
  }
  
  void WH_Example::dump() const{
    
    DH_Example* InDHptr = (DH_Example*)getDataManager().getInHolder(0);
    DH_Example* OutDHptr = (DH_Example*)getDataManager().getOutHolder(0);

    cout << "InHolder(0) : " << *InDHptr->getBuffer() << endl;
    cout << "OutHolder(0): " << *OutDHptr->getBuffer() << endl;
    
  }

} // namespace LOFAR
