//  WH_Sink.cc: WorkHolder class using DH_Growsize() objects and 
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

#include "AsyncTest/WH_Sink.h"
#include "AsyncTest/StopWatch.h"
#include "AsyncTest/AsyncTest.h"

using namespace LOFAR;

int  WH_Sink::itsMeasurements = 1000;
bool WH_Sink::itsFirstcall = true;

WH_Sink::WH_Sink (const string& name, 
			  unsigned int nin, 
			  unsigned int nout,
			  unsigned int nbuffer,
			  bool sizeFixed,
		          bool syncRead)
: WorkHolder    (nin, nout, name),
  itsBufLength  (nbuffer),
  itsSizeFixed (sizeFixed),
  itsIteration(itsMeasurements),
  itsTime(0),
  itsSyncRead(syncRead)
{
    AssertStr (nin > 0,     "0 input DH_IntArray is not possible");
    AssertStr (nout > 0,    "0 output DH_IntArray is not possible");
  //   AssertStr (nout == nin, "number of inputs and outputs must match");
  
  char str[8];

  for (unsigned int i=0; i<nin; i++) {
    sprintf (str, "%d", i);
    getDataManager().addInDataHolder(i, new DH_GrowSize (std::string("in_") + str,
				     nbuffer, itsSizeFixed), itsSyncRead);

    DbgAssertStr(getDataManager().getGeneralInHolder(i)->getType() == "DH_GrowSize",
               "DataHolder is not of type DH_GrowSize");    

    ((DH_GrowSize*)getDataManager().getGeneralInHolder(i))->
      setInitialDataPacketSize(nbuffer);

  }
  
  for (unsigned int i=0; i<nout; i++) {
    sprintf (str, "%d", i);
    getDataManager().addOutDataHolder(i, new DH_GrowSize(std::string("out_") + str,
				      nbuffer, itsSizeFixed), true);

    DbgAssertStr(getDataManager().getGeneralOutHolder(i)->getType() == "DH_GrowSize",
               "DataHolder is not of type DH_GrowSize");    

    ((DH_GrowSize*)getDataManager().getGeneralOutHolder(i))->
	setInitialDataPacketSize(nbuffer);
  }

}


WH_Sink::~WH_Sink()
{
}

WorkHolder* WH_Sink::make(const string& name)
{
  return new WH_Sink(name, 
		     getDataManager().getInputs(), 
		     getDataManager().getOutputs(), 
		     itsBufLength, 
		     itsSizeFixed,
		     itsSyncRead);
}

//  void WH_Sink::preprocess() {
//    return;
//  }

void WH_Sink::process()
{  
  getDataManager().getInHolder(1); //Necessary, otherwise buffer won't 
                                   //be emptied
  getDataManager().getInHolder(0);

//    unsigned long inputTime = getDataManager().getInHolder(0)->getTimeStamp();
//    if (inputTime != getDataManager().getInHolder(1)->getTimeStamp())
//    {
//      cout << "Timestamps not equal" << endl;
//    }
//    if ((unsigned long)itsTime != inputTime)
//    {
//      cout << "Timestamp is different" << endl;
//    }
//    cout << inputTime << endl;

    if (!itsFirstcall)
    {
      // watch.stop();

      if (itsIteration == 0)
      {

	watch.stop();
	// first measurement; print packet sizes etc.
		cout << endl;
	int packetSize = getDataManager().getInHolder(0)->getDataPacketSize(); 
	cout <<  packetSize << " "
	     << log10(packetSize) << " ";

	// report the bandwidth per output channel (in MB/s)
	int perf = (int)(getDataManager().getOutHolder(0)->getDataPacketSize() * getDataManager().getInputs() * itsMeasurements
			    /(1024.*1024.*watch.elapsed()));
	cout << perf 
	     << "  "
	     << watch.elapsed()
	     << "  ";
	watch.start();
      }

    }
    else
    {
      if (itsIteration == itsMeasurements) watch.start();
      if (itsIteration == 1) itsFirstcall = false;
    }

  // perform Measurements times before printing
  if (itsIteration-- == 0 )
  {
    itsIteration = itsMeasurements;
  }

  getDataManager().getOutHolder(0)->setTimeStamp(itsTime++);
  //dump();
}

void WH_Sink::dump()
{
  cout << "WH_Sink " << getName() << " dump:" << endl;
  for (int i = 0; i < getDataManager().getInputs(); i++)
  {
    ((DH_GrowSize*)getDataManager().getInHolder(i))->dump();
  }
  for (int j = 0; j < getDataManager().getOutputs(); j++)
  {
    ((DH_GrowSize*)getDataManager().getOutHolder(j))->dump();
  }
}

