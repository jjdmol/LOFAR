//#  WH_Dump.cc: Output workholder for BG Correlator application
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


#include <stdio.h>           // for sprintf

// General includes
#include <Common/Debug.h>
#include <Common/KeyValueMap.h>

// Application specific includes
#include <WH_Dump.h>
#include <DH_Vis.h>
#include <DH_Empty.h> 

using namespace LOFAR;


WH_Dump::WH_Dump(const string& name,
		 unsigned int nin, 
		 unsigned int nout) 
  : WorkHolder(nin, nout, name, "WH_Dump"),
    itsIndex   (0),
    itsCounter (0)  
{
  char str[8];
  for (unsigned int i = 0; i < nin; i++) {
    
    sprintf (str, "%d", i);
    getDataManager().addInDataHolder(i, new DH_Vis(string("in_") + str));

  }
  for (unsigned int i = 0; i < nout; i++) {
    
    sprintf (str, "%d", i);
    getDataManager().addOutDataHolder(i, new DH_Empty(string("out_") + str));

  }
}

WH_Dump::~WH_Dump() {

}

WorkHolder* WH_Dump::construct (const string& name, 
				unsigned int nin, 
				unsigned int nout) {
  return new WH_Dump(name, nin, nout);
}

WH_Dump* WH_Dump::make(const string& name) {

  return new WH_Dump(name, getDataManager().getInputs(), getDataManager().getOutputs());
}

void WH_Dump::process() {
  complex<float> corr[NSTATIONS][NSTATIONS][NCHANNELS];
  
  memcpy (corr, 
	  ((DH_Vis*)getDataManager().getInHolder(0))->getBuffer(),
	  NSTATIONS*NSTATIONS*NCHANNELS*sizeof(DH_Vis::BufferType));

  // negative indices are invalid. This are results calculated from 
  // uninitialized values.
  cout << "COR [" << -2+itsIndex++ <<"]: "<< corr[0][0][0] << endl;
}

void WH_Dump::dump() {
}
		 
