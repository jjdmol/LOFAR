//#  BDBMessage.cc: contains a message that one bdbreplicated site can send to another
//#
//#  Copyright (C) 2005
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <BDBReplication/BDBMessage.h>

namespace LOFAR {
  namespace BDBReplication {

    BDBMessage::BDBMessage(int mtype)
      : itsType(mtype),
	itsCBuffer(0),
	itsRBuffer(0),
	itsPort(0)
    {}

    BDBMessage::~BDBMessage()
    {
      delete [] itsCBuffer;
      delete [] itsRBuffer;
    }
    
    bool BDBMessage::receive(TransportHolder* th){
      // first get messageType
      if (th->recvNonBlocking(&itsType, 4, BDBTAG)) {
	int size = 0;
	char* hostName;
	switch (itsType) {
	case NO_MESSAGE:
	  ASSERTSTR(false,"Received illegal message");
	case CONNECT:	  
	  th->recvBlocking(&itsPort, 4, BDBTAG);
	  th->recvBlocking(&size, 4, BDBTAG);
	  delete hostName;
	  hostName = new char[size + 1];
	  memset(hostName, 0, size + 1);
	  th->recvBlocking(hostName, size, BDBTAG);
	  itsHostName = (string) hostName;
	  delete [] hostName;
	  hostName = 0;
	  break;
	case LIBBDB:
	  delete [] itsCBuffer;
	  delete [] itsRBuffer;
	  th->recvBlocking(&size, 4, BDBTAG);
	  itsCBuffer = new char[size];
	  th->recvBlocking(&itsCBuffer, size, BDBTAG);
	  itsControl.set_data(itsCBuffer);
	  itsControl.set_size(size);
	  th->recvBlocking(&size, 4, BDBTAG);
	  itsRBuffer = new char[size];
	  th->recvBlocking(&itsRBuffer, size, BDBTAG);
	  itsRec.set_data(itsRBuffer);
	  itsRec.set_size(size);
	  break;
	case SYNC_REQUEST:
	  ASSERTSTR(false,"TODO");
	  break;
	case SYNC_REPLY:
	  ASSERTSTR(false,"TODO");
	  break;
	default:
	  ASSERTSTR(false,"Received illegal message");
	}
	return true;
      } else {
	// no message present
	itsType = NO_MESSAGE;
	return false;
      }
    };

    bool BDBMessage::send(TransportHolder* th){
      th->sendBlocking(&itsType, 4, BDBTAG);
      bool succesFull = true;
      int size = 0;
      switch (itsType) {
      case NO_MESSAGE:
	ASSERTSTR(false,"Sending illegal message");
      case CONNECT:
	succesFull &= th->sendBlocking(&itsPort, 4, BDBTAG);
	size = itsHostName.size();
	succesFull &= th->sendBlocking(&size, 4, BDBTAG);
	succesFull &= th->sendBlocking((void*)itsHostName.c_str(), size, BDBTAG);
	break;
      case LIBBDB:
	size = itsControl.get_size();
	succesFull &= th->sendBlocking(&size, 4, BDBTAG);
	succesFull &= th->sendBlocking(&itsCBuffer, size, BDBTAG);
	size = itsRec.get_size();
	succesFull &= th->sendBlocking(&size, 4, BDBTAG);
	succesFull &= th->sendBlocking(&itsRBuffer, size, BDBTAG);
	break;
      case SYNC_REQUEST:
	ASSERTSTR(false,"TODO");
	break;
      case SYNC_REPLY:
	ASSERTSTR(false,"TODO");
	break;
      default:
	ASSERTSTR(false,"Sending illegal message");
      }
      return succesFull;
    }

    // TODO: set and get functions
    // for LIBBDB
    Dbt& BDBMessage::getRec() {
      return itsRec;};
    void BDBMessage::setRec(Dbt& rec){
      delete []itsRBuffer;
      itsRBuffer = new char[rec.get_size()];
      memcpy(itsRBuffer, rec.get_data(), rec.get_size());
      itsRec.set_data(itsRBuffer);
      itsRec.set_size(rec.get_size());
    }
    Dbt& BDBMessage::getControl(){
      return itsControl; };
    void BDBMessage::setControl(Dbt& control) {
      delete []itsCBuffer;
      itsCBuffer = new char[control.get_size()];
      memcpy(itsCBuffer, control.get_data(), control.get_size());
      itsControl.set_data(itsCBuffer);
      itsControl.set_size(control.get_size());
    };      

    // for CONNECT
    int BDBMessage::getPort() { return itsPort; };
    void BDBMessage::setPort(int port) { itsPort = port; };
    string BDBMessage::getHostName() { return itsHostName; };
    void BDBMessage::setHostName(string hostName) { itsHostName = hostName; };


  } // namespace BDBReplication
} // namespace LOFAR
