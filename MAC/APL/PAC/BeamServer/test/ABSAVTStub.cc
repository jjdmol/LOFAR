//#
//#  AVTStub.cc: implementation of AVTStub class
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

#include "suite.h"
#define DECLARE_SIGNAL_NAMES
#include "ABS_Protocol.ph"

#include "ABSAVTStub.h"

#include <iostream>
#include <sys/time.h>
#include <string.h>

using namespace ABS;
using namespace std;

AVTStub::AVTStub(string name)
    : GCFTask((State)&AVTStub::initial, name), Test("AVTStub", &cout)
{
  registerProtocol(ABS_PROTOCOL, ABS_PROTOCOL_signalnames);

  beam_server.init(*this, "beam_server", GCFPortInterface::SAP, ABS_PROTOCOL);
}

AVTStub::~AVTStub()
{}

GCFEvent::TResult AVTStub::initial(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  cout << "initial received event on port " << port.getName() << endl;

  switch(e.signal)
  {
      case F_INIT_SIG:
      {
      }
      break;

      case F_ENTRY_SIG:
      {
	  beam_server.open();
      }
      break;

      case F_CONNECTED_SIG:
      {
	  cout << "port connected: " << port.getName() << endl;
	  TRAN(AVTStub::enabled);
      }
      break;

      case F_DISCONNECTED_SIG:
      {
	  cout << "port disconnected: " << port.getName() << endl;
	  port.setTimer((long)1);
      }
      break;

      case F_TIMER_SIG:
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

GCFEvent::TResult AVTStub::enabled(GCFEvent& e, GCFPortInterface& /*port*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (e.signal)
  {
      case F_ENTRY_SIG:
      {
      }
      break;

      default:
	  status = GCFEvent::NOT_HANDLED;
	  break;
  }

  return status;
}

void AVTStub::run()
{
  start(); // make initial transition
  GCFTask::run();
}

int main(int argc, char** argv)
{
  GCFTask::init(argc, argv);

  Suite s("Beam Server Process Test Suite", &cout);
  s.addTest(new AVTStub("AVTStub"));
  s.run();
  long nFail = s.report();
  s.free();
  return nFail;
}
