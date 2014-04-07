//# TH_Mem.cc: In-memory transport mechanism
//#
//# Copyright (C) 2000, 2001
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Transport/TH_Mem.h>
#include <Transport/DataHolder.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{

/**
 * Declare messages map which keeps track of all sent messages
 * until they have been received. This map is indexed with
 * the tag which is a unique for each connection created in
 * the Transport.
 */
map<int, DataHolder*> TH_Mem::theirSources;
#ifdef USE_THREADS
map<int, pthread_cond_t> TH_Mem::dataAvailable;
map<int, pthread_cond_t> TH_Mem::dataReceived;

pthread_mutex_t TH_Mem::theirMapLock = PTHREAD_MUTEX_INITIALIZER;  
#endif

TH_Mem::TH_Mem()
  : itsFirstSendCall (true),
    itsFirstRecvCall (true),
    itsDataSource    (0),
    itsFirstCall     (true)
{
  LOG_TRACE_FLOW("TH_Mem constructor");
}

TH_Mem::~TH_Mem()
{
  LOG_TRACE_FLOW("TH_Mem destructor");
}

string TH_Mem::getType() const
{
  return "TH_Mem";
}

TH_Mem* TH_Mem::clone() const
{
  return new TH_Mem();
}

void TH_Mem::initConditionVariables(int tag)
{
#ifdef USE_THREADS
  if (dataAvailable.find(tag) == dataAvailable.end())
  {
    pthread_cond_t condAv;
    pthread_cond_init(&condAv, NULL);
    dataAvailable[tag] = condAv;
  }
  if (dataReceived.find(tag) == dataReceived.end())
  {
    pthread_cond_t condRecv;
    pthread_cond_init(&condRecv, NULL);
    dataReceived[tag] = condRecv;
  }
#else
  LOG_WARN("initConditionVariables not executed since compiled without USE_THREADS");
#endif
}

int32 TH_Mem::recvNonBlocking(void* buf, int32 nbytes, int tag, int32 offset, DataHolder*)
{ 
  LOG_TRACE_RTTI("TH_Mem recvNonBlocking()");  
  // If first time, get the source DataHolder.
  if (itsFirstRecvCall) {
    itsDataSource = theirSources[tag];
    ASSERTSTR (itsDataSource != 0, "TH_Mem: no matching send for recv");
    // erase the record
    theirSources.erase (tag);
    itsFirstRecvCall = false;
  }
  char* dataPtr = static_cast<char*>(itsDataSource->getDataPtr()) + offset;
  memcpy(buf, dataPtr, nbytes);
  return nbytes;
}

/**
   The send function must now add its DataHolder to the map
   containing theirSources.
 */
bool TH_Mem::sendNonBlocking(void*, int, int tag, DataHolder* dh)
{
  LOG_TRACE_RTTI("TH_Mem sendNonBlocking()"); 
  if (itsFirstSendCall) {
    ASSERTSTR (dh!=0, "Source DataHolder needs to be specified in TH_Mem::sendNonBlocking method!");
    theirSources[tag] = dh;
    itsFirstSendCall = false;
  }
  return true;
}

bool TH_Mem::recvBlocking(void* buf, int nbytes, int tag, int nrBytesRead, DataHolder*)
{ 
#ifndef USE_THREADS
  LOG_ERROR("recvBlocking not available without USE_THREADS");
  return false;
#else
  LOG_WARN("TH_Mem::recvBlocking().Using blocking in-memory transport can cause a dead-lock.");

  pthread_mutex_lock(&theirMapLock);
  if (itsFirstCall)
  {
    initConditionVariables(tag);
    itsFirstCall = false;
  }

  if (theirSources.end() == theirSources.find(tag))
  {
    pthread_cond_wait(&dataAvailable[tag], &theirMapLock); // Wait for sent message
  }
  
  itsDataSource = theirSources[tag];

  char* dataPtr = static_cast<char*>(itsDataSource->getDataPtr()) + nrBytesRead;
  /// do the memcpy
  memcpy(buf, dataPtr, nbytes);

  // erase the record
  theirSources.erase(tag);        

  pthread_cond_signal(&dataReceived[tag]);
  pthread_mutex_unlock(&theirMapLock);
  return true;
#endif // USE_THREADS
}

/**
   The send function must now add its DataHolder to the map containing theirSources.
 */
bool TH_Mem::sendBlocking(void*, int, int tag, DataHolder* dh)
{
#ifndef USE_THREADS
  LOG_ERROR("sendBlocking not available without USE_THREADS");
  return false;
#else
  LOG_WARN("TH_Mem::sendBlocking(). Using blocking in-memory transport can cause a dead-lock."); 

  pthread_mutex_lock(&theirMapLock);
  if (itsFirstCall)
  {
    initConditionVariables(tag);
    itsFirstCall = false;
  }

  ASSERTSTR (dh!=0, "Source DataHolder needs to be specified in TH_Mem::sendBlocking method!");
  theirSources[tag] = dh;
  pthread_cond_signal(&dataAvailable[tag]); // Signal data available
  pthread_cond_wait(&dataReceived[tag], &theirMapLock);  // Wait for data received
  pthread_mutex_unlock(&theirMapLock);
  
  return true;
#endif //USE_THREADS
}

void TH_Mem::readTotalMsgLengthBlocking(int tag, int& nrBytes)
{
#ifndef USE_THREADS
  LOG_ERROR("readTotalMsgLengthBlocking not available without USE_THREADS");
#else
  LOG_TRACE_RTTI("TH_Mem readTotalMsgLengthBlocking()");  
  pthread_mutex_lock(&theirMapLock);

  if (itsFirstCall)
  {
    initConditionVariables(tag);
    itsFirstCall = false;
  }

  if (theirSources.end() == theirSources.find(tag))
  {
    pthread_cond_wait(&dataAvailable[tag], &theirMapLock); // Wait for sent message
  }
  
  itsDataSource = theirSources[tag];

  nrBytes = itsDataSource->getDataSize();
  pthread_mutex_unlock(&theirMapLock);

#endif //USE_THREADS
}

bool TH_Mem::readTotalMsgLengthNonBlocking(int tag, int& nrBytes)
{
  LOG_TRACE_RTTI("TH_Mem readTotalMsgLengthNonBlocking()");  
  // If first time, get the source DataHolder.
  if (itsFirstRecvCall) {
    itsDataSource = theirSources[tag];
    ASSERTSTR(itsDataSource != 0, "TH_Mem::readTotalMsgLengthNonBlocking: "<< 
	      " no matching send.");
    // erase the record
    theirSources.erase (tag);
    itsFirstRecvCall = false;
  }
  nrBytes = itsDataSource->getDataSize();
  return true;
}

// NB: The following method doesn't do what you expect! It does not
// guarantee the buffer contents has been safely sent.
// Your application should make sure write/read are called alternately.
void TH_Mem::waitForSent(void*, int, int)
{
  LOG_WARN("TH_Mem::waitForSent does not guarantee the safety of your buffer. You should handle this in your application! (call write/read alternately)");
}

void TH_Mem::reset()
{
  LOG_TRACE_FLOW("TH_Mem reset");
  itsFirstSendCall = true;
  itsFirstRecvCall = true;
  itsDataSource = 0;
}

void TH_Mem::waitForReceived(void*, int, int)
{}

void TH_Mem::waitForBroadCast()
{
  LOG_TRACE_RTTI("TH_Mem waitForBroadCast()"); 
}

void TH_Mem::waitForBroadCast(unsigned long&)
{
  LOG_TRACE_RTTI("TH_Mem waitForBroadCast(..)");
}


void TH_Mem::sendBroadCast(unsigned long)
{
  LOG_TRACE_RTTI("TH_Mem sendBroadCast()");
}

int TH_Mem::getCurrentRank()
{
    return -1;
}

int TH_Mem::getNumberOfNodes()
{
    return 1;
}

void TH_Mem::init(int, const char* [])
{
  LOG_TRACE_RTTI("TH_Mem init()");
}

void TH_Mem::finalize()
{
  LOG_TRACE_RTTI("TH_Mem finalize()");
}

void TH_Mem::synchroniseAllProcesses()
{ 
  LOG_TRACE_RTTI("TH_Mem synchroniseAllProcesses()");
}

}
