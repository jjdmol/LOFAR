//  WH_GrowSize.cc: WorkHolder class using DH_Growsize() objects and 
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

#include "P2Perf/WH_GrowSize.h"
#include "P2Perf/StopWatch.h"
#include "P2Perf/P2Perf.h"

using namespace LOFAR;

int  WH_GrowSize::itsMeasurements = 3;
bool WH_GrowSize::itsFirstcall = true;

WH_GrowSize::WH_GrowSize (const string& name, 
			  bool first,
			  unsigned int nin, 
			  unsigned int nout,
			  unsigned int nbuffer,
			  bool destside,
			  bool sizeFixed)
: WorkHolder    (nin, nout, name),
  itsBufLength  (nbuffer),
  itsIsDestSide (destside),
  itsSizeFixed (sizeFixed),
  itsFirst      (first),
  itsIteration(itsMeasurements),
  itsTime(0),
  itsReportPerformance(false)
{
    AssertStr (nin > 0,     "0 input DH_IntArray is not possible");
    AssertStr (nout > 0,    "0 output DH_IntArray is not possible");
  //   AssertStr (nout == nin, "number of inputs and outputs must match");
  
  char str[8];

  for (unsigned int i=0; i<nin; i++) {
    sprintf (str, "%d", i);
    getDataManager().addInDataHolder(i, new DH_GrowSize (std::string("in_") + str,
				     nbuffer, itsSizeFixed), true);

    if (itsSizeFixed)
    {
      DbgAssertStr(getDataManager().getInHolder(0)->getType() == "DH_GrowSize",
               "DataHolder is not of type DH_GrowSize");    

      ((DH_GrowSize*)getDataManager().getGeneralInHolder(i))->
      setInitialDataPacketSize(nbuffer);
    }
  }
  
  for (unsigned int i=0; i<nout; i++) {
    sprintf (str, "%d", i);
    getDataManager().addOutDataHolder(i, new DH_GrowSize(std::string("out_") + str,
				      nbuffer, itsSizeFixed), true);
    if (itsSizeFixed)
    {
      DbgAssertStr(getDataManager().getOutHolder(0)->getType() == "DH_GrowSize",
               "DataHolder is not of type DH_GrowSize");    

      ((DH_GrowSize*)getDataManager().getGeneralOutHolder(i))->
	setInitialDataPacketSize(nbuffer);
    }
  }

  // decrease the iteration counter at the destination side for
  // proper synchronisation of the increaseSize() method calls
  if (itsIsDestSide) {
    itsIteration --;
  }
}


WH_GrowSize::~WH_GrowSize()
{
}

WorkHolder* WH_GrowSize::make(const string& name)
{
  return new WH_GrowSize(name, 
			 itsFirst, 
			 getDataManager().getInputs(), 
			 getDataManager().getOutputs(), 
			 itsBufLength, 
			 itsIsDestSide,
			 itsSizeFixed);
}

//  void WH_GrowSize::preprocess() {
//    return;
//  }

void WH_GrowSize::process()
{  
  getDataManager().getOutHolder(0)->setTimeStamp(itsTime++);
  getDataManager().getInHolder(0);

  if (itsReportPerformance){
    if (!itsFirstcall) {
       watch.stop();

#ifdef DO_WORK
      char* pcInData = (char*)getDataManager().getInHolder(0)->getDataPtr();
      char* pcOutData = (char*)getDataManager().getOutHolder(0)->getDataPtr();

      for (int i=0; i < 2; i++)
      {
	for (int j=0; j < getDataManager().getInHolder(0)->getDataPacketSize(); j++)
	{
	  pcOutData[j] = pcInData[j] * 2;
	}
      }
#endif

      if (itsIteration == 0) {
	// first measurement; print packet sizes etc.
	cout << endl;
	itsLastSize = getDataManager().getInHolder(0)->getDataPacketSize(); 
	cout <<  itsLastSize << " "
	     << log10(itsLastSize) << " ";
      }
      // report the bandwidth per output channel (in MB/s)
      itsLastPerf = (int)(getDataManager().getOutHolder(0)->getDataPacketSize() * getDataManager().getOutputs()
		     /(1024.*1024.*watch.elapsed()));
      cout << itsLastPerf 
	   << "  "
	   << watch.elapsed()
	   << "  ";
      watch.start();
     
    } else {
      if (itsIteration == 1) itsFirstcall = false;
    }
    watch.start();
  }
  {
    // perform every measurement Measurements times
    if (itsIteration-- == 0 )
      {
	itsIteration = itsMeasurements;
	if (!itsSizeFixed)
	{
	  for (int i=0; i<getDataManager().getInputs(); i++) {
	    TRACER3("Increase size of " << getName() << " input " << i);
	    DbgAssertStr(getDataManager().getOutHolder(0)->getType() == 
			 "DH_GrowSize", "DataHolder is not of type DH_GrowSize");  
	    (void)((DH_GrowSize*)getDataManager().getInHolder(i))->increaseSize(exp(log(MAX_GROW_SIZE)/1000));
	  }
	  for (int i=0; i<getDataManager().getOutputs(); i++) {
	    TRACER3("Increase size of " << getName() << " output " << i);
	    DbgAssertStr(getDataManager().getOutHolder(0)->getType() == 
			 "DH_GrowSize", "DataHolder is not of type DH_GrowSize");  
	    (void)((DH_GrowSize*)getDataManager().getOutHolder(i))->increaseSize(exp(log(MAX_GROW_SIZE)/1000));
	  }
	}
      }
  }
}

void WH_GrowSize::dump()
{
}

/**
   This getMonitorValue method pruduces output as follows:
   "size"  : current packet size
   "perf"  : current bandwidth
 */
int WH_GrowSize::getMonitorValue(const char* name){
  TRACER2("Called WH_GrowSize::getMonitorValue" << name);
  int result = 0;
  if (strcmp(name,"size") == 0) {
    result = itsLastSize;
    itsReportPerf = itsLastPerf;
  } else if (strcmp(name,"perf") == 0) {
    result = itsReportPerf > 0 ? itsReportPerf : itsLastPerf;
    itsReportPerf = -1;
  }
  TRACER2("WH_Empty::getMonitorValue resturns " << result);
  return result;
}
