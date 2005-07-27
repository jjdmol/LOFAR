//#  KeyValueLoggerDaemon.cc: 
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

#include "KeyValueLoggerDaemon.h"
#include <GCF/ParameterSet.h>
#include <KVLDefines.h>
#include <sys/time.h>
#include <time.h>
#include <GCF/PAL/GCF_Answer.h>
#include <GCF/PAL/GCF_PVSSInfo.h>
#include <ManagerIdentifier.hxx>

namespace LOFAR 
{
using TYPES::uint8;
 namespace GCF 
 {
using namespace TM;
using namespace PAL;
using namespace Common;
  namespace LogSys 
  {

KeyValueLoggerDaemon::KeyValueLoggerDaemon() :
  GCFTask((State)&KeyValueLoggerDaemon::initial, KVL_DAEMON_TASK_NAME),
  _nrOfBufferedEvents(0),
  _curEventsBufSize(0),
  _registerID(0),
  _curSeqNr(0),
  _oldestUnanswerdSeqNr(1),
  _propertyLogger(*this)
{
  // register the protocol for debugging purposes
  registerProtocol(KVL_PROTOCOL, KVL_PROTOCOL_signalnames);

  // initialize the port
  _kvlMasterClientPort.init(*this, "client", GCFPortInterface::SAP, KVL_PROTOCOL);
  _kvlDaemonPortProvider.init(*this, "server", GCFPortInterface::MSPP, KVL_PROTOCOL);
}

KeyValueLoggerDaemon::~KeyValueLoggerDaemon()
{
  KVLUnregisterEvent indication;
  indication.curID = _registerID;
  _kvlMasterClientPort.send(indication);
}

GCFEvent::TResult KeyValueLoggerDaemon::initial(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  switch (e.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    case F_TIMER:
      if (!_kvlDaemonPortProvider.isConnected())
      {
        _kvlDaemonPortProvider.open();
      }
      break;

    case F_CONNECTED:
      TRAN(KeyValueLoggerDaemon::operational);
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

GCFEvent::TResult KeyValueLoggerDaemon::operational(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  static long hourTimerID = -1;
  
  switch (e.signal)
  {
    case F_DISCONNECTED:
      DBGFAILWHEN(&_kvlDaemonPortProvider == &p && "Daemon port provider may not be disconnected."); 
      if (&_kvlMasterClientPort == &p)
      {
        LOG_WARN("Connection lost to KeyValue Logger master. Tries a reconnect!!!");
        _kvlMasterClientPort.cancelAllTimers();
        hourTimerID = _kvlMasterClientPort.setTimer(CONNECTION_TIMEOUT);
      }            
      else
      {
        LOG_INFO("Connection lost to KeyValue Logger daemon client.");
      }
      p.close();
      break;
      
    case F_TIMER:
      if (&_kvlDaemonPortProvider == &p)
      {        
        GCFTimerEvent* pTimer = (GCFTimerEvent*)(&e);      
        if (hourTimerID == (long) pTimer->id)
        {
          // about 1 hour no connection:
          // reset register ID, master also will release this number
          // all buffered updates will lost
          _registerID = 0;
          _curSeqNr = 0;
          hourTimerID = -2; // still no connection
        }
        else
        {
          // cleanup the garbage with closed ports to daemon clients
          GCFPortInterface* pPort;
          for (TClients::iterator iter = _clientsGarbage.begin();
               iter != _clientsGarbage.end(); ++iter)
          {
            pPort = *iter;
            delete pPort;
          }
          _clientsGarbage.clear();
        }
      }
      else if (&_kvlMasterClientPort == &p)
      {
        if (!p.isConnected())
        {
          // reconnect to master
          p.open(); 
        }
        else
        {           
          if (_seqList.empty())
          {
            if (_nrOfBufferedEvents > 0)
            {
              // send current collected updates
              sendEventsBuffer();
            }
          }
          else if (_kvlMasterClientPort.isConnected())
          {
            // resend max. 10 not answered messages
            uint8 i = 0;
            KVLEventCollectionEvent* pUpdateEvents;
            TSequenceList::iterator iter;
            // find the first not yet sent events buffer
            for (iter = _seqList.find(_oldestUnanswerdSeqNr);
                 i < MAX_NR_OF_RETRY_MSG; ++iter)
            {
              if (iter == _seqList.end())
              {
                _oldestUnanswerdSeqNr++;
              }
              else
              {
                // found
                break;
              }
              i++;
            }
            for (i = 0; i < MAX_NR_OF_RETRY_MSG; i++)
            {
              if (iter == _seqList.end())
              {
                // because the map is sorted, we have to force jump from end to begin;
                iter = _seqList.begin();
              }
              if (iter != _seqList.end())
              {               
                pUpdateEvents = iter->second;
                if (pUpdateEvents->daemonID == 0)
                {
                  pUpdateEvents->daemonID = _registerID;
                }
                _kvlMasterClientPort.send(*pUpdateEvents);
              }
              else
              {
                break;
              }
              ++iter;
            }
          }
        }
      }
      break;
      
    case F_CLOSED:
      DBGFAILWHEN(&_kvlDaemonPortProvider == &p); 
      if (&_kvlMasterClientPort == &p)
      {
        _kvlMasterClientPort.setTimer(1.0); // try to reconnect again after 1 second;
      }            
      else
      {
        _clientsGarbage.push_back(&p);
        _propertyLogger.clientGone(p);        
      }
      break;
      
    case F_CONNECTED:
      DBGFAILWHEN(&_kvlDaemonPortProvider == &p); 
      if (&_kvlMasterClientPort == &p)
      {
        _kvlDaemonPortProvider.cancelTimer(hourTimerID);
        KVLRegisterEvent request;
        request.curID = _registerID;
        if (_seqList.size() > 0)
        {
          request.firstSeqNr = _oldestUnanswerdSeqNr - 1;
        }
        else
        {
          request.firstSeqNr = _curSeqNr;
        }
        _kvlMasterClientPort.send(request);
        hourTimerID = -1;
      }
      break;
      
    case F_ACCEPT_REQ:
    {
      LOG_INFO("New daemon client accepted!");
      GCFTCPPort* pNewDCPort = new GCFTCPPort();
      assert(pNewDCPort);
      pNewDCPort->init(*this, "kvld-client", GCFPortInterface::SPP, KVL_PROTOCOL);
      _kvlDaemonPortProvider.accept(*pNewDCPort);      
      break;
    }
    case F_ENTRY:
      _kvlDaemonPortProvider.setTimer(1.0, 5.0); // garbage timer
      if (!_kvlMasterClientPort.isConnected())
      {
        _kvlMasterClientPort.open();
      }
      break;

    case KVL_UPDATE:
    case KVL_ADD_ACTION:
    {
      if (hourTimerID == -2) 
      {
        LOG_DEBUG("More than 1 hour no connection with the master, so dump all key value updates.");
        break;
      }
      
      if (_seqList.size() == 0xFFFF)
      {
        LOG_DEBUG("Cannot buffer more events. Dump as long as the buffer not decreases.");        
        break;
      }

      unsigned int neededSize = SIZEOF_EVENT(e);

      if (_curEventsBufSize + neededSize > MAX_EVENTS_BUFF_SIZE)
      {
        sendEventsBuffer();
      }
      memcpy(_eventsBuf + _curEventsBufSize, &e.signal, sizeof(e.signal));
      _curEventsBufSize += sizeof(e.signal);
      memcpy(_eventsBuf + _curEventsBufSize, &e.length, sizeof(e.length));
      _curEventsBufSize += sizeof(e.length);
      char* eBuf = (char*) &e;
      memcpy(_eventsBuf + _curEventsBufSize, eBuf + sizeof(GCFEvent), e.length);
      _curEventsBufSize += e.length;
               
      _nrOfBufferedEvents++;
      if (_nrOfBufferedEvents == MAX_NR_OF_EVENTS)
      {
        sendEventsBuffer();
      }      
      break;
    }
    case KVL_ANSWER:
    {
      KVLAnswerEvent answer(e);
      TSequenceList::iterator iter = _seqList.find(answer.seqNr);
      LOG_DEBUG(formatString(
          "Message with nr. %d was successfully received by the master.",
          answer.seqNr));        

      if (iter != _seqList.end())
      {
        delete iter->second;
      }
      _seqList.erase(answer.seqNr);
      assert(answer.seqNr == _oldestUnanswerdSeqNr);
      _oldestUnanswerdSeqNr++;
      break;
    }
    
    case KVL_REGISTERED:
    {
      KVLRegisteredEvent response(e);
      if (_registerID != response.ID)
      {
        LOG_DEBUG(formatString(
            "Registered on master with nr. %d",
            response.ID));        
      }
      _registerID = response.ID;

      for (uint64 i = 0; i < MAX_NR_OF_RETRY_MSG; i++)
      {
        _seqList.erase(response.curSeqNr - i);
      }
      _kvlMasterClientPort.setTimer(1.0, 1.0); // start the (re)send heartbeat
      break;
    }    
    case KVL_SKIP_UPDATES_FROM:
    {
      KVLSkipUpdatesFromEvent request(e);
      _propertyLogger.skipUpdatesFrom(request.man_id, p);
      break;
    }
    case F_VCHANGEMSG:
    {
      GCFPropValueEvent& pve = (GCFPropValueEvent&) e;
      KVLUpdateEvent ue;
      ue.key = pve.pPropName;
      ue.value._pValue = pve.pValue;
      ue.origin = (GCFPVSSInfo::getLastEventManType() == API_MAN ? 
                   KVL_ORIGIN_MAC : KVL_ORIGIN_OPERATOR);
      ue.timestamp = GCFPVSSInfo::getLastEventTimestamp();
      ue.description = "";

      unsigned int neededSize;
      void* buf = ue.pack(neededSize);
      
      if (_curEventsBufSize + neededSize > MAX_EVENTS_BUFF_SIZE)
      {
        sendEventsBuffer();
      }

      memcpy(_eventsBuf + _curEventsBufSize, buf, neededSize);
      _curEventsBufSize += neededSize;

      _nrOfBufferedEvents++;
      if (_nrOfBufferedEvents == MAX_NR_OF_EVENTS)
      {
        sendEventsBuffer();
      }      
      break;
    }
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

void KeyValueLoggerDaemon::sendEventsBuffer()
{
  _curSeqNr++;

  LOG_DEBUG(formatString(
      "Message with nr. %d is prepared to send.",
      _curSeqNr));        

  KVLEventCollectionEvent* pCollectionEvent = new KVLEventCollectionEvent;
  pCollectionEvent->seqNr = _curSeqNr;
  pCollectionEvent->daemonID = _registerID;
  pCollectionEvent->nrOfEvents = _nrOfBufferedEvents;
  pCollectionEvent->events.buf.setValue(_eventsBuf, _curEventsBufSize, true);

  if (_kvlMasterClientPort.isConnected())
  {
    _kvlMasterClientPort.send(*pCollectionEvent);
  }
  _seqList[_curSeqNr] = pCollectionEvent;
  _curEventsBufSize = 0;
  _nrOfBufferedEvents = 0;        
}

  } // namespace LogSys
 } // namespace GCF
} // namespace LOFAR
