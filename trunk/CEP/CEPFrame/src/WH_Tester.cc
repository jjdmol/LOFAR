//  WH_Tester.cc:
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

#include "CEPFrame/WH_Tester.h"


WH_Tester::WH_Tester (const string& name)
: WorkHolder       (1, 1, name,"WH_Tester"),
  itsInDataHolder  ("in"),
  itsOutDataHolder ("out")
{}


WH_Tester::~WH_Tester()
{}

WorkHolder* WH_Tester::construct (const string& name, int, int,
				  const ParamBlock&)
{
  return new WH_Tester (name);
}

WH_Tester* WH_Tester::make (const string& name) const
{
  return new WH_Tester (name);
}

void WH_Tester::process()
{
  for (int ch=0; ch<10; ch++){
    getOutHolder(0)->getBuffer()[ch] = 
      getInHolder(0)->getBuffer()[ch] + complex<float>(0.01,0);
  }
  getOutHolder(0)->copyTimeStamp (getInHolder(0));
  getOutHolder(0)->setCounter    (getInHolder(0)->getCounter() + 1);
}

void WH_Tester::dump() const
{
  cout << "WH_Tester" << endl;
  cout << "Timestamp " << 0 << " = "
       << (const_cast<WH_Tester*>(this))->getOutHolder(0)->getTimeStamp() << endl;
  cout << "Counter   " << 0 << " = "
       << (const_cast<WH_Tester*>(this))->getOutHolder(0)->getCounter() << endl;
  cout << "Buffer    ";
  for (int ch=0; ch<10; ch++){
    cout << (const_cast<WH_Tester*>(this))->getInHolder(0)->getBuffer()[ch] << ' ';
  }
  cout << endl;
}


DH_Tester* WH_Tester::getInHolder (int)
{
  return &itsInDataHolder;
}

DH_Tester* WH_Tester::getOutHolder (int)
{
  return &itsOutDataHolder;
}
