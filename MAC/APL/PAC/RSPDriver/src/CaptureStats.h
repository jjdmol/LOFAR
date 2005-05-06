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

#include <GCF/TM/GCF_Control.h>
#include <GCF/TM/GCF_ETHRawPort.h>
#include <bitset>

class CaptureStats : public GCFTask
{
public:

  /**
   * Options for statistics capturing.
   */
  typedef struct {
    int                     type;
    std::bitset<MAX_N_RCUS> device_set;
    int                     n_devices;
    int                     duration;
    int                     integration;
    uint8                   rcucontrol;
    bool                    onefile;
    bool                    xinetd_mode;
  } Options;

  /**
   * The constructor of the CaptureStats task.
   * @param name The name of the task. The name is used for looking
   * up connection establishment information using the GTMNameService and
   * GTMTopologyService classes.
   */
  CaptureStats(string name, const Options& options);
  virtual ~CaptureStats();

  // state methods

  /**
   * The initial state. In this state a connection with the RSP
   * driver is attempted. When the connection is established,
   * a transition is made to the enabled state.
   */
  GCFEvent::TResult initial(GCFEvent& e, GCFPortInterface &p);

  /**
   * This state is used to wait for input (in xinetd_mode)
   */
  GCFEvent::TResult wait4command(GCFEvent& e, GCFPortInterface &p);

  /**
   * This state is used to perform a command.
   */
  GCFEvent::TResult handlecommand(GCFEvent& e, GCFPortInterface &p);

  /**
   * Load and integrate statistics
   */
  bool capture_statistics(blitz::Array<double, 2>& stats);

  /**
   * Write statistics to file
   */
  void output_statistics(blitz::Array<double, 2>& stats);

  /**
   * Run the tests.
   */
  void run();

private:
  // member variables

private:
  // ports
  GCFPort m_server;

  // options
  Options m_options;

  blitz::Array<double, 2> m_values;
  int m_nseconds;
  FILE** m_file;  // array of file descriptors
  string m_format; // format of xinetd output
  uint32 m_statushandle; // handle for status update subscripton
  uint32 m_statshandle;  // handle for stats update subscription
  char m_line[128]; // line buffer for getopt options
};
     
#endif /* CAPTURESTATS_H_ */
