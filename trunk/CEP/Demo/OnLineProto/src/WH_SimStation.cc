//  WH_SimStation.cc:
//
//  Copyright (C) 2000, 2001
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
//
//////////////////////////////////////////////////////////////////////

#include <stdio.h>             // for sprintf
#include <Common/Debug.h>

// CEPFrame general includes
#include "CEPFrame/DH_Empty.h"
#include "CEPFrame/Step.h"
#include <Common/KeyValueMap.h>

// OnLineProto specific include
#include "OnLineProto/WH_SimStation.h"

namespace LOFAR
{
  WH_SimStation::WH_SimStation (const string& name,
				const int nout,
				const string fileName,
				MAC mac,
				const int ID)
    : WorkHolder  (1, nout, name,"WH_SimStation"),
      itsFileName (fileName),
      itsMac      (mac),
      itsID       (ID)
  {
    char str[8];
    LoVec_float FreqSlice = mac.getFrequencies ();

    // create the dummy input dataholder
    sprintf (str, "%d", 1);
    getDataManager().addInDataHolder(0, new DH_Empty (string("in_") + str), true);
    
    // create the output dataholders
    for (int i = 0; i < mac.getNumberOfBeamlets (); i++) {
      sprintf (str, "%d", i);
      int bs = mac.getBeamletSize (); 
      getDataManager().addOutDataHolder (i, 
					 new DH_Beamlet (string("out_") + str,
							 ID,
							 FreqSlice (blitz::Range (i*bs, i*bs+bs-1)),
							 mac.getStartHourangle (),
							 mac.getBeamletSize ()
							 )
					 );
    }    

    ReadData (fileName);
  }
  
  WH_SimStation::~WH_SimStation()
  {
  }
  
  WorkHolder* WH_SimStation::construct (const string& name, 
					const int nout,
					const string fileName,
					const MAC mac,
					const int ID)
  {
    return new WH_SimStation (name, nout, fileName, mac, ID);
  }
  
  WH_SimStation* WH_SimStation::make (const string& name)
  {
    return new WH_SimStation (name, getDataManager().getOutputs(), 
			      itsFileName, itsMac, itsID);
  }
  
  void WH_SimStation::process()
  {
    TRACER4("WH_SimStation::Process()");  
    float ha;
  
    for (int i = 0; i < itsMac.getNumberOfBeamlets (); ++i) {
      ha = (float)(itsData(i,0).real());
      ((DH_Beamlet*)getDataManager().getOutHolder(i))->setHourangle(ha);
      for (int j = 1; j < itsMac.getBeamletSize (); j++) { 
	*((DH_Beamlet*)getDataManager().getOutHolder(i))->getBufferElement(j) 
	  = itsData(i,j);
      }
    }
  }
  
  void WH_SimStation::dump()
  {
//     cout << "WH_ReadSignal " << getName () << " Buffers:" << endl;
//     cout << itsOutHolders[0]->getBuffer ()[0] << ','
// 	 << itsOutHolders[getOutputs () - 1]->getBuffer ()[0] << endl;
  }

  void WH_SimStation::ReadData (const string fileName)
  {
    // Open a stream to the specified file
    ifstream inputFile(fileName.c_str(), ifstream::in);
    
    // Read the data into the blitz array / matrix
    inputFile >> itsData;
  }
}// namespace LOFAR
