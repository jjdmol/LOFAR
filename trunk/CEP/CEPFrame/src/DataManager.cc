//  DataManager.cc:
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
// DataManager.cc: implementation of the DataManager class.
//
//////////////////////////////////////////////////////////////////////

#include "CEPFrame/DataManager.h"
#include "CEPFrame/SynchronisityManager.h"
#include "CEPFrame/DHPoolManager.h"
#include "Common/Debug.h"
#include "CEPFrame/Selector.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DataManager::DataManager (int inputs, int outputs)
  : itsNinputs (inputs),
    itsNoutputs(outputs),
    itsInputSelector(0),
    itsOutputSelector(0)
{
  TRACER2("DataManager constructor");
  itsSynMan = new SynchronisityManager(inputs, outputs);
  itsInDHs = new DH_info[inputs];
  itsOutDHs = new DH_info[outputs];
  for (int i = 0; i < inputs; i++)
  {
    itsInDHs[i].currentDH = 0;
    itsInDHs[i].id = -1;
  }
  for (int j = 0; j < outputs; j++)
  {
    itsOutDHs[j].currentDH = 0;
    itsOutDHs[j].id = -1;
  }
}

DataManager::DataManager (const DataManager& that)
  : itsNinputs (that.itsNinputs),
    itsNoutputs(that.itsNoutputs)
{
  itsSynMan = new SynchronisityManager(itsNinputs, itsNoutputs);
  itsInDHs = new DH_info[itsNinputs];
  itsOutDHs = new DH_info[itsNoutputs];
  for (int i = 0; i < itsNinputs; i++)
  {
    itsInDHs[i].currentDH = 0;
    itsInDHs[i].id = -1;
  }
  for (int j = 0; j < itsNoutputs; j++)
  {
    itsOutDHs[j].currentDH = 0;
    itsOutDHs[j].id = -1;
  }
}

DataManager::~DataManager()
{
  delete itsSynMan;
  delete itsInDHs;
  delete itsOutDHs;
  if (itsInputSelector != 0)
  {
    delete itsInputSelector;
  }
  if (itsOutputSelector != 0)
  {
    delete itsOutputSelector;
  }
}

DataHolder* DataManager::getInHolder(int channel)
{
  DbgAssertStr (channel >= 0,          "input channel too low");
  DbgAssertStr (channel < getInputs(), "input channel too high");
  DHPoolManager* dhpPtr = itsSynMan->getInPoolManagerPtr(channel);

  if (itsInDHs[channel].currentDH == 0)
  {
    // If dataholder is shared between input and output, get read/write permission 
    if (dhpPtr->getSharing() == true)
    {
      itsInDHs[channel].currentDH =  dhpPtr->getRWLockedDH(&itsInDHs[channel].id);
    }
    else
    {
      itsInDHs[channel].currentDH =  dhpPtr->getReadLockedDH(&itsInDHs[channel].id); 
    }
  }
  return itsInDHs[channel].currentDH;
}

DataHolder* DataManager::getOutHolder(int channel)
{
  DbgAssertStr(channel >= 0, "output channel too low");
  DbgAssertStr(channel < getOutputs(), "output channel too high");
  DHPoolManager* dhpPtr = itsSynMan->getOutPoolManagerPtr(channel);

  if (itsOutDHs[channel].currentDH == 0)
  {
    // If dataholder is shared between input and output, get read/write permission
    if (dhpPtr->getSharing() == true)
    {
      itsOutDHs[channel].currentDH = dhpPtr->getRWLockedDH(&itsOutDHs[channel].id);
    }
    else
    {
      itsOutDHs[channel].currentDH = dhpPtr->getWriteLockedDH(&itsOutDHs[channel].id);
    }
  }

  return itsOutDHs[channel].currentDH;
}

void DataManager::readyWithInHolder(int channel)
{
  DbgAssertStr (channel >= 0,          "input channel too low");
  DbgAssertStr (channel < getInputs(), "input channel too high");

  if (itsInDHs[channel].id >= 0)
  {

    DHPoolManager* dhpPtr = itsSynMan->getInPoolManagerPtr(channel);

    if (itsSynMan->isInSynchronous(channel) == true)
    {
      itsInDHs[channel].currentDH->read();
    }
    else
    {
      itsSynMan->readAsynchronous(channel);
    }

    if (dhpPtr->getSharing() == true)                // Shared in- and output
    {
      dhpPtr->writeUnlock(itsInDHs[channel].id);
    }
    else
    {
      dhpPtr->readUnlock(itsInDHs[channel].id);
    } 

    itsInDHs[channel].id = -1;
    itsInDHs[channel].currentDH = 0;
  }
  else
  {
    TRACER2("DataManager::readyWithInHolder() Inholder not previously requested with getInHolder() function");
  }
}

void DataManager::readyWithOutHolder(int channel)
{
  DbgAssertStr(channel >= 0, "output channel too low");
  DbgAssertStr(channel < getOutputs(), "output channel too high");

  if (itsOutDHs[channel].id >= 0)
  {
    if (itsSynMan->isOutSynchronous(channel) == true)
    {
      TRACER4("DataManager::readyWithOutHolder synchronous write");
      itsOutDHs[channel].currentDH->write();
    }
    else
    {
      TRACER4("DataManager::readyWithOutHolder asynchronous write");
      itsSynMan->writeAsynchronous(channel);
    }

    itsSynMan->getOutPoolManagerPtr(channel)->writeUnlock(itsOutDHs[channel].id);

    itsOutDHs[channel].id = -1;
    itsOutDHs[channel].currentDH = 0;
  }
  else
  {
    TRACER2("DataManager::readyWithOutHolder Outholder not previously requested with getOutHolder() function");
  }
}

void DataManager::addInDataHolder(int channel, DataHolder* dhptr, 
				  bool synchronous, bool shareIO)
{
  DbgAssertStr (channel >= 0, "input channel too low");
  DbgAssertStr (channel < getInputs(), "input channel too high");
  itsSynMan->setInSynchronous(channel, synchronous);
  itsSynMan->getInPoolManagerPtr(channel)->setDataHolder(dhptr);
  itsSynMan->getInPoolManagerPtr(channel)->setSharing(shareIO);

  if (shareIO == true)
  {
    DbgAssertStr(channel < getOutputs(), 
		 "corresponding output channel does not exist");
    itsSynMan->setOutPoolManagerPtr(channel, itsSynMan->getInPoolManagerPtr(channel));
    itsSynMan->setOutSynchronous(channel, synchronous);
  }
} 

void DataManager::addOutDataHolder(int channel, DataHolder* dhptr, 
				   bool synchronous, bool shareIO)
{
  DbgAssertStr (channel >= 0,"output channel too low");
  DbgAssertStr (channel < getOutputs(), "output channel too high");
  itsSynMan->setOutSynchronous(channel, synchronous);
  itsSynMan->getOutPoolManagerPtr(channel)->setDataHolder(dhptr);
  itsSynMan->getOutPoolManagerPtr(channel)->setSharing(shareIO);

  if (shareIO == true)
  {
    DbgAssertStr(channel < getInputs(), 
		 "corresponding input channel does not exist");
    itsSynMan->setInPoolManagerPtr(channel, itsSynMan->getOutPoolManagerPtr(channel));
    itsSynMan->setInSynchronous(channel, synchronous);
  }
}

DataHolder* DataManager::getGeneralInHolder(int channel)
{  
  DbgAssertStr(channel >= 0, "input channel too low");
  DbgAssertStr(channel < getInputs(), "input channel too high");
  return itsSynMan->getInPoolManagerPtr(channel)->getGeneralDataHolder();
}

DataHolder* DataManager::getGeneralOutHolder(int channel)
{
  DbgAssertStr(channel >= 0, "output channel too low");
  DbgAssertStr(channel < getOutputs(), "output channel too high");
  return itsSynMan->getOutPoolManagerPtr(channel)->getGeneralDataHolder();
}

void DataManager::preprocess()
{
  TRACER4("DataManager::preprocess()");
  itsSynMan->preprocess();
}

void DataManager::postprocess()
{
  itsSynMan->postprocess();
}

void DataManager::initializeInputs()
{
  for (int ch = 0; ch < itsNinputs; ch++)
  {
    if (itsInDHs[ch].currentDH == 0)
    {
      if (itsSynMan->isInSynchronous(ch) == true)
      {
	itsInDHs[ch].currentDH = itsSynMan->getInPoolManagerPtr(ch)->
	                         getWriteLockedDH(&itsInDHs[ch].id);
	itsInDHs[ch].currentDH->read();
	itsSynMan->getInPoolManagerPtr(ch)->writeUnlock(itsInDHs[ch].id);
	itsInDHs[ch].id = -1;
	itsInDHs[ch].currentDH = 0;
      }
      else
      {
        itsSynMan->readAsynchronous(ch);
      }
    }
  }

}

bool DataManager::isInSynchronous(int channel)
{
  DbgAssertStr(channel >= 0, "input channel too low");
  DbgAssertStr(channel < getInputs(), "input channel too high");
  return itsSynMan->isInSynchronous(channel);
}

bool DataManager::isOutSynchronous(int channel)
{
  DbgAssertStr(channel >= 0, "output channel too low");
  DbgAssertStr(channel < getOutputs(), "output channel too high");
  return itsSynMan->isOutSynchronous(channel);
}

void DataManager::setInputSelector(Selector* selector)
{ 
  for (int ch = 0; ch < itsNinputs; ch++)
  {
    AssertStr(!isInSynchronous(ch), 
	      "Input " << ch << " is not asynchronous."); 
  }
  itsInputSelector = selector;
}

void DataManager::setOutputSelector(Selector* selector)
{ 
  for (int ch = 0; ch < itsNoutputs; ch++)
  {
    AssertStr(!isOutSynchronous(ch), 
	      "Output " << ch << " is not asynchronous."); 
  }
  itsOutputSelector = selector;
}

DataHolder* DataManager::selectInHolder()
{
  AssertStr(itsInputSelector != 0, "No input selector set");
  unsigned int ch =  itsInputSelector->getNext();
  return getInHolder(ch);
}
 
DataHolder* DataManager::selectOutHolder()
{
  AssertStr(itsOutputSelector != 0, "No output selector set");
  unsigned int ch =  itsOutputSelector->getNext();
  return getOutHolder(ch);
}

bool DataManager::hasInputSelector()
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

bool DataManager::hasOutputSelector()
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
