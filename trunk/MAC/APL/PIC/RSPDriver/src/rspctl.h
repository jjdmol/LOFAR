//#  -*- mode: c++ -*-
//#
//#  rspctl.h: command line interface to the RSPDriver
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

#ifndef RSPCTL_H_
#define RSPCTL_H_

#include "RSP_Protocol.ph"
#include <GCF/TM/GCF_Control.h>
#include <GCF/TM/GCF_ETHRawPort.h>
#include <bitset>
#include <list>

#include <Timestamp.h>

#include <blitz/array.h>

namespace rspctl
{

  /**
   * Base class for control commands towards the RSPDriver.
   */
  class Command
    {
    public:
      virtual ~Command() {}

      /**
       * Send the command to the RSPDriver
       */
      virtual void send(GCFPortInterface& port) = 0;

      /**
       * Check the acknowledgement sent by the RSPDriver.
       */
      virtual GCFEvent::TResult ack(GCFEvent& e) = 0;

      /**
       * Set seletable.
       */
      void setSelectable(bool selectable) { m_selectable = selectable; }

      /**
       * Get selectable.
       */
       bool getSelectable() { return m_selectable; }

      /**
       * Set selection.
       */
      void setSelect(std::list<int> select) { m_select = select; }

      /**
       * Get the mask (MAX_N_RCUS bits).
       */
      std::bitset<MAX_N_RCUS> getRCUMask() const
	{
	  std::bitset<MAX_N_RCUS> mask;

	  mask.reset();
	  std::list<int>::const_iterator it;
	  int count = 0; // limit to ndevices
	  for (it = m_select.begin(); it != m_select.end(); ++it, ++count) {
	    if (count >= get_ndevices()) break;
	    if (*it < MAX_N_RCUS) mask.set(*it);
	  }

	  return mask;
	}

      /**
       * Get the mask (MAX_N_TDS bits).
       */
      std::bitset<MAX_N_TDS> getTDMask() const
	{
	  std::bitset<MAX_N_TDS> mask;

	  mask.reset();
	  std::list<int>::const_iterator it;
	  int count = 0; // limit to ndevices
	  for (it = m_select.begin(); it != m_select.end(); ++it, ++count) {
	    if (count >= get_ndevices()) break;
	    if (*it < MAX_N_TDS) mask.set(*it);
	  }

	  return mask;
	}

      /**
       * Set mode (true == get, false = set)
       */
      void setMode(bool get) { m_get = get; }

      /**
       * Get mode
       */
      bool getMode() const { return m_get; }

      /**
       * Set ndevices.
       */
      void set_ndevices(int ndevices) { m_ndevices = ndevices; }

      /**
       * Get ndevices.
       */
      int get_ndevices() const { return m_ndevices; }

    protected:
      Command() : m_get(true), m_selectable(true), m_ndevices(0) {}

    private:
      std::list<int> m_select;
      bool           m_get; // get or set
      bool           m_selectable; // is selection possible?
      int            m_ndevices;
    };

  class WeightsCommand : public Command
    {
    public:
      WeightsCommand();
      virtual ~WeightsCommand() {}
      virtual void send(GCFPortInterface& port);
      virtual GCFEvent::TResult ack(GCFEvent& e);
      void setValue(double value) { m_value = value; }
    private:
      double m_value;
    };

  class SubbandsCommand : public Command
    {
    public:
      SubbandsCommand();
      virtual ~SubbandsCommand() {}
      virtual void send(GCFPortInterface& port);
      virtual GCFEvent::TResult ack(GCFEvent& e);
      void setSubbandList(std::list<int> subbandlist) { m_subbandlist = subbandlist; }
    private:
      std::list<int> m_subbandlist;
    };

  class RCUCommand : public Command
    {
    public:
      RCUCommand();
      virtual ~RCUCommand() {}
      virtual void send(GCFPortInterface& port);
      virtual GCFEvent::TResult ack(GCFEvent& e);

      void setControl(uint8 control) { m_control = control; }
    private:
      uint8 m_control;
    };

  class WGCommand : public Command
    {
    public:
      WGCommand();
      virtual ~WGCommand() {}
      virtual void send(GCFPortInterface& port);
      virtual GCFEvent::TResult ack(GCFEvent& e);
      void setFrequency(double frequency) { m_frequency = frequency; }
      void setPhase(int phase) { m_phase = phase; }
      void setAmplitude(double amplitude) { m_amplitude = (uint8)(amplitude*(double)(1<<7)/100.0); }
    private:
      double m_frequency;
      uint8  m_phase;
      uint8  m_amplitude;
    };

  class StatusCommand : public Command
    {
    public:
      StatusCommand();
      virtual ~StatusCommand() {}
      virtual void send(GCFPortInterface& port);
      virtual GCFEvent::TResult ack(GCFEvent& e);
    private:
    };

  class StatisticsCommand : public Command
    {
    public:
      StatisticsCommand();
      virtual ~StatisticsCommand() {}
      virtual void send(GCFPortInterface& port);
      void plot_statistics(blitz::Array<double, 2>& stats, const RTC::Timestamp& timestamp);
      virtual GCFEvent::TResult ack(GCFEvent& e);
      void setType(uint8 type) { m_type = type; }
    private:
      uint8 m_type;
    };

  class XCStatisticsCommand : public Command
    {
    public:
      XCStatisticsCommand();
      virtual ~XCStatisticsCommand() {}
      virtual void send(GCFPortInterface& port);
      void plot_xcstatistics(blitz::Array<std::complex<double>, 4>& stats, const RTC::Timestamp& timestamp);
      virtual GCFEvent::TResult ack(GCFEvent& e);
    private:
    };

  class ClocksCommand : public Command
    {
    public:
      ClocksCommand();
      virtual ~ClocksCommand() {}
      virtual void send(GCFPortInterface& port);
      virtual GCFEvent::TResult ack(GCFEvent& e);

      void setClock(uint32 clock) { m_clock = clock; }
    private:
      uint32 m_clock;
    };

  class VersionCommand : public Command
    {
    public:
      VersionCommand();
      virtual ~VersionCommand() {}
      virtual void send(GCFPortInterface& port);
      virtual GCFEvent::TResult ack(GCFEvent& e);
    private:
    };

  /**
   * Controller class for rspctl
   */
  class RSPCtl : public GCFTask
    {
    public:

      /**
       * The constructor of the RSPCtl task.
       * @param name The name of the task. The name is used for looking
       * up connection establishment information using the GTMNameService and
       * GTMTopologyService classes.
       */
      RSPCtl(string name, int argc, char** argv);
      virtual ~RSPCtl();

      // state methods

      /**
       * The initial state. In this state a connection with the RSP
       * driver is attempted. When the connection is established,
       * a transition is made to the connected state.
       */
      GCFEvent::TResult initial(GCFEvent& e, GCFPortInterface &p);

      /**
       * In this state the command is sent and the acknowledge handled.
       * Any relevant output is printed.
       */
      GCFEvent::TResult docommand(GCFEvent& e, GCFPortInterface &p);

      /**
       * Start the controller main loop.
       */
      void mainloop();

    private:
      // private methods
      Command* parse_options(int argc, char** argv);

    private:
      // ports
      GCFPort m_server;

      // the command to execute
      Command* m_command;

      // dimensions of the connected hardware
      int m_nrcus;
      int m_nrspboards;
      int m_ntdboards;

      // commandline parameters
      int    m_argc;
      char** m_argv;
    };

};

#endif /* RSPCTL_H_ */
