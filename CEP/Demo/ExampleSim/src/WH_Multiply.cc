//  WH_Multiply.cc:
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

#include "ExampleSim/WH_Multiply.h"

using namespace LOFAR;

WH_Multiply::WH_Multiply (const string& name, int nin)
  : WorkHolder (nin, 1, name,"WH_Multiply"),
    itsNInputs (nin)
{
  char str[8];
  for (int i=0; i<itsNInputs; i++)
  {
    sprintf(str, "%d", i);
    getDataManager().addInDataHolder(i, 
			       new DH_ExampleSim(string("in_") + str));
  }
  getDataManager().addOutDataHolder(0, new DH_ExampleSim("out"));
}


WH_Multiply::~WH_Multiply()
{}

WorkHolder* WH_Multiply::construct (const string& name, int ninput, int,
				    const KeyValueMap&)
{
  return new WH_Multiply (name, ninput);
}

WH_Multiply* WH_Multiply::make (const string& name)
{
  return new WH_Multiply (name, itsNInputs);
}

void WH_Multiply::process()
{
  int temp[10];
  for (int i=0; i<itsNInputs; i++)
  {
    DH_ExampleSim* input = (DH_ExampleSim*)getDataManager().getInHolder(i);
    if (i==0)
    {
       for (int ch=0; ch<10; ch++)
       {
	 temp[ch] = input->getBuffer()[ch];
       }
    }
    else
    {
      for (int ch=0; ch<10; ch++)
      {
	temp[ch] *= input->getBuffer()[ch];
      }
    }
  }

  DH_ExampleSim* outputDH = (DH_ExampleSim*)getDataManager().getOutHolder(0);
  for (int ch=0; ch<10; ch++)
  {
    outputDH->getBuffer()[ch] = temp[ch];
  }
  ((DH_ExampleSim*)getDataManager().getOutHolder(0))->setCounter(
             ((DH_ExampleSim*)getDataManager().getInHolder(0))->getCounter());

  dump();
}

void WH_Multiply::dump()
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
}

