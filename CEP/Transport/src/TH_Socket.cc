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
#include <Transport/Transporter.h>
#include <Transport/DataHolder.h>
#include <Common/LofarLogger.h>
#include <unistd.h>


namespace LOFAR
{
  
  TH_Socket::TH_Socket (const std::string& sendhost, 
			const std::string& recvhost, 
			int portno,
			bool serverAtSender)
    : itsSendingHostName   (sendhost), 
      itsReceivingHostName (recvhost), 
      itsPortNo            (portno),
      itsIsConnected       (false),
      itsServerAtSender    (serverAtSender),
      itsServerSocket      (0),
      itsDataSocket        (0)
  {
    LOG_TRACE_FLOW_STR( "TH_Socket constructor. Creating a socket...(" 
			<< (serverAtSender ? "SS" : "SC" ) << ")" );;
  }

  TH_Socket::~TH_Socket()
  {
    LOG_TRACE_FLOW( "TH_Socket destructor" );
    if (itsIsConnected) {
      itsDataSocket->shutdown (true, true);
    }
    delete itsServerSocket;
    delete itsDataSocket;
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
  
  bool TH_Socket::connectionPossible (int srcRank, int dstRank) const
  {
    return srcRank == dstRank;
  }
  
  bool TH_Socket::recvBlocking (void* buf, int nbytes, int)
  { 
    LOG_TRACE_RTTI( "TH_Socket recvBlocking()" );
    if (!itsIsConnected) {
      if (itsServerAtSender) {
	// Client side behaviour (default)
	connectToServer();
      } else {
	// Server side behaviour
	connectToClient();
      } 
    }// Now we should have a connection

    int received_len = itsDataSocket->readblock (buf, nbytes);
    //cout << "TH_Socket received " << received_len << "/" <<nbytes<<endl; 
    bool result = received_len == nbytes;
    DBGASSERTSTR(result,"TH_Socket: data not succesfully received")
    return result;
  }
  
  bool TH_Socket::recvVarBlocking (int tag)
  {
    // Read the blob header.
    DataHolder* target = getTransporter()->getDataHolder();
    void* buf = target->getDataPtr();
    int hdrsz = target->getHeaderSize();
    bool result = recvBlocking (buf, hdrsz, tag);
    if (result) {
      // Extract the length and resize the buffer.
      int size = DataHolder::getDataLength (buf);
      target->resizeBuffer (size);
      buf = target->getDataPtr();
      // Read the remainder.
      result = recvBlocking (static_cast<char*>(buf)+hdrsz, size-hdrsz, tag);
    }
    return result;
  }

  bool TH_Socket::recvNonBlocking(void* buf, int nbytes, int tag)
  {
    LOG_WARN( "TH_Socket::recvNonBlocking() is not implemented. recvBlocking() is used instead." );
    return recvBlocking (buf, nbytes, tag);
  }

  bool TH_Socket::recvVarNonBlocking(int tag)
  {
    LOG_WARN( "TH_Socket::recvVarNonBlocking() is not implemented. recvVarBlocking() is used instead." );  
    return recvVarBlocking (tag);
  }

  bool TH_Socket::waitForReceived(void*, int, int)
  {
    LOG_TRACE_RTTI( "TH_Socket waitForReceived()" );
    return true;
  }

  bool TH_Socket::sendBlocking (void* buf, int nbytes, int /*tag*/)
  {
    LOG_TRACE_RTTI( "TH_Socket sendBlocking()" );
    if (!itsIsConnected) {
      if (itsServerAtSender) {
	// Server side behaviour
	connectToClient();
      } else {
	connectToServer();
      } 
    }// Now we should have a connection
    int sent_len=0;
    sent_len = itsDataSocket->writeblock (buf, nbytes);
    //cout << "TH_Socket sent " << sent_len << "/" << nbytes << " bytes" << endl;
    return true;
  }  
   
  bool TH_Socket::sendNonBlocking(void* buf, int nbytes, int tag)
  {
    LOG_WARN( "TH_Socket::sendNonBlocking() is not implemented. The sendBlocking() method is used instead." );    
    return sendBlocking (buf, nbytes, tag);
  }
  
  bool TH_Socket::waitForSent(void*, int, int)
  {
    LOG_TRACE_RTTI( "TH_Socket waitForSent()" );
    return true;
  }


  bool TH_Socket::init () { 
    LOG_TRACE_RTTI( "TH_Socket init()" );
    // Code to open the connection
    return true; 
  }

  // Called by Client
  bool TH_Socket::connectToServer (void)
  {
    // The port number of the connection must be based the tag after
    // succesful testing.
    std::stringstream str;
    str << itsPortNo;
    itsDataSocket = new Socket ("TH_Socket", itsSendingHostName, str.str());
    itsDataSocket->setBlocking();
    int sts = itsDataSocket->connect();
    int nr = 0;
    while (sts <= 0  &&  nr++ < 1000) {
      // Sleep 1 millisec.
      usleep(1000);
      delete itsDataSocket;
      itsDataSocket = new Socket ("TH_Socket", itsSendingHostName, str.str());
      itsDataSocket->setBlocking();
      sts = itsDataSocket->connect();
    }
    ASSERTSTR (sts > 0, "TH_Socket: could not connect to server");
    itsIsConnected = true;
    return true;
  }

  // Called by Server
  bool TH_Socket::connectToClient (void)
  {
    std::stringstream str;
    str << itsPortNo;
    itsServerSocket = new Socket("TH_Socket", str.str());
    LOG_INFO( "Waiting to accept connection..." );
    itsDataSocket = itsServerSocket->accept();
    while (itsDataSocket == 0) {
      // Sleep 0.1 millisec.
      usleep(100);
      itsDataSocket = itsServerSocket->accept();
    }
    itsDataSocket->setBlocking();
    cout << "Accepted connection" << endl;
    itsIsConnected = true;
    return true;
  }
}
