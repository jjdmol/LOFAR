//  TH_Socket.cc: POSIX Socket based Transport Holder
//  (C) 2000, 2001
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


#include "CEPFrame/TH_Socket.h"
#include "CEPFrame/StepRep.h"
#include <Common/Debug.h>
#include <unistd.h>

// Implementation decision (0417-1): The Socket class from LCS/ACC will be
// used instead of the Socket class from LCS/Common. The reason for this
// choice is that the interface ACC/Socket class is more intuitive for the
// TH_Socket class.

// The current (0417-4) version of ACC/Socket makes use of
// log4cplus. Log4cplus is not yet fully implemented in the LOFAR build
// tree. Because of this reason, ACC/Socket generates compiler errors. A
// copy of ACC/Socket has been made, with the log4cplus calls
// removed. This version of Socket has been checked in as
// CEPFrame/src/Socket. After log4cplus is implemented correctly, the
// CEPFrame/Socket class must be removed and ACC/Socket be used.

#include <CEPFrame/Socket.h>

namespace LOFAR
{

TH_Socket TH_Socket::proto;

TH_Socket::TH_Socket() 
{
}

TH_Socket::~TH_Socket()
{
}

TH_Socket* TH_Socket::make() const
{
    return new TH_Socket();
}

string TH_Socket::getType() const
{
  return "TH_Socket";
}

bool TH_Socket::connectionPossible(int srcRank, int dstRank) const
{
  return srcRank == dstRank;
}

bool TH_Socket::recv(void* buf, int nbytes, int, int tag)
{ 
  cout << "sockServer started." << endl;

  cout << "Creating socket..." << endl;
  Socket socket;

  // Socket creation and connection must be moved out of the recv method
  // after initial succesful testing.

  // The port number of the connection must be based the tag after
  // succesful testing.

  cout << "Listening on port 4567..." << endl;
  socket.openListener (4567);

  cout << "Waiting to accept..." << endl;
  Socket dataSocket;
  dataSocket = socket.accept ();

  char *str;
  int32 len;

  dataSocket.poll (& str, & len, 100000);

  // For now, memcpy is used. The ACC/Socket class must be modified to
  // remove this memcpy for performance.

  memcpy (buf, str, nbytes);

  // Must use memcpy

  return true;
}


bool TH_Socket::send(void* buf, int nbytes, int, int tag)
{
  cout << "sockClient started..." << endl;

  cout << "Creating a socket..." << endl;
  Socket socket;

  // The port number of the connection must be based the tag after
  // succesful testing.

  cout << "Connecting..." << endl;
  socket.connect ("127.0.0.1", 4567);

  // Socket creation and connection must be moved out of the send method
  // after initial succesful testing.

  cout << "Sending..." << endl;
  socket.send ((char *) buf, nbytes);
  
  return true;
}

void TH_Socket::waitForBroadCast()
{}

void TH_Socket::waitForBroadCast(unsigned long&)
{}


void TH_Socket::sendBroadCast(unsigned long)
{}

int TH_Socket::getCurrentRank()
{
    return -1;
}

int TH_Socket::getNumberOfNodes()
{
    return 1;
}

void TH_Socket::init(int, const char* [])
{}

void TH_Socket::finalize()
{}

void TH_Socket::synchroniseAllProcesses()
{}

}
