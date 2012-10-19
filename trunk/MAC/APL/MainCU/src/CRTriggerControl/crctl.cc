//#
//#  crctl.cc: command line interface to TriggerControl
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
//#  $Id: crctl.cc 17606 2011-03-22 12:49:57Z schoenmakers $
//#


#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>
#include <Common/Exception.h>

#include <MACIO/MACServiceInfo.h>

#include <Common/lofar_iostream.h>
#include <Common/lofar_sstream.h>
#include <getopt.h>

#include <Common/lofar_string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <Common/lofar_set.h>
#include <time.h>

//#include <APL/RTCCommon/gnuplot_i.h>

#include <netinet/in.h>
#include <net/ethernet.h>
#include <cstdio>
#include <signal.h>

#include "crctl.h"

using namespace std;
//using namespace blitz;
using namespace LOFAR;
using namespace GCF::TM;
using namespace CR_Protocol;
using namespace CR_Ctl;

// Use a terminate handler that can produce a backtrace.
Exception::TerminateHandler t(Exception::terminate);

static const double DELAY = 60.0;

// static pointer to this object for signalhandler
static CRCtl *thisCRCtl = 0;

//---- STOP  ----------------------------------------------------------------
StopCmd::StopCmd(GCFPortInterface& port, GCFTimerPort& timer) : Command(port, timer)
{
    cout << endl;
    cout << "== RTDB ================================================= stop tbb recording ====" << endl;
    cout << endl;
}

//-----------------------------------------------------------------------------
// example from tCR_Protocol.cc
//CRstopRequest    CR1("[CS001,CS002..CS006]", "[0..191]", NsTimestamp(676767.2223));
void StopCmd::send()
{
    CRStopEvent event;
    
    event.triggerID = 50001;
    event.observationID = itsObservationID;
    event.stopVector.requests.push_back(CRstopRequest(itsStationStr, itsRcuStr, itsStopTime));
    itsPort.send(event);
    itsTimer.setTimer(DELAY);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult StopCmd::ack(GCFEvent& e)
{
    CRStopAckEvent ack(e);
    
    if (ack.result == CR_NO_ERR) {
        cout << "STOP command arrived add TBBControl" << endl;
        cout << "for Station: " << itsStationStr << endl;
        cout << "for RCU    : " << itsRcuStr << endl;
        cout << "recording stopped at: " << (double)itsStopTime << " sec" << endl;
    }
    else {
        cout << "Error in STOP command" << errorName(ack.result) << endl;
    }
    setCmdDone(true);

    return(GCFEvent::HANDLED);
}

//---- READ  ----------------------------------------------------------------
ReadCmd::ReadCmd(GCFPortInterface& port, GCFTimerPort& timer) : Command(port, timer)
{
    cout << endl;
    cout << "== CRTriggerControl =============================== read tbb data(cep) ====" << endl;
    cout << endl;
}

//-----------------------------------------------------------------------------
// example from tCR_Protocol.cc
//CRstopRequest    CR1("[CS001,CS002..CS006]", "[0..191]", NsTimestamp(676767.2223));
void ReadCmd::send()
{
    CRReadEvent event;
    
    event.triggerID = 50001;
    event.observationID = itsObservationID;
    event.readVector.requests.push_back(CRreadRequest(itsStationStr,
                                                      itsRcuStr,
                                                      itsReadTime,
                                                      itsTimeBefore,
                                                      itsTimeAfter) );
    itsPort.send(event);
    itsTimer.setTimer(DELAY);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult ReadCmd::ack(GCFEvent& e)
{
    CRReadAckEvent ack(e);
    
    if (ack.result == CR_NO_ERR) {
        cout << "READ command arrived add TBBControl" << endl;
        cout << "for Station: " << itsStationStr << endl;
        cout << "for RCU    : " << itsRcuStr << endl;
        cout << "read time  : " << (double)itsReadTime << " sec" << endl;
        cout << "time before: " << (double)itsTimeBefore << " sec" << endl;
        cout << "time after : " << (double)itsTimeAfter << " sec" << endl;
    }
    else {
        cout << "Error in READ command" << errorName(ack.result) << endl;
    }
    setCmdDone(true);

    return(GCFEvent::HANDLED);
}

//---- RECORD  ----------------------------------------------------------------
RecordCmd::RecordCmd(GCFPortInterface& port, GCFTimerPort& timer) : Command(port, timer)
{
    cout << endl;
    cout << "== CRTriggerControl ===================================== start record ====" << endl;
    cout << endl;
}

//-----------------------------------------------------------------------------
// example from tCR_Protocol.cc
//CRstopRequest    CR1("[CS001,CS002..CS006]", "[0..191]", NsTimestamp(676767.2223));
void RecordCmd::send()
{
    CRRecordEvent event;
    
    event.triggerID = 50001;
    event.observationID = itsObservationID;
    event.recordVector.requests.push_back(CRrecordRequest(itsStationStr,
                                                          itsRcuStr) );
    itsPort.send(event);
    itsTimer.setTimer(DELAY);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult RecordCmd::ack(GCFEvent& e)
{
    CRRecordAckEvent ack(e);
    
    if (ack.result == CR_NO_ERR) {
        cout << "RECORD command arrived add TBBControl" << endl;
        cout << "for Station: " << itsStationStr << endl;
        cout << "for RCU    : " << itsRcuStr << endl;
    }
    else {
        cout << "Error in RECORD command" << errorName(ack.result) << endl;
    }
    setCmdDone(true);

    return(GCFEvent::HANDLED);
}

//---- CEPSPEED  ----------------------------------------------------------------
CepSpeedCmd::CepSpeedCmd(GCFPortInterface& port, GCFTimerPort& timer) : Command(port, timer)
{
    cout << endl;
    cout << "== CRTriggerControl ==================================== set cep speed ====" << endl;
    cout << endl;
}

//-----------------------------------------------------------------------------
// example from tCR_Protocol.cc
//CRstopRequest    CR1("[CS001,CS002..CS006]", "[0..191]", NsTimestamp(676767.2223));
void CepSpeedCmd::send()
{
    CRCepSpeedEvent event;
    
    event.triggerID = 50001;
    event.observationID = itsObservationID;
    event.stationList = itsStationStr;
    event.cepDelay = itsDelay;
    event.cepDatapaths = itsDatapaths;

    itsPort.send(event);
    itsTimer.setTimer(DELAY);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult CepSpeedCmd::ack(GCFEvent& e)
{
    CRCepSpeedAckEvent ack(e);
    
    if (ack.result == CR_NO_ERR) {
        cout << "CEPSPEED command arrived add TBBControl" << endl;
        cout << "for Station: " << itsStationStr << endl;
    }
    else {
        cout << "Error in CEPSPEED command" << errorName(ack.result) << endl;
    }
    setCmdDone(true);

    return(GCFEvent::HANDLED);
}

//---- STOPDUMPS  ----------------------------------------------------------------
StopDumpsCmd::StopDumpsCmd(GCFPortInterface& port, GCFTimerPort& timer) : Command(port, timer)
{
    cout << endl;
    cout << "== CRTriggerControl ======================================= stop dumps ====" << endl;
    cout << endl;
}

//-----------------------------------------------------------------------------
// example from tCR_Protocol.cc
//CRstopRequest    CR1("[CS001,CS002..CS006]", "[0..191]", NsTimestamp(676767.2223));
void StopDumpsCmd::send()
{
    CRStopDumpsEvent event;
    
    event.triggerID = 50001;
    event.observationID = itsObservationID;
    event.stationList = itsStationStr;
    itsPort.send(event);
    itsTimer.setTimer(DELAY);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult StopDumpsCmd::ack(GCFEvent& e)
{
    CRStopDumpsAckEvent ack(e);
    
    if (ack.result == CR_NO_ERR) {
        cout << "STOPDUMPS command arrived add TBBControl" << endl;
        cout << "for Station: " << itsStationStr << endl;
    }
    else {
        cout << "Error in STOPDUMPS command" << errorName(ack.result) << endl;
    }
    setCmdDone(true);

    return(GCFEvent::HANDLED);
}


//====END OF ETRIG COMMANDS==========================================================================


//---- HELP --------------------------------------------------------------------
void CRCtl::commandHelp()
{
    cout << endl;
    cout << endl;
    cout << "> > > > crctl COMMAND USAGE > > > > > > > > > > > > > > > > > > > > > > > > > > > > > > > > > > > > > > > > >" << endl;
    cout << endl;
    cout << " #  --command                   : all boards or active rcu's are selected, and will be displayed" << endl;
    cout << " #  --command --select=<...set> : only information for all selected boards or rcu's is displayed" << endl;
    cout << " #    Example: --select=0,1,4  or  --select=0..6  or  --select=0,1,2,8..11" << endl;
    cout << endl;
    cout << " crctl --stop=time --obsid=observationId [--station=<stations>][--rcu=<rcuset>]" << endl;
    cout << "         # stop recording at given time for selected observation" << endl;
    cout << " crctl --read=time,timebefore, timeafter --obsid=observationId [--station=<stations>][--select=<rcuset>]" << endl;
    cout << "         # start dumping data to cep for given time and span" << endl;
    cout << " crctl --record --obsid=observationId [--station=<stations>][--rcu=<rcuset>]" << endl;
    cout << "         # start recording for selected observation" << endl;
    cout << " crctl --cepspeed=delay,datapaths --obsid=observationId [--station=<stations>]" << endl;
    cout << "         # set cep speed settings for selected observation" << endl;
    cout << "         # delay = delay between frames in nSec (1=5nSec)" << endl;
    cout << "         # datapaths = number of datapaths to use 1..6" << endl;
    cout << " crctl --stopdumps --obsid=observationId [--station=<stations>]" << endl;
    cout << "         # stop dumping data to cep for selected observation" << endl;
    cout << endl;
    cout << " for all optional arguments:  if [--station=<stations>] is not used all stations are selected" << endl;
    cout << "                              if [--rcu=<rcus>] is not used all rcus are selected" << endl;
      
    cout << " crctl --help                                                # this help screen" << endl;
    cout << "< < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < " << endl;
    cout << endl;
}
//-----------------------------------------------------------------------------

//====END OF COMMANDS==========================================================================
//-----------------------------------------------------------------------------
CRCtl::CRCtl(string name, int argc, char** argv): GCFTask((State)&CRCtl::initial, name),
    itsCommand(0),itsArgc(argc),itsArgv(argv)
{
    registerProtocol (CR_PROTOCOL, CR_PROTOCOL_STRINGS);
    itsCRTrigPort   = new GCFTCPPort(*this, MAC_SVCMASK_TRIGGERCTRL, GCFPortInterface::SAP, CR_PROTOCOL);
    ASSERTSTR(itsCRTrigPort, "Can't allocate itsCRTrigPort");
    
    itsTimerPort = new GCFTimerPort(*this, "timerPort");
}

//-----------------------------------------------------------------------------
CRCtl::~CRCtl()
{
    if (itsCommand) { delete itsCommand; }
    if (itsCRTrigPort) { delete itsCRTrigPort; }
    if (itsTimerPort) { delete itsTimerPort; }
}

//
// sigintHandler(signum)
//
void CRCtl::sigintHandler(int signum)
{
    LOG_DEBUG (formatString("SIGINT signal detected (%d)",signum));

    if (thisCRCtl) {
        thisCRCtl->finish();
    }
}

//
// finish
//
void CRCtl::finish()
{
    cout << "crctl stopped by user" << endl;
    sleep(1);
    GCFScheduler::instance()->stop();
    //TRAN(StationControl::finishing_state);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult CRCtl::initial(GCFEvent& e, GCFPortInterface& port)
{
    GCFEvent::TResult status = GCFEvent::HANDLED;

    switch (e.signal) {
        case F_INIT: {
        } break;

        case F_ENTRY: {
            if (!itsCRTrigPort->isConnected()) {
                itsCRTrigPort->open();
                itsTimerPort->setTimer(5.0);
            }
            // first redirect signalHandler to our finishing state to leave PVSS
            // in the right state when we are going down
            thisCRCtl = this;
            signal (SIGINT,  CRCtl::sigintHandler);    // ctrl-c
            signal (SIGTERM, CRCtl::sigintHandler);    // kill
        } break;

        case F_CONNECTED: {
            if (itsCRTrigPort->isConnected()) {
                itsTimerPort->cancelAllTimers();
                cout << "connected, execute command" << endl;
                TRAN(CRCtl::docommand);
            }
        } break;
        
        case F_DISCONNECTED: {
            port.close();
        } break;

        case F_TIMER: {
            // try again
            cout << "   =x=x=x=x=x=x=x=x=x=x=x=x=x=x=x=x=x=x=x=x=x=x=x=x=x=x=" << endl;
            cout << "   =x=      CRTriggerControl is NOT responding       =x=" << endl;
            cout << "   =x=x=x=x=x=x=x=x=x=x=x=x=x=x=x=x=x=x=x=x=x=x=x=x=x=x=" << endl;
            exit(EXIT_FAILURE);
        } break;

        default: {
            status = GCFEvent::NOT_HANDLED;
        } break;
    }
    return(status);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult CRCtl::docommand(GCFEvent& e, GCFPortInterface& port)
{
    //cout << "docommand signal:" << eventName(e) << endl;
    GCFEvent::TResult status = GCFEvent::HANDLED;

    switch (e.signal) {
        case F_INIT: {
        } break;

        case F_ENTRY: {
            // reparse options
            itsCommand = parse_options(itsArgc, itsArgv);
            if (itsCommand == 0) {
                cout << "Warning: no command specified." << endl;
                exit(EXIT_FAILURE);
            }
            itsCommand->send();
        } break;

        case F_CONNECTED: {
        } break;

        case F_DISCONNECTED: {
            port.close();
            cout << formatString("Error: port '%s' disconnected.",port.getName().c_str()) << endl;
            exit(EXIT_FAILURE);
        } break;

        case F_TIMER: {
            if (&port == itsTimerPort) {
                itsCommand->send();
            } else {
                cout << "Timeout, tbbctl stopped" << endl;
                cout << endl;
                GCFScheduler::instance()->stop();
            }
        } break;

        case CR_STOP_ACK:
        case CR_READ_ACK:
        case CR_RECORD_ACK:
        case CR_CEP_SPEED_ACK:
        case CR_STOP_DUMPS_ACK: {
            itsTimerPort->cancelAllTimers();
            status = itsCommand->ack(e); // handle the acknowledgement
            if (!itsCommand->isCmdDone()) {
                // not done send next command
                itsTimerPort->setTimer(0.0);
            }
        } break;

        default: {
            cout << "Error: unhandled event." << endl;
            cout << formatString("Error: unhandled event. %d",e.signal) << endl;
            GCFScheduler::instance()->stop();
        } break;
    }

    if (itsCommand->isCmdDone()) {
        cout << flush;
        GCFScheduler::instance()->stop();
    }

    return(status);
}

//-----------------------------------------------------------------------------
Command* CRCtl::parse_options(int argc, char** argv)
{
    Command*  command = 0;
    std::list<int> select;

    optind = 0; // reset option parsing

    while(1) {
        static struct option long_options[] = {
            { "rcu",        required_argument, 0, 'a' },
            { "station",    required_argument, 0, 'g' },
            { "obsid",      required_argument, 0, 'h' },
            { "stop",       required_argument, 0, 'b' },
            { "read",       required_argument, 0, 'c' },
            { "record",     no_argument,       0, 'd' },
            { "cepspeed",   required_argument, 0, 'e' },
            { "stopdumps",  no_argument,       0, 'f' },
            { 0,            0,                 0,  0 },
        };

        int option_index = 0;
        int c = getopt_long( argc, argv,
                                    "a:g:h:b:c:d:e:f:",
                                    long_options,
                                    &option_index );

        if (c == -1) {
            break;
        }

        switch (c) {

            case 'a': {    // --rcu
                if (optarg) {
                    if (!command) {
                        cout << "Error: 'command' argument should come before --rcu argument" << endl;
                        exit(EXIT_FAILURE);
                    }
                    command->setRcuStr(optarg);
                }
                else {
                    cout << "Error: option '--rcu' requires an argument" << endl;
                    exit(EXIT_FAILURE);
                }
            } break;

            case 'g': {    // --station
                if (optarg) {
                    if (!command) {
                        cout << "Error: 'command' argument should come before --station argument" << endl;
                        exit(EXIT_FAILURE);
                    }
                    command->setStationStr(optarg);
                }
                else {
                    cout << "Error: option '--station' requires an argument" << endl;
                    exit(EXIT_FAILURE);
                }
            } break;
            
            case 'h': {    // --obsid
                if (optarg) {
                    if (!command) {
                        cout << "Error: 'command' argument should come before --station argument" << endl;
                        exit(EXIT_FAILURE);
                    }
                    uint32 obsid;
                    int numitems = sscanf(optarg, "%u",&obsid);
                    if ( numitems < 1 || numitems == EOF) {
                        cout << "Error: invalid number of arguments. Should be of the format " << endl;
                        cout << "       '--obsid=id' (use int value)" << endl;
                        exit(EXIT_FAILURE);
                    }
                    command->setOnservationID(obsid);
                }
                else {
                    cout << "Error: option '--onsid' requires an argument" << endl;
                    exit(EXIT_FAILURE);
                }
            } break;
            
            case 'b': {   // --stop
                if (command) delete command;
                StopCmd* cmd = new StopCmd(*itsCRTrigPort, *itsTimerPort);
                command = cmd;
                
                double stopTime = 0.;
                if (optarg) {
                    int numitems = sscanf(optarg, "%lf",&stopTime);
                    // check if valid arguments
                    if ( numitems < 1 || numitems == EOF) {
                        cout << "Error: invalid number of arguments. Should be of the format " << endl;
                        cout << "       '--stop=stoptime' (use float value)" << endl;
                        exit(EXIT_FAILURE);
                    }
                }
                cmd->setStopTime(RTC::NsTimestamp(stopTime));
            } break;

            case 'c': {   // --read
                if (command) delete command;
                ReadCmd* cmd = new ReadCmd(*itsCRTrigPort, *itsTimerPort);
                command = cmd;
                
                double time = 0.;
                double timeBefore = 0.;
                double timeAfter = 0.;
                if (optarg) {
                    int numitems = sscanf(optarg, "%lf, %lf, %lf",&time, &timeBefore, &timeAfter);
                    // check if valid arguments
                    if ( numitems < 3 || numitems == EOF) {
                        cout << "Error: invalid number of arguments. Should be of the format " << endl;
                        cout << "       '--read=time,timebefore,timeafter' (use float values)" << endl;
                        exit(EXIT_FAILURE);
                    }
                }
                cmd->setTime(RTC::NsTimestamp(time));
                cmd->setTimeBefore(RTC::NsTimestamp(timeBefore));
                cmd->setTimeAfter(RTC::NsTimestamp(timeAfter));
            } break;

            case 'd': {   // --record
                if (command) delete command;
                RecordCmd* cmd = new RecordCmd(*itsCRTrigPort, *itsTimerPort);
                command = cmd;
            } break;

            case 'e': {   // --cepspeed
                if (command) delete command;
                CepSpeedCmd* cmd = new CepSpeedCmd(*itsCRTrigPort, *itsTimerPort);
                command = cmd;
                uint32 delay = 0;
                uint32 datapaths = 0;
                if (optarg) {
                    int numitems = sscanf(optarg, "%u, %u",&delay, &datapaths);
                    // check if valid arguments
                    if ( numitems < 2 || numitems == EOF) {
                        cout << "Error: invalid number of arguments. Should be of the format " << endl;
                        cout << "       '--cepspeed=delay,datapaths' (use integer values)" << endl;
                        cout << "       delay = delay in nSec between frames 0..xx, in 5nSec increments" << endl;
                        cout << "       datapaths = number of boards that dump simultaneously 1..6" << endl;
                        exit(EXIT_FAILURE);
                    }
                }
                delay = delay/5;
                cmd->setDelay(delay);
                cmd->setDatapaths(datapaths);
            } break;
            
            case 'f': {   // --stopdumps
                if (command) delete command;
                StopDumpsCmd* cmd = new StopDumpsCmd(*itsCRTrigPort, *itsTimerPort);
                command = cmd;
            } break;

            case 'L':
            case '?': {
                commandHelp();
                exit(0);
            } break;

            default: {
                commandHelp();
                exit(EXIT_FAILURE);
            } break;
        }
    }
    if (command) {
        if (!command->isSetObservationID()) {
            cout << "Warning, ObservationID not set" << endl;
            exit(EXIT_FAILURE);
        }
    }
    return(command);
}

//-----------------------------------------------------------------------------
std::list<int> CRCtl::strtolist(const char* str, int max)
{
    string inputstring(str);
    char* start  = (char*)inputstring.c_str();
    char* end    = 0;
    bool range   = false;
    long prevval = 0;
    std::list<int> resultset;

    resultset.clear();

    while(start) {
        long val = strtol(start, &end, 10); // read decimal numbers
        start = (end ? (*end ? end + 1 : 0) : 0); // determine next start
        if (val >= max || val < 0) {
            cout << formatString("Error: value %ld out of range",val) << endl;
            resultset.clear();
            return(resultset);
        }

        if (end) {
            switch (*end) {
                case ',':
                case 0: {
                    if (range) {
                        if (0 == prevval && 0 == val) {
                            val = max - 1;
                        }
                        if (val < prevval) {
                            cout << "Error: invalid range specification" << endl;
                            resultset.clear();
                            return(resultset);
                        }
                        for (long i = prevval; i <= val; i++) {
                            resultset.push_back(i);
                        }
                    } else {
                        resultset.push_back(val);
                    }
                    range = false;
                } break;

                case ':': {
                    range = true;
                } break;

                default: {
                    cout << formatString("Error: invalid character %c",*end) << endl;
                    resultset.clear();
                    return(resultset);
                } break;
            }
        }
        prevval = val;
    }
    return(resultset);
}

//-----------------------------------------------------------------------------
void CRCtl::mainloop()
{
    start(); // make initial transition
    GCFScheduler::instance()->run();
}

//-----------------------------------------------------------------------------
int main(int argc, char** argv)
{
    GCFScheduler::instance()->init(argc, argv, "CRCtl");

    LOG_DEBUG(formatString("Program %s has started", argv[0]));

    CRCtl CRCtl("CRCtl", argc, argv);

    try {
        CRCtl.mainloop();
    }
    catch (Exception& e) {
        cout << "Exception: " << e.text() << endl;
        cout << endl;
        cout << "== abnormal termination of CRCtl ============================================" << endl;
        exit(EXIT_FAILURE);
    }
    cout << endl;
    // cout << "== normal termination of CRCtl ==============================================" << endl;

    return(0);
}

