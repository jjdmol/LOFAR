//#
//#  ABSBeamServerTask.cc: implementation of ABSBeamServerTask class
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

#include "ABSBeamServerTask.h"

#include "ABSBeam.h"
#include "ABSBeamlet.h"

#include <iostream>

using namespace ABS;
using namespace std;

BeamServerTask::BeamServerTask(string name)
    : GCFTask((State)&BeamServerTask::initial, name)
{
  registerProtocol(ABS_PROTOCOL, ABS_PROTOCOL_signalnames);

  client.init(*this, "client", GCFPortInterface::SPP, ABS_PROTOCOL);
  board.init(*this, "board", GCFPortInterface::SAP, 0);

  (void)Beam::setNInstances(ABS_Protocol::N_BEAMLETS);
  (void)Beamlet::setNInstances(ABS_Protocol::N_SUBBANDS);
}

BeamServerTask::~BeamServerTask()
{}

GCFEvent::TResult BeamServerTask::initial(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  cout << "initial received event on port " << port.getName() << endl;

  switch(e.signal)
  {
      case F_INIT_SIG:
      {
	  cout << "Hello, world!" << endl;

	  // create a spectral window from 10MHz to 90Mhz
	  // steps of 256kHz
	  SpectralWindow* spw = new SpectralWindow(10e6, 256*1e3, 80*(1000/256));
	  m_spws[0] = spw;

	  // create subband set
	  std::set<int> subbands;

	  // to store the subband selection
	  map<int,int> selection;

	  //
	  // setup beam 0
          //
	  Beam* beam = Beam::getInstance(0);

	  subbands.insert(0);
	  subbands.insert(10);
	  subbands.insert(20);
	  
	  if (beam->allocate(*m_spws[0], subbands) < 0)
	  {
	      cerr << "failed to allocate beam 0" << endl;
	  }
	  else
	  {
	      beam->getSubbandSelection(selection);
	  }

	  //
	  // setup beam 1
	  //
	  beam = Beam::getInstance(1);

	  subbands.clear();
	  subbands.insert(30);
	  subbands.insert(22);
	  subbands.insert(44);
	  subbands.insert(101);
	  subbands.insert(121);
	  subbands.insert(141);

	  if (beam->allocate(*m_spws[0], subbands) < 0)
	  {
	      cerr << "failed to allocate beam 1" << endl;
	  }
	  else
	  {
	      beam->getSubbandSelection(selection);
	  }

	  beam = Beam::getInstance(2);
	  if (beam->allocate(*m_spws[0], subbands) < 0)
	  {
	      cerr << "failed to allocate beam 2" << endl;
	  }
	  else
	  {
	      beam->getSubbandSelection(selection);
	  }

	  //
	  // show total mapping for beam 0 and 1
	  //
	  for (map<int,int>::iterator sel = selection.begin();
	       sel != selection.end(); ++sel)
	  {
	      cout << "(" << sel->first
		   << "," << sel->second << ")" << endl;
	  }

	  beam->deallocate();
      }
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

      case ABS_BEAMALLOC:
      {
	  ABSBeamallocEvent* event = static_cast<ABSBeamallocEvent*>(&e);
	  beamalloc_action(event, port);
      }
      break;

      case ABS_BEAMFREE:
      {
	  ABSBeamfreeEvent* event = static_cast<ABSBeamfreeEvent*>(&e);
	  beamfree_action(event, port);
      }
      break;

      case ABS_BEAMPOINTTO:
      {
	  ABSBeampointtoEvent* event = static_cast<ABSBeampointtoEvent*>(&e);
	  beampointto_action(event, port);
      }
      break;

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

void BeamServerTask::beamalloc_action(ABSBeamallocEvent* ba,
				      GCFPortInterface& port)
{
  int   spwindex = 0;
  Beam* beam = 0;
  ABSBeamalloc_AckEvent ack(ba->beam_index, STATUS_SUCCESS);

  if (!(beam = Beam::getInstance(ba->beam_index))
      || ((spwindex = ba->spectral_window) != 0)
      || !(ba->n_subbands > 0 && ba->n_subbands < N_BEAMLETS))
  {
      ack.status = STATUS_ERR_RANGE;
      port.send(ack);
      return;                         // RETURN
  }

  set<int> subbands;
  for (int i = 0; i < ba->n_subbands; i++)
  {
      subbands.insert(ba->subbands[i]);

      // allocate the beam
      if (beam->allocate(*m_spws[spwindex], subbands) < 0)
      {
	  ack.status = STATUS_ERR_BEAMALLOC;
	  port.send(ack);
      }

      m_beams.insert(ba->beam_index);
  }

  port.send(ack);
}

void BeamServerTask::beamfree_action(ABSBeamfreeEvent* bf,
				     GCFPortInterface& port)
{
  ABSBeamfree_AckEvent ack(bf->beam_index, STATUS_SUCCESS);

  Beam* beam = 0;
  if (!(beam = Beam::getInstance(bf->beam_index)))
  {
      ack.status = STATUS_ERR_RANGE;
      port.send(ack);
      return;                      // RETURN
  }

  if (beam->deallocate() < 0)
  {
      ack.status = STATUS_ERR_BEAMFREE;
      port.send(ack);
      return;                     // RETURN
  }

  port.send(ack);
}

void BeamServerTask::beampointto_action(ABSBeampointtoEvent* pt,
					GCFPortInterface& /*port*/)
{
  Beam* beam = Beam::getInstance(pt->beam_index);

  if (beam)
  {
      beam->addPointing(Pointing(Direction(pt->angle1,
					   pt->angle2,
					   (Direction::Types)pt->type),
				 pt->time));
  }
  else cerr << "invalid beam_index in BEAMPOINTTO" << endl;
}

int main(int argc, char** argv)
{
  GCFTask::init(argc, argv);

  BeamServerTask abs("ABS");

  abs.start(); // make initial transition

  GCFTask::run();

  return 0;
}
