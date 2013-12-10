//#  CodeLoggingProcessor.cc: 
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

// REO:210507: Disables link to KVL, we don't use that at this moment.
//			   We will evaluate this solution when we need it.

#include <lofar_config.h>

#include "CodeLoggingProcessor.h"
#include <log4cplus/helpers/socketbuffer.h>
#include <log4cplus/socketappender.h>
//#include <GCF/LogSys/GCF_KeyValueLogger.h>
#include <GCF/GCF_ServiceInfo.h>
#include <GCF/GCF_PVString.h>
#include <GCF/GCF_PVChar.h>
#include <GCF/Utils.h>
#include <Common/StringUtil.h>

using namespace log4cplus;
using namespace log4cplus::helpers;
namespace LOFAR 
{
 namespace GCF 
 {
using namespace Common;
using namespace TM;
  namespace LogSys 
  {

CodeLoggingProcessor::CodeLoggingProcessor() :
  GCFTask((State)&CodeLoggingProcessor::initial, "GCF-CLPD")
{
  // initialize the port
  _clpPortProvider.init(*this, "listener", GCFPortInterface::MSPP, 0, true);
	_clpPortProvider.setPortNumber(MAC_CODELOGGING_PORT);
}

GCFEvent::TResult CodeLoggingProcessor::initial(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  switch (e.signal) {
    case F_INIT:
      break;

    case F_ENTRY:
    case F_TIMER:
      if (!_clpPortProvider.isConnected()) {
        _clpPortProvider.open();
      }
      break;

    case F_CONNECTED:
      TRAN(CodeLoggingProcessor::operational);
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

GCFEvent::TResult CodeLoggingProcessor::operational(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  static unsigned long garbageTimerID = 0;

  switch (e.signal) {
    case F_DISCONNECTED:
      DBGFAILWHEN(&_clpPortProvider == &p && "CLP port provider may not be disconnected."); 
      LOG_INFO("Connection lost to a LofarLogger client.");
      p.close();
      break;
      
    case F_TIMER: {
      // cleanup the garbage of closed ports to master clients
      GCFPortInterface* pPort;
      for (TClients::iterator iter = _clientsGarbage.begin();
           iter != _clientsGarbage.end(); ++iter) {
        pPort = *iter;
        delete pPort;
      }
      _clientsGarbage.clear();
      break;
    }  
    case F_CLOSED:
      DBGFAILWHEN(&_clpPortProvider == &p);
      _clients.erase(&p);
      _clientsGarbage.push_back(&p);
      break;
      
    case F_CONNECTED:
      DBGFAILWHEN(&_clpPortProvider == &p);
      break;
      
    case F_ACCEPT_REQ: {
      LOG_INFO("New LofarLogger client accepted!");
      GCFTCPPort* pNewCLPPort = new GCFTCPPort();
      ASSERT(pNewCLPPort);
      pNewCLPPort->init(*this, "clp-client", GCFPortInterface::SPP, 0, true);
      _clpPortProvider.accept(*pNewCLPPort);
      break;
    }
    case F_ENTRY:
      garbageTimerID = _clpPortProvider.setTimer(1.0, 5.0); 
      break;

    case F_DATAIN: {
      // extract the incomming data to a Logger event object
      SocketBuffer msgSizeBuffer(sizeof(unsigned int));
      if (!readFromPortData(p, msgSizeBuffer)) {
		break;
	  }

      unsigned int msgSize = msgSizeBuffer.readInt();

      SocketBuffer buffer(msgSize);
      if (!readFromPortData(p, buffer)) {
		break;
	  }
      
      spi::InternalLoggingEvent event = readFromBuffer(buffer);           

      // prepare conversions
      TLoggerClients::iterator iter = _clients.find(&p);
      vector<string> key;
      if (iter == _clients.end()) {
        bool startSequenceFound(false);
        if (event.getLogLevel() == INFO_LOG_LEVEL) {
          string scope = event.getMessage();
          
          if (scope.find("MACProcessScope:") == 0) {
            scope.erase(0, sizeof("MACProcessScope:"));
            ltrim(scope);
            rtrim(scope);
            if (isValidPropName(scope.c_str())) {
              // context (can) contain '.' (dots) (KeyValue TreeDB notation)
              key.push_back(scope);
              
              // '.' must be converted to the '_' in case of DP names
              string::size_type dotSepPos;
              while ((dotSepPos = scope.find('.')) != string::npos) {
                scope[dotSepPos] = '_';
              } 
              key.push_back(formatString("%s.logMsg", scope.c_str()));
              _clients[&p] = key;
              startSequenceFound = true;
              // this start sequence message not needed to be logged
              break;
            }
          }         
        }
        if (!startSequenceFound) {
          // skip all incomming logger events 
          break;
        }
      }
      else {
        key = iter->second;
      }
      
      string file = event.getFile();
      string::size_type sepPos = file.rfind("/"); 
      if (sepPos != string::npos) {
        file.erase(0, sepPos + 1);
      }

      string msg(formatString("%s|%s|%s|%s:%d",
          getLogLevelManager().toString(event.getLogLevel()).c_str(),
          event.getLoggerName().c_str(),
          event.getMessage().c_str(),
          file.c_str(),
          event.getLine()));
       
      // convert the logger event to the key value
      GCFPVString value(msg);
      LOG_DEBUG(formatString("Msg: %s", msg.c_str()));
                
      // timestamp conversion
      timeval kvlTimestamp;  
      Time l4pTimestamp(event.getTimestamp());
      kvlTimestamp.tv_sec = l4pTimestamp.sec();
      kvlTimestamp.tv_usec = l4pTimestamp.usec();

      // log!
//TODO
//      LOG_KEYVALUE_TS(key[0], value, KVL_ORIGIN_MAC, kvlTimestamp);
//
      
      // convert logger event to DP log msg
      string plMsg = event.getTimestamp().getFormattedTime("%d-%m-%y %H:%M:%S.%q") + "|" + msg;

      GCFPVString plValue(plMsg);
      _propertyProxy.setPropValue(key[1], plValue);      
      break;
    }
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

bool CodeLoggingProcessor::readFromPortData(GCFPortInterface& port, SocketBuffer& buf)
{
  size_t res, read = 0;  
  do { 
    res = port.recv(buf.getBuffer() + read, buf.getMaxSize() - read);
    if ( res <= 0 )
      break;

    read += res;
  } while ( read < buf.getMaxSize() );
  
  return (read == buf.getMaxSize());
}

  } // namespace LogSys
 } // namespace GCF
} // namespace LOFAR
