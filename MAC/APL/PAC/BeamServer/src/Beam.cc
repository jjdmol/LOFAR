//#  ABSBeamServerTask.h: class definition for the Beam Server task.
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

#include <ABSBeamServerTask.h>
#define DECLARE_SIGNAL_NAMES
#include "ABS_Protocol.ph"

#include <iostream>

using namespace ABS;
using namespace std;

BeamServerTask::BeamServerTask(string name)
    : GCFTask((State)&BeamServerTask::initial, name)
{
  registerProtocol(ABS_PROTOCOL, ABS_PROTOCOL_signalnames);

  client.init(*this, "client", GCFPortInterface::SPP, ABS_PROTOCOL);
  board.init(*this, "board", GCFPortInterface::SAP, 0);
}

GCFEvent::TResult BeamServerTask::initial(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  cout << "initial received event on port " << port.getName() << endl;

  switch(e.signal)
  {
      case F_INIT_SIG:
	  cout << "Hello, world!" << endl;
	  break;

      case F_ENTRY_SIG:
	  client.open();
	  break;

      case F_CONNECTED_SIG:
	  TRAN(BeamServerTask::enabled);
	  break;

      case F_DISCONNECTED_SIG:
	  exit(EXIT_FAILURE);
	  break;

      default:
	  status = GCFEvent::NOT_HANDLED;
	  break;
  }

  return status;
}

GCFEvent::TResult BeamServerTask::enabled(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  cout << "enabled event on port " << port.getName() << endl;
  
  switch (e.signal)
  {
#if 0
      case F_ACCEPT_REQ_SIG:
	  client.getPortProvider().accept();
	  break;
#endif

      case F_DATAIN_SIG:
	  cout << "datain" << endl;
	  break;

      case F_DATAOUT_SIG:
	  cout << "dataout" << endl;
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

  BeamServerTask abs("ABS");

  abs.start(); // make initial transition

  GCFTask::run();

  return 0;
}
