//  WH_PreProcess.cc:
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
#include "CEPFrame/Step.h"
#include <Common/KeyValueMap.h>

// OnLineProto specific include
#include "OnLineProto/DH_CorrectionMatrix.h"
#include "OnLineProto/WH_PreProcess.h"

namespace LOFAR
{
WH_PreProcess::WH_PreProcess (const string& name, 
			      const int nbeamlets, 
			      const ACC::ParameterSet& ps, 
			      const int StationID)
  : WorkHolder    (nbeamlets+1, nbeamlets, name,"WH_PreProcess"),
    itsPS (ps),
    itsStationID (StationID)
{
  char str[8];
  int bs = itsPS.getInt("station.nchannels"); 
    
  for (int i = 0; i < nbeamlets; i++) {
    sprintf (str, "%d", i);

    // create the input dataholders
    getDataManager().addInDataHolder(i, new DH_Beamlet (string("PP_in_") + str,
							itsStationID,
							itsPS.getFloat(string("station.beamlet.") + str),
							itsPS.getFloat("station.chan_bw"),
							itsPS.getFloat("observation.ha_0"),
							itsPS.getInt("station.nchannels")));

    // create the output dataholders
    getDataManager().addOutDataHolder(i, new DH_Beamlet (string("PP_out_") + str,
							 itsStationID,
							 itsPS.getFloat(string("station.beamlet.") + str),
							 itsPS.getFloat("station.chan_bw"),
							 itsPS.getFloat("observation.ha_0"),
							 itsPS.getInt("station.nchannels")));
  }

  getDataManager().addInDataHolder(nbeamlets, new DH_CorrectionMatrix ("in_fringe", 1, itsPS.getInt("station.nchannels")));  
}
  
WH_PreProcess::~WH_PreProcess()
{
}

WorkHolder* WH_PreProcess::construct (const string& name, 
				      const int nbeamlets, 
				      const ACC::ParameterSet& ps,
				      const int StationID)
{
  return new WH_PreProcess (name, nbeamlets, ps, StationID);
}

WH_PreProcess* WH_PreProcess::make (const string& name)
{
  return new WH_PreProcess (name, getDataManager().getOutputs(), itsPS, itsStationID);
}

void WH_PreProcess::process()
{
  TRACER4("WH_PreProcess::Process()");

  float t_i;
  float ha;
  complex<float> phase;
  complex<float> i2pi (0,2*itsPS.getFloat("general.pi"));
  int b =  itsPS.getInt("station.nbeamlets");
  char str[8];
  sprintf(str, "%d",itsStationID);
  string sID (string("station.")+str+string("."));

  for (int i = 0; i < b; i++) {
    // set elapsed time
    ((DH_Beamlet*)getDataManager().getOutHolder(i))->
      setElapsedTime(((DH_Beamlet*)getDataManager().getInHolder(i))->getElapsedTime());

    for (int j = 0; j < itsPS.getInt("station.nchannels"); j++) {
      
      if (itsPS.getInt("general.enable_fs") == 1) 
	{
	  
	  // calculate hourangle
	  ha = ((DH_Beamlet*)getDataManager().getInHolder(i))->getElapsedTime() * 
	    itsPS.getFloat("general.we") + itsPS.getFloat("observation.ha_0");
	  
	  // calculate time delay
	  if (itsStationID == 0) 
	    {
	      t_i = (itsPS.getFloat(sID+"y") * sin(ha) * cos(itsPS.getFloat("observation.dec")) 
		     + itsPS.getFloat(sID+"x") * cos(ha) * cos(itsPS.getFloat("observation.dec"))) 
		/ itsPS.getFloat("general.c");
	    } 
	  else 
	    {
	      t_i = (350000 * cos(ha) * cos(itsPS.getFloat("observation.dec"))) 
	       / itsPS.getFloat("general.c");
	    }
	  
	  // delay tracking phase rotation
	  phase = exp (i2pi * (complex<float>)(i *(32000000/32768)*256  + 
					       j * 32000000/32768 + 32000000/(32768*2)) * t_i);
	  
	  // fringe stopping phase rotation
	  phase *= exp (i2pi * (complex<float>)224000000 * t_i);
	  
	  // apply phase shift
	  *((DH_Beamlet*)getDataManager().getOutHolder(i))->getBufferElement(j) 
	    = phase * *((DH_Beamlet*)getDataManager().getInHolder(i))->getBufferElement(j);
	} 
      else 
	{
	  *((DH_Beamlet*)getDataManager().getOutHolder(i))->getBufferElement(j)
	    = *((DH_Beamlet*)getDataManager().getInHolder(i))->getBufferElement(j);
	}
    }
  }
}

void WH_PreProcess::dump()
{    
  cout << "WH_PreProcess " << getName () << " Buffers:" << endl;
  for (int i = 0; i < MIN(itsPS.getInt("station.nbeamlets"),1); i++) {
    for (int j = 0; j < MIN(itsPS.getInt("station.nchannels"),10); j++) {
      cout << *((DH_Beamlet*)getDataManager().getOutHolder(i))->getBufferElement(j) << ' ';
    }
    cout << endl;
  }
}

}// namespace LOFAR
