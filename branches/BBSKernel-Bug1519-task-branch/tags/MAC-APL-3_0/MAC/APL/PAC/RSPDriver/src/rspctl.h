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
#include <set>

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
       * Set selection.
       */
      void setSelect(std::set<int> select) { m_select = select; }

      /**
       * Get the RCU mask.
       */
      std::bitset<MAX_N_RCUS> getRCUMask() const
	{
	  std::bitset<MAX_N_RCUS> rcumask;

	  rcumask.reset();
	  std::set<int>::iterator it;
	  for (it = m_select.begin(); it != m_select.end(); it++)
	    {
	      if (*it < MAX_N_RCUS) rcumask.set(*it);
	    }

	  return rcumask;
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
       * Set nrcus.
       */
      void set_nrcus(int nrcus) { m_nrcus = nrcus; }

      /**
       * Get nrcus.
       */
      int get_nrcus() const { return m_nrcus; }

    protected:
      Command() : m_get(true) {}

    private:
      std::set<int> m_select;
      bool          m_get; // get or set
      int           m_nrcus;
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
      void setSubbandSet(std::set<int> subbandset) { m_subbandset = subbandset; }
    private:
      std::set<int> m_subbandset;
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
    private:
      double m_frequency;
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
      virtual GCFEvent::TResult ack(GCFEvent& e);
      void setType(uint8 type) { m_type = type; }
    private:
      uint8 m_type;
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
      RSPCtl(string name, Command& command);
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
      // member variables

    private:
      // ports
      GCFPort m_server;

      // the command to execute
      Command& m_command;
    };

};

#endif /* RSPCTL_H_ */
