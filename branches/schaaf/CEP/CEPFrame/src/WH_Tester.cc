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
//  $Log$
//  Revision 1.11  2002/06/27 10:49:33  schaaf
//  %[BugId: 57]%
//  Modified getIn/OutHolder methods and use in process() and dump() methods.
//
//  Revision 1.10  2002/05/03 11:21:32  gvd
//  Changed for new build environment (mostly added package name to include)
//
//  Revision 1.9  2002/03/15 13:28:09  gvd
//  Added construct function to WH classes (for XML parser)
//  Added getX functions to ParamBlock
//  Added SAX classes for XML parser
//  Improved testing scripts (added .run)
//
//  Revision 1.8  2002/03/01 08:27:57  gvd
//  Replaced firewall by Debug and changed code accordingly
//  Added lofar_*.h for correct use of namespaces (for KAI and Intel C++)
//
//  Revision 1.7  2002/01/02 14:02:00  rdam
//  Made getType() return class name
//
//  Revision 1.6  2001/09/21 12:19:02  gvd
//  Added make functions to WH classes to fix memory leaks
//
//  Revision 1.5  2001/03/01 13:15:47  gvd
//  Added type argument in DataHolder constructor which is used in
//  the connect functions to check if the DH types match
//  Improved the simulator parser
//  Improved documentation
//
//  Revision 1.4  2001/02/05 14:53:05  loose
//  Added GPL headers
//
//////////////////////////////////////////////////////////////////////

#include "BaseSim/WH_Tester.h"


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
