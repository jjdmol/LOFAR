//#  ParamTransportManager.cc: Manages transports of a parameter
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
////////////////////////////////////////////////////////////////////

#include "CEPFrame/ParamTransportManager.h"
#include TRANSPORTERINCLUDE
#include "CEPFrame/PH_Int.h"


ParamTransportManager::ParamTransportManager()
  : itsSourceTransport(0),
    itsOutputBuf(0),
    itsInputBuf(0),
    itsWLockedID(-1)
{
  itsTermMsgPH = new PH_Int("temp");
}

ParamTransportManager::~ParamTransportManager()
{
  delete itsTermMsgPH;
  if (itsSourceTransport != 0) { delete itsSourceTransport; }
  itsTgtTPMutex.lock();
  while (!itsTgtTransports.empty())
  {
    //   delete
  }
  itsTgtTPMutex.unlock();
}

void ParamTransportManager::preprocess()
{
  itsTermMsgPH->preprocess();
}

void ParamTransportManager::writeParam()
{
  itsTgtTPMutex.lock();
  if (!itsTgtTransports.empty())
  {
    map<unsigned int, ParamTransport*>::iterator tpIter;
    for (tpIter = itsTgtTransports.begin(); 
         tpIter != itsTgtTransports.end();
	 tpIter++)
    {
      DbgAssertStr(tpIter->second->getTargetAddr() != 0, 
		       "No target ParamHolder specified"); 
      pthread_t temp;
      // Begin writer threads to all targets
      ParamTransport* ptPtr = tpIter->second;
      ptPtr->lock();
      if ((ptPtr->hasReceivedLast() == false) &&
	  (ptPtr->hasSentLast() == false))
      {	
        AssertStr(itsOutputBuf != 0, "Outputbuffer not set");
        ptPtr->setOutParamHolder(itsOutputBuf);
	int status;
        status = pthread_create(&temp , NULL, startWriterThread, ptPtr);
	itsWritersMutex.lock();
	itsWriterThreads.push(temp);
	itsWritersMutex.unlock();
	if (status != 0)
	{
	  TRACER2("ParamTransportManager::writeParam  Failed to create parameter writer thread");
	}
      }
      ptPtr->unlock();
    }
  }
  itsTgtTPMutex.unlock();
}

void* ParamTransportManager::startWriterThread(void* theTP)
{
  ParamTransport* thisTP = (ParamTransport*)theTP;
  AssertStr(thisTP != 0, "Target transport is 0");
  thisTP->getOutParamHolder()->readLock();
  thisTP->lock();
  if (thisTP->hasSentLast() == false)
  {
    thisTP->write();
  }
  thisTP->unlock();
  thisTP->getOutParamHolder()->readUnlock();
  return NULL;
}

void ParamTransportManager::joinWriterThreads()
{
  itsTgtTPMutex.lock();
  if (!(itsTgtTransports.empty()))
  {
    void* threadResult;
    itsWritersMutex.lock();
    while (!(itsWriterThreads.empty()))
    {
      TRACER2("Joining " << itsWriterThreads.size() << " threads with id: "
	      << itsWriterThreads.top());   
      pthread_join(itsWriterThreads.top(), &threadResult);
      itsWriterThreads.pop();
    }
    itsWritersMutex.unlock();
  }
  itsTgtTPMutex.unlock();
}

void ParamTransportManager::startReading()
{
  pthread_t temp;
  if (itsSourceTransport != 0)
  {
    TRACER4("Starting reader thread");
    pthread_create(&temp, NULL, startReaderThread, this);
  }

  itsTgtTPMutex.lock();
  if (itsOutputBuf != 0 && itsOutputBuf->isParamOwner()
      && (!itsTgtTransports.empty()))                     // 'Read termination
  {                                                       // message' thread
    map<unsigned int, ParamTransport*>::iterator tpIter;
    for (tpIter = itsTgtTransports.begin(); tpIter != itsTgtTransports.end();
	 tpIter++)
    {
      TRACER4("Starting 'wait for termination message' thread");
      DbgAssertStr(tpIter->second->getSourceAddr() != 0, 
		       "No source ParamHolder specified"); 
      
      ParamTransport* theTP = tpIter->second;
      itsTermMsgPH->setStep(itsOutputBuf->getStep());
      itsTermMsgPH->setPTManager(this);
      theTP->setInParamHolder(itsTermMsgPH);
      pthread_create(&temp, NULL, startWaitForTerminationThread, theTP); 
    }   
  }
  itsTgtTPMutex.unlock();

}

void* ParamTransportManager::startReaderThread(void* thePTM)
{
  ParamTransportManager* thisPTM = (ParamTransportManager*)thePTM;
  AssertStr(thisPTM->itsSourceTransport != 0, "No transport available");
  while (1)
  {
    //Get new inputbuffer pointer
     ParamHolder* ph = thisPTM->itsInputBuf->GetWriteLockedDataItem(&thisPTM->itsWLockedID);
    ParamTransport* ptPtr = thisPTM->getSourceTransport();
    ptPtr->setInParamHolder(ph);
    ptPtr->read();

    // Check for final message
    if (((ParamHolder::ParamPacket*)(ph->getParamPacket()))->itsFinalMsg 
          == PARAM_FINAL_MSG_MAGIC_NO) 
    {
      ptPtr->lock();
      ptPtr->setReceivedLast();
      TRACER2("Received final message on transport with id " 
	      << ptPtr->getItsID() << " Temp value " 
	      << ((PH_Int*)ph)->getValue());

      if (ptPtr->hasSentLast() == false)
      {
	ptPtr->unlock();
	thisPTM->sendTerminationMsg();
      }
      else
      {
	thisPTM->itsSourceTransport = 0;
	ptPtr->unlock();
	delete ptPtr;
      }
      thisPTM->unlockInputBuffer();
      return NULL;
    }

    thisPTM->unlockInputBuffer();
  }

  return NULL;
}

void* ParamTransportManager::startWaitForTerminationThread(void* theTP)
{
  ParamTransport* ptPtr = (ParamTransport*)theTP;
  AssertStr(ptPtr != 0, "No transport");

  AssertStr(ptPtr->read(), "Read in WaitForTerminationThread failed");

  PH_Int* recvMsg = (PH_Int*)ptPtr->getInParamHolder();
  DbgAssertStr(recvMsg->getValue() == PARAM_FINAL_MSG_MAGIC_NO, 
	       "Received a different message");
  TRACER2("Termination message received for Transport with id " << 
	  ptPtr->getItsID());
  ptPtr->lock();
  ptPtr->setReceivedLast();
  if (ptPtr->hasSentLast() == false)
  {
    ptPtr->unlock();
    TRACER2("Termination thread calling: sendFinalMsg");
    recvMsg->getPTManager()->sendFinalMsg(ptPtr);
  }
  else
  {
    recvMsg->getPTManager()->removeTargetTransport(ptPtr->getItsID());
    ptPtr->unlock();
    delete ptPtr;
  }

  return NULL;
}

void ParamTransportManager::unlockInputBuffer()
{
  if (itsWLockedID >= 0)
  {
    DbgAssertStr(itsInputBuf != 0, "Input buffer pointer is 0");
    itsInputBuf->WriteUnlockElement(itsWLockedID);
  }
}

ParamTransport* ParamTransportManager::makeNewTargetTransport(unsigned int id)
{
    TRACER4("Creating new target transport");
    ParamTransport* ptPtr = new ParamTransport(itsOutputBuf, true);
    itsTgtTPMutex.lock();
    itsTgtTransports[id] = ptPtr;    // Add transport
    itsTgtTPMutex.unlock();
    return ptPtr;
}

ParamTransport* ParamTransportManager::makeNewSourceTransport()
{
    TRACER4("Creating new target transport");
    ParamTransport* ptPtr = new ParamTransport(0, false);
    AssertStr(itsSourceTransport == 0, "Still connected, disconnect first");
    itsSourceTransport = ptPtr;    // Add transport
    return ptPtr;
}

void ParamTransportManager::removeTargetTransport(unsigned int id)
{
  map<unsigned int, ParamTransport*>::iterator tpIter;
  itsTgtTPMutex.lock();
  tpIter = itsTgtTransports.find(id);
  if (tpIter != itsTgtTransports.end())
  {
    itsTgtTransports.erase(id);
  }
  itsTgtTPMutex.unlock();
}

void ParamTransportManager::disconnectParam()
{
  sendTerminationMsg();
  if (itsOutputBuf != 0)
  {
    sendFinalMsg();
  }
}

void ParamTransportManager::sendTerminationMsg()
{
  if (itsSourceTransport != 0)
  {
    ParamTransport* ptPtr = itsSourceTransport;
    ptPtr->lock();
    if (ptPtr->hasSentLast() == false)
    {
      ptPtr->setSentLast();
      TRACER2("Sending termination message on transport with id " 
	      << ptPtr->getItsID());
      ptPtr->setOutParamHolder(itsTermMsgPH);
      itsTermMsgPH->setValue(PARAM_FINAL_MSG_MAGIC_NO);
      ptPtr->write();
    }
    if (ptPtr->hasReceivedLast())
    {
      itsSourceTransport = 0;
      ptPtr->unlock();
      delete ptPtr;
    }
    else
    {
      ptPtr->unlock();
    }
  }
  else
  {
    TRACER2("No source transport could be found");
  }
}

void ParamTransportManager::sendFinalMsg()
{
  map<unsigned int, ParamTransport*>::iterator tpIter;
  itsTgtTPMutex.lock();
  for (tpIter = itsTgtTransports.begin(); 
       tpIter != itsTgtTransports.end();
       tpIter++)
  {
    itsTgtTPMutex.unlock();
    sendFinalMsg(tpIter->second);
    itsTgtTPMutex.lock();
  }
}

void ParamTransportManager::sendFinalMsg(ParamTransport* ptPtr)
{
  AssertStr(ptPtr != 0, "Target transport is 0");
  ptPtr->lock();
  if (ptPtr->hasSentLast() == false)
  {
    ptPtr->setSentLast();
    joinWriterThreads();  // Wait for previous writer threads to finish
                          // before changing output buffer
    itsOutputBuf->writeLock();       // Lock output buffer
    TRACER2("Sending final message on transport with id " 
	    << ptPtr->getItsID());
    // Send final message
    ((ParamHolder::ParamPacket*)(itsOutputBuf->getParamPacket()))
                                ->itsFinalMsg = PARAM_FINAL_MSG_MAGIC_NO;
       ptPtr->setOutParamHolder(itsOutputBuf);
    // Write to target
    ptPtr->write();
    if (ptPtr->hasReceivedLast())
    {
      removeTargetTransport(ptPtr->getItsID());
      ptPtr->unlock();
      delete ptPtr;
    }
    else
    {
      ptPtr->unlock();
    }
    itsOutputBuf->writeUnlock();     // Unlock output buffer
  }
  else
  {
    ptPtr->unlock();
  }
}

