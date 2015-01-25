//
//  Echo.cc: Implementation of the Echo task class.
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

#include <lofar_config.h>
#include <GCF/GCF_PVBool.h>
#include <GCF/GCF_PVInteger.h>
#include <Common/lofar_iostream.h>
#include "Echo_Protocol.ph"
#include <GCF/PAL/GCF_PVSSInfo.h>
#include "Echo.h"

using std::cout;
using std::endl;

namespace LOFAR 
{
 namespace GCF 
 {
using namespace Common;
using namespace TM;
  namespace PAL
  {
Echo::Echo(string name) : GCFTask((State)&Echo::initial, name) , _pService(0)
{
  // register the protocol for debugging purposes
  TM::registerProtocol(ECHO_PROTOCOL, ECHO_PROTOCOL_STRINGS);

  // initialize the port
  server.init(*this, "server", GCFPortInterface::SPP, ECHO_PROTOCOL);
}

Echo::~Echo()
{
  cout << "Deleting (SAL)echoapp" << endl;
  if (_pService) delete _pService;
}

GCFEvent::TResult Echo::initial(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_INIT:
    {
      break;
    }
    case F_ENTRY:
      server.open();
      break;

    case F_CONNECTED:
    {
      TRAN(Echo::connected);
      break;
    }
    case F_DISCONNECTED:
      server.setTimer(1.0); // try again after 1 second
      break;

    case F_TIMER:
      server.open(); // try again
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
  static string propName("Test_Prop");

  switch (e.signal)
  {
    case F_DISCONNECTED:
      if (_pService)
      {
        _pService->dpDelete(propName);
        _pService->dpDelete(propName + "_test");
      }
      cout << "Lost connection to client" << endl;
      p.close();
      break;
    case F_CLOSED:
      GCFTask::stop();
      break;

    case ECHO_PING:
    {
      EchoPingEvent ping(e);

      switch (ping.seqnr % 13)
      {
        case 0:
          if (_pService) delete _pService;
          _pService = new Service();
          _pService->dpCreate(propName, "ExampleDP_Bit");
          break;
        case 1:
        {
          _pService->dpCreate(propName + "_test", "ExampleDP_Int");
          vector<string> allProps;
          GCFPVSSInfo::getAllProperties("T*", "*", allProps);  
          break;
        }
        case 2:
          _pService->dpeSubscribe(propName);
          break;
        case 3:
          _pService->dpeSubscribe(propName + "_test");
          break;
        case 4:
          _pService->dpeGet(propName);
          break;
        case 5:
        {
          GCFPVBool wrongTestVal(true);
          _pService->dpeSet(propName + "_test", wrongTestVal, 0.0);
          GCFPVInteger goodTestVal(1000);
          _pService->dpeSet(propName + "_test", goodTestVal, 0.0);
          break;
        }
        case 6:
          _pService->dpeUnsubscribe(propName + "_test1");
          _pService->dpeUnsubscribe(propName + "_test");
          _pService->dpeUnsubscribe(propName);
          break;
        case 7:
        {
          GCFPVInteger testVal(2000);
          _pService->dpeSet(propName + "_test", testVal, 0.0, true);
          break;
        }
        case 8:
        {
          _pService->dpeSubscribe(propName);
          _pService->dpeUnsubscribe(propName);
          GCFPVBool testVal(true);
          _pService->dpeSet(propName, testVal, 0.0);
          break;
        }
        case 9:
          _pService->dpeSubscribe(propName);
          break;
        case 10:
          _pService->dpeGet(propName);
          _pService->dpeUnsubscribe(propName);
          break;
        case 11:
        {
          _pService->dpeSubscribe(propName);
          _pService->dpeUnsubscribe(propName);
          _pService->dpeSubscribe(propName);
          GCFPVBool testVal(false);
          _pService->dpeSet(propName, testVal, 0.0);
          break;
        }
        case 12:
          _pService->dpDelete(propName);
          _pService->dpDelete(propName + "_test");
          break;                
      }
      
      
      cout << "PING received (seqnr=" << ping.seqnr << ")" << endl;
      
      timeval echo_time;
      gettimeofday(&echo_time, 0);
      EchoEchoEvent echo;
      echo.seqnr = ping.seqnr;
      echo.ping_time = ping.ping_time;
      echo.echo_time = echo_time;
      
      server.send(echo);
      
      cout << "ECHO sent" << endl;
      break;
    }
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}
  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR

using namespace LOFAR::GCF;

int main(int argc, char* argv[])
{
  TM::GCFTask::init(argc, argv);

  PAL::Echo echo_task("ECHO");  
  echo_task.start(); // make initial transition
  
  TM::GCFTask::run();

  return 0;
}
