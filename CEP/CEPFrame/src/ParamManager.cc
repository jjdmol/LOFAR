//  ParamManager.cc:
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

#include "CEPFrame/ParamManager.h"
#include "CEPFrame/ParamTransportManager.h"
#include "Common/Debug.h"
#include <pthread.h>

#include "CEPFrame/TH_MPI.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ParamManager::ParameterBuffer::ParameterBuffer()
  :  param(0), 
     outputBuf(0)
{} 

ParamManager::ParameterBuffer::~ParameterBuffer()
{
  inputBuf.RemoveElements();  // delete 2 elements?
  delete param;
  if (outputBuf != 0)
  {
    delete outputBuf;
  }
}

ParamManager::ParamManager ()
{
  TRACER2("ParamManager constructor");
}

ParamManager::~ParamManager()
{
  TRACER4("ParamManager destructor");
}

ParamHolder* ParamManager::getParamHolder(const string& name)
{
  if (itsParamBuffers.find(name) == itsParamBuffers.end())
  {
    return 0;
  }
  else
  {
    return itsParamBuffers[name]->param;
  }
}

void ParamManager::preprocess()
{
  TRACER4("ParamManager::preprocess");
  map<string, ParameterBuffer*>::iterator iter;
  for (iter=itsParamBuffers.begin(); iter!=itsParamBuffers.end(); iter++)
  {
    int id1;
    int id2;
    ParameterBuffer* pBuf = iter->second;
    DbgAssertStr(pBuf->inputBuf.GetSize() == 2, 
		 "Input buffer does not have two elements");
    // preprocess two elements of inputbuffer
    ParamHolder* ph1 = pBuf->inputBuf.GetWriteLockedDataItem(&id1);
    ParamHolder* ph2 = pBuf->inputBuf.GetWriteLockedDataItem(&id2);
    ph1->preprocess();
    ph2->preprocess();
    pBuf->inputBuf.WriteUnlockElement(id1);
    pBuf->inputBuf.WriteUnlockElement(id2);

    pBuf->inputBuf.GetReadDataItem(&id1);
    pBuf->inputBuf.ReadUnlockElement(id1);
    pBuf->inputBuf.GetReadDataItem(&id1);
    pBuf->inputBuf.ReadUnlockElement(id1);
    // preprocess parameter
    pBuf->param->preprocess();
     // preprocess outputbuffer
    if (pBuf->outputBuf != 0)
    {
      pBuf->outputBuf->preprocess();
    }
    ParamTransportManager* ptMan = pBuf->param->getPTManager();
    ptMan->preprocess();
    ptMan->startReading();
  }
}

void ParamManager::postprocess()
{
#ifdef HAVE_MPI
  if (itsParamBuffers.size() > 0)    // Only use this temporary solution
  {                                  // when parameters are used 
    TH_MPI::synchroniseAllProcesses();
  }
#endif
  map<string, ParameterBuffer*>::iterator iter;
  for (iter = itsParamBuffers.begin(); iter != itsParamBuffers.end();
       iter++)
  {
    disconnectParam(iter->first);
  }
}

bool ParamManager::addParamHolder(ParamHolder* phptr, const string& name,
				  bool isParamOwner)
{
  if (itsParamBuffers.find(name) == itsParamBuffers.end())
  {
    phptr->setPTManager(new ParamTransportManager());
    phptr->setParamOwner(isParamOwner);
    ParameterBuffer* pBuf = new ParameterBuffer();        // create new buffers
    pBuf->inputBuf.AddBufferElement(phptr->clone()); 
    pBuf->inputBuf.AddBufferElement(phptr->clone());
    pBuf->param = phptr;
    phptr->getPTManager()->setInputBuffer(&(pBuf->inputBuf));
    if (isParamOwner)
    {
      pBuf->outputBuf = phptr->clone();
    }
    else
    {
      pBuf->outputBuf = 0;
    }
    phptr->getPTManager()->setOutputBuffer(pBuf->outputBuf);
    itsParamBuffers[name] = pBuf;
    return true;
  }
  else
  {
    return false;
  }
} 

void ParamManager::publishParam(string paramName)
{
  map<string, ParameterBuffer*>::iterator it = itsParamBuffers.find(paramName);
  if (it != itsParamBuffers.end())
  { 
    ParamTransportManager* ptManager = it->second->param->getPTManager();

    if ((ptManager != 0) || (ptManager->getSourceTransport()->hasSentLast()) 
	|| (ptManager->getSourceTransport()->hasReceivedLast()))
    { 
      ParamHolder* outputPH = it->second->outputBuf;
      ptManager->joinWriterThreads(); // wait for previous writer threads 
                                      // to finish
      outputPH->writeLock();

      // copy packet to output buffer
      memcpy(outputPH->getParamPacket(), 
	     it->second->param->getParamPacket(),
	     it->second->param->getParamPacketSize());

      ptManager->writeParam();

      outputPH->writeUnlock(); // Unlock after writeParam() call to prevent
                               // further writing in the time between 
                               // starting of writer thread(s)

    }
    else
    { 
      TRACER4("No writing of param: no ParamTransportManager");
    }
  }
  else
  {
    TRACER4("Unknown parameter: no writing");
  }

}


void ParamManager::getLastValue(string paramName)
{
  map<string, ParameterBuffer*>::iterator it = itsParamBuffers.find(paramName);
  if (it != itsParamBuffers.end())
  { 
    ParamTransportManager* ptManager = it->second->param->getPTManager();
    if (ptManager != 0)
    { 
	if (it->second->inputBuf.GetCount() >= 2) // Check if a new value has
	{                                          // been received
	  int id;
	  ParamHolder* inputPh = it->second->inputBuf.GetReadDataItem(&id);
          
	  if (!(((ParamHolder::ParamPacket*)(inputPh->getParamPacket()))
	      ->itsFinalMsg == PARAM_FINAL_MSG_MAGIC_NO))
	  {
	    // copy input buffer packet to paramholder
	    memcpy(it->second->param->getParamPacket(), 
		   inputPh->getParamPacket(),
		   inputPh->getParamPacketSize());
	  }
	  else
	  {
	    TRACER4("Last value is final message. No value copied to parameter");
	  } 
	
	  it->second->inputBuf.ReadUnlockElement(id);
	}
     }
    else
    { 
      TRACER4("No ParamTransportManager set");
    }
  }
  else
  {
    TRACER4("Unknown parameter");
  }  
}

bool ParamManager::getNewValue(string paramName)
{
  map<string, ParameterBuffer*>::iterator it = itsParamBuffers.find(paramName);
  if (it != itsParamBuffers.end())
  { 
    ParamTransportManager* ptManager = it->second->param->getPTManager();
    if ((ptManager != 0) && (ptManager->getSourceTransport() != 0))
    { 
      if ((ptManager->getSourceTransport()->hasSentLast()) || 
	  (ptManager->getSourceTransport()->hasReceivedLast()) )
      {
	TRACER2("Source has been terminated");
	return false;
      }
      else
      {
	int id;
	ParamHolder* inputPh = it->second->inputBuf.GetReadDataItem(&id);

	if (!(((ParamHolder::ParamPacket*)(inputPh->getParamPacket()))
	    ->itsFinalMsg == PARAM_FINAL_MSG_MAGIC_NO))
	{
	  // copy input buffer packet to paramholder
	  memcpy(it->second->param->getParamPacket(), 
		 inputPh->getParamPacket(),
		 inputPh->getParamPacketSize());
	  it->second->inputBuf.ReadUnlockElement(id);
	  return true;
	}
	else
	{
	  it->second->inputBuf.ReadUnlockElement(id);
          TRACER4("New value is final message. No value copied to parameter");
	  return false; 
	}
      }
    }
    else
    { 
      TRACER4("No ParamTransportManager set or no ParamTransport present");
      return false;
    }
  }
  else
  {
    TRACER4("Unknown parameter");
    return false;
  }
}

void ParamManager::setStep(StepRep& step)
{
  map<string, ParameterBuffer*>::iterator iter;  
  for (iter=itsParamBuffers.begin(); iter!=itsParamBuffers.end(); iter++)
  {
    int id1;
    int id2;
    DbgAssertStr(iter->second->inputBuf.GetSize() == 2, 
		 "Input buffer does not have two elements");
    // set Step of two elements of inputbuffer
    ParamHolder* ph1 = iter->second->inputBuf.GetWriteLockedDataItem(&id1);
    ParamHolder* ph2 = iter->second->inputBuf.GetWriteLockedDataItem(&id2);
    ph1->setStep(step);
    ph2->setStep(step);
    iter->second->inputBuf.WriteUnlockElement(id1);
    iter->second->inputBuf.WriteUnlockElement(id2);

    iter->second->inputBuf.GetReadDataItem(&id1);
    iter->second->inputBuf.ReadUnlockElement(id1);
    iter->second->inputBuf.GetReadDataItem(&id1);
    iter->second->inputBuf.ReadUnlockElement(id1);
    // set Step of parameter
    iter->second->param->setStep(step);
     // set Step of outputbuffer
    if (iter->second->outputBuf != 0)
    {
      iter->second->outputBuf->setStep(step);
    }
  }
}

void ParamManager::disconnectParam(const string& name)
{
  AssertStr(itsParamBuffers.find(name) != itsParamBuffers.end(),
    "No such parameter");
  ParamHolder* phPtr = itsParamBuffers[name]->param;
  phPtr->getPTManager()->disconnectParam();
}
