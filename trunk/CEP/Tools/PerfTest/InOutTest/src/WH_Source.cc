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

#include <CEPFrame/Step.h>

#include <InOutTest/WH_Source.h>
#include <InOutTest/StopWatch.h>
#include <CEPFrame/DH_Example.h>

using namespace LOFAR;

WH_Source::WH_Source (const string& name, unsigned int nbuffer)
: WorkHolder    (0, 1, name),
  itsBufLength  (nbuffer),
  itsIteration(0)
{
  getDataManager().addOutDataHolder(0, new DH_Example("out_", nbuffer));
}


WH_Source::~WH_Source()
{
}

WorkHolder* WH_Source::make(const string& name)
{
  return new WH_Source(name, itsBufLength);
}


void WH_Source::process()
{ 
  DH_Example* dhOut = (DH_Example*)getDataManager().getOutHolder(0);
  DH_Example::BufferType* buf = dhOut->getBuffer(); 
  dhOut->setCounter(itsIteration);
  for (int i=0; i < itsBufLength; i++) {
    buf[i] = makefcomplex(itsIteration+2, itsIteration+2);
  }
  itsIteration++;
}

void WH_Source::dump()
{
  cout << "WH_Source " << getName() << " dump:" << endl;
  getDataManager().getOutHolder(0)->dump();
}



