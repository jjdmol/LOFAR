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

#include <lofar_config.h>

#include <math.h>

#include <AsyncTest/WH_Sink.h>
#include <AsyncTest/DH_Buffer.h>
#include <AsyncTest/StopWatch.h>
#include <AsyncTest/AsyncTest.h>
#include <Common/LofarLogger.h>

using namespace LOFAR;

int  WH_Sink::itsMeasurements = 10000;
bool WH_Sink::itsFirstcall = true;

WH_Sink::WH_Sink (const string& name, 
			  unsigned int nin, 
			  unsigned int nout,
			  unsigned int nbuffer,
			  bool sizeFixed,
		          bool syncRead)
: WorkHolder    (nin, nout, name),
  itsBufLength  (nbuffer),
  itsSizeFixed  (sizeFixed),
  itsSyncRead   (syncRead),
  itsIteration  (itsMeasurements),
  itsTime       (0)
{
  char str[8];

  for (unsigned int i=0; i<nin; i++) {
    sprintf (str, "%d", i);
    getDataManager().addInDataHolder(i, new DH_Buffer (std::string("in_") + str,
				     nbuffer));
  }
  
  for (unsigned int i=0; i<nout; i++) {
    sprintf (str, "%d", i);
    getDataManager().addOutDataHolder(i, new DH_Buffer(std::string("out_") + str,
				      nbuffer));

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
  int count = ((DH_Buffer*)(getDataManager().getInHolder(0)))->getCounter();
  for (int i = 1; i < getDataManager().getInputs(); i++)
  {
    if (((DH_Buffer*)(getDataManager().getInHolder(i)))->getCounter() != count)
    {
      cout << "Incorrect data received!" << endl;
    }                                    
    //NB: It is necessary to request all InHolders, otherwise buffer won't 
    // be emptied
  } 
  cout << "  Received counter: "
       << count << endl;
  
    if (!itsFirstcall)
    {
      // watch.stop();

      if (itsIteration == 0)
      {

	watch.stop();
	// first measurement; print packet sizes etc.
		cout << endl;
	int dataSize = getDataManager().getInHolder(0)->getDataSize(); 
	cout <<  "Data size " << dataSize << " ";

	// report the bandwidth per output channel (in MB/s)
	int perf = (int) (dataSize * getDataManager().getInputs() * itsMeasurements
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

  //dump();
}

void WH_Sink::dump() const
{
  cout << "WH_Sink " << getName() << " dump:" << endl;
  for (int i = 0; i < getDataManager().getInputs(); i++)
  {
    getDataManager().getInHolder(i)->dump();
  }
  for (int j = 0; j < getDataManager().getOutputs(); j++)
  {
    getDataManager().getOutHolder(j)->dump();
  }
}

