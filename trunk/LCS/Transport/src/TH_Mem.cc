//# TH_Mem.cc: In-memory transport mechanism
//#
//# Copyright (C) 2000, 2001
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>


#include <Transport/TH_Mem.h>
#include <Transport/Transporter.h>
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
map<int, DataHolder*> TH_Mem::theSources;
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

TH_Mem* TH_Mem::make() const
{
  return new TH_Mem();
}

string TH_Mem::getType() const
{
  return "TH_Mem";
}

bool TH_Mem::connectionPossible(int srcRank, int dstRank) const
{
  LOG_TRACE_RTTI_STR( "TH_Mem::connectionPossible between "
		      << srcRank << " and "
		      << dstRank << "?" );
  return srcRank == dstRank;
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
  LOG_WARN("initConditionVAriables not executed since compiled without USE_THREADS");
#endif
}

bool TH_Mem::recvNonBlocking(void* buf, int nbytes, int tag)
{ 
  LOG_TRACE_RTTI("TH_Mem recvNonBlocking()");  
  // If first time, get the source DataHolder.
  if (itsFirstRecvCall) {
    itsDataSource = theSources[tag];
    ASSERTSTR (itsDataSource != 0, "TH_Mem: no matching send for recv");
    ASSERT (nbytes == itsDataSource->getDataSize());
    // erase the record
    theSources.erase (tag);
    itsFirstRecvCall = false;
  }
  DBGASSERT (nbytes == itsDataSource->getDataSize());
  memcpy(buf, itsDataSource->getDataPtr(), nbytes);
  return true;
}

/**
   The send function must now add its DataHolder to the map
   containing theSources.
 */
bool TH_Mem::sendNonBlocking(void* /*buf*/, int nbytes, int tag)
{
  LOG_TRACE_RTTI("TH_Mem sendNonBlocking()"); 
  if (itsFirstSendCall) {
    theSources[tag] = getTransporter()->getDataHolder();
    itsFirstSendCall = false;
  }
  return true;
}

bool TH_Mem::recvVarNonBlocking(int tag)
{ 
  LOG_TRACE_RTTI("TH_Mem recvVarNonBlocking()"); 
  // If first time, get the source DataHolder.
  if (itsFirstRecvCall) {
    itsDataSource = theSources[tag];
    ASSERTSTR (itsDataSource != 0, "TH_Mem: no matching send for recv");
    // erase the record
    theSources.erase (tag);
    itsFirstRecvCall = false;
  }
  int nb = itsDataSource->getDataSize();
  DataHolder* target = getTransporter()->getDataHolder();
  target->resizeBuffer (nb);
  memcpy (target->getDataPtr(), itsDataSource->getDataPtr(), nb);
  return true;
}

bool TH_Mem::recvBlocking(void* buf, int nbytes, int tag)
{ 
#ifndef USE_THREADS
  LOG_ERROR("recvBlocking not available without USE_THREADS");
  return false;
#else
  LOG_WARN("TH_Mem::recvBlocking().Using blocking in-memory transport can cause a dead-lock.")

  pthread_mutex_lock(&theirMapLock);
  if (itsFirstCall)
  {
    initConditionVariables(tag);
    itsFirstCall = false;
  }

  if (theSources.end() == theSources.find(tag))
  {
    pthread_cond_wait(&dataAvailable[tag], &theirMapLock); // Wait for sent message
  }
  
  itsDataSource = theSources[tag];

  if (nbytes == itsDataSource->getDataSize())
  {
    /// do the memcpy
    memcpy(buf, itsDataSource->getDataPtr(), nbytes);
        
    // erase the record
    theSources.erase(tag);
  }
  else
  {
    // erase the record
    theSources.erase(tag);
        
    THROW(LOFAR::Exception, "Number of bytes do not match");
  }

  pthread_cond_signal(&dataReceived[tag]);
  pthread_mutex_unlock(&theirMapLock);
  return true;
#endif // USE_THREADS
}

/**
   The send function must now add its DataHolder to the map containing theSources.
 */
bool TH_Mem::sendBlocking(void* /*buf*/, int nbytes, int tag)
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

  theSources[tag] = getTransporter()->getDataHolder();
  pthread_cond_signal(&dataAvailable[tag]); // Signal data available
  pthread_cond_wait(&dataReceived[tag], &theirMapLock);  // Wait for data received
  pthread_mutex_unlock(&theirMapLock);
  
  return true;
#endif //USE_THREADS
}

bool TH_Mem::recvVarBlocking(int tag)
{ 
#ifndef USE_THREADS
  LOG_ERROR("recvVarBlocking not available without USE_THREADS");
  return false;
#else

  LOG_WARN("TH_Mem::recvVarBlocking(). Using blocking in-memory transport can cause a dead-lock.");

  pthread_mutex_lock(&theirMapLock);
  if (itsFirstCall)
  {
    initConditionVariables(tag);
    itsFirstCall = false;
  }

  if (theSources.end() == theSources.find(tag))
  {
    pthread_cond_wait(&dataAvailable[tag], &theirMapLock); // Wait for sent message
  }
  
  itsDataSource = theSources[tag];
  int nb = itsDataSource->getDataSize();
  DataHolder* target = getTransporter()->getDataHolder();
  target->resizeBuffer(nb);
  /// do the memcpy
  memcpy(target->getDataPtr(), itsDataSource->getDataPtr(), nb);
  
  // erase the record
  theSources.erase(tag);

  pthread_cond_signal(&dataReceived[tag]);
  pthread_mutex_unlock(&theirMapLock);
  return true;
#endif //USE_THREADS
}

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
