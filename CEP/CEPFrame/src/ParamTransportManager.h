//#  ParamTransportManager.h: Manages transports of a parameter
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

#if !defined(CEPFRAME_PARAMTRANSPORTMANAGER_H)
#define CEPFRAME_PARAMTRANSPORTMANAGER_H

//# Includes
#include <Common/lofar_map.h>
#include "CEPFrame/ParamTransport.h"
#include "CEPFrame/CyclicOWBuffer.h"
#include "CEPFrame/PH_Int.h"

#include <Common/lofar_stack.h>
#include "CEPFrame/Lock.h"

#define PARAM_FINAL_MSG_MAGIC_NO 555

//# Forward Declarations


// This class manages the ParamTransports of a parameter

class ParamTransportManager
{
 public:
  // Constructor
  ParamTransportManager();
  // Destructor
  virtual ~ParamTransportManager();

  // Preprocessing function
  void preprocess();

  // Write the parameter to all connected ParamHolders
  void writeParam();

  // Get the transport to a parameter holder (used on receiving side)
  ParamTransport* getSourceTransport();

  // Make new transport to a parameter holder 'target'
  ParamTransport* makeNewTargetTransport(unsigned int id);

  // Make new transport to a parameter holder 'source'
  ParamTransport* makeNewSourceTransport();

  // Remove target transport from the transport map
  void removeTargetTransport(unsigned int id);

  // Terminate all connections with this parameter
  void disconnectParam();

  // Start parameter reader thread
  void startReading();

  // Set pointer to output buffer
  void setOutputBuffer(ParamHolder* outputBuf);

  // Set pointer to input buffer
  void setInputBuffer(CyclicOWBuffer<ParamHolder*>* inputBuf);

  // Wait for all writer threads to finish
  void joinWriterThreads();

 private:
  // Start function for reader thread
  static void* startReaderThread(void* thePTM);
  // Start function for writer thread
  static void* startWriterThread(void* theTP);
  // Start function for thread waiting for termination signal
  static void* startWaitForTerminationThread(void* theTP);

  // The following two functions are used to signal that a connection between
  // paramHolders is to be terminated. The paramTransportManager of a 'sender'
  // must send a FinalMsg and receive a TerminationMsg (not necessarily in 
  // that order) for the connection to be terminated. The paramTransportManager
  // of a 'receiver' parameter must send a TerminationMsg and receive a 
  // FinalMsg.

  // Send a message to the 'publisher' of this Parameter in order to 
  // indicate this connection has to be terminated. 
  void sendTerminationMsg();

  // Send a message to all 'listeners' of this Parameter in order to
  // indicate this connection is terminated
  void sendFinalMsg();

  // Send a termination message on transport id 
  void sendFinalMsg(ParamTransport* ptPtr);

  // Remove lock on input buffer
  void unlockInputBuffer();

  ParamTransport* itsSourceTransport;
  ThreadMutex itsTgtTPMutex;
  map<unsigned int, ParamTransport*> itsTgtTransports; //transports to 
                                                       //'subscribers'
  ParamHolder* itsOutputBuf;
  CyclicOWBuffer<ParamHolder*>* itsInputBuf;  // input buffer
  PH_Int* itsTermMsgPH;
  int itsWLockedID;       // ID of bufferelement which is write locked
  stack<pthread_t> itsWriterThreads;
  ThreadMutex itsWritersMutex; // Mutex to control access to itsWriterThreads
};

inline void ParamTransportManager::setOutputBuffer(ParamHolder* outputBuf)
{ itsOutputBuf = outputBuf; }

inline void ParamTransportManager::setInputBuffer(CyclicOWBuffer<ParamHolder*>* inputBuf)
{ itsInputBuf = inputBuf; }

inline ParamTransport* ParamTransportManager::getSourceTransport()
{ return itsSourceTransport; }

#endif
