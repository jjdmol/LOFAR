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

#include <lofar_config.h>

#include <math.h>

#include <AsyncTest/WH_Source.h>
#include <AsyncTest/DH_Buffer.h>
#include <AsyncTest/StopWatch.h>
#include <AsyncTest/AsyncTest.h>

using namespace LOFAR;

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
  itsSyncWrite(syncWrite),
  itsIteration(1),
  itsTime(0)
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
  for (int i = 0; i < getDataManager().getOutputs(); i++)
  {
    ((DH_Buffer*)(getDataManager().getOutHolder(i)))->setCounter(itsIteration);
  }
  itsIteration++;
  //dump();
}

void WH_Source::dump()
{
  cout << "WH_Source " << getName() << " dump:" << endl;
  for (int i = 0; i < getDataManager().getInputs(); i++)
  {
    getDataManager().getInHolder(i)->dump();
  }
  for (int j = 0; j < getDataManager().getOutputs(); j++)
  {
    getDataManager().getOutHolder(j)->dump();
  }
}
