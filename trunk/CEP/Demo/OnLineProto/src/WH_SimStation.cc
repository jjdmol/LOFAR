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
				const string& fileName,
				const ACC::ParameterSet& ps,
				const int ID)
    : WorkHolder   (1, nout, name,"WH_SimStation"),
      itsFileName  (fileName),
      itsInputFile (fileName.c_str(),  ifstream::binary|ifstream::in),
      itsPS        (ps),
      itsID        (ID)
  {
    char str[8];

    // create the dummy input dataholder
    sprintf (str, "%d", 1);
    getDataManager().addInDataHolder(0, new DH_Empty (string("SimS_in_") + str));
    int bs = itsPS.getInt("station.nbeamlets"); 
    
    // create the output dataholders
    for (int i = 0; i < bs; i++) {
      sprintf (str, "%d", i);
      getDataManager().addOutDataHolder (i, 
					 new DH_Beamlet (string("SimS_out_") + str,
							 ID,
							 itsPS.getFloat(string("station.beamlet.") + str),
							 itsPS.getFloat("station.chan_bw"),
							 itsPS.getFloat("observation.ha_0"),
							 itsPS.getInt("station.nchannels")));
    }

    // Allocate buffer data, the add one is for the elapsed time
    // so the structure is t0 c1 ... cn  
    //                     t1 c1 ... cn  where n is the total number of channels
    itsData = (complex<float>*)malloc((bs*itsPS.getInt("station.nchannels")+1) * sizeof (complex<float>));
    itsCounter = 0;
  }
  
  WH_SimStation::~WH_SimStation()
  {
    free (itsData);
    itsInputFile.close();
  }
  
  WorkHolder* WH_SimStation::construct (const string& name, 
					const int nout,
					const string fileName,
					const ACC::ParameterSet& ps,
					const int ID)
  {
    return new WH_SimStation (name, nout, fileName, ps, ID);
  }
  
  WH_SimStation* WH_SimStation::make (const string& name)
  {
    return new WH_SimStation (name, getDataManager().getOutputs(), 
			      itsFileName, itsPS, itsID);
  }
  
  void WH_SimStation::process()
  {
    TRACER4("WH_SimStation::Process()");  

    if (itsCounter == 0) {
      ReadData ();
      itsCounter = itsPS.getInt("corr.tsize");
    } else {
      itsCounter--;
    }

    float t = (float)(itsData[0].real())+ (0.05*(itsPS.getInt("corr.tsize")-itsCounter)
					   /itsPS.getInt("corr.tsize"));
    
    for (int i = 0; i < itsPS.getInt("station.nbeamlets"); ++i) {
      ((DH_Beamlet*)getDataManager().getOutHolder(i))->setElapsedTime(t);
      for (int j = 0; j < itsPS.getInt("station.nchannels"); j++) { 
	*((DH_Beamlet*)getDataManager().getOutHolder(i))->getBufferElement(j) 
	  = itsData[i * itsPS.getInt("station.nchannels") + j + 1];
      }
    }
  }
  
  void WH_SimStation::dump()
  {
    cout << "WH_SimStation " << getName () << " Buffers:" << endl;

    for (int i = 0; i < MIN(itsPS.getInt("station.nbeamlets"), 1) ; i++) { 
      for (int j = 0; j < MIN(itsPS.getInt("station.nchannels"), 10) ; j++) { 
	cout << *((DH_Beamlet*)getDataManager().getOutHolder(i))->getBufferElement(j) << ' ';
      }
      cout << endl;
    }
  }

  void WH_SimStation::ReadData ()
  {
    std::streamsize buffersize = (NINPUT_BEAMLETS*itsPS.getInt("station.nchannels")+1)*sizeof(complex<float>);
    char* InputBuffer = (char*)malloc(buffersize);

    ASSERTSTR( itsInputFile.is_open(), "WH_SimStation input file " 
	       << itsFileName << " could not be opened.");


    // Read the data
    if (!itsInputFile.eof()) {
      itsInputFile.read(InputBuffer, buffersize);
    } else {
      // Wrap around
      itsInputFile.seekg(0);
      itsInputFile.read(InputBuffer, buffersize);
    }

    memcpy (itsData, InputBuffer, itsPS.getInt("station.nbeamlets")*itsPS.getInt("station.nchannels")*sizeof(complex<float>));
    free (InputBuffer);
  }
}// namespace LOFAR
