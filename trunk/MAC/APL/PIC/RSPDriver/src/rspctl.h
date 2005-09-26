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
#ifdef ENABLE_RSPFE
#include "RSPFE_Protocol.ph"
#endif
#include <GCF/TM/GCF_Control.h>
#include <GCF/TM/GCF_ETHRawPort.h>
#include <bitset>
#include <list>

#include <Timestamp.h>

#include <blitz/array.h>

namespace LOFAR {
  namespace rspctl {

    /**
     * Base class for control commands towards the RSPDriver.
     */
    class Command
    {
    public:
      virtual ~Command()
      {}

      /**
       * Send the command to the RSPDriver
       */
      virtual void send() = 0;

      /**
       * rspctl is stopped. perform cleanup code here
       */
      virtual void stop()
      {
      }

      /**
       * Check the acknowledgement sent by the RSPDriver.
       */
      virtual GCFEvent::TResult ack(GCFEvent& e) = 0;

      /**
       * Set seletable.
       */
      void setSelectable(bool selectable)
      {
        m_selectable = selectable;
      }

      /**
       * Get selectable.
       */
      bool getSelectable()
      {
        return m_selectable;
      }

      /**
       * Set selection.
       */
      void setSelect(std::list<int> select)
      {
        m_select = select;
      }

      /**
       * Get the mask (MAX_N_RCUS bits).
       */
      std::bitset<MAX_N_RCUS> getRCUMask() const
      {
        std::bitset<MAX_N_RCUS> mask;
      
        mask.reset();
        std::list<int>::const_iterator it;
        int count = 0; // limit to ndevices
        for (it = m_select.begin(); it != m_select.end(); ++it, ++count)
        {
          if (count >= get_ndevices())
            break;
          if (*it < MAX_N_RCUS)
            mask.set(*it);
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
        for (it = m_select.begin(); it != m_select.end(); ++it, ++count)
        {
          if (count >= get_ndevices())
            break;
          if (*it < MAX_N_TDS)
            mask.set(*it);
        }

        return mask;
      }

      /**
       * Set mode (true == get, false = set)
       */
      void setMode(bool get)
      {
        m_get = get;
      }

      /**
       * Get mode
       */
      bool getMode() const
      {
        return m_get;
      }

      /**
       * Set ndevices.
       */
      void set_ndevices(int ndevices)
      {
        m_ndevices = ndevices;
      }

      /**
       * Get ndevices.
       */
      int get_ndevices() const
      {
        return m_ndevices;
      }
  
      virtual void logMessage(ostream& stream, const string& message)
      {
        stream << message << endl;
      }

    protected:
      explicit Command(GCFPortInterface& port) : 
        m_rspport(port),
        m_select(),
        m_get(true), 
        m_selectable(true), 
        m_ndevices(0)
      {}
      Command(); // no default construction allowed

    protected:
      GCFPortInterface& m_rspport;

    private:
      std::list<int> m_select;
      bool           m_get; // get or set
      bool           m_selectable; // is selection possible?
      int            m_ndevices;
    };

    class FECommand : public Command
    {
    public:
      virtual ~FECommand()
      {}

      void setFrontEnd(string frontend)
      {
        string::size_type sep=frontend.find(':');
        m_host = frontend.substr(0,sep);
        m_port = atoi(frontend.substr(sep+1).c_str());
      }
  
      bool isFrontEndSet()
      {
        return (m_port != 0 && m_host.length() > 0);
      }
  
#ifdef ENABLE_RSPFE
      bool isConnected(GCFPortInterface& port)
      {
        return (&port == &m_feClient && m_feClient.isConnected());
      }
#else
      bool isConnected(GCFPortInterface&) { return false; }
#endif
  
#ifdef ENABLE_RSPFE
      void connect(GCFTask& task)
      {
        m_feClient.init(task, "client", GCFPortInterface::SAP, RSPFE_PROTOCOL);
        m_feClient.setHostName(m_host);
        m_feClient.setPortNumber(m_port);
        m_feClient.open();
      }
#else
      void connect(GCFTask&) {}
#endif
  
#ifdef ENABLE_RSPFE
      virtual void logMessage(ostream& stream, const string& message)
      {
        if(m_feClient.isConnected())
        {
          RSPFEStatusUpdateEvent statusUpdateEvent;
          statusUpdateEvent.status = message;
          m_feClient.send(statusUpdateEvent);
        }
        stream << message << endl;
      }
#else
      virtual void logMessage(ostream&stream , const string& message) {
        stream << message << endl;
      }
#endif
  
    protected:
      explicit FECommand(GCFPortInterface& port) : Command(port),m_host(""),m_port(0),m_feClient()
      {}
      FECommand(); // no default construction allowed
    private:
      string m_host;
      uint16 m_port;
      GCFTCPPort m_feClient;
    };

    class WeightsCommand : public Command
    {
    public:
      WeightsCommand(GCFPortInterface& port);
      virtual ~WeightsCommand()
      {}
      virtual void send();
      virtual GCFEvent::TResult ack(GCFEvent& e);
      void setValue(double value)
      {
        m_value = value;
      }
    private:
      double m_value;
    };

    class SubbandsCommand : public Command
    {
    public:
      SubbandsCommand(GCFPortInterface& port);
      virtual ~SubbandsCommand()
      {}
      virtual void send();
      virtual GCFEvent::TResult ack(GCFEvent& e);
      void setSubbandList(std::list<int> subbandlist)
      {
        m_subbandlist = subbandlist;
      }
      void setType(int type)
      {
	m_type = type;
      }
    private:
      std::list<int> m_subbandlist;
      int m_type;
    };

    class RCUCommand : public Command
    {
    public:
      RCUCommand(GCFPortInterface& port);
      virtual ~RCUCommand()
      {}
      virtual void send();
      virtual GCFEvent::TResult ack(GCFEvent& e);

      void setControl(uint8 control)
      {
        m_control = control;
      }
    private:
      uint8 m_control;
    };

    class WGCommand : public Command
    {
    public:
      WGCommand(GCFPortInterface& port);
      virtual ~WGCommand()
      {}
      virtual void send();
      virtual GCFEvent::TResult ack(GCFEvent& e);
      void setFrequency(double frequency)
      {
        m_frequency = frequency;
      }
      void setPhase(int phase)
      {
        m_phase = phase;
      }
      void setAmplitude(double amplitude)
      {
        m_amplitude = (uint8)(amplitude*(double)(1<<7)/100.0);
      }
    private:
      double m_frequency;
      uint8  m_phase;
      uint8  m_amplitude;
    };

    class StatusCommand : public Command
    {
    public:
      StatusCommand(GCFPortInterface& port);
      virtual ~StatusCommand()
      {}
      virtual void send();
      virtual GCFEvent::TResult ack(GCFEvent& e);
    private:
    };

    class StatisticsBaseCommand : public FECommand
    {
    public:
      StatisticsBaseCommand(GCFPortInterface& port);
      virtual ~StatisticsBaseCommand()
      {
        if(m_file)
        {
          delete[] m_file;
          m_file=0;
        }
      }
      void setDuration(uint16 duration)
      {
        m_duration=duration;
        m_endTime.setNow((double)m_duration);
      }
      void setIntegration(uint16 integration)
      {
        if(integration > 0)
          m_integration=integration;
      }
      void setDirectory(const char* dir)
      {
        m_directory = dir;
        if(dir[strlen(dir)-1] != '/')
        {
          m_directory += "/";
        }
      }
      FILE* getFile(int rcu, char* fileName)
      {
        if(!m_file)
        {
          m_file = new (FILE*)[get_ndevices()];
          if(!m_file)
          {
            logMessage(cerr,"Error: failed to allocate memory for file handles.");
            exit(EXIT_FAILURE);
          }
          memset(m_file,0,sizeof(FILE*)*get_ndevices());
        }
        if(!m_file[rcu])
        {
          m_file[rcu] = fopen(fileName, "w+");
        }
        if(!m_file[rcu])
        {
          logMessage(cerr,formatString("Error: Failed to open file: %s",fileName));
          exit(EXIT_FAILURE);
        }
        return m_file[rcu];
      }
    protected:
      uint32 m_subscriptionHandle;
      uint16 m_duration;
      RTC::Timestamp m_endTime;
      uint16 m_integration;
      uint16 m_nseconds;
      string m_directory;
      FILE** m_file; // array of file descriptors, one for each rcu
    private:
    };

    class StatisticsCommand : public StatisticsBaseCommand
    {
    public:
      StatisticsCommand(GCFPortInterface& port);
      virtual ~StatisticsCommand()
      {
      }
      virtual void send();
      virtual void stop();
      void capture_statistics(blitz::Array<double, 2>& stats, const RTC::Timestamp& timestamp);
      void plot_statistics(blitz::Array<double, 2>& stats, const RTC::Timestamp& timestamp);
      void dump_statistics(blitz::Array<double, 2>& stats, const RTC::Timestamp& timestamp);
      virtual GCFEvent::TResult ack(GCFEvent& e);
      void setType(uint8 type)
      {
        m_type = type;
      }
    private:
      uint8 m_type;
      blitz::Array<double, 2> m_stats;
    };

    class XCStatisticsCommand : public StatisticsBaseCommand
    {
    public:
      XCStatisticsCommand(GCFPortInterface& port);
      virtual ~XCStatisticsCommand()
      {
      }
      virtual void send();
      virtual void stop();
      void capture_xcstatistics(blitz::Array<std::complex<double>, 4>& stats, const RTC::Timestamp& timestamp);
      void plot_xcstatistics(blitz::Array<std::complex<double>, 4>& stats, const RTC::Timestamp& timestamp);
      void dump_xcstatistics(blitz::Array<std::complex<double>, 4>& stats, const RTC::Timestamp& timestamp);
      virtual GCFEvent::TResult ack(GCFEvent& e);
    private:
      blitz::Array<std::complex<double>, 4> m_stats;
    };

    class ClocksCommand : public Command
    {
    public:
      ClocksCommand(GCFPortInterface& port);
      virtual ~ClocksCommand()
      {}
      virtual void send();
      virtual GCFEvent::TResult ack(GCFEvent& e);

      void setClock(uint32 clock)
      {
        m_clock = clock;
      }
    private:
      uint32 m_clock;
    };

    class VersionCommand : public Command
    {
    public:
      VersionCommand(GCFPortInterface& port);
      virtual ~VersionCommand()
      {}
      virtual void send();
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
      std::list<int> strtolist(const char* str, int max);
      void logMessage(ostream& stream, const string& message);
  
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
};

#endif /* RSPCTL_H_ */
