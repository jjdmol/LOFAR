//# TH_Mem_Bl.cc: In-memory transport mechanism
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


#include <Transport/TH_Mem_Bl.h>
#include <Common/Debug.h>
#include <unistd.h>

namespace LOFAR
{

/**
 * Declare messages map which keeps track of all sent messages
 * until they have been received. This map is indexed with
 * the tag which is a unique for each connection created in
 * the Transport.
 */
map<int, TH_Mem_Bl::Msg> TH_Mem_Bl::messages;
map<int, pthread_cond_t> TH_Mem_Bl::dataAvailable;
map<int, pthread_cond_t> TH_Mem_Bl::dataReceived;

pthread_mutex_t TH_Mem_Bl::theirTHMemLock = PTHREAD_MUTEX_INITIALIZER;

/**
 * Prototype variable declaration. Can be
 * used in functions requiring a prototype
 * argument (for the prototype design patterns).
 */
TH_Mem_Bl TH_Mem_Bl::proto;

TH_Mem_Bl::TH_Mem_Bl() :
  itsFirstCall (true)
{
}

TH_Mem_Bl::~TH_Mem_Bl()
{
}

TH_Mem_Bl* TH_Mem_Bl::make() const
{
    return new TH_Mem_Bl();
}

string TH_Mem_Bl::getType() const
{
  return "TH_Mem_Bl";
}

bool TH_Mem_Bl::connectionPossible(int srcRank, int dstRank) const
{
  cdebug(3) << "TH_Mem::connectionPossible between "
            << srcRank << " and "
            << dstRank << "?" << endl;

  return srcRank == dstRank;
}

void TH_Mem_Bl::initConditionVariables(int tag)
{
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
}

int TH_Mem_Bl::recvLengthBlocking(int tag)
{ 
  cerr << "Warning: TH_Mem_Bl::recvLengthBlocking() "  
       << "Using blocking in-memory transport can cause a dead-lock." 
       << endl;
  Msg m;
  pthread_mutex_lock(&theirTHMemLock);
  if (itsFirstCall)
  {
    initConditionVariables(tag);
    itsFirstCall = false;
  }

  if (messages.end() == messages.find(tag))
  {
    pthread_cond_wait(&dataAvailable[tag], &theirTHMemLock); // Wait for sent message
  }
    
  m = messages[tag];

  int leng = m.getNBytes();
  pthread_mutex_unlock(&theirTHMemLock);
  return leng;
}

bool TH_Mem_Bl::recvBlocking(void* buf, int nbytes, int tag)
{ 
  cerr << "Warning: TH_Mem_Bl::recvBlocking() "  
       << "Using blocking in-memory transport can cause a dead-lock." 
       << endl;
  Msg m;
  pthread_mutex_lock(&theirTHMemLock);
  if (itsFirstCall)
  {
    initConditionVariables(tag);
    itsFirstCall = false;
  }

  if (messages.end() == messages.find(tag))
  {
    pthread_cond_wait(&dataAvailable[tag], &theirTHMemLock); // Wait for sent message
  }
    
  m = messages[tag];

  if (nbytes == m.getNBytes())
  {
    /// do the memcpy
    memcpy(buf, m.getBuf(), m.getNBytes());
	
    // erase the record
    messages.erase(tag);
  }
  else
  {
    // erase the record
    messages.erase(tag);
	
    Throw("Number of bytes do not match");
  }

  pthread_cond_signal(&dataReceived[tag]);
  pthread_mutex_unlock(&theirTHMemLock);
  return true;
}

/**
   The send function must now add the buffer to the map containing messages.
 */
bool TH_Mem_Bl::sendBlocking(void* buf, int nbytes, int tag)
{
  cerr << "Warning: TH_Mem_Bl::sendBlocking() "  
       << "Using blocking in-memory transport can cause a dead-lock." 
       << endl;

  pthread_mutex_lock(&theirTHMemLock);
  if (itsFirstCall)
  {
    initConditionVariables(tag);
    itsFirstCall = false;
  }

  Msg      m(buf, nbytes, tag);
  messages[tag] = m;
  pthread_cond_signal(&dataAvailable[tag]); // Signal data available
  pthread_cond_wait(&dataReceived[tag], &theirTHMemLock);  // Wait for data received
  pthread_mutex_unlock(&theirTHMemLock);
  
  return true;
}

bool TH_Mem_Bl::sendNonBlocking(void* buf, int nbytes, int tag)
{
  pthread_mutex_lock(&theirTHMemLock);
  if (itsFirstCall)
  {
    initConditionVariables(tag);
    itsFirstCall = false;
  }

  if (messages.find(tag) != messages.end())  // Wait for previous send to finish
  {
    pthread_cond_wait(&dataReceived[tag], &theirTHMemLock);
  }

  Msg      m(buf, nbytes, tag);
  messages[tag] = m;
  pthread_cond_signal(&dataAvailable[tag]);
  pthread_mutex_unlock(&theirTHMemLock);
  
  return true;
}

bool TH_Mem_Bl::waitForSent(void* buf, int nbytes, int tag)
{
  return waitForRecvAck(buf, nbytes, tag);
}

bool TH_Mem_Bl::waitForRecvAck(void*, int, int tag)
{
  pthread_mutex_lock(&theirTHMemLock);
  if (messages.find(tag) != messages.end())  // Wait for send to finish
  {
    pthread_cond_wait(&dataReceived[tag], &theirTHMemLock);
  }
  pthread_mutex_unlock(&theirTHMemLock);
  
  return true;
}

void TH_Mem_Bl::waitForBroadCast()
{}

void TH_Mem_Bl::waitForBroadCast(unsigned long&)
{}


void TH_Mem_Bl::sendBroadCast(unsigned long)
{}

int TH_Mem_Bl::getCurrentRank()
{
    return -1;
}

int TH_Mem_Bl::getNumberOfNodes()
{
    return 1;
}

void TH_Mem_Bl::init(int, const char* [])
{}

void TH_Mem_Bl::finalize()
{}

void TH_Mem_Bl::synchroniseAllProcesses()
{}

TH_Mem_Bl::Msg::Msg()
{
}

TH_Mem_Bl::Msg::Msg(void* buf, int nbytes, int tag) :
    itsBuf(buf), itsNBytes(nbytes), itsTag(tag)
{}

}
