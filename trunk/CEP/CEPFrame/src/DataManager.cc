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

namespace LOFAR
{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DataManager::DataManager (int inputs, int outputs)
  : TinyDataManager(inputs, outputs)
{
  TRACER2("DataManager constructor");
  itsSynMan = new SynchronisityManager(inputs, outputs);
  itsInDHsinfo = new DH_info[inputs];
  itsOutDHsinfo = new DH_info[outputs];

  for (int i = 0; i < inputs; i++)
  {
    itsInDHsinfo[i].currentDH = 0;
    itsInDHsinfo[i].id = -1;
  }
  for (int j = 0; j < outputs; j++)
  {
    itsOutDHsinfo[j].currentDH = 0;
    itsOutDHsinfo[j].id = -1;
  }
}

DataManager::DataManager (const DataManager& that)
  : TinyDataManager(that.itsNinputs, that.itsNoutputs)
{
  itsSynMan = new SynchronisityManager(itsNinputs, itsNoutputs);
  itsInDHsinfo = new DH_info[itsNinputs];
  itsOutDHsinfo = new DH_info[itsNoutputs];
  for (int i = 0; i < itsNinputs; i++)
  {
    itsInDHsinfo[i].currentDH = 0;
    itsInDHsinfo[i].id = -1;
  }
  for (int j = 0; j < itsNoutputs; j++)
  {
    itsOutDHsinfo[j].currentDH = 0;
    itsOutDHsinfo[j].id = -1;
  }
}

DataManager::DataManager (const TinyDataManager& that)
  : TinyDataManager(that)                       // Copy all data from TinyDM
{
  TRACER2("Copy TinyDataManager to DataManager");
  itsSynMan = new SynchronisityManager(itsNinputs, itsNoutputs);
  itsInDHsinfo = new DH_info[itsNinputs];
  itsOutDHsinfo = new DH_info[itsNoutputs];

  for (int i = 0; i < itsNinputs; i++)
  {
    itsSynMan->getInPoolManagerPtr(i)->setDataHolder(itsInDHs[i]);

    itsInDHsinfo[i].currentDH = 0;
    itsInDHsinfo[i].id = -1;
  }
  for (int j = 0; j < itsNoutputs; j++)
  {
    itsSynMan->getOutPoolManagerPtr(j)->setDataHolder(itsOutDHs[j]);

    itsOutDHsinfo[j].currentDH = 0;
    itsOutDHsinfo[j].id = -1;
  }
}

DataManager::~DataManager()
{
  TRACER4("DataManager destructor");
  delete itsSynMan;
  delete [] itsInDHsinfo;
  delete [] itsOutDHsinfo;
}

DataHolder* DataManager::getInHolder(int channel)
{
  DbgAssertStr (channel >= 0,          "input channel too low");
  DbgAssertStr (channel < getInputs(), "input channel too high");
  DHPoolManager* dhpPtr = itsSynMan->getInPoolManagerPtr(channel);

  if (itsInDHsinfo[channel].currentDH == 0)
  {
    // If dataholder is shared between input and output, get read/write permission 
    if (dhpPtr->getSharing() == true)
    {
      itsInDHsinfo[channel].currentDH =  dhpPtr->getRWLockedDH(&itsInDHsinfo[channel].id);
    }
    else
    {
      itsInDHsinfo[channel].currentDH =  dhpPtr->getReadLockedDH(&itsInDHsinfo[channel].id); 
    }
  }
  return itsInDHsinfo[channel].currentDH;
}

DataHolder* DataManager::getOutHolder(int channel)
{
  DbgAssertStr(channel >= 0, "output channel too low");
  DbgAssertStr(channel < getOutputs(), "output channel too high");
  DHPoolManager* dhpPtr = itsSynMan->getOutPoolManagerPtr(channel);

  if (itsOutDHsinfo[channel].currentDH == 0)
  {
    // If dataholder is shared between input and output, get read/write permission
    if (dhpPtr->getSharing() == true)
    {
      itsOutDHsinfo[channel].currentDH = dhpPtr->getRWLockedDH(&itsOutDHsinfo[channel].id);
    }
    else
    {
      itsOutDHsinfo[channel].currentDH = dhpPtr->getWriteLockedDH(&itsOutDHsinfo[channel].id);
    }
  }

  return itsOutDHsinfo[channel].currentDH;
}

void DataManager::readyWithInHolder(int channel)
{
  DbgAssertStr (channel >= 0,          "input channel too low");
  DbgAssertStr (channel < getInputs(), "input channel too high");

  if (itsInDHsinfo[channel].id >= 0)
  {

    DHPoolManager* dhpPtr = itsSynMan->getInPoolManagerPtr(channel);

    if (itsSynMan->isInSynchronous(channel) == true)
    {
      itsInDHsinfo[channel].currentDH->read();
    }
    else
    {
      itsSynMan->readAsynchronous(channel);
    }

    if (dhpPtr->getSharing() == true)                // Shared in- and output
    {
      dhpPtr->writeUnlock(itsInDHsinfo[channel].id);
    }
    else
    {
      dhpPtr->readUnlock(itsInDHsinfo[channel].id);
    } 

    itsInDHsinfo[channel].id = -1;
    itsInDHsinfo[channel].currentDH = 0;
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

  if (itsOutDHsinfo[channel].id >= 0)
  {
    if (itsSynMan->isOutSynchronous(channel) == true)
    {
      TRACER4("DataManager::readyWithOutHolder synchronous write");
      itsOutDHsinfo[channel].currentDH->write();
    }
    else
    {
      TRACER4("DataManager::readyWithOutHolder asynchronous write");
      itsSynMan->writeAsynchronous(channel);
    }

    itsSynMan->getOutPoolManagerPtr(channel)->writeUnlock(itsOutDHsinfo[channel].id);

    itsOutDHsinfo[channel].id = -1;
    itsOutDHsinfo[channel].currentDH = 0;
  }
  else
  {
    TRACER2("DataManager::readyWithOutHolder Outholder not previously requested with getOutHolder() function");
  }
}

DataHolder* DataManager::getGeneralInHolder(int channel) const
{  
  DbgAssertStr(channel >= 0, "input channel too low");
  DbgAssertStr(channel < getInputs(), "input channel too high");
  return itsSynMan->getInPoolManagerPtr(channel)->getGeneralDataHolder();
}

DataHolder* DataManager::getGeneralOutHolder(int channel) const
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
    if (doAutoTriggerIn(ch) && (itsInDHsinfo[ch].currentDH == 0))
    {
      if (itsSynMan->isInSynchronous(ch) == true)
      {
	itsInDHsinfo[ch].currentDH = itsSynMan->getInPoolManagerPtr(ch)->
	                         getWriteLockedDH(&itsInDHsinfo[ch].id);
	itsInDHsinfo[ch].currentDH->read();
	itsSynMan->getInPoolManagerPtr(ch)->writeUnlock(itsInDHsinfo[ch].id);
	itsInDHsinfo[ch].id = -1;
	itsInDHsinfo[ch].currentDH = 0;
      }
      else
      {
        itsSynMan->readAsynchronous(ch);
      }
    }
  }

}

void DataManager::setInBufferingProperties(int channel, bool synchronous,
					   bool shareDHs) const
{
  AssertStr(itsInDHs[channel]->getTransporter().getTransportHolder()==0, 
	    "Input " << channel << 
	    " is already connected. Buffering properties must be set before connecting."); 
  DataHolder* dhPtr = getGeneralInHolder(channel);
  itsSynMan->setInSynchronous(channel, synchronous);
  itsSynMan->getInPoolManagerPtr(channel)->setDataHolder(dhPtr);

  if (shareDHs == true)
  {
    DbgAssertStr(channel < getOutputs(), 
 		 "corresponding output channel does not exist");
    itsSynMan->sharePoolManager(channel);   
  }

}

void DataManager::setOutBufferingProperties(int channel, bool synchronous,
					    bool shareDHs) const
{
  AssertStr(itsOutDHs[channel]->getTransporter().getTransportHolder()==0, 
	    "Output " << channel << 
	    " is already connected. Buffering properties must be set before connecting."); 
  DataHolder* dhPtr = getGeneralOutHolder(channel);
  itsSynMan->setOutSynchronous(channel, synchronous);
  itsSynMan->getOutPoolManagerPtr(channel)->setDataHolder(dhPtr);

  if (shareDHs == true)
  {
    DbgAssertStr(channel < getInputs(), 
 		 "corresponding input channel does not exist");
    DbgAssertStr(((itsSynMan->isInSynchronous(channel)) == synchronous), 
		 "Synchronisity of output is not equal to synchronisity of input. Changing output synchronisity to match input.");

    itsSynMan->sharePoolManager(channel);
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

}
