//#  MiniDataManager.cc: a DM implementation for tinyCEP
//#
//#  Copyright (C) 2002-2004
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

#include <tinyCEP/BaseDataManager.h>
#include <tinyCEP/MiniDataManager.h>

namespace LOFAR
{

  MiniDataManager::MiniDataManager(int ninputs, int noutputs):
    itsNinputs(ninputs), itsNoutputs(noutputs) {
    
    itsInTRs = new DataHolder* [ninputs];
    itsOutTRs = new DataHolder* [noutputs];

  }

  MiniDataManager::~MiniDataManager() {

  }

  DataHolder* MiniDataManager::getInHolder(int channel) {
    return itsInTRs[channel];
  }

  DataHolder* MiniDataManager::getOutHolder(int channel) {
    return itsOutTRs[channel];
  }

  void MiniDataManager::addInDataHolder(int channel, DataHolder* dhptr) {
    itsInTRs[channel] = dhptr;
  }

  void MiniDataManager::addOutDataHolder(int channel, DataHolder* dhptr) {
    itsOutTRs[channel] = dhptr;
  }

} // namespace LOFAR
