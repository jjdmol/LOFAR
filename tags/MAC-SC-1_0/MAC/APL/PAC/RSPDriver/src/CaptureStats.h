//#  -*- mode: c++ -*-
//#
//#  CaptureStats.h: class definition for the CaptureStats program
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

#ifndef CAPTURESTATS_H_
#define CAPTURESTATS_H_

#include <Suite/test.h>
#include <GCF/TM/GCF_Control.h>
#include <GCF/TM/GCF_ETHRawPort.h>

class CaptureStats : public GCFTask, public Test
{
  public:
    /**
     * The constructor of the CaptureStats task.
     * @param name The name of the task. The name is used for looking
     * up connection establishment information using the GTMNameService and
     * GTMTopologyService classes.
     */
    CaptureStats(string name, int type, std::bitset<MAX_N_RCUS> device_set, int n_devices = 1,
		 int duration = 1, int integration = 1, uint8 rcucontrol = 0xB9, bool onfile = false);
    virtual ~CaptureStats();

    // state methods

    /**
     * The initial state. In this state a connection with the RSP
     * driver is attempted. When the connection is established,
     * a transition is made to the enabled state.
     */
    GCFEvent::TResult initial(GCFEvent& e, GCFPortInterface &p);

    /**
     * The test states. This state is reached when the
     * beam_server port is connected.
     */
    GCFEvent::TResult enabled(GCFEvent& e, GCFPortInterface &p);

    /**
     * Load and integrate statistics
     */
    void capture_statistics(blitz::Array<double, 2>& stats);

    /**
     * Write statistics to file
     */
    void write_statistics(blitz::Array<double, 2>& stats);

    /**
     * Run the tests.
     */
    void run();

  private:
    // member variables

  private:
    // ports
    GCFPort m_server;
    int m_type;
    std::bitset<MAX_N_RCUS> m_device_set;
    int m_n_devices;
    int m_duration;
    int m_integration;
    uint8 m_rcucontrol;

    blitz::Array<double, 2> m_values;
    int m_nseconds;
    FILE** m_file;  // array of file descriptors
    bool m_onefile; // output one big file? if false output separate file for each capture
};
     
#endif /* CAPTURESTATS_H_ */
