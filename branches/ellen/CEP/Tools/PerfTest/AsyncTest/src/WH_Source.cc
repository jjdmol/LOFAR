//  WH_Source.cc: WorkHolder class using DH_Growsize() objects and 
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

#include "AsyncTest/WH_Source.h"
#include "AsyncTest/StopWatch.h"
#include "AsyncTest/AsyncTest.h"

int  WH_Source::itsMeasurements = 1000;
bool WH_Source::itsFirstcall = true;

WH_Source::WH_Source (const string& name, 
			  unsigned int nin, 
			  unsigned int nout,
			  unsigned int nbuffer,
			  bool sizeFixed, 
		          bool syncWrite)
: WorkHolder    (nin, nout, name),
  itsBufLength  (nbuffer),
  itsSizeFixed (sizeFixed),
  itsIteration(itsMeasurements),
  itsTime(0),
  itsSyncWrite(syncWrite)
{
    AssertStr (nin > 0,     "0 input DH_IntArray is not possible");
    AssertStr (nout > 0,    "0 output DH_IntArray is not possible");
  //   AssertStr (nout == nin, "number of inputs and outputs must match");
  
  char str[8];

  for (unsigned int i=0; i<nin; i++) {
    sprintf (str, "%d", i);
    getDataManager().addInDataHolder(i, new DH_GrowSize (std::string("in_") + str,
				     nbuffer, itsSizeFixed), true);

    DbgAssertStr(getDataManager().getGeneralInHolder(i)->getType() == "DH_GrowSize",
               "DataHolder is not of type DH_GrowSize");    

    ((DH_GrowSize*)getDataManager().getGeneralInHolder(i))->
      setInitialDataPacketSize(nbuffer);

  }
  
  for (unsigned int i=0; i<nout; i++) {
    sprintf (str, "%d", i);
    getDataManager().addOutDataHolder(i, new DH_GrowSize(std::string("out_") + str,
				      nbuffer, itsSizeFixed), itsSyncWrite);

    DbgAssertStr(getDataManager().getGeneralOutHolder(i)->getType() == "DH_GrowSize",
               "DataHolder is not of type DH_GrowSize");    

    ((DH_GrowSize*)getDataManager().getGeneralOutHolder(i))->
	setInitialDataPacketSize(nbuffer);
  }

}


WH_Source::~WH_Source()
{
}

WorkHolder* WH_Source::make(const string& name)
{
  return new WH_Source(name, 
		       getDataManager().getInputs(), 
		       getDataManager().getOutputs(), 
		       itsBufLength, 
		       itsSizeFixed,
		       itsSyncWrite);
}

//  void WH_Source::preprocess() {
//    return;
//  }

void WH_Source::process()
{ 
    getDataManager().getOutHolder(0)->setTimeStamp(itsTime++);
    // getDataManager().getInHolder(0);
   //dump();
}

void WH_Source::dump()
{
  cout << "WH_Source " << getName() << " dump:" << endl;
  for (int i = 0; i < getDataManager().getInputs(); i++)
  {
    ((DH_GrowSize*)getDataManager().getInHolder(i))->dump();
  }
  for (int j = 0; j < getDataManager().getOutputs(); j++)
  {
    ((DH_GrowSize*)getDataManager().getOutHolder(j))->dump();
  }
}
