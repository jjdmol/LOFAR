//  WH_Connection.cc:
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

#include "PSS3/WH_Connection.h"
#include <math.h>

using namespace LOFAR;

WH_Connection::WH_Connection (const string& name, int nInDHs,
			      int nOutDHs, dhType dh1Type, dhType dh2Type)
  : WorkHolder(nInDHs, nOutDHs, name, "WH_Connection"), 
    itsNInDHs(nInDHs),
    itsNOutDHs(nOutDHs),
    itsDH1Type(dh1Type),
    itsDH2Type(dh2Type)
{
  AssertStr((nInDHs == 0) || (nOutDHs == 0), "Number of inputs or number of outputs should be 0");
  if (nInDHs != 0)
  {
    AssertStr((nInDHs==1 || nInDHs == 2), "Number of dataholders is not equal to 1 or 2 : " 
	      << nInDHs);
    switch (dh1Type) {
    case WorkOrder:
      getDataManager().addInDataHolder(0, new DH_WorkOrder("in", "Conn"), true);
      break;
    case Parms:
      getDataManager().addInDataHolder(0, new DH_Parms("in"), true);
      break;
    }
    if (nInDHs == 2)
    {
      switch (dh2Type) {
      case WorkOrder:
        getDataManager().addInDataHolder(1, new DH_WorkOrder("in", "Conn"), true);
        break;
      case Parms:
        getDataManager().addInDataHolder(1, new DH_Parms("in"), true);
        break;
      }
    }
  }
  else
  {
    AssertStr((nOutDHs==1 || nOutDHs == 2), "Number of dataholders is not equal to 1 or 2 : " 
	      << nOutDHs);
    switch (dh1Type) {
    case WorkOrder:
      getDataManager().addOutDataHolder(0, new DH_WorkOrder("in", "Conn"), true);
      break;
    case Parms:
      getDataManager().addOutDataHolder(0, new DH_Parms("in"), true);
      break;
    }
    if (nOutDHs == 2)
    {
      switch (dh2Type) {
      case WorkOrder:
        getDataManager().addOutDataHolder(1, new DH_WorkOrder("in", "Conn"), true);
        break;
      case Parms:
        getDataManager().addOutDataHolder(1, new DH_Parms("in"), true);
        break;
      }
    }
  }    

}



WH_Connection::~WH_Connection()
{}

WorkHolder* WH_Connection::construct (const string& name, int, int,
				  const ParamBlock&)
{
  return new WH_Connection (name, 0, 0, Parms, Parms);
}

WH_Connection* WH_Connection::make (const string& name)
{
  return new WH_Connection (name, itsNInDHs, itsNOutDHs, itsDH1Type, itsDH2Type);
}

void WH_Connection::process()
{
}

void WH_Connection::dump()
{
}

