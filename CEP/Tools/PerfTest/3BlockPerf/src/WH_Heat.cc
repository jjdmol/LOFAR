//  WH_Heat.cc: WorkHolder class that performs a number of flops per byte
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

#include "3BlockPerf/WH_Heat.h"

namespace LOFAR {

WH_Heat::WH_Heat (const string& name, 
		  unsigned int size,  // size of the packet in bytes
		  unsigned int flopsPB) // number of flops per byte
               : WorkHolder    (1, 1, name),
		 itsSize (size),
		 itsFlopsPerByte(flopsPB),
		 itsNoValues(0),
		 itsFactor(0)

{
  DH_FixedSize* dhptr = new DH_FixedSize("in_of_WH_Heat", itsSize);

  getDataManager().addInDataHolder(0, dhptr);
  getDataManager().addOutDataHolder(0,
				    new DH_FixedSize("out_of_WH_Heat",
                                                     itsSize));
  itsNoValues = dhptr->getNoValues(); // number of values in a DataHolder
}

WH_Heat::~WH_Heat()
{
}

WorkHolder* WH_Heat::make(const string& name)
{
  return new WH_Heat(name,
		     itsSize,
		     itsFlopsPerByte);
}

void WH_Heat::preprocess()
{
  itsFactor = 1.001; // the factor to multiple with
  itsNoMultiplications = itsFlopsPerByte * sizeof(DH_FixedSize::valueType); // number of flops to perform
}

void WH_Heat::process()
{
  DH_FixedSize* dh_in = (DH_FixedSize*)getDataManager().getInHolder(0);
  DH_FixedSize::valueType* data_in = dh_in->getPtr2Data(); //pointer to the outgoing data
  DH_FixedSize::valueType* data_out = ((DH_FixedSize*)getDataManager().getOutHolder(0))->getPtr2Data(); // pointer to the ingoing data
  for (int i=0; i<itsNoValues; i++) {
    DH_FixedSize::valueType result = data_in[i];
    // do the specified number of flops
    for (int j=0; j<itsNoMultiplications; j++) {
      result = itsFactor + result;
    }
    data_out[i] = result;
  }
}

}
