//  WH_Dest.cc: WorkHolder class using DH_FixedSize() objects 
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

#include "CEPFrame/Step.h"
#include <Common/Debug.h>

#include "3BlockPerf/WH_Dest.h"

namespace LOFAR {
                
WH_Dest::WH_Dest (const string& name, 
		  unsigned int size)    // size of the packet in bytes
         : WorkHolder    (1, 0, name),
           itsSize (size)
{
  getDataManager().addInDataHolder(0, 
				   new DH_FixedSize ("in_of_WH_Dest",
                                                     itsSize));
}

WH_Dest::~WH_Dest()
{
}

WorkHolder* WH_Dest::make(const string& name)
{
  return new WH_Dest(name, 
		     itsSize);
}

void WH_Dest::process()
{  
  getDataManager().getInHolder(0);
  // do nothing
}

}
