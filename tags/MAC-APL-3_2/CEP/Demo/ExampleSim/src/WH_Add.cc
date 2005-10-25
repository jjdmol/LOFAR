//  WH_Add.cc:
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

#include "ExampleSim/WH_Add.h"

using namespace LOFAR;

WH_Add::WH_Add (const string& name, int factor)
  : WorkHolder (1, 1, name,"WH_Add"),
    itsFactor  (factor)
{
  getDataManager().addInDataHolder(0, new DH_ExampleSim("in_"));
  getDataManager().addOutDataHolder(0, new DH_ExampleSim("out"));
}


WH_Add::~WH_Add()
{}

WorkHolder* WH_Add::construct (const string& name, int, int,
			       const KeyValueMap&)
{
  return new WH_Add (name);
}

WH_Add* WH_Add::make (const string& name)
{
  return new WH_Add (name, itsFactor);
}

void WH_Add::process()
{
  for (int ch=0; ch<10; ch++)
  {
    ((DH_ExampleSim*)getDataManager().getOutHolder(0))->getBuffer()[ch] =
      ((DH_ExampleSim*)getDataManager().getInHolder(0))->getBuffer()[ch] 
      + itsFactor;
  }

  ((DH_ExampleSim*)getDataManager().getOutHolder(0))->setCounter(
             ((DH_ExampleSim*)getDataManager().getInHolder(0))->getCounter());
  dump();
}

void WH_Add::dump() const
{
  cout << getName() << endl;
  cout << "Counter" << " = "
       << ((DH_ExampleSim*)getDataManager().getOutHolder(0))->getCounter()
       << ", ";
  cout << "Buffer = ";
  for (int ch=0; ch<10; ch++){
    cout << ((DH_ExampleSim*)getDataManager().getOutHolder(0))
             ->getBuffer()[ch] << ' ';
  }
  cout << endl;
  cout << endl;
  cout <<"-------------------------------------------------------------------"
       << endl;
  cout << endl;
}

