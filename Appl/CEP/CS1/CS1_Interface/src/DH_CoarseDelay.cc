//#  DH_CoarseDelay.cc: dataholder to hold the delay information to perform
//#              station synchronization
//#
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$


#include <lofar_config.h>

#include <CS1_Interface/DH_CoarseDelay.h>

namespace LOFAR
{

DH_CoarseDelay::DH_CoarseDelay(const string &name, int nrRSPs)
  : DataHolder     (name, "DH_CoarseDelay"),
    itsNrRSPs      (nrRSPs)
{
  
}
  
DH_CoarseDelay::DH_CoarseDelay(const DH_CoarseDelay &that)
  : DataHolder (that),
    itsNrRSPs  (that.itsNrRSPs)
{   
}

DH_CoarseDelay::~DH_CoarseDelay()
{
}

DataHolder *DH_CoarseDelay::clone() const
{
  return new DH_CoarseDelay(*this);
}

void DH_CoarseDelay::init()
{
  // add the fields to the data definition
  addField ("Delay", BlobField<int>(1, itsNrRSPs));
  
  // create the data blob
  createDataBlock();

  for (int i=0; i<itsNrRSPs; i++) {
    itsDelayPtr[i] = 0;
  }

}

void DH_CoarseDelay::fillDataPointers() 
{
  itsDelayPtr = getData<int> ("Delay");
}

const int DH_CoarseDelay::getDelay(int index) const
{ 
  ASSERTSTR((index < itsNrRSPs) && (index >= 0), "index is not within range");
  return itsDelayPtr[index]; 
}

void DH_CoarseDelay::setDelay(int index, int value)
{ 
  ASSERTSTR((index < itsNrRSPs) && (index >= 0), "index is not within range");
  itsDelayPtr[index] = value;
}

}  // end namespace
