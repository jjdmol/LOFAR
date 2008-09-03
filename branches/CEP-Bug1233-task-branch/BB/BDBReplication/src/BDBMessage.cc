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
    
    bool BDBMessage::receive(TransportHolder* th, bool blocking){ //moet soms blocking (connect)
      // first get messageType
      int result = 0;
      if (blocking == true) {
	result = th->recvBlocking(&itsType, 4, BDBTAG);
      } else {
	result = th->recvNonBlocking(&itsType, 4, BDBTAG);
      }
      if (result) {
	int size = 0;
	char* hostName = 0;
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
	  LOG_TRACE_FLOW_STR("Received connect message from "<<hostName<<":"<<itsPort);
	  delete [] hostName;
	  hostName = 0;
	  break;
	case LIBBDB:
	  LOG_TRACE_FLOW_STR("Receiving LIBDB message");
	  delete [] itsCBuffer;
	  delete [] itsRBuffer;
	  th->recvBlocking(&size, 4, BDBTAG);
	  itsCBuffer = new char[size];
	  th->recvBlocking(itsCBuffer, size, BDBTAG);
	  itsControl.set_data(itsCBuffer);
	  itsControl.set_size(size);
	  th->recvBlocking(&size, 4, BDBTAG);
	  itsRBuffer = new char[size];
	  if (size>0) {
	    th->recvBlocking(itsRBuffer, size, BDBTAG);
	  };
	  itsRec.set_data(itsRBuffer);
	  itsRec.set_size(size);
	  break;
	case STARTUP_DONE:
	  LOG_TRACE_FLOW("Message STARTUP_DONE received.");
	  break;
	case SYNC_REPLY:
	  LOG_TRACE_FLOW("Message SYNC_REPLY received.");
	  th->recvBlocking(&itsSyncRequestNumber, sizeof(SyncReqType), BDBTAG);
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
	LOG_TRACE_FLOW("Sending connect message");
	succesFull &= th->sendBlocking(&itsPort, 4, BDBTAG);
	size = itsHostName.size();
	succesFull &= th->sendBlocking(&size, 4, BDBTAG);
	succesFull &= th->sendBlocking((void*)itsHostName.c_str(), size, BDBTAG);
	break;
      case LIBBDB:
	LOG_TRACE_FLOW("Sending LIBDB message");
	size = itsControl.get_size();
	succesFull &= th->sendBlocking(&size, 4, BDBTAG);
	succesFull &= th->sendBlocking(itsCBuffer, size, BDBTAG);
	size = itsRec.get_size();
	succesFull &= th->sendBlocking(&size, 4, BDBTAG);
	if (size>0) {
	  succesFull &= th->sendBlocking(itsRBuffer, size, BDBTAG);
	};
	break;
      case STARTUP_DONE:
	LOG_TRACE_FLOW("Message STARTUP_DONE sent.");
	break;
      case SYNC_REPLY:
	LOG_TRACE_FLOW("Message SYNC_REPLY sent.");
	th->sendBlocking(&itsSyncRequestNumber, sizeof(SyncReqType), BDBTAG);
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
    void BDBMessage::setRec(const Dbt& rec){
      delete []itsRBuffer;
      itsRBuffer = new char[rec.get_size()];
      memcpy(itsRBuffer, rec.get_data(), rec.get_size());
      itsRec.set_data(itsRBuffer);
      itsRec.set_size(rec.get_size());
    }
    Dbt& BDBMessage::getControl(){
      return itsControl; };
    void BDBMessage::setControl(const Dbt& control) {
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
