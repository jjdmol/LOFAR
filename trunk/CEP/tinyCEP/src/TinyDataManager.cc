//#  TinyDataManager.cc: a DM implementation for tinyCEP
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

#include <tinyCEP/TinyDataManager.h>
#include <tinyCEP/Selector.h>
#include <Common/Debug.h>

namespace LOFAR
{

  int TinyDataManager::DataHolderID=0;

  TinyDataManager::TinyDataManager(int ninputs, int noutputs)
    : itsNinputs (ninputs),
      itsNoutputs(noutputs),
      itsProcessRate(1),
      itsInputRate(1),
      itsOutputRate(1),
      itsInputSelector(0),
      itsOutputSelector(0)
  {
    itsInDHs = new DataHolder* [ninputs];
    itsOutDHs = new DataHolder* [noutputs];
    itsDoAutoTriggerIn = new bool[ninputs];
    itsDoAutoTriggerOut = new bool[noutputs];
    for (int i = 0; i < ninputs; i++)
    {
      itsDoAutoTriggerIn[i]=true;
    }
    for (int j = 0; j < noutputs; j++)
    {
      itsDoAutoTriggerOut[j]=true;
    }

  }

  TinyDataManager::~TinyDataManager() {
    delete itsInputSelector;
    delete itsOutputSelector;
    delete [] itsDoAutoTriggerIn;
    delete [] itsDoAutoTriggerOut;
   
    // DataHolder clean-up:
    for (int i = 0; i < itsNinputs; i++)
    { 
      delete itsInDHs[i]; 
    }
    delete [] itsInDHs;

    for (int j = 0; j < itsNoutputs; j++)
    { 
      delete itsOutDHs[j];
    }
    delete [] itsOutDHs;
  }



  DataHolder* TinyDataManager::getInHolder(int channel) {
    assertChannel(channel, true);
    return itsInDHs[channel];
  }


  TinyDataManager::TinyDataManager(const TinyDataManager& that)
    : itsNinputs (that.itsNinputs),
      itsNoutputs(that.itsNoutputs),
      itsProcessRate(that.itsProcessRate),
      itsInputRate(that.itsInputRate),
      itsOutputRate(that.itsOutputRate),
      itsInputSelector(0),
      itsOutputSelector(0)
  {
    if (that.itsInputSelector != 0)
    {
      itsInputSelector = that.itsInputSelector->clone();// Copies the selectors
    }
    if (that.itsOutputSelector != 0)
    {
      itsOutputSelector = that.itsOutputSelector->clone();
    }

    itsInDHs = new DataHolder* [itsNinputs];          // Clones the DataHolders
    itsDoAutoTriggerIn = new bool[itsNinputs];
    for (int ch = 0; ch < itsNinputs; ch++)
    {
      itsInDHs[ch] = that.itsInDHs[ch]->clone();
      itsDoAutoTriggerIn[ch] = that.doAutoTriggerIn(ch);
    }
    itsOutDHs = new DataHolder* [itsNoutputs];
    itsDoAutoTriggerOut = new bool[itsNoutputs];
    for (int ch = 0; ch < itsNoutputs; ch++)
    {
      itsOutDHs[ch] = that.itsOutDHs[ch]->clone();
      itsDoAutoTriggerOut[ch]=that.doAutoTriggerOut(ch);
    }
  }

  DataHolder* TinyDataManager::getOutHolder(int channel) {
    assertChannel(channel, false);
    return itsOutDHs[channel];
  }

  void TinyDataManager::addInDataHolder(int channel, DataHolder* dhptr) {
    assertChannel(channel, true);
    dhptr->setID(TinyDataManager::DataHolderID++);
    itsInDHs[channel] = dhptr;
  }

  void TinyDataManager::addOutDataHolder(int channel, DataHolder* dhptr) {
    assertChannel(channel, false);
    dhptr->setID(TinyDataManager::DataHolderID++);
    itsOutDHs[channel] = dhptr;
  }

  void TinyDataManager::assertChannel(int channel, bool input) {
    if (input) {
      DbgAssertStr (channel >= 0,          "input channel too low");
      DbgAssertStr (channel < getInputs(), "input channel too high");
    } else {
      DbgAssertStr (channel >= 0,           "output channel too low");
      DbgAssertStr (channel < getOutputs(), "output channel too high");
    }
  }

  void TinyDataManager::preprocess() {
    for (int i = 0; i < itsNinputs; i++) {
      itsInDHs[i]->preprocess();
    }
    

    for (int i = 0; i < itsNoutputs; i++) {
      itsOutDHs[i]->preprocess();
    }
  }

  void TinyDataManager::postprocess() {
  }

  void TinyDataManager::initializeInputs() {
  }

  DataHolder* TinyDataManager::getGeneralInHolder(int channel) {
    assertChannel(channel, true);
    return itsInDHs[channel];
  }

  DataHolder* TinyDataManager::getGeneralOutHolder(int channel) {
    assertChannel(channel, false);
    return itsOutDHs[channel];
  }
  
  void TinyDataManager::readyWithInHolder(int channel) {
    // The user has to call the ready with InHolder method 
    // in his own WorkHolder
    
    itsInDHs[channel]->read();

  }
  
  void TinyDataManager::readyWithOutHolder(int channel) {
    // The user has to call the ready with InHolder method 
    // in his own WorkHolder

    itsOutDHs[channel]->write();

  }

void TinyDataManager::setInputSelector(Selector* selector)
{ 
  itsInputSelector = selector;
}

void TinyDataManager::setOutputSelector(Selector* selector)
{ 
  itsOutputSelector = selector;
}

DataHolder* TinyDataManager::selectInHolder()
{
  AssertStr(itsInputSelector != 0, "No input selector set");
  unsigned int ch =  itsInputSelector->getNext();
  return getInHolder(ch);
}
 
DataHolder* TinyDataManager::selectOutHolder()
{
  AssertStr(itsOutputSelector != 0, "No output selector set");
  unsigned int ch =  itsOutputSelector->getNext();
  return getOutHolder(ch);
}

bool TinyDataManager::hasInputSelector()
{
  if (itsInputSelector == 0)
  {
    return false;
  }
  else
  {
    return true;
  }
}

bool TinyDataManager::hasOutputSelector()
{
  if (itsOutputSelector == 0)
  {
    return false;
  }
  else
  {
    return true;
  }
}

bool TinyDataManager::doAutoTriggerIn(int channel) const {
  DbgAssertStr(channel >= 0, "input channel too low");
  DbgAssertStr(channel < getInputs(), "input channel too high");
  return itsDoAutoTriggerIn[channel];
}

bool TinyDataManager::doAutoTriggerOut(int channel) const {
  DbgAssertStr(channel >= 0, "output channel too low");
  DbgAssertStr(channel < getOutputs(), "output channel too high");
  return itsDoAutoTriggerOut[channel];
}

void TinyDataManager::setAutoTriggerIn(int channel, 
				   bool newflag) const {
  DbgAssertStr(channel >= 0, "input channel too low");
  DbgAssertStr(channel < getInputs(), "input channel too high");
  itsDoAutoTriggerIn[channel] = newflag;
}

void TinyDataManager::setAutoTriggerOut(int channel, 
				    bool newflag) const {
  DbgAssertStr(channel >= 0, "output channel too low");
  DbgAssertStr(channel < getOutputs(), "output channel too high");
  itsDoAutoTriggerOut[channel] = newflag;
}

  
} // namespace LOFAR
