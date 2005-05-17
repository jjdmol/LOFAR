//#  BDBListener.cc: Listen to incoming connections from ConHandlers
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

#ifndef BDBLISTENER_H
#define BDBLISTENER_H

#include <Common/Net/Socket.h>
 
#include <BDBReplication/BDBConHandler.h>

using namespace LOFAR;


class BDBListenThread {
 public:
  // called from outside the thread
  BDBListenThread(const int port, 
		  BDBCHThread* ConnectionHandler);
  BDBListenThread(const BDBListenThread& other);

  ~BDBListenThread();

  void stop();

  // this contains the loop of the thread
  void operator()();

 private:
  BDBCHThread& itsConnectionHandler;
  int itsPort;

  // used from outside and within so protected by a mutex
  bool itsShouldStop;
  bool shouldStop();

};

#endif
