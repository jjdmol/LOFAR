//  DH_FineDelay.cc:
//
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

#include <DH_FineDelay.h>
//#include <ACC/ParameterSet.h>


namespace LOFAR
{

DH_FineDelay::DH_FineDelay(const string &name)
: DataHolder(name, "DH_FineDelay"),
  itsDelays(0)
{
}

DH_FineDelay::DH_FineDelay(const DH_FineDelay &that)
: DataHolder(that),
  itsDelays(that.itsDelays)
{
}

DH_FineDelay::~DH_FineDelay()
{
}

DataHolder *DH_FineDelay::clone() const
{
  return new DH_FineDelay(*this);
}

void DH_FineDelay::init()
{
  addField("Delays", BlobField<float>(1, sizeof(AllDelaysType) / sizeof(float)));
  createDataBlock();
}

void DH_FineDelay::fillDataPointers()
{
  itsDelays = (AllDelaysType *) getData<float>("Delays");
}

void DH_FineDelay::setTestPattern()
{
  memset(itsDelays, 0, sizeof(AllDelaysType));

#if NR_STATIONS >= 2
  (*itsDelays)[1].delayAtBegin  = 2.499804413e-9;
  (*itsDelays)[1].delayAfterEnd = 2.499804413e-9;
#endif
}

}
