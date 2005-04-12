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
//
//////////////////////////////////////////////////////////////////////

#include <CEPFrame/WH_Empty.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{

WH_Empty::WH_Empty (const string& name)
: WorkHolder (1, 1, name,"WH_Empty")
{
  getDataManager().addInDataHolder(0, new DH_Empty());
  getDataManager().addOutDataHolder(0, new DH_Empty());
}

WH_Empty::~WH_Empty()
{}

WorkHolder* WH_Empty::construct (const string& name, int, int,
				 const KeyValueMap&)
{
  return new WH_Empty (name);
}

WH_Empty* WH_Empty::make (const string& name)
{
  return new WH_Empty (name);
}

void WH_Empty::process()
{}

void WH_Empty::dump()
{
  LOG_TRACE_FLOW("WH_Empty Dump");
}


/**
   This example getMonitorValue method pruduces output as follows:
   "hi"  : std output text including the WorkHolder's name; return 0
   "one" : return 1
 */
int WH_Empty::getMonitorValue(const char* name){
  LOG_TRACE_FLOW("Called WH_Empty::getMonitorValue");
  int result =0;
  if (strcmp(name,"hi") == 0) {
    cout << "Hi I'm " << getName() << endl;  
  } else if (strcmp(name,"one") == 0) {
    result = 1;
  }
  LOG_TRACE_FLOW_STR("WH_Empty::getMonitorValue returns " << result);
  return result;
}

}
