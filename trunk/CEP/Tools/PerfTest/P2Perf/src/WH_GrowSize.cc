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
//  $Log$
//  Revision 1.15  2002/05/02 12:21:56  schaaf
//  Produce simple monitoring data in getMonitorValue() method
//
//  Revision 1.14  2002/04/18 07:55:03  schaaf
//  Documentation and code update
//
//  Revision 1.13  2002/04/12 15:51:44  schaaf
//  Explicit definition of source and destination side
//
//  Revision 1.12  2002/03/27 09:47:47  schaaf
//  Use get{Cur/Max}DataPacketSize
//
//  Revision 1.11  2002/03/18 12:18:56  schaaf
//  Uncommented performance output....
//
//  Revision 1.10  2002/03/08 11:38:42  wierenga
//  Upgraded from firewalls.h use to Debug.h use. This version was used for performance tests.
//
//  Revision 1.9  2001/12/18 12:57:58  schaaf
//  Added tag for performance measurement report
//
//  Revision 1.8  2001/12/17 16:30:00  schaaf
//  new logic in process() measurements counting
//
//  Revision 1.7  2001/11/28 16:15:40  schaaf
//  .
//
//  Revision 1.6  2001/11/05 17:01:07  schaaf
//  Output of multiple measurement on one line
//
//  Revision 1.5  2001/10/31 11:34:18  wierenga
//  LOFAR CVS Repository structure change and transition to autotools (autoconf, automake and libtool).
//
//  Revision 1.4  2001/10/26 10:06:28  wierenga
//  Wide spread changes to convert from Makedefs to autoconf/automake/libtool build environment
//
//  Revision 1.3  2001/09/19 08:10:51  wierenga
//  Changes to do perform bandwidth tests.
//
//  Revision 1.2  2001/09/19 08:00:13  wierenga
//  Added code to do performance tests.
//
//  Revision 1.1  2001/08/16 15:14:23  wierenga
//  Implement GrowSize DH and WH for performance measurements. Timing code still needs to be added.
//
//
//////////////////////////////////////////////////////////////////////

#include <stdio.h>             // for sprintf
#include <math.h>

#include "BaseSim/Step.h"
#include "Common/Debug.h"

#include "P2Perf/WH_GrowSize.h"
#include "P2Perf/StopWatch.h"
#include "P2Perf/P2Perf.h"

int  WH_GrowSize::itsMeasurements = 3;
bool WH_GrowSize::itsFirstcall = true;

WH_GrowSize::WH_GrowSize (const string& name, 
			  bool first,
			  unsigned int nin, 
			  unsigned int nout,
			  unsigned int nbuffer,
			  bool destside)
: WorkHolder    (nin, nout, name),
  itsInHolders  (0),
  itsOutHolders (0),
  itsBufLength  (nbuffer),
  itsIsDestSide (destside),
  itsFirst      (first),
  itsIteration(itsMeasurements),
  itsTime(0),
  itsReportPerformance(false)
{
  AssertStr (nin > 0,     "0 input DH_IntArray is not possible");
  AssertStr (nout > 0,    "0 output DH_IntArray is not possible");
  //   AssertStr (nout == nin, "number of inputs and outputs must match");
  
  itsInHolders  = new DH_GrowSize* [nin];
  itsOutHolders = new DH_GrowSize* [nout];
  char str[8];
  for (unsigned int i=0; i<nin; i++) {
    sprintf (str, "%d", i);
    itsInHolders[i] = new DH_GrowSize (std::string("in_") + str, nbuffer);
  }
  for (unsigned int i=0; i<nout; i++) {
    sprintf (str, "%d", i);
    itsOutHolders[i] = new DH_GrowSize (std::string("out_") + str, nbuffer);
  }

  // decrease the iteration counter at the destination side for
  // proper synchronisation of the increaseSize() method calls
  if (itsIsDestSide) {
    itsIteration --;
  }
}


WH_GrowSize::~WH_GrowSize()
{
  for (int i=0; i<getInputs(); i++) {
    delete itsInHolders[i];
  }
  for (int i=0; i<getOutputs(); i++) {
    delete itsOutHolders[i];
  }
  delete [] itsInHolders;
  delete [] itsOutHolders;
}

WorkHolder* WH_GrowSize::make(const string& name) const
{
  return new WH_GrowSize(name, 
			 itsFirst, 
			 getInputs(), 
			 getOutputs(), 
			 itsBufLength, 
			 itsIsDestSide);
}

void WH_GrowSize::preprocess() {
  return;
}


void WH_GrowSize::process()
{  
  itsOutHolders[0]->setTimeStamp(itsTime++);
  if (itsReportPerformance){
    if (!itsFirstcall) {
      watch.stop();
      if (itsIteration == 0) {
	// first measurement; print packet sizes etc.
	cout << endl;
	itsLastSize = itsInHolders[0]->getDataPacketSize(); 
	cout <<  itsLastSize << " "
	     << log10(itsLastSize) << " ";
      }
      // report the bandwidth per output channel (in MB/s)
      itsLastPerf = (int)(itsOutHolders[0]->getDataPacketSize() * getOutputs()
		     /(1024.*1024.*watch.elapsed()));
      cout << itsLastPerf 
	   << "  "
	   << watch.elapsed()
	   << "  ";
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
	for (int i=0; i<getInputs(); i++) {
	  TRACER3("Increase size of " << getName() << " input " << i);
	  (void)itsInHolders[i]->increaseSize(exp(log(MAX_GROW_SIZE)/1000));
	}
	for (int i=0; i<getOutputs(); i++) {
	  TRACER3("Increase size of " << getName() << " output " << i);
	  (void)itsOutHolders[i]->increaseSize(exp(log(MAX_GROW_SIZE)/1000));
	}
      }
  }
}

void WH_GrowSize::dump() const
{
}

DH_GrowSize* WH_GrowSize::getInHolder (int channel)
{
  AssertStr (channel >= 0,          "input channel too low");
  AssertStr (channel < getInputs(), "input channel too high");
  return itsInHolders[channel];
}
DH_GrowSize* WH_GrowSize::getOutHolder (int channel)
{
  AssertStr (channel >= 0,           "output channel too low");
  AssertStr (channel < getOutputs(), "output channel too high");
  return itsOutHolders[channel];
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
