//  WH_Sink.cc: WorkHolder class using DH_Example() objects and 
//                  measuring performance
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
#include <math.h>

#include "CEPFrame/Step.h"
#include <Common/Debug.h>

#include "InOutTest/WH_Sink.h"
#include "InOutTest/StopWatch.h"
#include "InOutTest/InOutTest.h"

bool WH_Sink::itsFirstcall = true;

WH_Sink::WH_Sink (const string& name, 
			  bool first,
			  unsigned int nbuffer,
		          unsigned int niterations)
: WorkHolder    (1, 0, name),
  itsBufLength  (nbuffer),
  itsFirst      (first),
  itsIteration(niterations),
  itsNIterations(niterations),
  itsTime(0)
{
  getDataManager().addInDataHolder(0, new DH_Example ("in", nbuffer), true);
}


WH_Sink::~WH_Sink()
{
}

WorkHolder* WH_Sink::make(const string& name)
{
  return new WH_Sink(name, 
		     itsFirst, 
		     itsBufLength,
		     itsNIterations);
}

//  void WH_Sink::preprocess() {
//    return;
//  }

void WH_Sink::process()
{  
  getDataManager().getInHolder(0);

  if (!itsFirstcall)
  {
    // watch.stop();

    if (itsIteration == 0)
    {

      watch.stop();
      // first measurement; print packet sizes etc.
      cout << endl;
      itsLastSize = getDataManager().getInHolder(0)->getDataPacketSize(); 
      cout <<  itsLastSize << " "
           << log10(itsLastSize) << " ";

      // report the bandwidth per output channel (in MB/s)
      itsLastPerf = (int)(getDataManager().getInHolder(0)->getDataPacketSize() * getDataManager().getInputs() * (itsNIterations
			    /(1024.*1024.*watch.elapsed())));
      cout << itsLastPerf 
	   << "  "
	   << watch.elapsed()
	   << "  ";
      watch.start();
    }

  }
  else
  {
    if (itsIteration == itsNIterations) watch.start();
    if (itsIteration == 1) itsFirstcall = false;
  }

  // perform Measurements times before printing
  if (itsIteration-- == 0 )
  {
    itsIteration = itsNIterations;
  }

  // Output buffer
//    DH_Example::BufferType* inbuf = 
//                    ((DH_Example*)getDataManager().getInHolder(0))->getBuffer();
//    cout << "Sink data "
//         << inbuf[0] << ',' << inbuf[itsBufLength-1] << endl;

}

void WH_Sink::dump()
{
  cout << "WH_Sink " << getName() << " dump:" << endl;
  ((DH_Example*)getDataManager().getInHolder(0))->dump();
}

