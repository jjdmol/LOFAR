//  WH_InOut.cc: WorkHolder class using DH_Example() objects and 
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

#include "CEPFrame/Step.h"
#include <Common/Debug.h>

#include "InOutTest/WH_InOut.h"
#include "InOutTest/InOutTest.h"


WH_InOut::WH_InOut (const string& name, bool IOshared, unsigned int nbuffer)
: WorkHolder    (1, 1, name),
  itsIOshared   (IOshared),
  itsBufLength  (nbuffer)
{
    getDataManager().addInDataHolder(0, new DH_Example ("DHIn",
				     nbuffer), true, 
				     IOshared);   // In- & output shared

    getDataManager().addOutDataHolder(0, new DH_Example("DHOut",
				      nbuffer), true, 
				      IOshared);   // In- & output shared
}


WH_InOut::~WH_InOut()
{
}

WorkHolder* WH_InOut::make(const string& name)
{
  return new WH_InOut(name, itsIOshared, itsBufLength);
}

//  void WH_InOut::preprocess() {
//    return;
//  }

void WH_InOut::process()
{  
  DH_Example::BufferType* outbuf = ((DH_Example*)getDataManager().getOutHolder(0))
                                 ->getBuffer();
  DH_Example::BufferType* inbuf = 
                  ((DH_Example*)getDataManager().getInHolder(0))->getBuffer();

//    cout << "Processing Data! "
//         << inbuf[0] << ',' << inbuf[itsBufLength-1] << "   --->  ";

  for (int m=0; m<itsBufLength; m++)
  {
    outbuf[m] = inbuf[m] + 1;
  }
//    cout << outbuf[0] << ',' << outbuf[itsBufLength-1] << endl;
}

void WH_InOut::dump()
{
  cout << "WH_InOut " << getName() << " dump:" << endl;
 ((DH_Example*)getDataManager().getInHolder(0))->dump();
 ((DH_Example*)getDataManager().getOutHolder(0))->dump();
}

