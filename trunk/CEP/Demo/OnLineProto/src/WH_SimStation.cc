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
    : WorkHolder   (1, nout, name,"WH_SimStation"),
      itsFileName  (fileName),
      itsInputFile (fileName.c_str(),  ifstream::binary|ifstream::in),
      itsMac       (mac),
      itsID        (ID)
  {
    char str[8];

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
							 mac.getFrequency(i),
							 mac.getChannelBandwidth(),
							 mac.getStartHourangle(),
							 mac.getBeamletSize()));
    }

    // Allocate buffer data, the add one is for the elapsed time
    // so the structure is t0 c1 ... cn  
    //                     t1 c1 ... cn  where n is the total number of channels
    itsData = (complex<float>*)malloc((mac.getNumberOfBeamlets()*mac.getBeamletSize()+1) * sizeof (complex<float>));
    itsCounter = 0;
  }
  
  WH_SimStation::~WH_SimStation()
  {
    itsInputFile.close();
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

    if (itsCounter == 0) {
      ReadData ();
      itsCounter = DATA_ITERATION;
    } else {
      itsCounter--;
    }

    float t = (float)(itsData[0].real())+
      (0.05*(DATA_ITERATION-itsCounter)/DATA_ITERATION);
    
    for (int i = 0; i < itsMac.getNumberOfBeamlets(); ++i) {
      ((DH_Beamlet*)getDataManager().getOutHolder(i))->setElapsedTime(t);
      for (int j = 0; j < itsMac.getBeamletSize(); j++) { 
	*((DH_Beamlet*)getDataManager().getOutHolder(i))->getBufferElement(j) 
	  = itsData[i * itsMac.getBeamletSize() + j + 1];
      }
    }
  }
  
  void WH_SimStation::dump()
  {
    cout << "WH_SimStation " << getName () << " Buffers:" << endl;
    //   for (int i = 0; i < itsMac.getNumberOfBeamlets(); i++) {
      for (int j = 0; j < itsMac.getBeamletSize(); j++) {
	cout << *((DH_Beamlet*)getDataManager().getOutHolder(0))->getBufferElement(j) << ' ';
      }
      cout << endl;
      //}
  }

  void WH_SimStation::ReadData ()
  {
    complex<float> InputData[NINPUT_BEAMLETS*itsMac.getBeamletSize()+1];

    // Read the data into the blitz array / matrix
    if (!itsInputFile.eof()) {
      itsInputFile.read((char*)InputData, 
		  (NINPUT_BEAMLETS*itsMac.getBeamletSize()+1)*sizeof(complex<float>));
    } else {
      // Wrap around
      itsInputFile.seekg(0);
      itsInputFile.read((char*)InputData,
		  (NINPUT_BEAMLETS*itsMac.getBeamletSize()+1)*sizeof(complex<float>));
    }

    memcpy (itsData, InputData, itsMac.getNumberOfBeamlets()*itsMac.getBeamletSize()*sizeof(complex<float>));
  }
}// namespace LOFAR
