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

#include <tinyCEP/MiniDataManager.h>

namespace LOFAR
{

  MiniDataManager::MiniDataManager(int ninputs, int noutputs):
    itsNinputs(ninputs), 
    itsNoutputs(noutputs) {
    
    itsInDHs = new DataHolder* [ninputs];
    itsOutDHs = new DataHolder* [noutputs];
  }

  MiniDataManager::~MiniDataManager() {
  }

  DataHolder* MiniDataManager::getInHolder(int channel) {
    assertChannel(channel, true);
    return itsInDHs[channel];
  }


  MiniDataManager::MiniDataManager(const MiniDataManager& that):
    itsNinputs  (that.itsNinputs),
    itsNoutputs (that.itsNoutputs){
    
    itsInDHs = new DataHolder* [itsNinputs];
    itsOutDHs = new DataHolder* [itsNoutputs];

  }

  DataHolder* MiniDataManager::getOutHolder(int channel) {
    assertChannel(channel, false);
    return itsOutDHs[channel];
  }

  void MiniDataManager::addInDataHolder(int channel, DataHolder* dhptr) {
    assertChannel(channel, true);
    itsInDHs[channel] = dhptr;
  }

  void MiniDataManager::addOutDataHolder(int channel, DataHolder* dhptr) {
    assertChannel(channel, false);
    itsOutDHs[channel] = dhptr;
  }

  void MiniDataManager::assertChannel(int channel, bool input) {
    if (input) {
      DbgAssertStr (channel >= 0,          "input channel too low");
      DbgAssertStr (channel < getInputs(), "input channel too high");
    } else {
      DbgAssertStr (channel >= 0,           "output channel too low");
      DbgAssertStr (channel < getOutputs(), "output channel too high");
    }
  }

  void MiniDataManager::preprocess() {
    for (int i = 0; i < itsNinputs; i++) {
      itsInDHs[i]->preprocess();
    }

    for (int i = 0; i < itsNoutputs; i++) {
      itsOutDHs[i]->preprocess();
    }
  }

  void MiniDataManager::postprocess() {
  }

  void MiniDataManager::initializeInputs() {
  }

  DataHolder* MiniDataManager::getGeneralInHolder(int channel) {
    assertChannel(channel, true);
    return itsInDHs[channel];
  }

  DataHolder* MiniDataManager::getGeneralOutHolder(int channel) {
    assertChannel(channel, false);
    return itsOutDHs[channel];
  }
  
  void MiniDataManager::readyWithInHolder(int channel) {
  }
  
  void MiniDataManager::readyWithOutHolder(int channel) {
  }
  
} // namespace LOFAR
