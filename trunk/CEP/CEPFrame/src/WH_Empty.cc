//  WH_Empty.cc: An empty WorkHolder (doing nothing)
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
//  Revision 1.10  2002/05/03 11:21:32  gvd
//  Changed for new build environment (mostly added package name to include)
//
//  Revision 1.9  2002/05/02 12:16:24  schaaf
//  Added method getMonitorValue
//
//  Revision 1.8  2002/04/18 07:52:59  schaaf
//  replaced cout by TRACER
//
//  Revision 1.7  2002/03/15 13:28:08  gvd
//  Added construct function to WH classes (for XML parser)
//  Added getX functions to ParamBlock
//  Added SAX classes for XML parser
//  Improved testing scripts (added .run)
//
//  Revision 1.6  2002/01/02 14:02:00  rdam
//  Made getType() return class name
//
//  Revision 1.5  2001/09/21 12:19:02  gvd
//  Added make functions to WH classes to fix memory leaks
//
//  Revision 1.4  2001/03/23 10:00:40  gvd
//  Improved documentation and test programs
//  Added clearEventCount function to Step
//
//  Revision 1.3  2001/02/05 14:53:05  loose
//  Added GPL headers
//
//////////////////////////////////////////////////////////////////////

#include "BaseSim/WH_Empty.h"
#include "Common/Debug.h"


WH_Empty::WH_Empty (const string& name)
: WorkHolder (1, 1, name,"WH_Empty")
{}

WH_Empty::~WH_Empty()
{}

WorkHolder* WH_Empty::construct (const string& name, int, int,
				 const ParamBlock&)
{
  return new WH_Empty (name);
}

WH_Empty* WH_Empty::make (const string& name) const
{
  return new WH_Empty (name);
}

void WH_Empty::process()
{}

void WH_Empty::dump() const
{
  TRACER2("WH_Empty Dump");
}


DataHolder* WH_Empty::getInHolder (int)
{
  return &itsInDataHolder; 
}

DataHolder* WH_Empty::getOutHolder (int)
{
  return &itsOutDataHolder;
}


/**
   This example getMonitorValue method pruduces output as follows:
   "hi"  : std output text including the WorkHolder's name; return 0
   "one" : return 1
 */
int WH_Empty::getMonitorValue(const char* name){
  TRACER2("Called WH_Empty::getMonitorValue");
  int result =0;
  if (strcmp(name,"hi") == 0) {
    cout << "Hi I'm " << getName() << endl;  
  } else if (strcmp(name,"one") == 0) {
    result = 1;
  }
  TRACER2("WH_Empty::getMonitorValue resturns " << result);
  return result;
}
