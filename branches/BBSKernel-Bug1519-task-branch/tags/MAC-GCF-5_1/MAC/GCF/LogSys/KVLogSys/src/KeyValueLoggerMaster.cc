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

#include "KeyValueLoggerMaster.h"
#include <GCF/ParameterSet.h>
#include <KVLDefines.h>
#include <sys/time.h>
#include <time.h>

namespace LOFAR 
{
 namespace GCF 
 {
using namespace TM;
  namespace LogSys 
  {

KeyValueLoggerMaster::KeyValueLoggerMaster() :
  GCFTask((State)&KeyValueLoggerMaster::initial, KVL_MASTER_TASK_NAME)
{
  // register the protocol for debugging purposes
  registerProtocol(KVL_PROTOCOL, KVL_PROTOCOL_signalnames);

  // initialize the port
  _kvlMasterPortProvider.init(*this, "server", GCFPortInterface::MSPP, KVL_PROTOCOL);
}

GCFEvent::TResult KeyValueLoggerMaster::initial(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  switch (e.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    case F_TIMER:
      if (!_kvlMasterPortProvider.isConnected())
      {
        _kvlMasterPortProvider.open();
      }
      break;

    case F_CONNECTED:
      TRAN(KeyValueLoggerMaster::operational);
      break;

    case F_DISCONNECTED:
      p.setTimer(1.0); // try again after 1 second
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult KeyValueLoggerMaster::operational(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  static unsigned long garbageTimerID = 0;

  switch (e.signal)
  {
    case F_DISCONNECTED:
      DBGFAILWHEN(&_kvlMasterPortProvider == &p && "Master port provider may not be disconnected."); 
      LOG_INFO("Connection lost to a KeyValue Logger daemon client.");
      p.close();
      break;
      
    case F_TIMER:
      if (&_kvlMasterPortProvider == &p)
      {   
        GCFTimerEvent* pTimer = (GCFTimerEvent*)(&e);      
        if (garbageTimerID == pTimer->id)
        {
          // cleanup the garbage of closed ports to master clients
          GCFPortInterface* pPort;
          for (TClients::iterator iter = _clientsGarbage.begin();
               iter != _clientsGarbage.end(); ++iter)
          {
            pPort = *iter;
            delete pPort;
          }
          _clientsGarbage.clear();
        }
        else
        {
          // erase the deamon client registration after one hour no connection
          for (TRegisteredClients::iterator iter = _clients.begin();
               iter != _clients.end(); ++iter)
          {
            if ((iter->second.pPort == 0) && (iter->second.hourTimerID == pTimer->id))
            {
              LOG_DEBUG(formatString(
                  "Hour timer %d for disconnected daemon %d is elapsed. Deletes the daemon client registration!",
                  iter->second.hourTimerID,
                  iter->first));
              _clients.erase(iter->first);
              break;
            }
          }
        }
      }
      break;
      
    case F_CLOSED:
      DBGFAILWHEN(&_kvlMasterPortProvider == &p);
      for (TRegisteredClients::iterator iter = _clients.begin();
           iter != _clients.end(); ++iter)
      {
        if (iter->second.pPort == &p)
        {
          iter->second.pPort = 0;
          iter->second.hourTimerID = _kvlMasterPortProvider.setTimer(CONNECTION_TIMEOUT);
          LOG_DEBUG(formatString(
              "Hour timer %d for disconnected daemon %d is started.",
              iter->second.hourTimerID,
              iter->first));          
          break;         
        }
      }
      _clientsGarbage.push_back(&p);
      break;
      
    case F_CONNECTED:
      DBGFAILWHEN(&_kvlMasterPortProvider == &p);
      break;
      
    case F_ACCEPT_REQ:
    {
      LOG_INFO("New master client accepted!");
      GCFTCPPort* pNewMCPort = new GCFTCPPort();
      assert(pNewMCPort);
      pNewMCPort->init(*this, "kvlm-client", GCFPortInterface::SPP, KVL_PROTOCOL);
      _kvlMasterPortProvider.accept(*pNewMCPort);      
      break;
    }
    case F_ENTRY:
      garbageTimerID = _kvlMasterPortProvider.setTimer(1.0, 5.0); 
      break;

    case KVL_UPDATES:
    {
      KVLUpdatesEvent inEvent(e);
      TRegisteredClients::iterator iter = _clients.find(inEvent.daemonID);

      LOG_DEBUG(formatString(
          "Daemon %d has new updates",
          inEvent.daemonID));
          
      DBGASSERT(iter != _clients.end());
      
      LOG_DEBUG(formatString(
          "^-Receives update collection with seqnr. %d.",
          inEvent.seqNr, 
          iter->first));
      if (((iter->second.curSeqNr + 1) % 0xFFFF) == inEvent.seqNr)
      {
        KVLAnswerEvent outAnswer;
        outAnswer.seqNr = inEvent.seqNr;      
        p.send(outAnswer);
        LOG_DEBUG(formatString(
            "^-Processing update collection with seqnr. %d.",
            inEvent.seqNr, 
            iter->first));
        iter->second.curSeqNr = inEvent.seqNr;
        
        unsigned char* updatesData = inEvent.updates.buf.getValue();
        uint16 updatesDataLength = inEvent.updates.buf.getLen();
        GCFEvent rawEvent(KVL_UPDATE);
        GCFEvent* fullUpdateEvent(0);
        char* eventBuf(0);

        for (uint16 i = 0; i < inEvent.nrOfUpdates; i++)
        {  
          // expects and reads the length field
          DBGASSERT(updatesDataLength > sizeof(rawEvent.length));
          memcpy(&rawEvent.length, updatesData, sizeof(rawEvent.length));
          updatesDataLength -= sizeof(rawEvent.length);
          updatesData += sizeof(rawEvent.length);

          // expects and reads the payload
          DBGASSERT(rawEvent.length > 0);
          DBGASSERT(rawEvent.length <= updatesDataLength);
          eventBuf = new char[sizeof(rawEvent) + rawEvent.length];
          fullUpdateEvent = (GCFEvent*)eventBuf;
          memcpy(eventBuf, &rawEvent, sizeof(rawEvent));
        
          // read the payload right behind the just memcopied basic event structure
          memcpy(eventBuf + sizeof(rawEvent), updatesData, rawEvent.length);
  
          KVLUpdateEvent updateEvent(*fullUpdateEvent);
          
          LOG_DEBUG(formatString(
              "key: %s, unr: %d, udl: %d",
              updateEvent.key.c_str(),
              i,
              updatesDataLength));
          
          delete [] eventBuf;
          updatesDataLength -= rawEvent.length;
          updatesData += rawEvent.length;
        }
      }
      else
      {
        LOG_DEBUG(formatString(
            "^-Skip update collection with seqnr. %d!",
            inEvent.seqNr, 
            iter->first));
      }
      break;
    }
    case KVL_REGISTER:
    {
      KVLRegisterEvent request(e);
      KVLRegisteredEvent response;
      TRegisteredClients::iterator iter;
      
      if (request.curID == 0)
      {
        TClient client;
        client.pPort = &p;
        client.curSeqNr = 0;
        uint8 newClientID = 0;
        do
        {
          newClientID++;
          iter = _clients.find(newClientID);
        }
        while (iter != _clients.end());
  
        LOG_DEBUG(formatString(
            "A (new) daemon is registered with ID %d",
            newClientID));
        _clients[newClientID] = client;
        response.ID = newClientID;
        response.curSeqNr = 0;
      }  
      else
      {
        LOG_DEBUG(formatString(
            "A daemon is re-registered with ID %d",
            request.curID));
        response.ID = request.curID;
        iter = _clients.find(request.curID);
        if (iter == _clients.end())
        {
          TClient client;
          client.pPort = &p;
          client.curSeqNr = request.firstSeqNr;
          client.hourTimerID = 0;
          _clients[request.curID] = client;
          response.curSeqNr = request.firstSeqNr;
        }
        else
        {
          DBGASSERT(iter->second.pPort == 0);
          iter->second.pPort = &p;
          _kvlMasterPortProvider.cancelTimer(iter->second.hourTimerID);
          LOG_DEBUG(formatString(
              "Cancel hour timer %d for daemon %d (if necessary!).",
              iter->second.hourTimerID,
              request.curID));
          iter->second.hourTimerID = 0;  
          response.curSeqNr = iter->second.curSeqNr;
        }
      }
      p.send(response);
      break;
    }
    case KVL_UNREGISTER:
    {
      KVLUnregisterEvent indication(e);
      LOG_DEBUG(formatString(
            "Daemon %d unregistered.",
            indication.curID));
      _clients.erase(indication.curID);      
      break;
    }
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

  } // namespace LogSys
 } // namespace GCF
} // namespace LOFAR
