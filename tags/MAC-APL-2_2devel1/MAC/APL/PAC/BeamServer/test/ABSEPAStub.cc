//#
//#  EPAStub.cc: implementation of EPAStub class
//#
//#  Copyright (C) 2002-2004
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

// this include needs to be first!
#define DECLARE_SIGNAL_NAMES
#include "ABS_Protocol.ph"
#include "EPA_Protocol.ph"

#include "ABSEPAStub.h"

#include "ABSBeam.h"
#include "ABSBeamlet.h"

#include <iostream>
#include <sys/time.h>
#include <string.h>

using namespace ABS;
using namespace std;

EPAStub::EPAStub(string name)
    : GCFTask((State)&EPAStub::initial, name)
{
  registerProtocol(ABS_PROTOCOL, ABS_PROTOCOL_signalnames);

  beam_server.init(*this, "client", GCFPortInterface::SAP, ABS_PROTOCOL);
}

EPAStub::~EPAStub()
{}

GCFEvent::TResult EPAStub::initial(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  cout << "initial received event on port " << port.getName() << endl;

  switch(e.signal)
  {
      case F_INIT:
      {
      }
      break;

      case F_ENTRY:
      {
	  beam_server.open();
      }
      break;

      case F_CONNECTED:
      {
	  cout << "port connected: " << port.getName() << endl;
	  TRAN(EPAStub::enabled);
      }
      break;

      case F_DISCONNECTED:
      {
	  cout << "port disconnected: " << port.getName() << endl;
	  port.setTimer((long)1);
	  port.close();
      }
      break;

      case F_TIMER:
      {
	  // try again
	  beam_server.open();
      }
      break;

      default:
	  status = GCFEvent::NOT_HANDLED;
	  break;
  }

  return status;
}

GCFEvent::TResult EPAStub::enabled(GCFEvent& e, GCFPortInterface& /*port*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (e.signal)
  {
      case F_ENTRY:
      {
      }
      break;

      default:
	  status = GCFEvent::NOT_HANDLED;
	  break;
  }

  return status;
}

int main(int argc, char** argv)
{
  GCFTask::init(argc, argv);

  EPAStub abs("ABS");

  abs.start(); // make initial transition

  GCFTask::run();

  return 0;
}
