//  WH_Src.cc: WorkHolder class using DH_VarSize() objects and 
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

#include "P2Perf/WH_Src.h"
#include "P2Perf/StopWatch.h"
#include "P2Perf/P2Perf.h"

using namespace LOFAR;

WH_Src::WH_Src (DHGrowStrategy* DHGS, // the object that will grow the DH's
                const string& name, 
                unsigned int nout,
		unsigned int size,    // size of the packet in bytes
                unsigned int measurementsPerGrowStep,
                unsigned int packetsPerMeasurement)
                
		: WorkHolder    (0, nout, name),
		itsSize (size),
		itsReportPerformance(false),
		itsDHGrowStrategy(DHGS),
                itsMeasurementsPerGrowStep(measurementsPerGrowStep),
                itsMeasurementNumber(0),
                itsPacketsPerMeasurement(packetsPerMeasurement),
                itsPacketNumber(0)

{
  char str[8];

  watch = StopWatch::getGlobalStopWatch();
  
  for (unsigned int i=0; i<nout; i++) {
    sprintf (str, "%d", i);
    getDataManager().addOutDataHolder(i, 
                                      new DH_VarSize(std::string("out_") + str,
                                                     itsSize));
  }
  // this makes the process method nicer, but it means the first measurement is incorrect
  if (itsReportPerformance)
    watch->start();
}

	       
WH_Src::~WH_Src()
{
}

WorkHolder* WH_Src::make(const string& name)
{
  return new WH_Src(itsDHGrowStrategy,
		    name, 
		    getDataManager().getOutputs(), 
		    itsSize,
                    itsMeasurementsPerGrowStep,
                    itsPacketsPerMeasurement
		    );
}

void WH_Src::process()
{  
  //getDataManager().getOutHolder(0)->setTimeStamp(itsTime++);
  // pausing the watch is nice to get more accurate measurements, but
  // it does not work when using multiple threads or processes
  // because each process will pause its own stopwatch and the time
  // will not be subtracted from the time of the measuring stopwatch
  watch->pause();
  if (++itsPacketNumber == itsPacketsPerMeasurement)
    {
      itsPacketNumber = 0;
      if (itsReportPerformance)
	{
          watch->stop();
          
	  // report performance to std out
	  int dataSize = getDataManager().getOutHolder(0)->getDataSize(); 
	  double totDataSize = dataSize;
	  totDataSize *= itsPacketsPerMeasurement;
	  totDataSize *= getDataManager().getOutputs();
	  totDataSize /= (1024*1024);
	  double speed = totDataSize;
	  speed /= watch->elapsed();

	  cout << (dataSize ) << " Bpp "
	       << speed << " MB/s     ("
	       << getDataManager().getOutputs() << " outputs x "
	       << itsPacketsPerMeasurement << " packets "
	       << watch->elapsed() << " seconds) measurement "
	       << itsMeasurementNumber
	       << endl;

	  watch->start();
	  watch->pause();
	}
      if (++itsMeasurementNumber == itsMeasurementsPerGrowStep)
	{
          itsMeasurementNumber = 0;

	  // change size of the DataHolder
	  //TRACER2("growing dataSize of " << getName() << " from " << getDataManager().getOutHolder(0)->getDataSize() << " to ");
	  itsDHGrowStrategy->growDHs (&getDataManager());
	  //TRACER2(getDataManager().getOutHolder(0)->getDataSize());
	}
    }
  watch->resume();
}

