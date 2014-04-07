//#  CEPlogProcessor.cc: Moves the operator info from the logfiles to PVSS
//#
//#  Copyright (C) 2009
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
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/LofarConstants.h>
#include <Common/LofarLocators.h>
#include <Common/StringUtil.h>
#include <Common/ParameterSet.h>

#include <GCF/PVSS/GCF_PVTypes.h>
#include <MACIO/MACServiceInfo.h>
#include <APL/APLCommon/ControllerDefines.h>
//#include <APL/RTDBCommon/RTDButilities.h>
//#include <APL/APLCommon/StationInfo.h>
#include <GCF/RTDB/DP_Protocol.ph>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h> // usleep

#include "CEPlogProcessor.h"
#include "PVSSDatapointDefs.h"


namespace LOFAR {
    using namespace APLCommon;
//  using namespace APL::RTDBCommon;
    using namespace GCF::TM;
    using namespace GCF::PVSS;
    using namespace GCF::RTDB;
    namespace APL {
    
// static pointer to this object for signal handler
static CEPlogProcessor*     thisLogProcessor = 0;
#define MPIProcs 16


//
// CEPlogProcessor()
//
CEPlogProcessor::CEPlogProcessor(const string&  cntlrName) :
    GCFTask             ((State)&CEPlogProcessor::initial_state,cntlrName),
    itsListener         (0),
    itsOwnPropertySet   (0),
    itsTimerPort        (0),
    itsNrInputBuffers   (0),
    itsNrAdders         (0),
    itsNrStorage        (0),
    itsBufferSize       (0)
{
    LOG_TRACE_OBJ_STR (cntlrName << " construction");

    // need port for timers.
    itsTimerPort = new GCFTimerPort(*this, "TimerPort");

    // prepare TCP port to accept connections on
    itsListener = new GCFTCPPort (*this, "BGPlogger:v1_0", GCFPortInterface::MSPP, 0);
//  itsListener = new GCFTCPPort (*this, MAC_SVCMASK_CEPPROCMONITOR, GCFPortInterface::MSPP, 0); // TODO
    ASSERTSTR(itsListener, "Cannot allocate listener port");
    itsListener->setPortNumber(globalParameterSet()->getInt("CEPlogProcessor.portNr"));

    itsBufferSize     = globalParameterSet()->getInt("CEPlogProcessor.bufferSize", 1024);
    itsNrInputBuffers = globalParameterSet()->getInt("CEPlogProcessor.nrInputBuffers", 128);
    itsNrAdders       = globalParameterSet()->getInt("CEPlogProcessor.nrAdders", 64);
    itsNrStorage      = globalParameterSet()->getInt("CEPlogProcessor.nrStorage", 192);

    registerProtocol(DP_PROTOCOL, DP_PROTOCOL_STRINGS);

}


//
// ~CEPlogProcessor()
//
CEPlogProcessor::~CEPlogProcessor()
{
    LOG_TRACE_OBJ_STR (getName() << " destruction");

    // database should be ready by ts, check if allocation was succesfull
    for (int    inputBuf = itsNrInputBuffers - 1; inputBuf >= 0; inputBuf--) {
        delete itsInputBuffers[inputBuf];
    }
    for (int    adder = itsNrAdders - 1; adder >= 0; adder--) {
        delete itsAdders[adder];
    }
    for (int    storage = itsNrStorage - 1; storage >= 0; storage--) {
        delete itsStorage[storage];
    }

    if (itsListener) {
        itsListener->close();
        delete itsListener;
    }

    delete itsTimerPort;
}

//
// signalHandler(signr)
//
void CEPlogProcessor::signalHandler(int signum)
{
    LOG_DEBUG_STR("SIGNAL " << signum << " detected");

    if (thisLogProcessor) {
        thisLogProcessor->finish();
    }
}

//
// finish()
//
void CEPlogProcessor::finish()
{
    TRAN(CEPlogProcessor::finish_state);
}



//
// initial_state(event, port)
//
// Setup connection with PVSS
//
GCFEvent::TResult CEPlogProcessor::initial_state(GCFEvent& event, 
                                                    GCFPortInterface& port)
{
    LOG_DEBUG_STR ("initial:" << eventName(event) << "@" << port.getName());

    GCFEvent::TResult status = GCFEvent::HANDLED;
  
    switch (event.signal) {
    case F_INIT:
        break;

    case F_ENTRY: {
        // Get access to my own propertyset.
        LOG_DEBUG_STR ("Activating PropertySet " << PSN_LOG_PROCESSOR);
        itsTimerPort->setTimer(2.0);
        itsOwnPropertySet = new RTDBPropertySet(PSN_LOG_PROCESSOR,
                                                PST_LOG_PROCESSOR,
                                                PSAT_WO,
                                                this);

        }
        break;

    case DP_CREATED: {
        // NOTE: this function may be called DURING the construction of the PropertySet.
        // Always exit this event in a way that GCF can end the construction.
        DPCreatedEvent      dpEvent(event);
        LOG_DEBUG_STR("Result of creating " << dpEvent.DPname << " = " << dpEvent.result);
        itsTimerPort->cancelAllTimers();
        itsTimerPort->setTimer(0.0);
        }
        break;

    case F_TIMER: {
        // update PVSS.
        LOG_TRACE_FLOW ("Updateing state to PVSS");
        itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION, GCFPVString("initial"));
        
        LOG_DEBUG_STR("Going to create the datapoints in PVSS");
        TRAN (CEPlogProcessor::createPropertySets);
    }
    
    case DP_SET:
        break;

    case F_QUIT:
        TRAN (CEPlogProcessor::finish_state);
        break;

    default:
        LOG_DEBUG_STR ("initial, DEFAULT");
        break;
    }    

    return (status);
}


//
// createPropertySets(event, port)
//
// Create PropertySets for all processes.
//
GCFEvent::TResult CEPlogProcessor::createPropertySets(GCFEvent& event, 
                                                    GCFPortInterface& port)
{
    LOG_DEBUG_STR ("createPropertySets:" << eventName(event) << "@" << port.getName());


    GCFEvent::TResult status = GCFEvent::HANDLED;
  
    switch (event.signal) {

    case F_ENTRY: {
        itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("create PropertySets"));

        // create propSets for the inputbuffer processes
        itsInputBuffers.resize(itsNrInputBuffers,  0);
        string  inputBufferNameMask (createPropertySetName(PSN_INPUT_BUFFER, getName()));
        for (int32  inputBuffer = 0; inputBuffer < itsNrInputBuffers; inputBuffer++) {
            string  PSname(formatString(inputBufferNameMask.c_str(), inputBuffer));
            if (!itsInputBuffers[inputBuffer]) {
                itsInputBuffers[inputBuffer] = new RTDBPropertySet(PSname, PST_INPUT_BUFFER, PSAT_WO | PSAT_CW, this);
            }
            usleep (2000); // wait 2 ms in order not to overload the system  
        }

        // create propSets for the adder processes
        itsAdders.resize (itsNrAdders, 0);
        string  adderNameMask(createPropertySetName(PSN_ADDER, getName()));
        for (int32  adder = 0; adder < itsNrAdders; adder++) {
            string  PSname(formatString(adderNameMask.c_str(), adder));
            if (!itsAdders[adder]) {
                itsAdders[adder] = new RTDBPropertySet(PSname, PST_ADDER, PSAT_WO | PSAT_CW, this);
            }
            usleep (2000); // wait 2 ms in order not to overload the system  
        }
        itsDroppingCount.resize (itsNrAdders, 0);


        // create propSets for the storage processes
        itsStorage.resize (itsNrStorage, 0);
        string  storageNameMask(createPropertySetName(PSN_STORAGE, getName()));
        for (int32  storage = 0; storage < itsNrStorage; storage++) {
            string  PSname(formatString(storageNameMask.c_str(), storage));
            if (!itsStorage[storage]) {
                itsStorage[storage] = new RTDBPropertySet(PSname, PST_STORAGE, PSAT_WO | PSAT_CW, this);
            }
            usleep (2000); // wait 2 ms in order not to overload the system  
        }
        itsStorageBuf.resize(itsNrStorage);
        for (int i=0; i < itsNrStorage; i++) {
          //set array sizes
          itsStorageBuf[i].timeStr.resize(MPIProcs);
          itsStorageBuf[i].count.resize(MPIProcs,0);
          itsStorageBuf[i].dropped.resize(MPIProcs);
        }



        LOG_INFO("Giving PVSS 5 seconds to process the requests");
        itsTimerPort->setTimer(5.0);    // give database some time to finish the job
    }
    break;

    case F_TIMER: {
        // database should be ready by ts, check if allocation was succesfull
        for (int32  inputBuffer = 0; inputBuffer < itsNrInputBuffers; inputBuffer++) {
            ASSERTSTR(itsInputBuffers[inputBuffer], "Allocation of PS for inputBuffer " << inputBuffer << " failed.");
        }
        for (int32  adder = 0; adder < itsNrAdders; adder++) {
            ASSERTSTR(itsAdders[adder], "Allocation of PS for adder " << adder << " failed.");
        }
        for (int32  storage = 0; storage < itsNrStorage; storage++) {
            ASSERTSTR(itsStorage[storage], "Allocation of PS for storage " << storage << " failed.");
        }
        LOG_DEBUG_STR("Allocation of all propertySets successfull, going to open the listener");
        TRAN(CEPlogProcessor::startListener);
    }
    break;

    case DP_SET:
        break;

    case F_QUIT:
        TRAN (CEPlogProcessor::finish_state);
        break;

    default:
        LOG_DEBUG_STR ("createPropertySets, DEFAULT");
        break;
    }    

    return (status);
}


//
// startListener(event, port)
//
GCFEvent::TResult CEPlogProcessor::startListener(GCFEvent&  event, GCFPortInterface&    port)
{
    LOG_DEBUG_STR("startListener:" << eventName(event) << "@" << port.getName());

    switch (event.signal) {
    case F_ENTRY:
        itsListener->autoOpen(0, 10, 2);    // report within 10 seconds.
        break;

    case F_CONNECTED:
        LOG_DEBUG("Listener is started, going to operational mode");
        TRAN (CEPlogProcessor::operational);
        break;

    case F_DISCONNECTED:
        LOG_FATAL_STR("Cannot open the listener on port " << itsListener->getPortNumber() << ". Quiting!");
        GCFScheduler::instance()->stop();
        break;
    }

    return (GCFEvent::HANDLED);
}


//
// operational(event, port)
//
GCFEvent::TResult CEPlogProcessor::operational(GCFEvent& event, GCFPortInterface& port)
{
    LOG_DEBUG_STR("operational:" << eventName(event) << "@" << port.getName());

    switch (event.signal) {
    case F_ENTRY:
        itsTimerPort->setTimer(1.0,1.0);
        break;
    case F_TIMER:

        LOG_DEBUG("Timer event, preparing PVSS arrays");
        for (int j=0; j < itsNrStorage; j++) {
          GCFPValueArray timeArray;
          GCFPValueArray countArray;
          GCFPValueArray droppedArray;

          for (int i = 0; i<MPIProcs;i++) {
              timeArray.push_back(new GCFPVString(itsStorageBuf[j].timeStr[i]));
              countArray.push_back(new GCFPVInteger(itsStorageBuf[j].count[i]));
              droppedArray.push_back(new GCFPVString(itsStorageBuf[j].dropped[i]));
          }
          
          itsStorage[j]->setValue(PN_STR_TIME, GCFPVDynArr(LPT_DYNSTRING, timeArray));
          itsStorage[j]->setValue(PN_STR_COUNT, GCFPVDynArr(LPT_DYNINTEGER, countArray));
          itsStorage[j]->setValue(PN_STR_DROPPED, GCFPVDynArr(LPT_DYNSTRING, droppedArray));
        }
        break;
    case F_ACCEPT_REQ:
        _handleConnectionRequest();
        break;

    case F_CONNECTED:
        break;

    case F_DISCONNECTED: {
        port.close();
        _deleteStream(port);
        break;
    }
    case F_DATAIN:
        _handleDataStream(&port);
        break;
    }

    return (GCFEvent::HANDLED);
}

//
// _deleteStream(GCFPortInterface&  port)
//
void CEPlogProcessor::_deleteStream(GCFPortInterface&   port) 
{
    LOG_DEBUG_STR("_deleteStream");
    map<GCFPortInterface*, streamBuffer_t>::iterator    theStream = itsLogStreams.find(&port);
    if (theStream != itsLogStreams.end()) {
        delete theStream->second.buffer;
        theStream->second.buffer = 0;
        itsLogStreams.erase(theStream);
    }
}

//
// _handleConnectionRequest()
//
void CEPlogProcessor::_handleConnectionRequest()
{
    GCFTCPPort*     pNewClient = new GCFTCPPort();
    ASSERT(pNewClient);

    pNewClient->init(*this, "newClient", GCFPortInterface::SPP, 0, true);
    if (!itsListener->accept(*pNewClient)) {
        LOG_WARN("Connection with new client went wrong");
        return;
    }

    // give stream its own buffer.
    streamBuffer_t      stream;
    stream.socket   = pNewClient;
    stream.buffer   = new CircularBuffer(itsBufferSize);
    itsLogStreams[pNewClient] = stream;
    LOG_INFO_STR("Added new client to my admin");
}

//
// _handleDataStream(sid)
//
void CEPlogProcessor::_handleDataStream(GCFPortInterface*   port)
{
    // read in the new bytes
    streamBuffer_t  &stream = itsLogStreams[port];
    int newBytes = stream.socket->recv( stream.buffer->tail, stream.buffer->tailFreeSpace() );
    if (newBytes < 0) {
        LOG_DEBUG_STR("Closing connection.");
        port->close();
        _deleteStream(*port);
        return;
    }

    LOG_DEBUG_STR("Read " << newBytes << " bytes.");
    stream.buffer->incTail( newBytes );

    char lineBuf[1024];
    while (stream.buffer->getLine( lineBuf, sizeof lineBuf )) {
      LOG_DEBUG_STR("Read log line " << lineBuf );
      _processLogLine(lineBuf);
    }
}

// Convert "23-02-11" and "01:02:58.687" into a time_t timestamp
time_t CEPlogProcessor::_parseDateTime(const char *datestr, const char *timestr) const
{
  struct tm tm;
  time_t ts;
  bool validtime = true;

  if (sscanf(datestr, "%u-%u-%u", 
    &tm.tm_year, &tm.tm_mon, &tm.tm_mday) != 3) {
    validtime = false;
   } else {
    // tm_year starts counting from 1900

    if (tm.tm_year > 1900) {
      // YYYY
      tm.tm_year -= 1900;
    } else {
      // YY -- we won't see loglines pre 2000.
      tm.tm_year += 10;
    }
   }

  if (sscanf(timestr, "%u:%u:%u",  // ignore milliseconds
    &tm.tm_hour, &tm.tm_min, &tm.tm_sec) != 3) {
    validtime = false;
  }

  if (validtime) {
    ts = mktime(&tm);
  } else {
    LOG_WARN_STR("Invalid timestamp: " << datestr << " " << timestr << "; using now()");

    ts = time(0L);
  }

  return ts;
}


//
// _processLogLine(char*)
//
//
void CEPlogProcessor::_processLogLine(const char *cString)
{
    // example log line:
    // Storage@00 09-12-10 11:33:13.240 DEBUG [obs 21855 output 1 subband 223] InputThread::~InputThread()
    unsigned bufsize = strlen( cString ) + 1;

    vector<char> processName(bufsize), date(bufsize), time(bufsize), loglevel(bufsize), msg(bufsize);
    int processNr;

    if (*cString == 0) {
      return;
    }

    if (sscanf(cString, "%[^@]@%d %s %s %s %[^\n]",
      &processName[0],
      &processNr,
      &date[0],
      &time[0],
      &loglevel[0],
      &msg[0]) != 6) {
      // this will include:
      // * mpi/bgp errors
      // * casacore messages
      // * ssh/login messages
      // * log4cplus/cxx messages
      // * mangled messages (happens occasionally)
      // * backtraces
      // * C++/libc errors
      LOG_DEBUG_STR("Unparsable log line: " << cString);
      return;
    }

    LOG_DEBUG_STR("Processname = " << &processName[0]);     // eg IONProc
    
    time_t ts = _parseDateTime(&date[0], &time[0]);

    if (!strcmp(&processName[0],"IONProc")) {
      _processIONProcLine(processNr, ts, &loglevel[0], &msg[0]);
    } else if (!strcmp(&processName[0],"CNProc")) {
      _processCNProcLine(processNr, ts, &loglevel[0], &msg[0]);
    } else if (!strcmp(&processName[0],"Storage")) {
      _processStorageLine(processNr, ts, &loglevel[0], &msg[0]);
    }
}

//
// _processIONProcLine(cstring)
//
void CEPlogProcessor::_processIONProcLine(int processNr, time_t ts, const char *loglevel, const char *msg)
{
    LOG_DEBUG_STR("_processIONProcLine(" << processNr << "," << ts << "," << loglevel << "," << msg << ")");

    if (processNr < 0 || processNr >= itsNrInputBuffers) {
        LOG_WARN_STR("Inputbuffer range = 0.." << itsNrInputBuffers << ". Index " << processNr << " is invalid");
        return;
    }

    char*   result;

    //
    // InputBuffer
    //

    // IONProc@01 23-02-11 01:02:58.687 INFO  [obs 23603 station CS005HBA1] [1298422977s, 80863], late: 17.6 ms, delays: [8.657333 us], flags 0: (0%), flags 1: (0%), flags 2: (0%), flags 3: (0%)
    // IONProc@05 07-01-11 20:57:56.765 INFO  [obs 1002069 station S10] [1294433876s, 0], late: 8.85 ms, delays: [-616.3421 ns], flags 0: [0..52992> (100%), flags 1: [0..52992> (100%), flags 2: [0..52992> (100%), flags 3: [0..52992> (100%)

    if ((result = strstr(msg, " late: "))) {
        float late;

        if (sscanf(result, " late: %f ", &late) == 1 ) {
            LOG_DEBUG_STR("[" << processNr << "] Late: " << late);
            itsInputBuffers[processNr]->setValue(PN_IPB_LATE, GCFPVDouble(late), ts);
        }

        // 0% flags look like : flags 0: (0%)
        // filled% flags look like : flags 0: [nr..nr> (10.5%)
        if ((result = strstr(msg, "flags 0:"))) {
            float flags0, flags1, flags2, flags3;

            if (sscanf(result, "flags 0:%*[^(](%f%%), flags 1:%*[^(](%f%%), flags 2:%*[^(](%f%%), flags 3:%*[^(](%f%%)",
              &flags0, &flags1, &flags2, &flags3) == 4) {
                LOG_DEBUG(formatString("[%d] %%bad: %.2f, %.2f, %.2f, %.2f", processNr, flags0, flags1, flags2, flags3));

                itsInputBuffers[processNr]->setValue(PN_IPB_STREAM0_PERC_BAD, GCFPVDouble(flags0), ts, false);
                itsInputBuffers[processNr]->setValue(PN_IPB_STREAM1_PERC_BAD, GCFPVDouble(flags1), ts, false);
                itsInputBuffers[processNr]->setValue(PN_IPB_STREAM2_PERC_BAD, GCFPVDouble(flags2), ts, false);
                itsInputBuffers[processNr]->setValue(PN_IPB_STREAM3_PERC_BAD, GCFPVDouble(flags3), ts, false);
                itsInputBuffers[processNr]->flush();
            }
        }
        return;
    }

    // IONProc@36 23-02-11 00:59:59.151 DEBUG [obs 23603 station CS003HBA0]  ION->CN:  483 ms
    if ((result = strstr(msg, "ION->CN:"))) {
        float   ioTime;
        if (sscanf(result, "ION->CN:%f", &ioTime) == 1) {
                LOG_DEBUG_STR("[" << processNr << "] ioTime: " << ioTime);
            itsInputBuffers[processNr]->setValue(PN_IPB_IO_TIME, GCFPVDouble(ioTime), ts);
            return;
        }
    }

    // IONProc@36 23-02-11 00:59:59.673 INFO  [station CS003HBA0] received packets = [12329,12328,12292,12329], us/sy/in/id(0): [21/20/10/51(25)]
    if ((result = strstr(msg, "received packets = ["))) {
        int received[4] = {0,0,0,0};
        int badsize[4] = {0,0,0,0};
        int badtimestamp[4] = {0,0,0,0};

        if (sscanf(result, "received packets = [%d,%d,%d,%d]", &received[0], &received[1], &received[2], &received[3]) == 4) {
          LOG_DEBUG(formatString("[%d] blocks: %d, %d, %d, %d", processNr, received[0], received[1], received[2], received[3]));
          itsInputBuffers[processNr]->setValue(PN_IPB_STREAM0_BLOCKS_IN, GCFPVInteger(received[0]), ts, false);
          itsInputBuffers[processNr]->setValue(PN_IPB_STREAM1_BLOCKS_IN, GCFPVInteger(received[1]), ts, false);
          itsInputBuffers[processNr]->setValue(PN_IPB_STREAM2_BLOCKS_IN, GCFPVInteger(received[2]), ts, false);
          itsInputBuffers[processNr]->setValue(PN_IPB_STREAM3_BLOCKS_IN, GCFPVInteger(received[3]), ts, false);
          itsInputBuffers[processNr]->flush();
        }

        // if rejected was found in same line this means that a certain amount of blocks was rejected, 
        // set this into the database. If no rejected was found, it means 0 blocks were rejected, so DB can be reset to 0
        if ((result = strstr(msg, " bad size = ["))) {
          if (sscanf(result, " bad size = [%d,%d,%d,%d]", &badsize[0], &badsize[1], &badsize[2], &badsize[3]) == 4) {
            LOG_DEBUG(formatString("[%d] rejected: bad size blocks: %d, %d, %d, %d", processNr, badsize[0], badsize[1], badsize[2], badsize[3]));
          } else {
            badsize[0] = 0;
            badsize[1] = 0;
            badsize[2] = 0;
            badsize[3] = 0;
          }
        }

        if ((result = strstr(msg, " bad timestamps = ["))) {
          if (sscanf(result, " bad timestamps = [%d,%d,%d,%d]", &badtimestamp[0], &badtimestamp[1], &badtimestamp[2], &badtimestamp[3]) == 4) {
            LOG_DEBUG(formatString("[%d] rejected: bad timestamp blocks: %d, %d, %d, %d", processNr, badtimestamp[0], badtimestamp[1], badtimestamp[2], badtimestamp[3]));
          } else {
            badtimestamp[0] = 0;
            badtimestamp[1] = 0;
            badtimestamp[2] = 0;
            badtimestamp[3] = 0;
          }
        }

        itsInputBuffers[processNr]->setValue(PN_IPB_STREAM0_REJECTED, GCFPVInteger(badsize[0] + badtimestamp[0]), ts, false);
        itsInputBuffers[processNr]->setValue(PN_IPB_STREAM1_REJECTED, GCFPVInteger(badsize[1] + badtimestamp[1]), ts, false);
        itsInputBuffers[processNr]->setValue(PN_IPB_STREAM2_REJECTED, GCFPVInteger(badsize[2] + badtimestamp[2]), ts, false);
        itsInputBuffers[processNr]->setValue(PN_IPB_STREAM3_REJECTED, GCFPVInteger(badsize[3] + badtimestamp[3]), ts, false);
        itsInputBuffers[processNr]->flush();
        return;
    }


    //
    // Adder
    //

    // IONProc@17 07-01-11 20:59:00.981 WARN  [obs 1002069 output 6 index L1002069_B102_S0_P000_bf.raw] Dropping data
    if ((result = strstr(msg, "Dropping data"))) {
        LOG_DEBUG(formatString("[%d] Dropping data started ",processNr));
        itsAdders[processNr]->setValue(PN_ADD_DROPPING, GCFPVBool(true), ts);
        itsAdders[processNr]->setValue(PN_ADD_LOG_LINE,GCFPVString(result),ts);
        itsDroppingCount[processNr]=itsDroppingCount[processNr]+1;
        LOG_DEBUG(formatString("Dropping count[%d] : %d", processNr,itsDroppingCount[processNr]));
        return;
    }

    // IONProc@23 07-01-11 20:58:27.848 WARN  [obs 1002069 output 6 index L1002069_B139_S0_P000_bf.raw] Dropped 9 blocks
    if ((result = strstr(msg, "Dropped "))) {
        int dropped(0);
        if (sscanf(result, "Dropped %d ", &dropped) == 1) {
                LOG_DEBUG(formatString("[%d] Dropped %d ",processNr,dropped));
            itsAdders[processNr]->setValue(PN_ADD_NR_BLOCKS_DROPPED, GCFPVInteger(dropped), ts);
        }
        itsAdders[processNr]->setValue(PN_ADD_LOG_LINE,GCFPVString(result),ts);
        itsDroppingCount[processNr]=itsDroppingCount[processNr]-1;
        LOG_DEBUG(formatString("Dropping count[%d] : %d", processNr,itsDroppingCount[processNr]));
        // if dropcount = 0 again, if so reset dropping flag
        if (itsDroppingCount[processNr] <= 0) {
            LOG_DEBUG(formatString("[%d] Dropping data ended ",processNr));
            itsAdders[processNr]->setValue(PN_ADD_DROPPING, GCFPVBool(false), ts);
        }
        return;
    }
}

void CEPlogProcessor::_processCNProcLine(int processNr, time_t ts, const char *loglevel, const char *msg)
{
    LOG_DEBUG_STR("_processCNProcLine(" << processNr << "," << ts << "," << loglevel << "," << msg << ")");

    // TODO
}

void CEPlogProcessor::_processStorageLine(int processNr, time_t ts, const char *loglevel, const char *msg)
{
    LOG_DEBUG_STR("_processStorageLine(" << processNr << "," << ts << "," << loglevel << "," << msg << ")");

    if (processNr < 0 || processNr >= itsNrStorage) {
        LOG_WARN_STR("Storage range = 0.." << itsNrStorage << ". Index " << processNr << " is invalid");
        return;
    }

#if 0
    char*   result;

    if ((result = strstr(msg, "time ="))) {
        int rank(0), count(0);
        char   tim[24]; 
        
        LOG_DEBUG_STR("_processStorageLine(" << processNr << "," << result << ")");
        if (sscanf(result, "time = %[^,], rank = %d, count = %d", tim, &rank, &count)== 3)
        {
            LOG_DEBUG(formatString("[%d] time: %s, rank: %d, count: %d", processNr, tim, rank, count));
            itsStorageBuf[processNr].timeStr[rank]  = tim;
            itsStorageBuf[processNr].count[rank] = count;
        }
        return;
    }
#endif

    /*
    if ((result = strstr(msg, "dropped "))) {
      int blocks(0), subband(0), output(0);
        
        LOG_DEBUG_STR("_processStorageLine(" << processNr << "," << result << ")");
        if (sscanf(result, "dropped %d block for subband %d and output %d", &blocks, &subband, &output)==3) {
        {
            LOG_DEBUG(formatString("[%d] blocks: %d, subband: %d, output: %d", blocks, subband, output));

    // dropped has no rank in yet 
    // itsStorageBuf[processNr].dropped[rank] = result;
        }
        return;
    }
    */
}


//
// finish_state(event, port)
//
// Write controller state to PVSS
//
GCFEvent::TResult CEPlogProcessor::finish_state(GCFEvent& event, GCFPortInterface& port)
{
    LOG_DEBUG_STR ("finish_state:" << eventName(event) << "@" << port.getName());

    GCFEvent::TResult status = GCFEvent::HANDLED;

    switch (event.signal) {
    case F_INIT:
        break;

    case F_ENTRY: {
        // update PVSS
        itsOwnPropertySet->setValue(string(PN_FSM_CURRENT_ACTION),GCFPVString("finished"));
        break;
    }
  
    case DP_SET:
        break;

    default:
        LOG_DEBUG("finishing_state, DEFAULT");
        status = GCFEvent::NOT_HANDLED;
        break;
    }    
    return (status);
}


}; // StationCU
}; // LOFAR
