//#  KeyValueLoggerMaster.cc: 
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

#include <lofar_config.h>

#include "KeyValueLoggerMaster.h"
#include <Common/ParameterSet.h>
#include <KVLDefines.h>
#include <sys/time.h>
#include <time.h>
#include <GCF/GCF_ServiceInfo.h>
#include <GCF/TM/GCF_Protocols.h>
#include <OTDB/TreeValue.h>
#include <OTDB/TreeTypeConv.h>
#include <OTDB/TreeStateConv.h>
#include <OTDB/ClassifConv.h>
#include <Common/lofar_datetime.h>

namespace LOFAR {
	using namespace OTDB;  
	namespace GCF {
		using namespace TM;
		namespace LogSys {

//
// KeyValueLoggerMaster()
//
KeyValueLoggerMaster::KeyValueLoggerMaster() :
	GCFTask((State)&KeyValueLoggerMaster::initial, KVL_MASTER_TASK_NAME),
	itsOTDBconn(0),
	itsTreeValue(0)
{
	// register the protocol for debugging purposes
	TM::registerProtocol(KVL_PROTOCOL, KVL_PROTOCOL_STRINGS);

	// initialize the port
	itsListener.init(*this, MAC_SVCMASK_KVLMASTER, GCFPortInterface::MSPP, KVL_PROTOCOL);
}

//
// ~KeyValueLoggerMaster()
//
KeyValueLoggerMaster::~KeyValueLoggerMaster()
{
	if (itsTreeValue) {
		delete itsTreeValue;
	}
	if (itsOTDBconn) {
		delete itsOTDBconn;
	}
}

//
// initial(event,port)
//
GCFEvent::TResult KeyValueLoggerMaster::initial(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR("initial:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
	switch (event.signal) {
	case F_INIT: {
		LOG_DEBUG ("Trying to connect to the OTDB");
		string	username(globalParameterSet()->getString("OTDBusername"));
		string	password(globalParameterSet()->getString("OTDBpassword"));
		string	DBname  (globalParameterSet()->getString("OTDBdatabasename"));
		itsOTDBconn = new OTDBconnection(username, password, DBname);
		ASSERTSTR (itsOTDBconn, "Memory allocation error (OTDB)");
		ASSERTSTR (itsOTDBconn->connect(),
					"Unable to connect to database " << DBname << " using " <<
					username << "," << password);
		LOG_INFO ("Connected to the OTDB");

		// load converter objects
		TreeTypeConv  TTconv(itsOTDBconn);
		TreeStateConv TSconv(itsOTDBconn);
		ClassifConv   CTconv(itsOTDBconn);      

		// get ID of latest hierarchicla tree.
		// NOTE: any ID will do since OTDB does not look yet after the treeID!!!!
		vector<OTDBtree>  treeList = itsOTDBconn->getTreeList(TTconv.get("VHtree"), 
															  CTconv.get("operational"));
		treeIDType  treeID = treeList[treeList.size()-1].treeID();
		itsTreeValue = new TreeValue(itsOTDBconn, treeID);
		break;
	}

	case F_ENTRY:
	case F_TIMER:
		if (!itsListener.isConnected()) {
			itsListener.open();
		}
		break;

	case F_CONNECTED:
		TRAN(KeyValueLoggerMaster::operational);
		break;

	case F_DISCONNECTED:
		port.setTimer(TO_TRY_RECONNECT); // try again after 1 second
		break;

	default:
		LOG_DEBUG ("initial:default");
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return (status);
}

//
// operational
//
GCFEvent::TResult KeyValueLoggerMaster::operational(GCFEvent& 			event, 
													GCFPortInterface&	port)
{
	LOG_DEBUG_STR("operational:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
	static unsigned long garbageTimerID = 0;

	switch (event.signal) {
	case F_DISCONNECTED:
		ASSERTSTR(&port != &itsListener, 
					"Received disconnect on listener port, bailing out!"); 

		LOG_INFO("Connection lost to a KeyValue Logger daemon client.");
		port.close();
		break;

	case F_TIMER: {
		if (&port != &itsListener) {
			break;
		}   

		GCFTimerEvent* pTimer = (GCFTimerEvent*)(&event);      
		if (garbageTimerID == pTimer->id) {
			// cleanup the garbage of closed ports to master clients
			GCFPortInterface* pPort;
			for (TClients::iterator iter = _clientsGarbage.begin();
									iter != _clientsGarbage.end(); ++iter) {
				pPort = *iter;
				delete pPort;
			}
			_clientsGarbage.clear();
		}
		else {
			// erase the deamon client registration after one hour no connection
			for (TRegisteredClients::iterator iter = _clients.begin();
											  iter != _clients.end(); ++iter) {
				if ((iter->second.pPort==0) && (iter->second.hourTimerID == pTimer->id)) {
					LOG_DEBUG(formatString(
						"Hour timer %d for disconnected daemon %d is elapsed. Deletes the daemon client registration!",
							iter->second.hourTimerID,
							iter->first));
					_clients.erase(iter->first);
					break;
				}
			}
		}
		break;
	}

	case F_CLOSED:
		DBGFAILWHEN(&itsListener == &port);
		for (TRegisteredClients::iterator iter = _clients.begin();
									  iter != _clients.end(); ++iter) {
			if (iter->second.pPort == &port) {
				iter->second.pPort 		 = 0;
				iter->second.hourTimerID = itsListener.setTimer(TO_DISCONNECTED);
				LOG_DEBUG(formatString(
					"Hour timer %d for disconnected daemon %d is started.",
					iter->second.hourTimerID, iter->first));          
				break;         
			}
		}
		_clientsGarbage.push_back(&port);
		break;

	case F_CONNECTED:
		DBGFAILWHEN(&itsListener == &port);
		break;

	case F_ACCEPT_REQ: {
		LOG_INFO("New master client accepted!");
		GCFTCPPort* pNewMCPort = new GCFTCPPort();
		ASSERT(pNewMCPort);
		pNewMCPort->init(*this, "kvlm-client", GCFPortInterface::SPP, KVL_PROTOCOL);
		itsListener.accept(*pNewMCPort);      
		break;
	}

	case F_ENTRY:
		garbageTimerID = itsListener.setTimer(1.0, 5.0); 
		break;

	case KVL_EVENT_COLLECTION: {
		KVLEventCollectionEvent inEvent(event);
		TRegisteredClients::iterator iter = _clients.find(inEvent.daemonID);

		LOG_DEBUG(formatString( "Daemon %d has new updates", inEvent.daemonID));

		DBGASSERT(iter != _clients.end());

		LOG_DEBUG(formatString("^-Receives event collection with seqnr. %ld.",
								inEvent.seqNr, iter->first));
		if (((iter->second.curSeqNr + 1) % 0xFFFF) == inEvent.seqNr) {
			KVLAnswerEvent outAnswer;
			outAnswer.seqNr = inEvent.seqNr;      
			port.send(outAnswer);
			LOG_DEBUG(formatString("^-Processing event collection with seqnr. %ld.",
									inEvent.seqNr, iter->first));
			iter->second.curSeqNr = inEvent.seqNr;

			unsigned char* eventsData = inEvent.events.buf.getValue();
			uint16 eventsDataLength   = inEvent.events.buf.getLen();
			GCFEvent rawEvent(KVL_UPDATE); // KVL_UPDATE == default
			GCFEvent* fullEvent(0);
			char* eventBuf(0);

			ptime otdbTime(not_a_date_time);
			for (uint16 i = 0; i < inEvent.nrOfEvents; i++) {  
				// expects and reads the length field
				DBGASSERT(eventsDataLength > sizeof(rawEvent.length));
				memcpy(&rawEvent.signal, eventsData, sizeof(rawEvent.signal));
				memcpy(&rawEvent.length, eventsData + sizeof(rawEvent.signal), sizeof(rawEvent.length));
				eventsDataLength -= (sizeof(rawEvent.length) + sizeof(rawEvent.signal));
				eventsData 		 += sizeof(rawEvent.length) + sizeof(rawEvent.signal);

				// expects and reads the payload
				DBGASSERT(rawEvent.length > 0);
				DBGASSERT(rawEvent.length <= eventsDataLength);
				eventBuf  = new char[sizeof(rawEvent) + rawEvent.length];
				fullEvent = (GCFEvent*)eventBuf;
				memcpy(eventBuf, &rawEvent, sizeof(rawEvent));

				// read the payload right behind the just memcopied basic event structure
				memcpy(eventBuf + sizeof(rawEvent), eventsData, rawEvent.length);

				switch (rawEvent.signal) {
				case KVL_UPDATE: {
					KVLUpdateEvent updateEvent(*fullEvent);

					LOG_DEBUG(formatString("key: %s, unr: %d, udl: %d",
									updateEvent.key.c_str(), i, eventsDataLength));

					otdbTime = from_time_t((time_t) updateEvent.timestamp.tv_sec) + 
										microseconds(updateEvent.timestamp.tv_usec);
					if (!itsTreeValue->addKVT(updateEvent.key, updateEvent.value._pValue->getValueAsString(), otdbTime)) {
						LOG_INFO("Could NOT add the key, key unknown?");
					}                  
					break;
				}
				case KVL_ADD_ACTION: {
					KVLAddActionEvent addActionEvent(*fullEvent);

					LOG_DEBUG(formatString("key: %s, unr: %d, udl: %d",
									addActionEvent.key.c_str(), i, eventsDataLength));
					break;
				} 
				}

				delete [] eventBuf;
				eventsDataLength -= rawEvent.length;
				eventsData 		 += rawEvent.length;
			}
		}
		else {
			LOG_DEBUG(formatString("^-Skip event collection with seqnr. %ld!",
													inEvent.seqNr, iter->first));
		}
		break;
	}

	case KVL_REGISTER: {
		KVLRegisterEvent request(event);
		KVLRegisteredEvent response;
		TRegisteredClients::iterator iter;

		if (request.curID == 0) {
			TClient client;
			client.pPort 	  = &port;
			client.curSeqNr   = request.firstSeqNr;
			uint8 newClientID = 0;
			do {
				newClientID++;
				iter = _clients.find(newClientID);
			} while (iter != _clients.end()); 
			LOG_DEBUG(formatString("A (new) daemon is registered with ID %d",
																	newClientID));
			_clients[newClientID] = client;
			response.ID 		  = newClientID;
			response.curSeqNr 	  = client.curSeqNr;
		}  
		else {
			LOG_DEBUG(formatString("A daemon is re-registered with ID %d",
																	request.curID));
			response.ID = request.curID;
			iter = _clients.find(request.curID);
			if (iter == _clients.end()) {
				TClient client;
				client.pPort 			= &port;
				client.curSeqNr 		= request.firstSeqNr;
				client.hourTimerID 		= 0;
				_clients[request.curID] = client;
				response.curSeqNr 		= request.firstSeqNr;
			}
			else {
				DBGASSERT(iter->second.pPort == 0);
				iter->second.pPort = &port;
				itsListener.cancelTimer(iter->second.hourTimerID);
				LOG_DEBUG(formatString("Cancel hour timer %d for daemon %d (if necessary!).",
											iter->second.hourTimerID, request.curID));
				iter->second.hourTimerID = 0;  
				response.curSeqNr 		 = iter->second.curSeqNr;
			}
		}
		port.send(response);
		break;
	}

	case KVL_UNREGISTER: {
		KVLUnregisterEvent indication(event);
		LOG_DEBUG(formatString("Daemon %d unregistered.", indication.curID));
		_clients.erase(indication.curID);      
		break;
	}

	default:
		LOG_DEBUG("oprational:default");
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return (status);
}

  } // namespace LogSys
 } // namespace GCF
} // namespace LOFAR
