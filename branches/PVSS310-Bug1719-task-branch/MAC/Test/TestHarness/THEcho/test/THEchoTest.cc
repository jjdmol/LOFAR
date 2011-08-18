//
//  THEchoTest.cc: Implementation of the Echo task class.
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
#include "THEchoTest.h"

#define DECLARE_SIGNAL_NAMES
#include "../src/THEcho_Protocol.ph"
#include "../src/THEchoRouting_Protocol.ph"

using std::cout;
using std::endl;
using namespace LOFAR;
using namespace THEchoTest;

EchoTest::EchoTest(string name) : 
  GCFTask((State)&EchoTest::initial, name),
  _client(),
  _routingClient(),
  _server(),
  _seqnr(0)
{
  LOG_TRACE_FLOW("EchoTest::EchoTest");
  // register the protocol for debugging purposes
  registerProtocol(THECHO_PROTOCOL, THECHO_PROTOCOL_signalnames);
  registerProtocol(THECHOROUTING_PROTOCOL, THECHOROUTING_PROTOCOL_signalnames);

  // initialize the port
  _client.init(*this, "client", GCFPortInterface::SAP, THECHO_PROTOCOL);
  _routingClient.init(*this, "routingClient", GCFPortInterface::SAP, THECHOROUTING_PROTOCOL);
  _server.init(*this, "server", GCFPortInterface::SPP, THECHOROUTING_PROTOCOL);
}

EchoTest::~EchoTest()
{
  LOG_TRACE_FLOW("EchoTest::~EchoTest");
}

bool EchoTest::_isClient(GCFPortInterface& p) const
{
  return (&_client == &p);
}

bool EchoTest::_isRoutingClient(GCFPortInterface& p) const
{
  return (&_routingClient == &p);
}

bool EchoTest::_isServer(GCFPortInterface& p) const
{
  return (&_server == &p);
}

void EchoTest::_send(GCFEvent& e, GCFPortInterface& p)
{
  if(_isClient(p))
  {
    _client.send(e);
    LOG_INFO(formatString("Ping sent on port %s",_client.getName().c_str()));
  }
  else if(_isRoutingClient(p))
  {
    _routingClient.send(e);
    LOG_INFO(formatString("Ping sent on port %s",_routingClient.getName().c_str()));
  }
}

GCFEvent::TResult EchoTest::initial(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_INIT:
      LOG_TRACE_FLOW("EchoTest::initial(F_INIT)");
      break;

    case F_ENTRY:
      LOG_TRACE_FLOW("EchoTest::initial(F_ENTRY)");
      _client.open();
      _routingClient.open();
      _server.open();
      break;

    case F_CONNECTED:
      LOG_TRACE_FLOW("EchoTest::initial(F_CONNECTED)");
      if(_client.isConnected() && _routingClient.isConnected() && _server.isConnected())
      {
        TRAN(EchoTest::test1);
      }
      break;

    case F_DISCONNECTED:
      LOG_TRACE_FLOW("EchoTest::initial(F_DISCONNECTED)");
      p.setTimer(5.0); // try again after 5 seconds
      break;

    case F_TIMER:
      LOG_TRACE_FLOW("EchoTest::initial(F_TIMER)");
      p.open(); // try again
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult EchoTest::test1(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {

    case F_ENTRY:
    {
      LOG_TRACE_FLOW("EchoTest::test1(F_ENTRY)");
      THEchoPingUintEvent ping;
      ping.seqnr = _seqnr;
      ping.uintParam = 123456789;
      _send(ping,_client);
      break;
    }
      
    case THECHO_ECHO_UINT:
    {
      LOG_TRACE_FLOW("EchoTest::test1(THECHO_ECHO_UINT)");
      if(!_isClient(p))
      {
        LOG_FATAL("reply received on wrong port!!");
        stop();
      }
      else
      {
        THEchoEchoUintEvent echo(e);
        LOG_INFO(formatString("ECHO_UINT received on port %s (seqnr=%d, uintParam=%d)",p.getName().c_str(),echo.seqnr,echo.uintParam));
        if(_seqnr != echo.seqnr)
        {
          LOG_FATAL("wrong seqnr received!!");
          stop();
        }
        else
        {
          _seqnr++;
          TRAN(EchoTest::test2);
        }
      }
      break;
    }

//    case THECHO_ECHO_UINT:
    case THECHO_ECHO_INT:
    case THECHO_ECHO_LONG:
    case THECHO_ECHO_ENUM:
    case THECHO_ECHO_DOUBLE:
    case THECHO_ECHO_STRING:
    case THECHO_ECHO_INT_ARRAY:
    case THECHOROUTING_ECHO_UINT:
    case THECHOROUTING_ECHO_INT:
    case THECHOROUTING_ECHO_LONG:
    case THECHOROUTING_ECHO_ENUM:
    case THECHOROUTING_ECHO_DOUBLE:
    case THECHOROUTING_ECHO_STRING:
    case THECHOROUTING_ECHO_INT_ARRAY:
    case THECHO_ECHO_INT_ARRAY_20:
    case THECHO_ECHO_INT_ARRAY_61:
    case THECHOROUTING_ECHO_INT_ARRAY_20:
    case THECHOROUTING_ECHO_INT_ARRAY_61:
      LOG_FATAL("wrong echo received!!");
      stop();
      break;
    
    case F_DISCONNECTED:
      LOG_TRACE_FLOW("EchoTest::test1(F_DISCONNECTED)");
      TRAN(EchoTest::initial);
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult EchoTest::test2(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {

    case F_ENTRY:
    {
      LOG_TRACE_FLOW("EchoTest::test2(F_ENTRY)");
      THEchoPingIntEvent ping;
      ping.seqnr = _seqnr;
      ping.intParam = 123456789;
      _send(ping,_client);
      break;
    }
      
    case THECHO_ECHO_INT:
    {
      LOG_TRACE_FLOW("EchoTest::test2(THECHO_ECHO_INT)");
      if(!_isClient(p))
      {
        LOG_FATAL("reply received on wrong port!!");
        stop();
      }
      else
      {
        THEchoEchoIntEvent echo(e);
        LOG_INFO(formatString("ECHO_INT received on port %s (seqnr=%d, intParam=%d)",p.getName().c_str(),echo.seqnr,echo.intParam));
        if(_seqnr != echo.seqnr)
        {
          LOG_FATAL("wrong seqnr received!!");
          stop();
        }
        else
        {
          _seqnr++;
          TRAN(EchoTest::test3);
        }
      }
      break;
    }

    case THECHO_ECHO_UINT:
//    case THECHO_ECHO_INT:
    case THECHO_ECHO_LONG:
    case THECHO_ECHO_ENUM:
    case THECHO_ECHO_DOUBLE:
    case THECHO_ECHO_STRING:
    case THECHO_ECHO_INT_ARRAY:
    case THECHOROUTING_ECHO_UINT:
    case THECHOROUTING_ECHO_INT:
    case THECHOROUTING_ECHO_LONG:
    case THECHOROUTING_ECHO_ENUM:
    case THECHOROUTING_ECHO_DOUBLE:
    case THECHOROUTING_ECHO_STRING:
    case THECHOROUTING_ECHO_INT_ARRAY:
    case THECHO_ECHO_INT_ARRAY_20:
    case THECHO_ECHO_INT_ARRAY_61:
    case THECHOROUTING_ECHO_INT_ARRAY_20:
    case THECHOROUTING_ECHO_INT_ARRAY_61:
      LOG_FATAL("wrong echo received!!");
      stop();
      break;
    
    case F_DISCONNECTED:
      LOG_TRACE_FLOW("EchoTest::test2(F_DISCONNECTED)");
      TRAN(EchoTest::initial);
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult EchoTest::test3(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {

    case F_ENTRY:
    {
      LOG_TRACE_FLOW("EchoTest::test3(F_ENTRY)");
      THEchoPingLongEvent ping;
      ping.seqnr = _seqnr;
      ping.longParam = 123456789;
      _send(ping,_client);
      break;
    }
      
    case THECHO_ECHO_LONG:
    {
      LOG_TRACE_FLOW("EchoTest::test3(THECHO_ECHO_LONG)");
      if(!_isClient(p))
      {
        LOG_FATAL("reply received on wrong port!!");
        stop();
      }
      else
      {
        THEchoEchoLongEvent echo(e);
        LOG_INFO(formatString("ECHO_LONG received on port %s (seqnr=%d, longParam=%d)",p.getName().c_str(),echo.seqnr,echo.longParam));
        if(_seqnr != echo.seqnr)
        {
          LOG_FATAL("wrong seqnr received!!");
          stop();
        }
        else
        {
          _seqnr++;
          TRAN(EchoTest::test4);
        }
      }
      break;
    }

    case THECHO_ECHO_UINT:
    case THECHO_ECHO_INT:
//    case THECHO_ECHO_LONG:
    case THECHO_ECHO_ENUM:
    case THECHO_ECHO_DOUBLE:
    case THECHO_ECHO_STRING:
    case THECHO_ECHO_INT_ARRAY:
    case THECHOROUTING_ECHO_UINT:
    case THECHOROUTING_ECHO_INT:
    case THECHOROUTING_ECHO_LONG:
    case THECHOROUTING_ECHO_ENUM:
    case THECHOROUTING_ECHO_DOUBLE:
    case THECHOROUTING_ECHO_STRING:
    case THECHOROUTING_ECHO_INT_ARRAY:
    case THECHO_ECHO_INT_ARRAY_20:
    case THECHO_ECHO_INT_ARRAY_61:
    case THECHOROUTING_ECHO_INT_ARRAY_20:
    case THECHOROUTING_ECHO_INT_ARRAY_61:
      LOG_FATAL("wrong echo received!!");
      stop();
      break;
    
    case F_DISCONNECTED:
      LOG_TRACE_FLOW("EchoTest::test3(F_DISCONNECTED)");
      TRAN(EchoTest::initial);
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}


GCFEvent::TResult EchoTest::test4(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {

    case F_ENTRY:
    {
      LOG_TRACE_FLOW("EchoTest::test4(F_ENTRY)");
      THEchoPingEnumEvent ping;
      ping.seqnr = _seqnr;
      ping.enumParam = ECHO_ENUM_FIRST;
      _send(ping,_client);
      break;
    }
      
    case THECHO_ECHO_ENUM:
    {
      LOG_TRACE_FLOW("EchoTest::test4(THECHO_ECHO_ENUM)");
      if(!_isClient(p))
      {
        LOG_FATAL("reply received on wrong port!!");
        stop();
      }
      else
      {
        THEchoEchoEnumEvent echo(e);
        LOG_INFO(formatString("ECHO_ENUM received on port %s (seqnr=%d, enumParam=%d)",p.getName().c_str(),echo.seqnr,echo.enumParam));
        if(_seqnr != echo.seqnr)
        {
          LOG_FATAL("wrong seqnr received!!");
          stop();
        }
        else
        {
          _seqnr++;
          TRAN(EchoTest::test5);
        }
      }
      break;
    }

    case THECHO_ECHO_UINT:
    case THECHO_ECHO_INT:
    case THECHO_ECHO_LONG:
//    case THECHO_ECHO_ENUM:
    case THECHO_ECHO_DOUBLE:
    case THECHO_ECHO_STRING:
    case THECHO_ECHO_INT_ARRAY:
    case THECHOROUTING_ECHO_UINT:
    case THECHOROUTING_ECHO_INT:
    case THECHOROUTING_ECHO_LONG:
    case THECHOROUTING_ECHO_ENUM:
    case THECHOROUTING_ECHO_DOUBLE:
    case THECHOROUTING_ECHO_STRING:
    case THECHOROUTING_ECHO_INT_ARRAY:
    case THECHO_ECHO_INT_ARRAY_20:
    case THECHO_ECHO_INT_ARRAY_61:
    case THECHOROUTING_ECHO_INT_ARRAY_20:
    case THECHOROUTING_ECHO_INT_ARRAY_61:
      LOG_FATAL("wrong echo received!!");
      stop();
      break;
    
    case F_DISCONNECTED:
      LOG_TRACE_FLOW("EchoTest::test4(F_DISCONNECTED)");
      TRAN(EchoTest::initial);
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}


GCFEvent::TResult EchoTest::test5(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {

    case F_ENTRY:
    {
      LOG_TRACE_FLOW("EchoTest::test5(F_ENTRY)");
      THEchoPingDoubleEvent ping;
      ping.seqnr = _seqnr;
      ping.doubleParam = 1234.56789;
      _send(ping,_client);
      break;
    }
      
    case THECHO_ECHO_DOUBLE:
    {
      LOG_TRACE_FLOW("EchoTest::test5(THECHO_ECHO_DOUBLE)");
      if(!_isClient(p))
      {
        LOG_FATAL("reply received on wrong port!!");
        stop();
      }
      else
      {
        THEchoEchoDoubleEvent echo(e);
        LOG_INFO(formatString("ECHO_DOUBLE received on port %s (seqnr=%d, doubleParam=%f)",p.getName().c_str(),echo.seqnr,echo.doubleParam));
        if(_seqnr != echo.seqnr)
        {
          LOG_FATAL("wrong seqnr received!!");
          stop();
        }
        else
        {
          _seqnr++;
          TRAN(EchoTest::test6);
        }
      }
      break;
    }

    case THECHO_ECHO_UINT:
    case THECHO_ECHO_INT:
    case THECHO_ECHO_LONG:
    case THECHO_ECHO_ENUM:
//    case THECHO_ECHO_DOUBLE:
    case THECHO_ECHO_STRING:
    case THECHO_ECHO_INT_ARRAY:
    case THECHOROUTING_ECHO_UINT:
    case THECHOROUTING_ECHO_INT:
    case THECHOROUTING_ECHO_LONG:
    case THECHOROUTING_ECHO_ENUM:
    case THECHOROUTING_ECHO_DOUBLE:
    case THECHOROUTING_ECHO_STRING:
    case THECHOROUTING_ECHO_INT_ARRAY:
    case THECHO_ECHO_INT_ARRAY_20:
    case THECHO_ECHO_INT_ARRAY_61:
    case THECHOROUTING_ECHO_INT_ARRAY_20:
    case THECHOROUTING_ECHO_INT_ARRAY_61:
      LOG_FATAL("wrong echo received!!");
      stop();
      break;
    
    case F_DISCONNECTED:
      LOG_TRACE_FLOW("EchoTest::test5(F_DISCONNECTED)");
      TRAN(EchoTest::initial);
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult EchoTest::test6(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {

    case F_ENTRY:
    {
      LOG_TRACE_FLOW("EchoTest::test6(F_ENTRY)");
      THEchoPingStringEvent ping;
      ping.seqnr = _seqnr;
      ping.stringParam = "123456789";
      _send(ping,_client);
      break;
    }
      
    case THECHO_ECHO_STRING:
    {
      LOG_TRACE_FLOW("EchoTest::test6(THECHO_ECHO_STRING)");
      if(!_isClient(p))
      {
        LOG_FATAL("reply received on wrong port!!");
        stop();
      }
      else
      {
        THEchoEchoStringEvent echo(e);
        LOG_INFO(formatString("ECHO_STRING received on port %s (seqnr=%d, stringParam=%s)",p.getName().c_str(),echo.seqnr,echo.stringParam.c_str()));
        if(_seqnr != echo.seqnr)
        {
          LOG_FATAL("wrong seqnr received!!");
          stop();
        }
        else
        {
          _seqnr++;
          TRAN(EchoTest::test7);
        }
      }
      break;
    }

    case THECHO_ECHO_UINT:
    case THECHO_ECHO_INT:
    case THECHO_ECHO_LONG:
    case THECHO_ECHO_ENUM:
    case THECHO_ECHO_DOUBLE:
//    case THECHO_ECHO_STRING:
    case THECHO_ECHO_INT_ARRAY:
    case THECHOROUTING_ECHO_UINT:
    case THECHOROUTING_ECHO_INT:
    case THECHOROUTING_ECHO_LONG:
    case THECHOROUTING_ECHO_ENUM:
    case THECHOROUTING_ECHO_DOUBLE:
    case THECHOROUTING_ECHO_STRING:
    case THECHOROUTING_ECHO_INT_ARRAY:
    case THECHO_ECHO_INT_ARRAY_20:
    case THECHO_ECHO_INT_ARRAY_61:
    case THECHOROUTING_ECHO_INT_ARRAY_20:
    case THECHOROUTING_ECHO_INT_ARRAY_61:
      LOG_FATAL("wrong echo received!!");
      stop();
      break;
    
    case F_DISCONNECTED:
      LOG_TRACE_FLOW("EchoTest::test6(F_DISCONNECTED)");
      TRAN(EchoTest::initial);
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult EchoTest::test7(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {

    case F_ENTRY:
    {
      LOG_TRACE_FLOW("EchoTest::test7(F_ENTRY)");
      THEchoPingIntArrayEvent ping;
      ping.seqnr = _seqnr;
      for(int i=0;i<10;i++)
        ping.intArrayParam[i]=i;
      _send(ping,_client);
      break;
    }
      
    case THECHO_ECHO_INT_ARRAY:
    {
      LOG_TRACE_FLOW("EchoTest::test7(THECHO_ECHO_INT_ARRAY)");
      if(!_isClient(p))
      {
        LOG_FATAL("reply received on wrong port!!");
        stop();
      }
      else
      {
        THEchoEchoIntArrayEvent echo(e);
        LOG_INFO(formatString("ECHO_INT_ARRAY received on port %s (seqnr=%d, intArrayParam=[%d,%d,%d,%d,%d,%d,%d,%d,%d,%d])",p.getName().c_str(),echo.seqnr,echo.intArrayParam[0],echo.intArrayParam[1],echo.intArrayParam[2],echo.intArrayParam[3],echo.intArrayParam[4],echo.intArrayParam[5],echo.intArrayParam[6],echo.intArrayParam[7],echo.intArrayParam[8],echo.intArrayParam[9]));
        if(_seqnr != echo.seqnr)
        {
          LOG_FATAL("wrong seqnr received!!");
          stop();
        }
        else
        {
          _seqnr++;
          TRAN(EchoTest::test8);
        }
      }
      break;
    }

    case THECHO_ECHO_UINT:
    case THECHO_ECHO_INT:
    case THECHO_ECHO_LONG:
    case THECHO_ECHO_ENUM:
    case THECHO_ECHO_DOUBLE:
    case THECHO_ECHO_STRING:
//    case THECHO_ECHO_INT_ARRAY:
    case THECHOROUTING_ECHO_UINT:
    case THECHOROUTING_ECHO_INT:
    case THECHOROUTING_ECHO_LONG:
    case THECHOROUTING_ECHO_ENUM:
    case THECHOROUTING_ECHO_DOUBLE:
    case THECHOROUTING_ECHO_STRING:
    case THECHOROUTING_ECHO_INT_ARRAY:
    case THECHO_ECHO_INT_ARRAY_20:
    case THECHO_ECHO_INT_ARRAY_61:
    case THECHOROUTING_ECHO_INT_ARRAY_20:
    case THECHOROUTING_ECHO_INT_ARRAY_61:
      LOG_FATAL("wrong echo received!!");
      stop();
      break;
    
    case F_DISCONNECTED:
      LOG_TRACE_FLOW("EchoTest::test7(F_DISCONNECTED)");
      TRAN(EchoTest::initial);
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult EchoTest::test8(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {

    case F_ENTRY:
    {
      LOG_TRACE_FLOW("EchoTest::test8(F_ENTRY)");
      THEchoRoutingPingUintEvent ping;
      ping.seqnr = _seqnr;
      ping.uintParam = 123456789;
      _send(ping,_routingClient);
      break;
    }
      
    case THECHOROUTING_ECHO_UINT:
    {
      LOG_TRACE_FLOW("EchoTest::test8(THECHOROUTING_ECHO_UINT)");
      if(!_isServer(p))
      {
        LOG_FATAL("reply received on wrong port!!");
        stop();
      }
      else
      {
        THEchoRoutingEchoUintEvent echo(e);
        LOG_INFO(formatString("ECHO_UINT received on port %s (seqnr=%d, uintParam=%d)",p.getName().c_str(),echo.seqnr,echo.uintParam));
        if(_seqnr != echo.seqnr)
        {
          LOG_FATAL("wrong seqnr received!!");
          stop();
        }
        else
        {
          _seqnr++;
          TRAN(EchoTest::test9);
        }
      }
      break;
    }

    case THECHO_ECHO_UINT:
    case THECHO_ECHO_INT:
    case THECHO_ECHO_LONG:
    case THECHO_ECHO_ENUM:
    case THECHO_ECHO_DOUBLE:
    case THECHO_ECHO_STRING:
    case THECHO_ECHO_INT_ARRAY:
//    case THECHOROUTING_ECHO_UINT:
    case THECHOROUTING_ECHO_INT:
    case THECHOROUTING_ECHO_LONG:
    case THECHOROUTING_ECHO_ENUM:
    case THECHOROUTING_ECHO_DOUBLE:
    case THECHOROUTING_ECHO_STRING:
    case THECHOROUTING_ECHO_INT_ARRAY:
    case THECHO_ECHO_INT_ARRAY_20:
    case THECHO_ECHO_INT_ARRAY_61:
    case THECHOROUTING_ECHO_INT_ARRAY_20:
    case THECHOROUTING_ECHO_INT_ARRAY_61:
      LOG_FATAL("wrong echo received!!");
      stop();
      break;
    
    case F_DISCONNECTED:
      LOG_TRACE_FLOW("EchoTest::test8(F_DISCONNECTED)");
      TRAN(EchoTest::initial);
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult EchoTest::test9(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {

    case F_ENTRY:
    {
      LOG_TRACE_FLOW("EchoTest::test9(F_ENTRY)");
      THEchoRoutingPingIntEvent ping;
      ping.seqnr = _seqnr;
      ping.intParam = 123456789;
      _send(ping,_routingClient);
      break;
    }
      
    case THECHOROUTING_ECHO_INT:
    {
      LOG_TRACE_FLOW("EchoTest::test9(THECHOROUTING_ECHO_INT)");
      if(!_isServer(p))
      {
        LOG_FATAL("reply received on wrong port!!");
        stop();
      }
      else
      {
        THEchoRoutingEchoIntEvent echo(e);
        LOG_INFO(formatString("ECHO_INT received on port %s (seqnr=%d, intParam=%d)",p.getName().c_str(),echo.seqnr,echo.intParam));
        if(_seqnr != echo.seqnr)
        {
          LOG_FATAL("wrong seqnr received!!");
          stop();
        }
        else
        {
          _seqnr++;
          TRAN(EchoTest::test10);
        }
      }
      break;
    }

    case THECHO_ECHO_UINT:
    case THECHO_ECHO_INT:
    case THECHO_ECHO_LONG:
    case THECHO_ECHO_ENUM:
    case THECHO_ECHO_DOUBLE:
    case THECHO_ECHO_STRING:
    case THECHO_ECHO_INT_ARRAY:
    case THECHOROUTING_ECHO_UINT:
//    case THECHOROUTING_ECHO_INT:
    case THECHOROUTING_ECHO_LONG:
    case THECHOROUTING_ECHO_ENUM:
    case THECHOROUTING_ECHO_DOUBLE:
    case THECHOROUTING_ECHO_STRING:
    case THECHOROUTING_ECHO_INT_ARRAY:
    case THECHO_ECHO_INT_ARRAY_20:
    case THECHO_ECHO_INT_ARRAY_61:
    case THECHOROUTING_ECHO_INT_ARRAY_20:
    case THECHOROUTING_ECHO_INT_ARRAY_61:
      LOG_FATAL("wrong echo received!!");
      stop();
      break;
    
    case F_DISCONNECTED:
      LOG_TRACE_FLOW("EchoTest::test9(F_DISCONNECTED)");
      TRAN(EchoTest::initial);
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult EchoTest::test10(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {

    case F_ENTRY:
    {
      LOG_TRACE_FLOW("EchoTest::test10(F_ENTRY)");
      THEchoRoutingPingLongEvent ping;
      ping.seqnr = _seqnr;
      ping.longParam = 123456789;
      _send(ping,_routingClient);
      break;
    }
      
    case THECHOROUTING_ECHO_LONG:
    {
      LOG_TRACE_FLOW("EchoTest::test10(THECHOROUTING_ECHO_LONG)");
      if(!_isServer(p))
      {
        LOG_FATAL("reply received on wrong port!!");
        stop();
      }
      else
      {
        THEchoRoutingEchoLongEvent echo(e);
        LOG_INFO(formatString("ECHO_LONG received on port %s (seqnr=%d, longParam=%d)",p.getName().c_str(),echo.seqnr,echo.longParam));
        if(_seqnr != echo.seqnr)
        {
          LOG_FATAL("wrong seqnr received!!");
          stop();
        }
        else
        {
          _seqnr++;
          TRAN(EchoTest::test11);
        }
      }
      break;
    }

    case THECHO_ECHO_UINT:
    case THECHO_ECHO_INT:
    case THECHO_ECHO_LONG:
    case THECHO_ECHO_ENUM:
    case THECHO_ECHO_DOUBLE:
    case THECHO_ECHO_STRING:
    case THECHO_ECHO_INT_ARRAY:
    case THECHOROUTING_ECHO_UINT:
    case THECHOROUTING_ECHO_INT:
//    case THECHOROUTING_ECHO_LONG:
    case THECHOROUTING_ECHO_ENUM:
    case THECHOROUTING_ECHO_DOUBLE:
    case THECHOROUTING_ECHO_STRING:
    case THECHOROUTING_ECHO_INT_ARRAY:
    case THECHO_ECHO_INT_ARRAY_20:
    case THECHO_ECHO_INT_ARRAY_61:
    case THECHOROUTING_ECHO_INT_ARRAY_20:
    case THECHOROUTING_ECHO_INT_ARRAY_61:
      LOG_FATAL("wrong echo received!!");
      stop();
      break;
    
    case F_DISCONNECTED:
      LOG_TRACE_FLOW("EchoTest::test10(F_DISCONNECTED)");
      TRAN(EchoTest::initial);
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult EchoTest::test11(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {

    case F_ENTRY:
    {
      LOG_TRACE_FLOW("EchoTest::test11(F_ENTRY)");
      THEchoRoutingPingEnumEvent ping;
      ping.seqnr = _seqnr;
      ping.enumParam = ECHOROUTING_ENUM_SECOND;
      _send(ping,_routingClient);
      break;
    }
      
    case THECHOROUTING_ECHO_ENUM:
    {
      LOG_TRACE_FLOW("EchoTest::test11(THECHOROUTING_ECHO_ENUM)");
      if(!_isServer(p))
      {
        LOG_FATAL("reply received on wrong port!!");
        stop();
      }
      else
      {
        THEchoRoutingEchoEnumEvent echo(e);
        LOG_INFO(formatString("ECHO_ENUM received on port %s (seqnr=%d, enumParam=%d)",p.getName().c_str(),echo.seqnr,echo.enumParam));
        if(_seqnr != echo.seqnr)
        {
          LOG_FATAL("wrong seqnr received!!");
          stop();
        }
        else
        {
          _seqnr++;
          TRAN(EchoTest::test12);
        }
      }
      break;
    }

    case THECHO_ECHO_UINT:
    case THECHO_ECHO_INT:
    case THECHO_ECHO_LONG:
    case THECHO_ECHO_ENUM:
    case THECHO_ECHO_DOUBLE:
    case THECHO_ECHO_STRING:
    case THECHO_ECHO_INT_ARRAY:
    case THECHOROUTING_ECHO_UINT:
    case THECHOROUTING_ECHO_INT:
    case THECHOROUTING_ECHO_LONG:
//    case THECHOROUTING_ECHO_ENUM:
    case THECHOROUTING_ECHO_DOUBLE:
    case THECHOROUTING_ECHO_STRING:
    case THECHOROUTING_ECHO_INT_ARRAY:
    case THECHO_ECHO_INT_ARRAY_20:
    case THECHO_ECHO_INT_ARRAY_61:
    case THECHOROUTING_ECHO_INT_ARRAY_20:
    case THECHOROUTING_ECHO_INT_ARRAY_61:
      LOG_FATAL("wrong echo received!!");
      stop();
      break;
    
    case F_DISCONNECTED:
      LOG_TRACE_FLOW("EchoTest::test11(F_DISCONNECTED)");
      TRAN(EchoTest::initial);
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult EchoTest::test12(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {

    case F_ENTRY:
    {
      LOG_TRACE_FLOW("EchoTest::test12(F_ENTRY)");
      THEchoRoutingPingDoubleEvent ping;
      ping.seqnr = _seqnr;
      ping.doubleParam = 1234.56789;
      _send(ping,_routingClient);
      break;
    }
      
    case THECHOROUTING_ECHO_DOUBLE:
    {
      LOG_TRACE_FLOW("EchoTest::test12(THECHOROUTING_ECHO_DOUBLE)");
      if(!_isServer(p))
      {
        LOG_FATAL("reply received on wrong port!!");
        stop();
      }
      else
      {
        THEchoRoutingEchoDoubleEvent echo(e);
        LOG_INFO(formatString("ECHO_DOUBLE received on port %s (seqnr=%d, doubleParam=%f)",p.getName().c_str(),echo.seqnr,echo.doubleParam));
        if(_seqnr != echo.seqnr)
        {
          LOG_FATAL("wrong seqnr received!!");
          stop();
        }
        else
        {
          _seqnr++;
          TRAN(EchoTest::test13);
        }
      }
      break;
    }

    case THECHO_ECHO_UINT:
    case THECHO_ECHO_INT:
    case THECHO_ECHO_LONG:
    case THECHO_ECHO_ENUM:
    case THECHO_ECHO_DOUBLE:
    case THECHO_ECHO_STRING:
    case THECHO_ECHO_INT_ARRAY:
    case THECHOROUTING_ECHO_UINT:
    case THECHOROUTING_ECHO_INT:
    case THECHOROUTING_ECHO_LONG:
    case THECHOROUTING_ECHO_ENUM:
//    case THECHOROUTING_ECHO_DOUBLE:
    case THECHOROUTING_ECHO_STRING:
    case THECHOROUTING_ECHO_INT_ARRAY:
    case THECHO_ECHO_INT_ARRAY_20:
    case THECHO_ECHO_INT_ARRAY_61:
    case THECHOROUTING_ECHO_INT_ARRAY_20:
    case THECHOROUTING_ECHO_INT_ARRAY_61:
      LOG_FATAL("wrong echo received!!");
      stop();
      break;
    
    case F_DISCONNECTED:
      LOG_TRACE_FLOW("EchoTest::test12(F_DISCONNECTED)");
      TRAN(EchoTest::initial);
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult EchoTest::test13(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {

    case F_ENTRY:
    {
      LOG_TRACE_FLOW("EchoTest::test13(F_ENTRY)");
      THEchoRoutingPingStringEvent ping;
      ping.seqnr = _seqnr;
      ping.stringParam = "123456789";
      _send(ping,_routingClient);
      break;
    }
      
    case THECHOROUTING_ECHO_STRING:
    {
      LOG_TRACE_FLOW("EchoTest::test13(THECHOROUTING_ECHO_STRING)");
      if(!_isServer(p))
      {
        LOG_FATAL("reply received on wrong port!!");
        stop();
      }
      else
      {
        THEchoRoutingEchoStringEvent echo(e);
        LOG_INFO(formatString("ECHO_STRING received on port %s (seqnr=%d, stringParam=%s)",p.getName().c_str(),echo.seqnr,echo.stringParam.c_str()));
        if(_seqnr != echo.seqnr)
        {
          LOG_FATAL("wrong seqnr received!!");
          stop();
        }
        else
        {
          _seqnr++;
          TRAN(EchoTest::test14);
        }
      }
      break;
    }

    case THECHO_ECHO_UINT:
    case THECHO_ECHO_INT:
    case THECHO_ECHO_LONG:
    case THECHO_ECHO_ENUM:
    case THECHO_ECHO_DOUBLE:
    case THECHO_ECHO_STRING:
    case THECHO_ECHO_INT_ARRAY:
    case THECHOROUTING_ECHO_UINT:
    case THECHOROUTING_ECHO_INT:
    case THECHOROUTING_ECHO_LONG:
    case THECHOROUTING_ECHO_ENUM:
    case THECHOROUTING_ECHO_DOUBLE:
//    case THECHOROUTING_ECHO_STRING:
    case THECHOROUTING_ECHO_INT_ARRAY:
    case THECHO_ECHO_INT_ARRAY_20:
    case THECHO_ECHO_INT_ARRAY_61:
    case THECHOROUTING_ECHO_INT_ARRAY_20:
    case THECHOROUTING_ECHO_INT_ARRAY_61:
      LOG_FATAL("wrong echo received!!");
      stop();
      break;
    
    case F_DISCONNECTED:
      LOG_TRACE_FLOW("EchoTest::test13(F_DISCONNECTED)");
      TRAN(EchoTest::initial);
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult EchoTest::test14(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {

    case F_ENTRY:
    {
      LOG_TRACE_FLOW("EchoTest::test14(F_ENTRY)");
      THEchoRoutingPingIntArrayEvent ping;
      ping.seqnr = _seqnr;
      for(int i=0;i<10;i++)
        ping.intArrayParam[i]=i;
      _send(ping,_routingClient);
      break;
    }
      
    case THECHOROUTING_ECHO_INT_ARRAY:
    {
      LOG_TRACE_FLOW("EchoTest::test14(THECHOROUTING_ECHO_INT_ARRAY)");
      if(!_isServer(p))
      {
        LOG_FATAL("reply received on wrong port!!");
        stop();
      }
      else
      {
        THEchoRoutingEchoIntArrayEvent echo(e);
        LOG_INFO(formatString("ECHO_INT_ARRAY received on port %s (seqnr=%d, intArrayParam=[%d,%d,%d,%d,%d,%d,%d,%d,%d,%d])",p.getName().c_str(),echo.seqnr,echo.intArrayParam[0],echo.intArrayParam[1],echo.intArrayParam[2],echo.intArrayParam[3],echo.intArrayParam[4],echo.intArrayParam[5],echo.intArrayParam[6],echo.intArrayParam[7],echo.intArrayParam[8],echo.intArrayParam[9]));
        if(_seqnr != echo.seqnr)
        {
          LOG_FATAL("wrong seqnr received!!");
          stop();
        }
        else
        {
          _seqnr++;
          TRAN(EchoTest::test15);
        }
      }
      break;
    }

    case THECHO_ECHO_UINT:
    case THECHO_ECHO_INT:
    case THECHO_ECHO_LONG:
    case THECHO_ECHO_ENUM:
    case THECHO_ECHO_DOUBLE:
    case THECHO_ECHO_STRING:
    case THECHO_ECHO_INT_ARRAY:
    case THECHOROUTING_ECHO_UINT:
    case THECHOROUTING_ECHO_INT:
    case THECHOROUTING_ECHO_LONG:
    case THECHOROUTING_ECHO_ENUM:
    case THECHOROUTING_ECHO_DOUBLE:
    case THECHOROUTING_ECHO_STRING:
//    case THECHOROUTING_ECHO_INT_ARRAY:
    case THECHO_ECHO_INT_ARRAY_20:
    case THECHO_ECHO_INT_ARRAY_61:
    case THECHOROUTING_ECHO_INT_ARRAY_20:
    case THECHOROUTING_ECHO_INT_ARRAY_61:
      LOG_FATAL("wrong echo received!!");
      stop();
      break;
    
    case F_DISCONNECTED:
      LOG_TRACE_FLOW("EchoTest::test14(F_DISCONNECTED)");
      TRAN(EchoTest::initial);
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult EchoTest::test15(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {

    case F_ENTRY:
    {
      LOG_TRACE_FLOW("EchoTest::test15(F_ENTRY)");
      THEchoPingIntArray20Event ping;
      ping.seqnr = _seqnr;
      int numInts=20;
      for(int i=0;i<numInts;i++)
        ping.intArrayParam[i]=i;
      _send(ping,_client);
      break;
    }
      
    case THECHO_ECHO_INT_ARRAY_20:
    {
      LOG_TRACE_FLOW("EchoTest::test15(THECHO_ECHO_INT_ARRAY_20)");
      if(!_isClient(p))
      {
        LOG_FATAL("reply received on wrong port!!");
        stop();
      }
      else
      {
        THEchoEchoIntArray20Event echo(e);

        int numInts=20;
        int i;
        char strTemp[50];
        string paramDump("[");
        for(i=0;i<numInts-1;i++)
        {
          sprintf(strTemp,"%d,",echo.intArrayParam[i]);
          paramDump += string(strTemp);
        }
        sprintf(strTemp,"%d]",echo.intArrayParam[i]);
        paramDump += string(strTemp);
        LOG_INFO(formatString("ECHO_INT_ARRAY_20 received on port %s (seqnr=%d, intArrayParam=%s)",p.getName().c_str(),echo.seqnr,paramDump.c_str()));

        if(_seqnr != echo.seqnr)
        {
          LOG_FATAL("wrong seqnr received!!");
          stop();
        }
        else
        {
          _seqnr++;
          TRAN(EchoTest::test16);
        }
      }
      break;
    }

    case THECHO_ECHO_UINT:
    case THECHO_ECHO_INT:
    case THECHO_ECHO_LONG:
    case THECHO_ECHO_ENUM:
    case THECHO_ECHO_DOUBLE:
    case THECHO_ECHO_STRING:
    case THECHO_ECHO_INT_ARRAY:
    case THECHOROUTING_ECHO_UINT:
    case THECHOROUTING_ECHO_INT:
    case THECHOROUTING_ECHO_LONG:
    case THECHOROUTING_ECHO_ENUM:
    case THECHOROUTING_ECHO_DOUBLE:
    case THECHOROUTING_ECHO_STRING:
    case THECHOROUTING_ECHO_INT_ARRAY:
//    case THECHO_ECHO_INT_ARRAY_20:
    case THECHO_ECHO_INT_ARRAY_61:
    case THECHOROUTING_ECHO_INT_ARRAY_20:
    case THECHOROUTING_ECHO_INT_ARRAY_61:
      LOG_FATAL("wrong echo received!!");
      stop();
      break;
    
    case F_DISCONNECTED:
      LOG_TRACE_FLOW("EchoTest::test15(F_DISCONNECTED)");
      TRAN(EchoTest::initial);
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult EchoTest::test16(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {

    case F_ENTRY:
    {
      LOG_TRACE_FLOW("EchoTest::test16(F_ENTRY)");
      THEchoPingIntArray61Event ping;
      ping.seqnr = _seqnr;
      int numInts=61;
      for(int i=0;i<numInts;i++)
        ping.intArrayParam[i]=i;
      _send(ping,_client);
      break;
    }
      
    case THECHO_ECHO_INT_ARRAY_61:
    {
      LOG_TRACE_FLOW("EchoTest::test16(THECHO_ECHO_INT_ARRAY_61)");
      if(!_isClient(p))
      {
        LOG_FATAL("reply received on wrong port!!");
        stop();
      }
      else
      {
        THEchoEchoIntArray61Event echo(e);

        int numInts=61;
        int i;
        char strTemp[50];
        string paramDump("[");
        for(i=0;i<numInts-1;i++)
        {
          sprintf(strTemp,"%d,",echo.intArrayParam[i]);
          paramDump += string(strTemp);
        }
        sprintf(strTemp,"%d]",echo.intArrayParam[i]);
        paramDump += string(strTemp);
        LOG_INFO(formatString("ECHO_INT_ARRAY_61 received on port %s (seqnr=%d, intArrayParam=%s)",p.getName().c_str(),echo.seqnr,paramDump.c_str()));

        if(_seqnr != echo.seqnr)
        {
          LOG_FATAL("wrong seqnr received!!");
          stop();
        }
        else
        {
          _seqnr++;
          TRAN(EchoTest::test17);
        }
      }
      break;
    }

    case THECHO_ECHO_UINT:
    case THECHO_ECHO_INT:
    case THECHO_ECHO_LONG:
    case THECHO_ECHO_ENUM:
    case THECHO_ECHO_DOUBLE:
    case THECHO_ECHO_STRING:
    case THECHO_ECHO_INT_ARRAY:
    case THECHOROUTING_ECHO_UINT:
    case THECHOROUTING_ECHO_INT:
    case THECHOROUTING_ECHO_LONG:
    case THECHOROUTING_ECHO_ENUM:
    case THECHOROUTING_ECHO_DOUBLE:
    case THECHOROUTING_ECHO_STRING:
    case THECHOROUTING_ECHO_INT_ARRAY:
    case THECHO_ECHO_INT_ARRAY_20:
//    case THECHO_ECHO_INT_ARRAY_61:
    case THECHOROUTING_ECHO_INT_ARRAY_20:
    case THECHOROUTING_ECHO_INT_ARRAY_61:
      LOG_FATAL("wrong echo received!!");
      stop();
      break;
    
    case F_DISCONNECTED:
      LOG_TRACE_FLOW("EchoTest::test16(F_DISCONNECTED)");
      TRAN(EchoTest::initial);
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult EchoTest::test17(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {

    case F_ENTRY:
    {
      LOG_TRACE_FLOW("EchoTest::test17(F_ENTRY)");
      THEchoRoutingPingIntArray20Event ping;
      ping.seqnr = _seqnr;
      int numInts=20;
      for(int i=0;i<numInts;i++)
        ping.intArrayParam[i]=i;
      _send(ping,_routingClient);
      break;
    }
      
    case THECHOROUTING_ECHO_INT_ARRAY_20:
    {
      LOG_TRACE_FLOW("EchoTest::test17(THECHOROUTING_ECHO_INT_ARRAY_20)");
      if(!_isServer(p))
      {
        LOG_FATAL("reply received on wrong port!!");
        stop();
      }
      else
      {
        THEchoRoutingEchoIntArray20Event echo(e);

        int numInts=20;
        int i;
        char strTemp[50];
        string paramDump("[");
        for(i=0;i<numInts-1;i++)
        {
          sprintf(strTemp,"%d,",echo.intArrayParam[i]);
          paramDump += string(strTemp);
        }
        sprintf(strTemp,"%d]",echo.intArrayParam[i]);
        paramDump += string(strTemp);
        LOG_INFO(formatString("ECHO_INT_ARRAY_20 received on port %s (seqnr=%d, intArrayParam=%s)",p.getName().c_str(),echo.seqnr,paramDump.c_str()));

        if(_seqnr != echo.seqnr)
        {
          LOG_FATAL("wrong seqnr received!!");
          stop();
        }
        else
        {
          _seqnr++;
          TRAN(EchoTest::test18);
        }
      }
      break;
    }

    case THECHO_ECHO_UINT:
    case THECHO_ECHO_INT:
    case THECHO_ECHO_LONG:
    case THECHO_ECHO_ENUM:
    case THECHO_ECHO_DOUBLE:
    case THECHO_ECHO_STRING:
    case THECHO_ECHO_INT_ARRAY:
    case THECHOROUTING_ECHO_UINT:
    case THECHOROUTING_ECHO_INT:
    case THECHOROUTING_ECHO_LONG:
    case THECHOROUTING_ECHO_ENUM:
    case THECHOROUTING_ECHO_DOUBLE:
    case THECHOROUTING_ECHO_STRING:
    case THECHOROUTING_ECHO_INT_ARRAY:
    case THECHO_ECHO_INT_ARRAY_20:
    case THECHO_ECHO_INT_ARRAY_61:
//    case THECHOROUTING_ECHO_INT_ARRAY_20:
    case THECHOROUTING_ECHO_INT_ARRAY_61:
      LOG_FATAL("wrong echo received!!");
      stop();
      break;
    
    case F_DISCONNECTED:
      LOG_TRACE_FLOW("EchoTest::test17(F_DISCONNECTED)");
      TRAN(EchoTest::initial);
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult EchoTest::test18(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {

    case F_ENTRY:
    {
      LOG_TRACE_FLOW("EchoTest::test18(F_ENTRY)");
      THEchoRoutingPingIntArray61Event ping;
      ping.seqnr = _seqnr;
      int numInts=61;
      for(int i=0;i<numInts;i++)
        ping.intArrayParam[i]=i;
      _send(ping,_routingClient);
      break;
    }
      
    case THECHOROUTING_ECHO_INT_ARRAY_61:
    {
      LOG_TRACE_FLOW("EchoTest::test18(THECHOROUTING_ECHO_INT_ARRAY_61)");
      if(!_isServer(p))
      {
        LOG_FATAL("reply received on wrong port!!");
        stop();
      }
      else
      {
        THEchoRoutingEchoIntArray61Event echo(e);

        int numInts=61;
        int i;
        char strTemp[50];
        string paramDump("[");
        for(i=0;i<numInts-1;i++)
        {
          sprintf(strTemp,"%d,",echo.intArrayParam[i]);
          paramDump += string(strTemp);
        }
        sprintf(strTemp,"%d]",echo.intArrayParam[i]);
        paramDump += string(strTemp);
        LOG_INFO(formatString("ECHO_INT_ARRAY_61 received on port %s (seqnr=%d, intArrayParam=%s)",p.getName().c_str(),echo.seqnr,paramDump.c_str()));

        if(_seqnr != echo.seqnr)
        {
          LOG_FATAL("wrong seqnr received!!");
          stop();
        }
        else
        {
          _seqnr++;
          TRAN(EchoTest::final);
        }
      }
      break;
    }

    case THECHO_ECHO_UINT:
    case THECHO_ECHO_INT:
    case THECHO_ECHO_LONG:
    case THECHO_ECHO_ENUM:
    case THECHO_ECHO_DOUBLE:
    case THECHO_ECHO_STRING:
    case THECHO_ECHO_INT_ARRAY:
    case THECHOROUTING_ECHO_UINT:
    case THECHOROUTING_ECHO_INT:
    case THECHOROUTING_ECHO_LONG:
    case THECHOROUTING_ECHO_ENUM:
    case THECHOROUTING_ECHO_DOUBLE:
    case THECHOROUTING_ECHO_STRING:
    case THECHOROUTING_ECHO_INT_ARRAY:
    case THECHO_ECHO_INT_ARRAY_20:
    case THECHO_ECHO_INT_ARRAY_61:
    case THECHOROUTING_ECHO_INT_ARRAY_20:
//    case THECHOROUTING_ECHO_INT_ARRAY_61:
      LOG_FATAL("wrong echo received!!");
      stop();
      break;
    
    case F_DISCONNECTED:
      LOG_TRACE_FLOW("EchoTest::test18(F_DISCONNECTED)");
      TRAN(EchoTest::initial);
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult EchoTest::final(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {

    case F_ENTRY:
    {
      LOG_TRACE_FLOW("EchoTest::final(F_ENTRY)");
      THEchoRoutingPingStopEvent ping;
      ping.seqnr = _seqnr;
      _send(ping,_routingClient);
      stop();
      break;
    }
      
    case THECHO_ECHO_UINT:
    case THECHO_ECHO_INT:
    case THECHO_ECHO_LONG:
    case THECHO_ECHO_ENUM:
    case THECHO_ECHO_DOUBLE:
    case THECHO_ECHO_STRING:
    case THECHO_ECHO_INT_ARRAY:
    case THECHOROUTING_ECHO_UINT:
    case THECHOROUTING_ECHO_INT:
    case THECHOROUTING_ECHO_LONG:
    case THECHOROUTING_ECHO_ENUM:
    case THECHOROUTING_ECHO_DOUBLE:
    case THECHOROUTING_ECHO_STRING:
    case THECHOROUTING_ECHO_INT_ARRAY:
      LOG_FATAL("wrong echo received!!");
      stop();
      break;
    
    case F_DISCONNECTED:
      LOG_TRACE_FLOW("EchoTest::final(F_DISCONNECTED)");
      TRAN(EchoTest::initial);
      break;

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

  EchoTest echoTestTask(string("THECHOTEST"));
  
  echoTestTask.start();

  GCFTask::run();
  
  LOG_INFO(formatString("Program %s has stopped", argv[0]));
  return 0;  
}

