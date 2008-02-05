//
//  THEcho.cc: Implementation of the Echo task class.
//
//  Copyright (C) 2003
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

#include <stdio.h>
#include <Common/LofarLogger.h>
#include <GCF/CmdLine.h>
#include "THEcho.h"

#define DECLARE_SIGNAL_NAMES
#include "THEcho_Protocol.ph"
#include "THEchoRouting_Protocol.ph"

using std::cout;
using std::endl;
using namespace LOFAR;
using namespace LOFAR::GCF::Common;
using namespace THEcho;

Echo::Echo(string name) : 
  GCFTask((State)&Echo::initial, name),
  _server(),
  _routingServer(),
  _client()
{
  LOG_TRACE_FLOW("Echo::Echo");
  // register the protocol for debugging purposes
  registerProtocol(THECHO_PROTOCOL, THECHO_PROTOCOL_signalnames);
  registerProtocol(THECHOROUTING_PROTOCOL, THECHOROUTING_PROTOCOL_signalnames);

  // initialize the port
  _server.init(*this, "server", GCFPortInterface::SPP, THECHO_PROTOCOL);
  _routingServer.init(*this, "routingServer", GCFPortInterface::SPP, THECHOROUTING_PROTOCOL);
  _client.init(*this, "client", GCFPortInterface::SAP, THECHOROUTING_PROTOCOL);
}

Echo::~Echo()
{
  LOG_TRACE_FLOW("Echo::~Echo");
}

bool Echo::_isServer(GCFPortInterface& p) const
{
  return (&_server == &p);
}

bool Echo::_isRoutingServer(GCFPortInterface& p) const
{
  return (&_routingServer == &p);
}

bool Echo::_isClient(GCFPortInterface& p) const
{
  return (&_client == &p);
}

void Echo::_reply(GCFEvent& e, GCFPortInterface& p)
{
  if(_isServer(p))
  {
    _server.send(e);
    LOG_INFO(formatString("ECHO sent on port %s",_server.getName().c_str()));
  }
  else if(_isRoutingServer(p))
  {
    _client.send(e);
    LOG_INFO(formatString("ECHO sent on port %s",_client.getName().c_str()));
  }
}

GCFEvent::TResult Echo::initial(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_INIT:
      LOG_TRACE_FLOW("Echo::initial(F_INIT)");
      break;

    case F_ENTRY:
      LOG_TRACE_FLOW("Echo::initial(F_ENTRY)");
      _server.open();
      _routingServer.open();
      _client.open();
      break;

    case F_CONNECTED:
      LOG_TRACE_FLOW("Echo::initial(F_CONNECTED)");
      TRAN(Echo::connected);
      break;

    case F_DISCONNECTED:
      LOG_TRACE_FLOW("Echo::initial(F_DISCONNECTED)");
      p.setTimer(5.0); // try again after 5 seconds
      break;

    case F_TIMER:
      LOG_TRACE_FLOW("Echo::initial(F_TIMER)");
      p.open(); // try again
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult Echo::connected(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {

    case F_DISCONNECTED:
      LOG_TRACE_FLOW("Echo::connected(F_DISCONNECTED)");
      p.close();
      p.setTimer(5.0); // try again after 5 seconds
      break;

    case F_TIMER:
      LOG_TRACE_FLOW("Echo::connected(F_TIMER)");
      p.open(); // try again
      break;

    case THECHO_PING_UINT:
    {
      LOG_TRACE_FLOW("Echo::connected(THECHO_PING_UINT)");
      THEchoPingUintEvent ping(e);
      LOG_INFO(formatString("PING_UINT received on port %s (seqnr=%d, uintParam=%d)",p.getName().c_str(),ping.seqnr,ping.uintParam));
      THEchoEchoUintEvent echo;
      echo.seqnr = ping.seqnr;
      echo.uintParam = ping.uintParam;
      _reply(echo,p);
      break;
    }

    case THECHO_PING_INT:
    {
      LOG_TRACE_FLOW("Echo::connected(THECHO_PING_INT)");
      THEchoPingIntEvent ping(e);
      LOG_INFO(formatString("PING_INT received on port %s (seqnr=%d, intParam=%d)",p.getName().c_str(),ping.seqnr,ping.intParam));
      THEchoEchoIntEvent echo;
      echo.seqnr = ping.seqnr;
      echo.intParam = ping.intParam;
      _reply(echo,p);
      break;
    }

    case THECHO_PING_LONG:
    {
      LOG_TRACE_FLOW("Echo::connected(THECHO_PING_LONG)");
      THEchoPingLongEvent ping(e);
      LOG_INFO(formatString("PING_LONG received on port %s (seqnr=%d, longParam=%d)",p.getName().c_str(),ping.seqnr,ping.longParam));
      THEchoEchoLongEvent echo;
      echo.seqnr = ping.seqnr;
      echo.longParam = ping.longParam;
      _reply(echo,p);
      break;
    }

    case THECHO_PING_ENUM:
    {
      LOG_TRACE_FLOW("Echo::connected(THECHO_PING_ENUM)");
      THEchoPingEnumEvent ping(e);
      LOG_INFO(formatString("PING_ENUM received on port %s (seqnr=%d, enumParam=%d)",p.getName().c_str(),ping.seqnr,ping.enumParam));
      THEchoEchoEnumEvent echo;
      echo.seqnr = ping.seqnr;
      echo.enumParam = ping.enumParam;
      _reply(echo,p);
      break;
    }

    case THECHO_PING_DOUBLE:
    {
      LOG_TRACE_FLOW("Echo::connected(THECHO_PING_DOUBLE)");
      THEchoPingDoubleEvent ping(e);
      LOG_INFO(formatString("PING_DOUBLE received on port %s (seqnr=%d, doubleParam=%f)",p.getName().c_str(),ping.seqnr,ping.doubleParam));
      THEchoEchoDoubleEvent echo;
      echo.seqnr = ping.seqnr;
      echo.doubleParam = ping.doubleParam;
      _reply(echo,p);
      break;
    }

    case THECHO_PING_STRING:
    {
      LOG_TRACE_FLOW("Echo::connected(THECHO_PING_STRING)");
      THEchoPingStringEvent ping(e);
      LOG_INFO(formatString("PING_STRING received on port %s (seqnr=%d, stringParam=%s)",p.getName().c_str(),ping.seqnr,ping.stringParam.c_str()));
      THEchoEchoStringEvent echo;
      echo.seqnr = ping.seqnr;
      echo.stringParam = ping.stringParam;
      _reply(echo,p);
      break;
    }

    case THECHO_PING_STOP:
    {
      LOG_TRACE_FLOW("Echo::connected(THECHO_PING_STOP)");
      THEchoPingStopEvent ping(e);
      LOG_INFO(formatString("PING_STOP received on port %s (seqnr=%d)",p.getName().c_str(),ping.seqnr));
      stop();      
      break;
    }

    case THECHO_PING_INT_ARRAY:
    {
      LOG_TRACE_FLOW("Echo::connected(THECHO_PING_INT_ARRAY)");
      THEchoPingIntArrayEvent ping(e);
      LOG_INFO(formatString("PING_INT_ARRAY received on port %s (seqnr=%d, intArrayParam=[%d,%d,%d,%d,%d,%d,%d,%d,%d,%d])",p.getName().c_str(),ping.seqnr,ping.intArrayParam[0],ping.intArrayParam[1],ping.intArrayParam[2],ping.intArrayParam[3],ping.intArrayParam[4],ping.intArrayParam[5],ping.intArrayParam[6],ping.intArrayParam[7],ping.intArrayParam[8],ping.intArrayParam[9]));
      THEchoEchoIntArrayEvent echo;
      echo.seqnr = ping.seqnr;
      for(int i=0;i<10;i++)
        echo.intArrayParam[i] = ping.intArrayParam[i];
      _reply(echo,p);
      break;
    }

    case THECHO_PING_INT_ARRAY_20:
    {
      LOG_TRACE_FLOW("Echo::connected(THECHO_PING_INT_ARRAY_20)");
      THEchoPingIntArray20Event ping(e);

      int numInts=20;
      int i;
      char strTemp[50];
      string paramDump("[");
      for(i=0;i<numInts-1;i++)
      {
        sprintf(strTemp,"%d,",ping.intArrayParam[i]);
        paramDump += string(strTemp);
      }
      sprintf(strTemp,"%d]",ping.intArrayParam[i]);
      paramDump += string(strTemp);
      LOG_INFO(formatString("PING_INT_ARRAY_20 received on port %s (seqnr=%d, intArrayParam=%s)",p.getName().c_str(),ping.seqnr,paramDump.c_str()));

      THEchoEchoIntArray20Event echo;
      echo.seqnr = ping.seqnr;
      for(i=0;i<numInts;i++)
        echo.intArrayParam[i] = ping.intArrayParam[i];
      _reply(echo,p);
      break;
    }

    case THECHO_PING_INT_ARRAY_61:
    {
      LOG_TRACE_FLOW("Echo::connected(THECHO_PING_INT_ARRAY_61)");
      THEchoPingIntArray61Event ping(e);

      int numInts=61;
      int i;
      char strTemp[50];
      string paramDump("[");
      for(i=0;i<numInts-1;i++)
      {
        sprintf(strTemp,"%d,",ping.intArrayParam[i]);
        paramDump += string(strTemp);
      }
      sprintf(strTemp,"%d]",ping.intArrayParam[i]);
      paramDump += string(strTemp);
      LOG_INFO(formatString("PING_INT_ARRAY_61 received on port %s (seqnr=%d, intArrayParam=%s)",p.getName().c_str(),ping.seqnr,paramDump.c_str()));

      THEchoEchoIntArray61Event echo;
      echo.seqnr = ping.seqnr;
      for(i=0;i<numInts;i++)
        echo.intArrayParam[i] = ping.intArrayParam[i];
      _reply(echo,p);
      break;
    }

    case THECHOROUTING_PING_UINT:
    {
      LOG_TRACE_FLOW("Echo::connected(THECHOROUTING_PING_UINT)");
      THEchoRoutingPingUintEvent ping(e);
      LOG_INFO(formatString("PING_UINT received on port %s (seqnr=%d, uintParam=%d)",p.getName().c_str(),ping.seqnr,ping.uintParam));
      THEchoRoutingEchoUintEvent echo;
      echo.seqnr = ping.seqnr;
      echo.uintParam = ping.uintParam;
      _reply(echo,p);
      break;
    }

    case THECHOROUTING_PING_INT:
    {
      LOG_TRACE_FLOW("Echo::connected(THECHOROUTING_PING_INT)");
      THEchoRoutingPingIntEvent ping(e);
      LOG_INFO(formatString("PING_INT received on port %s (seqnr=%d, intParam=%d)",p.getName().c_str(),ping.seqnr,ping.intParam));
      THEchoRoutingEchoIntEvent echo;
      echo.seqnr = ping.seqnr;
      echo.intParam = ping.intParam;
      _reply(echo,p);
      break;
    }

    case THECHOROUTING_PING_LONG:
    {
      LOG_TRACE_FLOW("Echo::connected(THECHOROUTING_PING_LONG)");
      THEchoRoutingPingLongEvent ping(e);
      LOG_INFO(formatString("PING_LONG received on port %s (seqnr=%d, longParam=%d)",p.getName().c_str(),ping.seqnr,ping.longParam));
      THEchoRoutingEchoLongEvent echo;
      echo.seqnr = ping.seqnr;
      echo.longParam = ping.longParam;
      _reply(echo,p);
      break;
    }

    case THECHOROUTING_PING_ENUM:
    {
      LOG_TRACE_FLOW("Echo::connected(THECHOROUTING_PING_ENUM)");
      THEchoRoutingPingEnumEvent ping(e);
      LOG_INFO(formatString("PING_ENUM received on port %s (seqnr=%d, enumParam=%d)",p.getName().c_str(),ping.seqnr,ping.enumParam));
      THEchoRoutingEchoEnumEvent echo;
      echo.seqnr = ping.seqnr;
      echo.enumParam = ping.enumParam;
      _reply(echo,p);
      break;
    }

    case THECHOROUTING_PING_DOUBLE:
    {
      LOG_TRACE_FLOW("Echo::connected(THECHOROUTING_PING_DOUBLE)");
      THEchoRoutingPingDoubleEvent ping(e);
      LOG_INFO(formatString("PING_DOUBLE received on port %s (seqnr=%d, doubleParam=%f)",p.getName().c_str(),ping.seqnr,ping.doubleParam));
      THEchoRoutingEchoDoubleEvent echo;
      echo.seqnr = ping.seqnr;
      echo.doubleParam = ping.doubleParam;
      _reply(echo,p);
      break;
    }

    case THECHOROUTING_PING_STRING:
    {
      LOG_TRACE_FLOW("Echo::connected(THECHOROUTING_PING_STRING)");
      THEchoRoutingPingStringEvent ping(e);
      LOG_INFO(formatString("PING_STRING received on port %s (seqnr=%d, stringParam=%s)",p.getName().c_str(),ping.seqnr,ping.stringParam.c_str()));
      THEchoRoutingEchoStringEvent echo;
      echo.seqnr = ping.seqnr;
      echo.stringParam = ping.stringParam;
      _reply(echo,p);
      break;
    }

    case THECHOROUTING_PING_STOP:
    {
      LOG_TRACE_FLOW("Echo::connected(THECHOROUTING_PING_STOP)");
      THEchoRoutingPingStopEvent ping(e);
      LOG_INFO(formatString("PING_STOP received on port %s (seqnr=%d)",p.getName().c_str(),ping.seqnr));
      stop();      
      break;
    }

    case THECHOROUTING_PING_INT_ARRAY:
    {
      LOG_TRACE_FLOW("Echo::connected(THECHOROUTING_PING_INT_ARRAY)");
      THEchoRoutingPingIntArrayEvent ping(e);
      LOG_INFO(formatString("PING_INT_ARRAY received on port %s (seqnr=%d, intArrayParam=[%d,%d,%d,%d,%d,%d,%d,%d,%d,%d])",p.getName().c_str(),ping.seqnr,ping.intArrayParam[0],ping.intArrayParam[1],ping.intArrayParam[2],ping.intArrayParam[3],ping.intArrayParam[4],ping.intArrayParam[5],ping.intArrayParam[6],ping.intArrayParam[7],ping.intArrayParam[8],ping.intArrayParam[9]));
      THEchoRoutingEchoIntArrayEvent echo;
      echo.seqnr = ping.seqnr;
      for(int i=0;i<10;i++)
        echo.intArrayParam[i] = ping.intArrayParam[i];
      _reply(echo,p);
      break;
    }

    case THECHOROUTING_PING_INT_ARRAY_20:
    {
      LOG_TRACE_FLOW("Echo::connected(THECHOROUTING_PING_INT_ARRAY_20)");
      THEchoRoutingPingIntArray20Event ping(e);
      
      int numInts=20;
      int i;
      char strTemp[50];
      string paramDump("[");
      for(i=0;i<numInts-1;i++)
      {
        sprintf(strTemp,"%d,",ping.intArrayParam[i]);
        paramDump += string(strTemp);
      }
      sprintf(strTemp,"%d]",ping.intArrayParam[i]);
      paramDump += string(strTemp);
      LOG_INFO(formatString("PING_INT_ARRAY_20 received on port %s (seqnr=%d, intArrayParam=%s)",p.getName().c_str(),ping.seqnr,paramDump.c_str()));

      THEchoRoutingEchoIntArray20Event echo;
      echo.seqnr = ping.seqnr;
      for(int i=0;i<numInts;i++)
        echo.intArrayParam[i] = ping.intArrayParam[i];
      _reply(echo,p);
      break;
    }

    case THECHOROUTING_PING_INT_ARRAY_61:
    {
      LOG_TRACE_FLOW("Echo::connected(THECHOROUTING_PING_INT_ARRAY_61)");
      THEchoRoutingPingIntArray61Event ping(e);
      
      int numInts=61;
      int i;
      char strTemp[50];
      string paramDump("[");
      for(i=0;i<numInts-1;i++)
      {
        sprintf(strTemp,"%d,",ping.intArrayParam[i]);
        paramDump += string(strTemp);
      }
      sprintf(strTemp,"%d]",ping.intArrayParam[i]);
      paramDump += string(strTemp);
      LOG_INFO(formatString("PING_INT_ARRAY_61 received on port %s (seqnr=%d, intArrayParam=%s)",p.getName().c_str(),ping.seqnr,paramDump.c_str()));

      THEchoRoutingEchoIntArray61Event echo;
      echo.seqnr = ping.seqnr;
      for(int i=0;i<numInts;i++)
        echo.intArrayParam[i] = ping.intArrayParam[i];
      _reply(echo,p);
      break;
    }

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
    }

  return status;
}

int main(int argc, char** argv)
{
  INIT_LOGGER(argv[0]);
  LOG_INFO(formatString("Program %s has started", argv[0]));
  
  GCFTask::init(argc, argv);

  string id("1");
  if (argv != 0)
  {
    CCmdLine cmdLine;

    // parse argc,argv 
    if (cmdLine.SplitLine(argc, argv) > 0)
    {
      id = cmdLine.GetSafeArgument("-id", 0, "1");
    }            
  }
  string taskName = string("THECHO") + id;
  LOG_INFO(formatString("Starting task %s. Use %s -id <id> to create a task with a different id",taskName.c_str(),argv[0],argv[0]));
  Echo echoTask(taskName);
  
  echoTask.start();

  GCFTask::run();
  
  LOG_INFO(formatString("Program %s has stopped", argv[0]));
  return 0;  
}

