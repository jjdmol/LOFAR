//  DH_RFI_Mitigation.cc:
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

#include <DH_RFI_Mitigation.h>
//#include <ACC/ParameterSet.h>

#include <stdlib.h>


namespace LOFAR
{

DH_RFI_Mitigation::DH_RFI_Mitigation(const string &name)
: DataHolder(name, "DH_RFI_Mitigation"),
  itsChannelFlags(0)
{
}

DH_RFI_Mitigation::DH_RFI_Mitigation(const DH_RFI_Mitigation &that)
: DataHolder(that),
  itsChannelFlags(that.itsChannelFlags)
{
}

DH_RFI_Mitigation::~DH_RFI_Mitigation()
{
}

DataHolder *DH_RFI_Mitigation::clone() const
{
  return new DH_RFI_Mitigation(*this);
}

void DH_RFI_Mitigation::init()
{
  addField("ChannelFlags", BlobField<uint32>(1, sizeof(ChannelFlagsType) / sizeof(uint32)));
  createDataBlock();
}

void DH_RFI_Mitigation::fillDataPointers()
{
  itsChannelFlags = (ChannelFlagsType *) getData<uint32>("ChannelFlags");
}

void DH_RFI_Mitigation::setTestPattern()
{
  memset(itsChannelFlags, 0, sizeof(ChannelFlagsType));

#if 0 && NR_STATIONS >= 3 && NR_SUBBAND_CHANNELS >= 256
  (*itsChannelFlags)[2][255] = true;
#endif
}

}
