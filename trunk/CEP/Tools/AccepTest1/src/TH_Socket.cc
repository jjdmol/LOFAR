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


#include "TH_Socket.h"
#include <Transport/Transporter.h>
#include <Common/Debug.h>
#include <unistd.h>


namespace LOFAR
{

//   TH_Socket TH_Socket::proto;
  
  TH_Socket::TH_Socket 
  (const std::string &sendhost, 
   const std::string &recvhost, 
   const int portno,
   const bool ServerAtSender) :
    itsSendingHostName (sendhost), 
    itsReceivingHostName (recvhost), 
    itsPortNo (portno),
    isConnected (false),
    itsServerAtSender(ServerAtSender)
{
  cout << "Creating a socket...(" << (ServerAtSender ? "SS" : "SC" ) << ")"<< endl;
}

TH_Socket::~TH_Socket()
{
  }
  
  TH_Socket* TH_Socket::make() const
  {
      return new TH_Socket(itsSendingHostName, 
			   itsReceivingHostName, 
			   itsPortNo,
			   itsServerAtSender);
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
    if (! isConnected) {
      if (itsServerAtSender) {
	// Client side behaviour (default)
	ConnectToServer ();
      } else {
	// Server side behaviour
	ConnectToClient ();
      } 
    }// Now we should have a connection
        
    int received_len = itsDataSocket.recv((char *) buf, nbytes);
    //cout << "TH_Socket received " << received_len << "/" <<nbytes<<endl; 
    bool result = received_len == nbytes;
    DbgAssertStr(result,"data not succesfully received")
    return result;
  }
  
  
  bool TH_Socket::recvNonBlocking (void * buf, int nbytes, int tag)
  { 
    cerr << "Warning (TH_Socket::recvNonBlocking ()): Non-blocking receive "
         << "not yet implemented. Calling recvBlocking () in stead." << endl;
  
    recvBlocking (buf, nbytes, tag);
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
    
    if (! isConnected) {
      if (itsServerAtSender) {
	// Server side behaviour
	ConnectToClient ();
      } else {
	ConnectToServer ();
      } 
    }// Now we should have a connection
    int sent_len=0;
    sent_len = itsDataSocket.send ((char *) buf, nbytes);
    //cout << "TH_Socket sent " << sent_len << "/" << nbytes << " bytes" << endl;
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


  // Called by Client
  bool TH_Socket::ConnectToServer (void) {
    // The port number of the connection must be based the tag after
    // succesful testing.
  
    itsDataSocket.connect (itsSendingHostName, itsPortNo);
    // tell the Socket object where to store data (e.g. in recv calls)
    itsDataSocket.setBuffer(getTransporter()->getDataSize(), 
			    (char*)(getTransporter()->getDataPtr()));
    
    isConnected = true;
    return true;
  }


  // Called by Server
   bool TH_Socket::ConnectToClient (void) {
    itsServerSocket.openListener (itsPortNo);
      
      
    cout << "Waiting to accept connection..." << itsPortNo << endl;
      itsDataSocket = itsServerSocket.accept();
      itsDataSocket.setBuffer(getTransporter()->getDataSize(), 
			      (char*)(getTransporter()->getDataPtr()));
      cout << "Accepted connection" << endl;
      isConnected = true;
      return true;
   }
}
