//#  BaseDataManager.cc: DataManager base class
//#
//#  Copyright (C) 2002-2003
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

#include <tinyCEP/DataManager.h>

namespace LOFAR
{
  DataManager::DataManager(int inputs, int outputs) {
  }

  DataManager::~DataManager(){
  }


  DataHolder* DataManager::getInHolder(int channel) {
    DbgAssertStr (channel >= 0,          "input channel too low");
    DbgAssertStr (channel < getInputs(), "input channel too high");

    return itsInDHs[channel].currentDH;
  }

  DataHolder* DataManager::getOutHolder(int channel) {
    DbgAssertStr (channel >= 0,          "output channel too low");
    DbgAssertStr (channel < getOutputs(), "output channel too high");

    return itsOutDHs[channel].currentDH;
  }



} // namespace LOFAR
