//#  BDBConnector.cc: Listen to incoming connections from ConHandlers
//#
//#  Copyright (C) 2002-2005
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

#ifndef BDBCONNECTOR_H
#define BDBCONNECTOR_H

#include <Common/Net/Socket.h>
 
#include <BDBReplication/BDBCommunicator.h>

using namespace LOFAR;

class BDBConnectorRep {
 public:
  // called from outside the thread
  BDBConnectorRep(int port,
		  BDBCommunicator* ConnectionHandler);

  ~BDBConnectorRep();

  void stop();  
  bool isListening();
  int getPort();

  // this contains the loop of the thread
  void operator()();
  
  int itsReferences;

 private:
  BDBCommunicator& itsConnectionHandler;
  int itsPort;
  
  // used from outside and within so protected by a mutex
  bool itsShouldStop;
  bool itsIsListening;
  bool shouldStop();
 
  ALLOC_TRACER_ALIAS(BDBSite);
};

inline bool BDBConnectorRep::isListening() {
  return itsIsListening; }

// this class is necessary because boost makes a copy of an object that is used in a thread
// in this way the copy and the original will have the same rep

class BDBConnector {
 public:
  // called from outside the thread
  BDBConnector(const int port, 
	       BDBCommunicator* ConnectionHandler);
  BDBConnector(const BDBConnector& other);

  ~BDBConnector();

  void stop();
  bool isListening();
  int getPort();

  // this contains the loop of the thread
  void operator()();

 private:
  BDBConnectorRep* itsRep;
};

inline bool BDBConnector::isListening() 
{ return itsRep->isListening(); }

inline void BDBConnector::stop() 
{ itsRep->stop();};
  
inline void BDBConnector::operator()()
{ (*itsRep)();};

inline int BDBConnector::getPort() 
{ return itsRep->getPort();}
inline int BDBConnectorRep::getPort() 
{ return itsPort;}

#endif
