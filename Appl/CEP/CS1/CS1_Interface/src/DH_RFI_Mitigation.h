//# DH_RFI_Mitigation.h: RFI_Mitigation DataHolder
//#
//#  Copyright (C) 2006
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
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

#ifndef LOFAR_APPL_CEP_CS1_CS1_INTERFACE_DH_RFI_MITIGATION_H
#define LOFAR_APPL_CEP_CS1_CS1_INTERFACE_DH_RFI_MITIGATION_H

#include <CS1_Interface/CS1_Config.h>
#include <CS1_Interface/bitset.h>
#include <Transport/DataHolder.h>


namespace LOFAR
{

class DH_RFI_Mitigation: public DataHolder
{
public:
  typedef LOFAR::bitset<NR_SUBBAND_CHANNELS> ChannelFlagsType[NR_STATIONS];

  explicit DH_RFI_Mitigation(const string& name);

  DH_RFI_Mitigation(const DH_RFI_Mitigation&);

  virtual ~DH_RFI_Mitigation();

  DataHolder *clone() const;

  virtual void init();

  ChannelFlagsType *getChannelFlags()
  {
    return itsChannelFlags;
  }

  const ChannelFlagsType *getChannelFlags() const
  {
    return itsChannelFlags;
  }

  const size_t nrChannelFlags() const
  {
    return NR_STATIONS * NR_SUBBAND_CHANNELS;
  }

private:
  /// Forbid assignment.
  DH_RFI_Mitigation &operator = (const DH_RFI_Mitigation&);

  ChannelFlagsType *itsChannelFlags;

  void fillDataPointers();
};


}
#endif 
