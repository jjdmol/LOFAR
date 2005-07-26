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

#include <InOutTest/WH_InOut.h>
#include <CEPFrame/DH_Example.h>

using namespace LOFAR;

WH_InOut::WH_InOut (const string& name, unsigned int nbuffer)
: WorkHolder    (1, 1, name),
  itsBufLength  (nbuffer)
{
    getDataManager().addInDataHolder(0, new DH_Example ("DHIn",
				     nbuffer));

    getDataManager().addOutDataHolder(0, new DH_Example("DHOut",
				      nbuffer));
}


WH_InOut::~WH_InOut()
{
}

WorkHolder* WH_InOut::make(const string& name)
{
  return new WH_InOut(name, itsBufLength);
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

    cout << "Processing Data! " 
	 << ((DH_Example*)getDataManager().getInHolder(0))->getCounter() << ": "
         << inbuf[0] << ',' << inbuf[itsBufLength-1] << "   --->  "
	 << ((DH_Example*)getDataManager().getOutHolder(0))->getCounter() 
	 << ": ";

  for (int m=0; m<itsBufLength; m++)
  {
    outbuf[m] = inbuf[m] + (float)1;
  }
    cout << outbuf[0] << ',' << outbuf[itsBufLength-1] << endl;
}

void WH_InOut::dump() const
{
  cout << "WH_InOut " << getName() << " dump:" << endl;
 ((DH_Example*)getDataManager().getInHolder(0))->dump();
 ((DH_Example*)getDataManager().getOutHolder(0))->dump();
}

