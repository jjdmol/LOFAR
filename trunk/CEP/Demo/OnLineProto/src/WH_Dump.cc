//  WH_Dump.cc:
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
#include "OnLineProto/WH_Dump.h"
#include "OnLineProto/DH_Vis.h"


// Print interval
#define INTERVAL 10

namespace LOFAR
{

WH_Dump::WH_Dump (const string& name,
			      unsigned int nin)
  : WorkHolder    (nin, 1, name,"WH_Dump"),
    itsIndex   (0),
    itsCounter (0),
    itsBuffer  (0),
    handle     (0)
{
  char str[8];
  // create the dummy input dataholder
  for (unsigned int i=0; i<nin; i++) {
    sprintf (str, "%d", i);
    getDataManager().addInDataHolder(i, 
				     new DH_Vis (string("in_") + str),
				     true);
  }

  // create the dummy output dataholder
    sprintf (str, "%d", 1);
    getDataManager().addOutDataHolder(0, 
				      new DH_Empty (string("out_") + str), 
				      true);
    // open output file. filename is hardcoded.
    itsOutputFile.open("output.out");

    itsBuffer.resize(NSTATIONS);
    itsBuffer = complex<float> (0,0);

    handle = gnuplot_init ();
}


WH_Dump::~WH_Dump()
{
  itsOutputFile.close();
}

WorkHolder* WH_Dump::construct (const string& name, 
				      int nin)
{
  return new WH_Dump (name, nin);
}

WH_Dump* WH_Dump::make (const string& name)
{
  return new WH_Dump (name, getDataManager().getInputs());
}

void WH_Dump::process()
{
  blitz::Array<complex<float>, 2> corr(NSTATIONS, NSTATIONS);
  blitz::Array<float, 1> plotBuffer (NSTATIONS) ;
  corr = complex<float> (0,0);

  TRACER4("WH_Dump::Process()");
  

  itsCounter++;
  DH_Vis *InDHptr = (DH_Vis*)getDataManager().getInHolder(0);
  if (itsCounter % INTERVAL == 0) {
    memcpy (corr.data(), 
	    InDHptr->getBuffer(), 
	    NSTATIONS*NSTATIONS*sizeof(DH_Vis::BufferType));

    cout << corr(blitz::Range(0,9), blitz::Range(0,9)) << endl;
    
    itsBuffer(itsIndex % NSTATIONS) = corr (0,NSTATIONS-1);
    
    if (itsIndex >= NSTATIONS) {
      // the buffer is filled, we can plot the resulting graph
      
//       gnuplot_plot_x ( handle,
// 		       sqrt ( sqr ( real ( itsBuffer ) ) +
// 			      sqr ( imag ( itsBuffer ) ) ).data(),
// 		       itsBuffer.size(),
// 		       "Power of correlation between station 0 and 99 over time" );

      plotBuffer = sqrt ( sqr ( imag ( itsBuffer ) ) +
			  sqr ( real ( itsBuffer ) ) );
		    

      gnuplot_plot_x ( handle, 
		       plotBuffer.data(),
		       itsBuffer.size(),
		       "Power of correlation between station 0 and 99 over time" );

//       gnuplot_plot_x ( handle, 
// 		       real ( itsBuffer ).data(),
// 		       itsBuffer.size(), 
// 		       "Real part of correlation between station 0 and 99 over time" );
    }

    itsIndex++;
  }
}

void WH_Dump::dump()
{
}

}// namespace LOFAR
