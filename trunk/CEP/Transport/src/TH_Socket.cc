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


#include <Transport/TH_Socket.h>
#include <Common/Debug.h>
#include <unistd.h>

// Implementation decision (0417-1): The Socket class from LCS/ACC will be
// used instead of the Socket class from LCS/Common. The reason for this
// choice is that the interface ACC/Socket class is more intuitive for the
// TH_Socket class.

namespace LOFAR
{

//   TH_Socket TH_Socket::proto;
  
  TH_Socket::TH_Socket 
    (const std::string &sendhost, const std::string &recvhost, const int portno) :
      itsSendingHostName (sendhost), 
      itsReceivingHostName (recvhost), 
      itsPortNo (portno),
      isConnected (false) {}

  TH_Socket::~TH_Socket()
  {
  }
  
  TH_Socket* TH_Socket::make() const
  {
      return new TH_Socket(itsSendingHostName, itsReceivingHostName, itsPortNo);
  }
  
  string TH_Socket::getType() const
  {
    return "TH_Socket";
  }
  
  bool TH_Socket::connectionPossible(int srcRank, int dstRank) const
  {
    return srcRank == dstRank;
  }
  
  bool TH_Socket::recvBlocking (void * buf, int nbytes, int /*tag*/)
  { 
    cout << "Creating socket..." << endl;
    Socket socket;
  
    if (! isConnected) {
      ConnectToClient ();
    }

    char *str;
    int32 len;
  
    itsDataSocket.poll (& str, & len, 100000);
  
    // For now, memcpy is used. The ACC/Socket class must be modified to
    // remove this memcpy for performance.
  
    // (Must use memcpy due to Socket design. Can be improved later.)
  
    memcpy (buf, str, nbytes);
  
    return true;
  }
  
  
  bool TH_Socket::recvNonBlocking (void * buf, int nbytes, int tag)
  { 
    cerr << "Warning (TH_Socket::recvNonBlocking ()): Non-blocking receive "
         << "not yet implemented. Calling recvBlocking () in stead." << endl;
  
    recvNonBlocking (buf, nbytes, tag);
    return true;
  }
  
  
  bool TH_Socket::waitForReceived (void * /*buf*/, int /*nbytes*/, int /*tag*/)
  { 
    cerr << "Warning (TH_Socket::waitForReceived ()): Non-blocking receive "
         << "not yet implemented. Recption ready, ingoring call." << endl;
  
    return true;
  }
  
  
  bool TH_Socket::sendBlocking (void* buf, int nbytes, int /*tag*/)
  {
    cout << "Creating a socket..." << endl;
    Socket socket;
  
    if (! isConnected) {
      ConnectToClient ();
    }

    // Socket creation and connection must be moved out of the send method
    // after initial succesful testing.
  
    cout << "Sending..." << endl;
    itsSocket.send ((char *) buf, nbytes);
    
    return true;
  }
  
  
  bool TH_Socket::sendNonBlocking (void* buf, int nbytes, int tag)
  {
    cerr << "Warning (TH_Socket::sendNonBlocking ()): Non-blocking send "
         << "not yet implemented. Calling recvBlocking () in stead." << endl;
  
    sendNonBlocking (buf, nbytes, tag);
  
    return true;
  }
  
  
  bool TH_Socket::waitForSend (void* /*buf*/, int /*nbytes*/, int /*tag*/)
  {
    cerr << "Warning (TH_Socket::waitForSend ()): Non-blocking send "
         << "not yet implemented. Sending is ready anyway. ignoring call." 
         << endl;
  
    return true;
  }
  
  
  bool TH_Socket::waitForReceiveAck (void* /*buf*/, int /*nbytes*/, int /*tag*/)
  {
    cerr << "Warning (TH_Socket::waitForSend ()): Non-blocking send "
         << "not yet implemented. Sending is ready anyway. ignoring call." 
         << endl;
  
    return true;
  }
  

  bool TH_Socket::init () { 
    // Code to open the connection
    return true; 
  }


  // Called by sender
  bool TH_Socket::ConnectToServer (void) {
    // The port number of the connection must be based the tag after
    // succesful testing.
  
    cout << "Connecting..." << endl;
    itsSocket.connect (itsSendingHostName, itsPortNo);
  
    isConnected = true;
    return true;
  }


  // Called by receiver
  bool TH_Socket::ConnectToClient (void) {
    itsSocket.openListener (itsPortNo);
  
    cout << "Waiting to accept..." << endl;
    itsDataSocket = itsSocket.accept ();
  
    isConnected = true;
    return true;
  }
}
