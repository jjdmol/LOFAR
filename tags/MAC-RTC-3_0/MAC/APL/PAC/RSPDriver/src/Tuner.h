//#  -*- mode: c++ -*-
//#
//#  Tuner.h: class definition for the Tuner program
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

#ifndef TUNER_H_
#define TUNER_H_

#include <Suite/test.h>
#include <GCF/TM/GCF_Control.h>
#include <GCF/TM/GCF_ETHRawPort.h>

class Tuner : public GCFTask
{
public:
  /**
   * The constructor of the Tuner task.
   * @param name The name of the task. The name is used for looking
   * up connection establishment information using the GTMNameService and
   * GTMTopologyService classes.
   */
  Tuner(string name, std::vector<int> centersubbands,
        std::bitset<MAX_N_RCUS> device_set, int n_devices = 1,
	uint8 rcucontrol = 0xB9, bool initialize = false);
  virtual ~Tuner();

  // state methods

  /**
   * The initial state. In this state a connection with the RSP
   * driver is attempted. When the connection is established,
   * a transition is made to the enabled state.
   */
  GCFEvent::TResult initial(GCFEvent& e, GCFPortInterface &p);

  /**
   * Initialize the boards.
   */
  GCFEvent::TResult initialize(GCFEvent& e, GCFPortInterface &p);

  /**
   * Tune in to a specific subband.
   */
  GCFEvent::TResult tunein(GCFEvent& e, GCFPortInterface &p);

  /**
   * Run the tests.
   */
  void run();

private:
  // member variables

private:
  // ports
  GCFPort m_server;

  std::vector<int>        m_centersubbands;
  std::bitset<MAX_N_RCUS> m_device_set;
  int                     m_n_devices;
  uint8                   m_rcucontrol;
  bool                    m_initialize;
};
     
#endif /* TUNER_H_ */
