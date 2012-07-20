//#  -*- mode: c++ -*-
//#
//#  trigctl.h: command line interface to the TBBControl external trigger input
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
//#  $Id: trigctl.h 17837 2011-04-22 07:56:37Z mol $

#ifndef _TRIGCTL_H_
#define _TRIGCTL_H_

#include <APL/CR_Protocol/CR_Protocol.ph>
#include <APL/RTCCommon/NsTimestamp.h>
#include <GCF/TM/GCF_Control.h>
#include <GCF/RTDB/GCF_RTDBPort.h>
#include <GCF/TM/GCF_TimerPort.h>
#include <Common/lofar_bitset.h>
#include <Common/lofar_list.h>
#include <Common/lofar_string.h>
#include <cstdio>


namespace LOFAR {
  using GCF::TM::GCFTask;
  using GCF::TM::GCFPort;
  using GCF::TM::GCFTimerPort;
  using GCF::TM::GCFPortInterface;
  namespace TrigCtl {

GCFTimerPort* itsCmdTimer;

static const int TRIGCTL_VERSION = 001;

//-----------------------------------------------------------------------------
// class Command :base class for control commands towards the TBBDriver.
//
class Command
{

public:
    virtual ~Command(){}

    // Send the command to the RTDB database.
    virtual void send() = 0;

    // Check the acknowledgement sent by the RTDB database.
    virtual GCFEvent::TResult ack(GCFEvent& e) = 0;

    //--------------------------------------------------------
    virtual void logMessage(ostream& stream, const string& message)
    {
      stream << message << endl;
    }

    //--------------------------------------------------------
    void setCmdDone(bool done)
    {
        itsCmdDone = done;
    }

    //--------------------------------------------------------
    bool isCmdDone() const
    {
        return(itsCmdDone);
    }
     //--set selection-----------------------------------------
    void setRcuStr(string optarg)
    {
       itsRcuStr.clear();
       itsRcuStr.append("["+optarg+"]");
    }
    
protected:
    explicit Command(GCF::RTDB::GCFRTDBPort& port, GCFTimerPort& timer) :
    itsPort(port),
    itsTimer(timer),
    itsCmdDone(false)
    {
        itsRcuStr.clear();
        itsRcuStr.append("[]");
    }
    Command(); // no default construction allowed

protected:
    GCF::RTDB::GCFRTDBPort& itsPort;
    GCFTimerPort& itsTimer;
    bool itsCmdDone;
    string itsRcuStr;

private:

}; // end class Command


//-----------------------------------------------------------------------------
class StopCmd : public Command
{
public:
    StopCmd(GCF::RTDB::GCFRTDBPort& port, GCFTimerPort& timer);
    virtual ~StopCmd() { }
    virtual void send();
    virtual GCFEvent::TResult ack(GCFEvent& e);
    void setStopTime(RTC::NsTimestamp time) { itsStopTime = time; }
private:
    RTC::NsTimestamp itsStopTime;
};

//-----------------------------------------------------------------------------
class ReadCmd : public Command
{
public:
    ReadCmd(GCF::RTDB::GCFRTDBPort& port, GCFTimerPort& timer);
    virtual ~ReadCmd() { }
    virtual void send();
    virtual GCFEvent::TResult ack(GCFEvent& e);
    void setTime(RTC::NsTimestamp time) { itsReadTime = time; }
    void setTimeBefore(RTC::NsTimestamp time) { itsTimeBefore = time; }
    void setTimeAfter(RTC::NsTimestamp time) { itsTimeAfter = time; }
private:
    RTC::NsTimestamp itsReadTime;
    RTC::NsTimestamp itsTimeBefore;
    RTC::NsTimestamp itsTimeAfter;
};

//-----------------------------------------------------------------------------
class RecordCmd : public Command
{
public:
    RecordCmd(GCF::RTDB::GCFRTDBPort& port, GCFTimerPort& timer);
    virtual ~RecordCmd() { }
    virtual void send();
    virtual GCFEvent::TResult ack(GCFEvent& e);
private:
};

//-----------------------------------------------------------------------------
class CepSpeedCmd : public Command
{
public:
    CepSpeedCmd(GCF::RTDB::GCFRTDBPort& port, GCFTimerPort& timer);
    virtual ~CepSpeedCmd() { }
    virtual void send();
    virtual GCFEvent::TResult ack(GCFEvent& e);
    void setDelay(uint32 delay) { itsDelay = delay; }
    void setDatapaths(uint32 datapaths) { itsDatapaths = datapaths; }
private:
    uint32 itsDelay;
    uint32 itsDatapaths;
};

//-----------------------------------------------------------------------------
class StopDumpsCmd : public Command
{
public:
    StopDumpsCmd(GCF::RTDB::GCFRTDBPort& port, GCFTimerPort& timer);
    virtual ~StopDumpsCmd() { }
    virtual void send();
    virtual GCFEvent::TResult ack(GCFEvent& e);
private:
};

//-----------------------------------------------------------------------------
class VhecrEnableCmd : public Command
{
public:
    VhecrEnableCmd(GCF::RTDB::GCFRTDBPort& port, GCFTimerPort& timer);
    virtual ~VhecrEnableCmd() { }
    virtual void send();
    virtual GCFEvent::TResult ack(GCFEvent& e);
    void setState(uint32 state) { itsState = state; }
private:
    uint32 itsState;
};


//-----------------------------------------------------------------------------
// Controller class for etrigctl
// class TRIGCtl
//
class TRIGCtl : public GCFTask
{
public:

    /**
    * The constructor of the TBBCtl task.
    * @param name The name of the task. The name is used for looking
    * up connection establishment information using the GTMNameService and
    * GTMTopologyService classes.
    */
    TRIGCtl(string name, int argc, char** argv);
    virtual ~TRIGCtl();

    // state methods

    /**
    * The initial state. In this state a connection with the TBB
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
    
    static void sigintHandler (int signum);
    void finish();
    
    void logMessage(ostream& stream, const string& message);
    void commandHelp();
private:
    GCF::RTDB::GCFRTDBPort*  itsRTDBPort;
    GCFTimerPort*            itsTimerPort;
    Command*                 itsCommand; // the command to execute

    // commandline parameters
    int    itsArgc;
    char** itsArgv;
};

    } // end namespace etrigctl
} // end namespace LOFAR

#endif /* _ETRIGCTL_H_ */
