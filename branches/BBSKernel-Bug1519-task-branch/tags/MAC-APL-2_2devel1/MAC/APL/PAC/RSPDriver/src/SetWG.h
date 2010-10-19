//#  -*- mode: c++ -*-
//#
//#  SetWG.h: class definition for the SetWG program
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

#ifndef SETWG_H_
#define SETWG_H_

#include <Suite/test.h>
#include <GCF/TM/GCF_Control.h>
#include <GCF/TM/GCF_ETHRawPort.h>

class SetWG : public GCFTask, public Test
{
  public:
    /**
     * The constructor of the SetWG task.
     * @param name The name of the task. The name is used for looking
     * up connection establishment information using the GTMNameService and
     * GTMTopologyService classes.
     */
    SetWG(string name, int blp = 0, uint8 phase = 0, uint8 ampl = 0x40, double freq = 1.5625e6);
    virtual ~SetWG();

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
     * The final state which terminates the program.
     */
    GCFEvent::TResult final(GCFEvent& e, GCFPortInterface &p);

    /**
     * Use gnuplot to plot the statistics.
     */
    void plot_statistics(blitz::Array<std::complex<double>, 3>& stats);

    /**
     * Run the tests.
     */
    void run();

  private:
    // member variables

  private:
    // ports
    GCFPort m_server;

    int    m_blp;
    uint8  m_phase;
    uint8  m_ampl;
    double m_freq;
};
     
#endif /* SETWG_H_ */
