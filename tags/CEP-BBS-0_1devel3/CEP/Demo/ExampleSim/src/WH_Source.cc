//  WH_Source.cc:
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

#include "ExampleSim/WH_Source.h"

using namespace LOFAR;

WH_Source::WH_Source (const string& name, unsigned int nout)
  : WorkHolder (0, nout, name,"WH_Source"),
    itsNOutputs(nout),
    itsIteration(1)
{
  char str[8];
  for (unsigned int i=0; i<itsNOutputs; i++)
  {
    sprintf (str, "%d", i);
    getDataManager().addOutDataHolder(i, new DH_ExampleSim(string("out_") + str));
  }
}


WH_Source::~WH_Source()
{}

WorkHolder* WH_Source::construct (const string& name, int, int noutput,
				  const KeyValueMap&)
{
  return new WH_Source (name, noutput);
}

WH_Source* WH_Source::make (const string& name)
{
  return new WH_Source (name, itsNOutputs);
}

void WH_Source::process()
{
  for (unsigned int i=0; i<itsNOutputs; i++)
  {
    for (int ch=0; ch<10; ch++)
    {
      ((DH_ExampleSim*)getDataManager().getOutHolder(i))->getBuffer()[ch] = i + itsIteration;
    }
    ((DH_ExampleSim*)getDataManager().getOutHolder(i))->setCounter(itsIteration);
  }
  itsIteration++;

  dump();
}

void WH_Source::dump() const
{
  cout << getName() << endl;
  for (unsigned int i=0; i<itsNOutputs; i++)
  {
    cout << getDataManager().getOutHolder(i)->getName() << " : " ;
    cout << "Counter" << " = "
	 << ((DH_ExampleSim*)getDataManager().getOutHolder(i))->getCounter()
         << ", ";
    cout << "Buffer = ";
    for (int ch=0; ch<10; ch++){
      cout << ((DH_ExampleSim*)getDataManager().getOutHolder(i))->getBuffer()[ch] << ' ';
    }
    cout << endl;
    cout << endl;
  }
}

