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
#include <ApplCommon/LofarDirs.h>
#include <ApplCommon/Observation.h>
#include <ApplCommon/StationInfo.h>

#include <GCF/PVSS/GCF_PVTypes.h>
#include <GCF/PVSS/PVSSinfo.h>
#include <MACIO/MACServiceInfo.h>
#include <APL/APLCommon/ControllerDefines.h>
#include <APL/APLCommon/Controller_Protocol.ph>
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

CEPFeedback::CEPFeedback()
:
  nrSubbands(0)
{
}


void CEPFeedback::write(const std::string &filename)
{
  LOG_DEBUG_STR("Writing feedback file " << filename);

  parset.replace(subbandSizeKey(), formatString("%u", nrSubbands));

  parset.writeFile(filename);
}


void CEPFeedback::addSubband(unsigned index)
{
  setSubbandKey(index, "fileFormat",           "AIPS++/CASA");
  setSubbandKey(index, "filename",             "");
  setSubbandKey(index, "size",                 "0");
  setSubbandKey(index, "location",             "");
  setSubbandKey(index, "percentageWritten",    "0");
  setSubbandKey(index, "startTime",            "");
  setSubbandKey(index, "duration",             "");
  setSubbandKey(index, "integrationInterval",  "");
  setSubbandKey(index, "centralFrequency",     "");
  setSubbandKey(index, "channelWidth",         "");
  setSubbandKey(index, "channelsPerSubband",   "");
  setSubbandKey(index, "subband",              "");
  setSubbandKey(index, "stationSubband",       "");
  setSubbandKey(index, "SAP",                  "");

  ++nrSubbands;
}

void CEPFeedback::setSubbandKey(unsigned index, const std::string &key, const std::string &value)
{
  LOG_DEBUG_STR("setSubbandKey for index " << index << ": " << key << " = " << value);

  parset.replace(subbandPrefix(index) + key, value);
}

std::string CEPFeedback::subbandSizeKey() const
{
  return "LOFAR.ObsSW.Observation.DataProducts.nrOfOutput_Correlated_";
}

std::string CEPFeedback::subbandPrefix(unsigned index) const
{
  return formatString("LOFAR.ObsSW.Observation.DataProducts.Output_Correlated_[%u].", index);
}

//
// CEPlogProcessor()
//
CEPlogProcessor::CEPlogProcessor(const string&  cntlrName) :
    GCFTask             ((State)&CEPlogProcessor::initial_state,cntlrName),
    itsListener         (0),
	itsControlPort		(0),
    itsOwnPropertySet   (0),
    itsTimerPort        (0),
    itsNrInputBuffers   (0),
    itsNrIONodes        (0),
    itsNrAdders         (0),
    itsNrStorage        (0),
    itsNrWriters        (0),
    itsBufferSize       (0)
{
    LOG_TRACE_OBJ_STR (cntlrName << " construction");

    // HACK: test environment uses 4-pset partitions, production environment 64 pset partition
    // anything else will break this code.

    string dbname = PVSSinfo::getMainDBName();

    LOG_DEBUG_STR("Connected to database " << dbname);

    if (dbname == "MCU099") {
      LOG_WARN("Detected test environment -- assuming 4 psets");
      itsNrPsets = 4;
    } else {
      LOG_WARN("Detected production environment -- assuming 64 psets");
      itsNrPsets = 64;
    }

    // need port for timers.
    itsTimerPort = new GCFTimerPort(*this, "TimerPort");

    // prepare TCP port to accept connections on
    itsListener = new GCFTCPPort (*this, MAC_SVCMASK_CEPLOGPROC, GCFPortInterface::MSPP, 0);
    itsListener->setPortNumber(CEP_LOGPROC_LOGGING);

    itsControlPort = new GCFTCPPort (*this, MAC_SVCMASK_CEPLOGCONTROL, GCFPortInterface::MSPP, 0);
    itsControlPort->setPortNumber(CEP_LOGPROC_CONTROL);

    itsBufferSize     = globalParameterSet()->getInt("CEPlogProcessor.bufferSize", 1024);
    itsNrInputBuffers = globalParameterSet()->getInt("CEPlogProcessor.nrInputBuffers", 64);
    itsNrIONodes      = globalParameterSet()->getInt("CEPlogProcessor.nrIONodes", 64);
    itsNrAdders       = globalParameterSet()->getInt("CEPlogProcessor.nrAdders", 10); // per io node
    itsNrStorage      = globalParameterSet()->getInt("CEPlogProcessor.nrStorageNodes", 100);
    itsNrWriters      = globalParameterSet()->getInt("CEPlogProcessor.nrWriters", 20); // per storage node

    registerProtocol(DP_PROTOCOL,         DP_PROTOCOL_STRINGS);
    registerProtocol(CONTROLLER_PROTOCOL, CONTROLLER_PROTOCOL_STRINGS);

    thisLogProcessor = this;
}


//
// ~CEPlogProcessor()
//
CEPlogProcessor::~CEPlogProcessor()
{
    LOG_TRACE_OBJ_STR (getName() << " destruction");

    // database should be ready by ts, check if allocation was succesful
    for (int    inputBuf = itsInputBuffers.size() - 1; inputBuf >= 0; inputBuf--) {
        delete itsInputBuffers[inputBuf];
    }
    for (int    adder = itsAdders.size() - 1; adder >= 0; adder--) {
        delete itsAdders[adder];
    }
    for (int    storage = itsWriters.size() - 1; storage >= 0; storage--) {
        delete itsWriters[storage];
    }

    // close all streams
    while( !itsLogStreams.empty() )
      _deleteStream( *((*itsLogStreams.begin()).first) );

    // and reap the port objects immediately
    collectGarbage();

    if (itsControlPort) {
        itsControlPort->close();
        delete itsControlPort;
    }

    if (itsListener) {
        itsListener->close();
        delete itsListener;
    }

    delete itsTimerPort;

    delete itsOwnPropertySet;
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
// Actual properties are listed in MAC/Deployment/data/PVSS/*.dpdef.
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
        for (unsigned inputBuffer = 0; inputBuffer < itsNrInputBuffers; inputBuffer++) {
            if (!itsInputBuffers[inputBuffer]) {
                string PSname(formatString("LOFAR_PermSW_PSIONode%02d_InputBuffer", inputBuffer));
                itsInputBuffers[inputBuffer] = new RTDBPropertySet(PSname, "InputBuffer", PSAT_WO | PSAT_CW, this);
            }

            usleep (2000); // wait 2 ms in order not to overload the system  
        }

        // create propSets for the adder processes
        itsAdders.resize (itsNrAdders * itsNrIONodes);

        for (unsigned ionode = 0; ionode < itsNrIONodes; ionode++) {
          for (unsigned adder = 0; adder < itsNrAdders; adder++) {
              unsigned index = ionode * itsNrAdders + adder;

              if (!itsAdders[index]) {
                  string PSname(formatString("LOFAR_ObsSW_OSIONode%02d_Adder%01d", ionode, adder));
                  itsAdders[index] = new RTDBPropertySet(PSname, "Adder", PSAT_WO | PSAT_CW, this);
              }

              usleep (2000); // wait 2 ms in order not to overload the system  
          }
        }

        // create propSets for the storage processes
        itsWriters.resize (itsNrWriters * itsNrStorage, 0);
        for (unsigned storage = 0; storage < itsNrStorage; storage++) {
          for (unsigned writer = 0; writer < itsNrWriters; writer++) {
            unsigned index = storage * itsNrWriters + writer;

            if (!itsWriters[index]) {
              // locus nodes start counting from 001
              string PSname(formatString("LOFAR_ObsSW_OSLocusNode%03d_Writer%02d", storage + 1, writer));
              itsWriters[index] = new RTDBPropertySet(PSname, "Writer", PSAT_WO | PSAT_CW, this);
            }

            usleep (2000); // wait 2 ms in order not to overload the system  
          }  
        }

        LOG_INFO("Giving PVSS 5 seconds to process the requests");
        itsTimerPort->setTimer(5.0);    // give database some time to finish the job
    }
    break;

    case F_TIMER: {
        // database should be ready by ts, check if allocation was succesfull
        for (unsigned inputBuffer = 0; inputBuffer < itsInputBuffers.size(); inputBuffer++) {
            ASSERTSTR(itsInputBuffers[inputBuffer], "Allocation of PS for inputBuffer " << inputBuffer << " failed.");
        }
        for (unsigned adder = 0; adder < itsAdders.size(); adder++) {
            ASSERTSTR(itsAdders[adder], "Allocation of PS for adder " << adder << " failed.");
        }
        for (unsigned storage = 0; storage < itsWriters.size(); storage++) {
            ASSERTSTR(itsWriters[storage], "Allocation of PS for storage " << storage << " failed.");
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
        LOG_DEBUG("Listener is started, going to open Controlport");
        TRAN (CEPlogProcessor::startControlPort);
        break;

    case F_DISCONNECTED:
        LOG_FATAL_STR("Cannot open the listener on port " << itsListener->getPortNumber() << ". Quiting!");
        GCFScheduler::instance()->stop();
        break;
    }

    return (GCFEvent::HANDLED);
}

//
// startControlPort(event, port)
//
GCFEvent::TResult CEPlogProcessor::startControlPort(GCFEvent&  event, GCFPortInterface&    port)
{
    LOG_DEBUG_STR("startControlPort:" << eventName(event) << "@" << port.getName());

    switch (event.signal) {
    case F_ENTRY:
        itsControlPort->autoOpen(0, 10, 2);    // report within 10 seconds.
        break;

    case F_CONNECTED:
        LOG_DEBUG("Listener is started, going to operational mode");
        TRAN (CEPlogProcessor::operational);
        break;

    case F_DISCONNECTED:
		// DISCO from listener of controlPort: in both cases quit.
        LOG_FATAL_STR("Cannot open the controlport on port " << itsControlPort->getPortNumber() << ". Quiting!");
        GCFScheduler::instance()->stop();
        break;
    }

    return (GCFEvent::HANDLED);
}

void CEPlogProcessor::collectGarbage()
{
  if (!itsLogStreamsGarbage.empty()) {
    LOG_DEBUG("Cleaning up garbage");
    for (unsigned i = 0; i < itsLogStreamsGarbage.size(); i++)
      delete itsLogStreamsGarbage[i];

    itsLogStreamsGarbage.clear();
  }  
}


void CEPlogProcessor::processParset( const std::string &observationID )
{
    time_t now = time(0L);
    unsigned obsID;

    if (sscanf(observationID.c_str(), "%u", &obsID) != 1) {
      LOG_ERROR_STR("Observation ID not numerical: " << observationID);
      return;
    }

    // parsets are in LOFAR_SHARE_LOCATION
    string filename(formatString("%s/Observation%s", 
                                 LOFAR_SHARE_LOCATION, observationID.c_str()));

    LOG_INFO_STR("Reading parset for observation " << observationID << " from " << filename);

    ParameterSet parset(filename);
    Observation obs(&parset, false, itsNrPsets);
    string observationPrefix = parset.locateModule("Observation") + "Observation.";

    CEPFeedback &feedback = itsCEPFeedback[obsID];

    unsigned nrStreams = obs.streamsToStorage.size();

    // process all the writers
    for( unsigned i = 0; i < nrStreams; i++ ) {
      Observation::StreamToStorage &s = obs.streamsToStorage[i];

      unsigned hostNr;

      if (sscanf(s.destStorageNode.c_str(), "%*[^0-9]%u", &hostNr) != 1) {
        LOG_WARN_STR("Could not extract host number from name: " << s.destStorageNode );
        continue;
      }

      hostNr--; // we use 0-based indexing in our arrays

      unsigned writerIndex = hostNr * itsNrWriters + s.writerNr;
      RTDBPropertySet *writer = itsWriters[writerIndex];

      // reset/fill all fields for this writer
      writer->setValue("written",         GCFPVInteger(0), now, false);
      writer->setValue("dropped",         GCFPVInteger(0), now, false);
      writer->setValue("fileName",        GCFPVString(s.filename), now, false);
      writer->setValue("dataRate",        GCFPVDouble(0.0), now, false);
      writer->setValue("dataProductType", GCFPVString(s.dataProduct), now, false);
      writer->setValue("observationName", GCFPVString(observationID), now, false);
      writer->flush();
    }

    // process all the adders
    for( unsigned i = 0; i < nrStreams; i++ ) {
      Observation::StreamToStorage &s = obs.streamsToStorage[i];

      unsigned adderIndex = s.sourcePset * itsNrAdders + s.adderNr;
      RTDBPropertySet *adder = itsAdders[adderIndex];

      // reset/fill all fields for this writer
      adder->setValue("dropping",        GCFPVBool(false), now, false);
      adder->setValue("dropped",         GCFPVInteger(0), now, false);
      adder->setValue("dataProductType", GCFPVString(s.dataProduct), now, false);
      adder->setValue("fileName",        GCFPVString(s.filename), now, false);
      adder->setValue("locusNode",       GCFPVString(s.destStorageNode), now, false);
      adder->setValue("directory",       GCFPVString(s.destDirectory), now, false);
      adder->setValue("observationName", GCFPVString(observationID), now, false);
      adder->flush();
    }

    if (parset.isDefined("_DPname")) {
      // register the temporary obs name
      registerObservation( obsID, parset.getString("_DPname") );
    }

    // process feedback for correlated data
    unsigned nrCorrelatedStreams = 0;

    for (unsigned i = 0; i < nrStreams; i++ ) {
      Observation::StreamToStorage &s = obs.streamsToStorage[i];

      if (s.dataProduct != "Correlated")
        continue;

      unsigned index = nrCorrelatedStreams;

      feedback.addSubband(index);
      feedback.setSubbandKey(index, "filename",             s.filename);
      feedback.setSubbandKey(index, "location",             s.destStorageNode + ":" + s.destDirectory);
      feedback.setSubbandKey(index, "startTime",            parset.getString(observationPrefix + "startTime"));

      nrCorrelatedStreams++; 
    }
}


void CEPlogProcessor::writeFeedback( int obsID )
{
    // feedback parsets are to be stored in in LOFAR_SHARE_LOCATION
    string filename(formatString("%s/Observation%d_feedback", 
                                 LOFAR_SHARE_LOCATION, obsID));

    // add a prefix
    ParameterSet prefixedFeedback = itsFeedback[obsID].makeSubset("", "LOFAR.ObsSW.");

    prefixedFeedback.writeFile(filename);
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
        collectGarbage();
        break;

    case F_ACCEPT_REQ:
        _handleConnectionRequest();
        break;

    case F_CONNECTED:
        break;

    case F_DISCONNECTED: 
        _deleteStream(port);
        break;
    
    case F_DATAIN:
        _handleDataStream(&port);
        break;
	
	case CONTROL_ANNOUNCE: {
		CONTROLAnnounceEvent	announce(event);
		LOG_DEBUG_STR("Received annoucement for Observation " << announce.observationID);

        processParset(announce.observationID);

		break;
	}
    }
    return (GCFEvent::HANDLED);
}

//
// _deleteStream(GCFPortInterface&  port)
//
void CEPlogProcessor::_deleteStream(GCFPortInterface&   port) 
{
    LOG_DEBUG_STR("_deleteStream");
    port.close();

    map<GCFPortInterface*, streamBuffer_t>::iterator    theStream = itsLogStreams.find(&port);
    if (theStream != itsLogStreams.end()) {
        streamBuffer_t &sb = theStream->second;
        delete sb.buffer;

        itsLogStreams.erase(theStream);
    }

    // schedule to delete, since the parent may still be referring to
    // port and require info from it
    itsLogStreamsGarbage.push_back(&port);
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
        _deleteStream(*port);
        return;
    }

    LOG_DEBUG_STR("Read " << newBytes << " bytes.");
    stream.buffer->incTail( newBytes );

    char lineBuf[1024];
    while (stream.buffer->getLine( lineBuf, sizeof lineBuf )) {
      LOG_DEBUG_STR("Read log line '" << lineBuf << "'" );
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
    &tm.tm_mday, &tm.tm_mon, &tm.tm_year) != 3) {
    validtime = false;
   } else {
    // tm_year starts counting from 1900

    if (tm.tm_year > 1900) {
      // YYYY
      tm.tm_year -= 1900;
    } else {
      // YY -- we won't see loglines pre 2000.
      tm.tm_year += 100;
    }

    // tm_mon starts counting from 0
    tm.tm_mon--;
   }

  if (sscanf(timestr, "%u:%u:%u",  // ignore milliseconds
    &tm.tm_hour, &tm.tm_min, &tm.tm_sec) != 3) {
    validtime = false;
  }

  if (validtime) {
    tm.tm_isdst = 0; // UTC knows no daylight saving

    ts = mktime(&tm);

    if (ts <= 0)
      validtime = false;
  }    

  if (!validtime) {
    LOG_WARN_STR("Invalid timestamp: " << datestr << " " << timestr << "; using now()");

    ts = time(0L);
  }

  LOG_DEBUG_STR("Timestamp: " << datestr << " " << timestr << " converted to " << ts);

  return ts;
}


//
// _processLogLine(char*)
//
//
void CEPlogProcessor::_processLogLine(const char *cString)
{
    if (*cString == 0) {
      return;
    }

    // debug hack
    if (!strcmp(cString,"quit")) {
      finish();
      return;
    }

    // example log line:
    // Storage@locus001 09-12-10 11:33:13.240 DEBUG [obs 21855 output 1 subband 223] InputThread::~InputThread()
    // ^^^^^^^ ^^^^^^^^ ^^^^^^^^ ^^^^^^^^^^^^ ^^^^^  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^  ^^^^^^^^^^^^^^^^^^^^^^^^^^^
    // |       |        date     time         |      target                          msg
    // |       |                              loglevel
    // |       processHost
    // processName
    unsigned bufsize = strlen(cString) + 1;

    vector<char> processName(bufsize), processHost(bufsize), date(bufsize), time(bufsize), loglevel(bufsize), msg(bufsize);
    vector<char> target(bufsize), tail(bufsize);

    // TODO: support both exe@nr (IONProc@00) and exe@host (Storage_main@locus002)
    if (sscanf(cString, "%[^@]@%s %s %s %s %[^\n]",
      &processName[0],
      &processHost[0],
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

    LOG_DEBUG_STR("Process: " << &processName[0] << " Host: " << &processHost[0] << " Date: " << &date[0] << " Time: " << &time[0] << " Loglevel: " << &loglevel[0] << " Message: " << &msg[0]);

    struct logline logline;

    logline.process   = &processName[0];
    logline.host      = &processHost[0];
    logline.date      = &date[0];
    logline.time      = &time[0];
    logline.loglevel  = &loglevel[0];
    logline.fullmsg   = cString;

    if (sscanf(&msg[0], "[%[^]]] %[^\n]", &target[0], &tail[0]) == 2) {
      logline.target = &target[0];
      logline.msg    = &tail[0];
    } else {
      logline.target = "";
      logline.msg    = &msg[0];
    }

    logline.timestamp   = _parseDateTime(logline.date, logline.time);
    logline.obsID       = _getParam(logline.target, "obs ");

    string tempObsName = logline.obsID >= 0 ? getTempObsName(logline.obsID, logline.msg) : "";

    logline.tempobsname = tempObsName.c_str();

    if (!strcmp(logline.process,"IONProc")) {
      _processIONProcLine(logline);
    } else if (!strcmp(logline.process,"CNProc")) {
      _processCNProcLine(logline);
    } else if (!strcmp(logline.process,"Storage_main")) {
      _processStorageLine(logline);
    } else {
      LOG_DEBUG_STR("Unknown process: " << logline.process);
    }
}

int CEPlogProcessor::_getParam(const char *msg,const char *param) const
{
  const char *result = strstr(msg, param);
  int value;

  if (!result)
    return -1;

  if (sscanf(result + strlen(param), "%d", &value) != 1)
    return -1;

  return value;
}

string CEPlogProcessor::getTempObsName(int obsID, const char *msg)
{
  vector<char> tempObsName(strlen(msg)+1);

  // register the tempObsName if this line announces it
  if (sscanf(msg,"PVSS name: %[^\n]", &tempObsName[0]) == 1) {
    LOG_INFO_STR("Observation " << obsID << " is mapped to " << &tempObsName[0]);

    registerObservation( obsID, string(&tempObsName[0]) );
  }

  if (!strcmp(msg,"----- Job finished successfully")
   || !strcmp(msg,"----- Job cancelled successfully")) {
    LOG_INFO_STR("Observation " << obsID << " ended");

    unregisterObservation(obsID);

    return "";
  }

  // lookup the obsID in our list
  if (!itsTempObsMapping.exists(obsID)) {
    LOG_ERROR_STR("Observation ID " << obsID << " not mapped onto a temporary observation in PVSS. Cannot process log line.");
    return "";
  }

  return itsTempObsMapping.lookup(obsID);
}

void CEPlogProcessor::registerObservation(int obsID, const std::string &tempObsName)
{
  if (itsTempObsMapping.exists(obsID)) {
    ASSERTSTR(itsTempObsMapping.lookup(obsID) == tempObsName, "Observation ID remapped from " << itsTempObsMapping.lookup(obsID) << " to " << tempObsName);
    return;
  }

  itsTempObsMapping.set(obsID, tempObsName);

  itsFeedback[obsID] = ParameterSet();

  processParset(formatString("%d",obsID));
}

void CEPlogProcessor::unregisterObservation(int obsID)
{
  if (!itsTempObsMapping.exists(obsID)) {
    LOG_ERROR_STR("Observation ID " << obsID << " not registered. Cannot unregister.");
    return;
  }

  writeFeedback(obsID);

  itsFeedback.erase(obsID);

  itsTempObsMapping.erase(obsID);
}


// returns true if the given logline should be recorded in process.logMsg
bool CEPlogProcessor::_recordLogMsg(const struct logline &logline) const
{
    if (!strcmp(logline.loglevel, "INFO"))
      return true;
    if (!strcmp(logline.loglevel, "WARN"))
      return true;
    if (!strcmp(logline.loglevel, "ERROR"))
      return true;
    if (!strcmp(logline.loglevel, "FATAL"))
      return true;
    if (!strcmp(logline.loglevel, "EXCEPTION"))
      return true;

    return false;  
}

//
// _processIONProcLine(cstring)
//
void CEPlogProcessor::_processIONProcLine(const struct logline &logline)
{
    unsigned processNr;

    if (sscanf(logline.host, "%u", &processNr) != 1) {
        LOG_WARN_STR("Could not extract host number from name: " << logline.host );
        return;
    }

    if (processNr >= itsNrInputBuffers) {
        LOG_WARN_STR("Inputbuffer range = 0.." << itsNrInputBuffers << ". Index " << processNr << " is invalid");
        return;
    }

    RTDBPropertySet *inputBuffer = itsInputBuffers[processNr];

    if (_recordLogMsg(logline)) {
        inputBuffer->setValue("process.logMsg", GCFPVString(logline.fullmsg), logline.timestamp, true);
    }

    char*   result;

    // IONProc@00 31-03-11 00:17:22.438 INFO  [obs 24811] ----- Creating new job
    // IONProc@00 31-03-11 00:17:22.550 INFO  [obs 24811] Waiting for job to start: sleeping until Thu Mar 31 00:18:50 2011
    // IONProc@00 31-03-11 00:18:50.008 INFO  Storage writer on lse012: starting as rank 0
    // IONProc@00 31-03-11 00:18:50.031 INFO  [obs 24811] ----- Observation start

    unsigned bufsize = strlen(logline.msg) + 1;

    if (!strcmp(logline.msg,"----- Creating new job")) {
      LOG_DEBUG_STR("obs " << logline.obsID << " created");
    }

    if (strstr(logline.msg,"Waiting for job to start")) {
      LOG_DEBUG_STR("obs " << logline.obsID << " waiting to start");
    }

    {
      vector<char> host(bufsize);
      int rank;

      if (sscanf(logline.msg,"Storage writer on %[^:]: starting as rank %d", &host[0], &rank) == 2) {
        LOG_DEBUG_STR("obs " << logline.obsID << " starts storage writer " << rank << " on host " << &host[0]);
      }
    }

    if (!strcmp(logline.msg,"----- Observation start")) {
      LOG_DEBUG_STR("obs " << logline.obsID << " run()");
    }

    //
    // InputBuffer = input from station
    //

    // IONProc@01 23-02-11 01:02:58.687 INFO  [obs 23603 station CS005HBA1] [1298422977s, 80863], late: 17.6 ms, delays: [8.657333 us], flags 0: (0%), flags 1: (0%), flags 2: (0%), flags 3: (0%)
    // IONProc@05 07-01-11 20:57:56.765 INFO  [obs 1002069 station S10] [1294433876s, 0], late: 8.85 ms, delays: [-616.3421 ns], flags 0: [0..52992> (100%), flags 1: [0..52992> (100%), flags 2: [0..52992> (100%), flags 3: [0..52992> (100%)

    if ((result = strstr(logline.msg, " late: "))) {
        float late = 0.0f;

        if (sscanf(result, " late: %f ", &late) == 1 ) {
            LOG_DEBUG_STR("[" << processNr << "] Late: " << late);
            inputBuffer->setValue("late", GCFPVDouble(late), logline.timestamp, false);
        }

        // 0% flags look like : flags 0: (0%)
        // filled% flags look like : flags 0: [nr..nr> (10.5%)
        if ((result = strstr(logline.msg, "flags 0:"))) {
            float flags0, flags1, flags2, flags3;

            if (sscanf(result, "flags 0:%*[^(](%f%%), flags 1:%*[^(](%f%%), flags 2:%*[^(](%f%%), flags 3:%*[^(](%f%%)",
              &flags0, &flags1, &flags2, &flags3) == 4) {
                LOG_DEBUG(formatString("[%d] %%bad: %.2f, %.2f, %.2f, %.2f", processNr, flags0, flags1, flags2, flags3));

                inputBuffer->setValue("stream0.percBad", GCFPVDouble(flags0), logline.timestamp, false);
                inputBuffer->setValue("stream1.percBad", GCFPVDouble(flags1), logline.timestamp, false);
                inputBuffer->setValue("stream2.percBad", GCFPVDouble(flags2), logline.timestamp, false);
                inputBuffer->setValue("stream3.percBad", GCFPVDouble(flags3), logline.timestamp, false);
            }
        }

        inputBuffer->flush();
        return;
    }

    // IONProc@36 23-02-11 00:59:59.151 DEBUG [obs 23603 station CS003HBA0]  ION->CN:  483 ms
    if ((result = strstr(logline.msg, "ION->CN:"))) {
        float ioTime = 0.0f;
        if (sscanf(result, "ION->CN:%f", &ioTime) == 1) {
            LOG_DEBUG_STR("[" << processNr << "] ioTime: " << ioTime);
            inputBuffer->setValue("IOTime", GCFPVDouble(ioTime), logline.timestamp);
            return;
        }
    }

    // IONProc@36 23-02-11 00:59:59.673 INFO  [station CS003HBA0] received packets = [12329,12328,12292,12329], us/sy/in/id(0): [21/20/10/51(25)]
    if ((result = strstr(logline.msg, "received packets = ["))) {
        int received[4] = {0,0,0,0};
        int badsize[4] = {0,0,0,0};
        int badtimestamp[4] = {0,0,0,0};

        if (sscanf(result, "received packets = [%d,%d,%d,%d]", &received[0], &received[1], &received[2], &received[3]) == 4) {
          LOG_DEBUG(formatString("[%d] blocks: %d, %d, %d, %d", processNr, received[0], received[1], received[2], received[3]));
          inputBuffer->setValue("stream0.blocksIn", GCFPVInteger(received[0]), logline.timestamp, false);
          inputBuffer->setValue("stream1.blocksIn", GCFPVInteger(received[1]), logline.timestamp, false);
          inputBuffer->setValue("stream2.blocksIn", GCFPVInteger(received[2]), logline.timestamp, false);
          inputBuffer->setValue("stream3.blocksIn", GCFPVInteger(received[3]), logline.timestamp, false);

          // flush will happen below
        }

        // if rejected was found in same line this means that a certain amount of blocks was rejected, 
        // set this into the database. If no rejected was found, it means 0 blocks were rejected, so DB can be reset to 0
        if ((result = strstr(logline.msg, " bad size = ["))) {
          if (sscanf(result, " bad size = [%d,%d,%d,%d]", &badsize[0], &badsize[1], &badsize[2], &badsize[3]) == 4) {
            LOG_DEBUG(formatString("[%d] rejected: bad size blocks: %d, %d, %d, %d", processNr, badsize[0], badsize[1], badsize[2], badsize[3]));
          } else {
            badsize[0] = 0;
            badsize[1] = 0;
            badsize[2] = 0;
            badsize[3] = 0;
          }
        }

        if ((result = strstr(logline.msg, " bad timestamps = ["))) {
          if (sscanf(result, " bad timestamps = [%d,%d,%d,%d]", &badtimestamp[0], &badtimestamp[1], &badtimestamp[2], &badtimestamp[3]) == 4) {
            LOG_DEBUG(formatString("[%d] rejected: bad timestamp blocks: %d, %d, %d, %d", processNr, badtimestamp[0], badtimestamp[1], badtimestamp[2], badtimestamp[3]));
          } else {
            badtimestamp[0] = 0;
            badtimestamp[1] = 0;
            badtimestamp[2] = 0;
            badtimestamp[3] = 0;
          }
        }

        inputBuffer->setValue("stream0.rejected", GCFPVInteger(badsize[0] + badtimestamp[0]), logline.timestamp, false);
        inputBuffer->setValue("stream1.rejected", GCFPVInteger(badsize[1] + badtimestamp[1]), logline.timestamp, false);
        inputBuffer->setValue("stream2.rejected", GCFPVInteger(badsize[2] + badtimestamp[2]), logline.timestamp, false);
        inputBuffer->setValue("stream3.rejected", GCFPVInteger(badsize[3] + badtimestamp[3]), logline.timestamp, false);
        inputBuffer->flush();
        return;
    }


    //
    // Adder
    //
    int adderNr = _getParam(logline.target, "adder ");

    if (adderNr >= 0) {
      int adderIndex = processNr * itsNrAdders + adderNr;
      RTDBPropertySet *adder = itsAdders[adderIndex];

      // TODO: reset drop count at start of obs --> maybe MAC should do that when assigning the mapping?

      // IONProc@17 07-01-11 20:59:00.981 WARN  [obs 1002069 output 6 index L1002069_B102_S0_P000_bf.raw] Dropping data
      if ((result = strstr(logline.msg, "Dropping data"))) {
        LOG_DEBUG(formatString("[%d] Dropping data started ", adderIndex));

        adder->setValue("dropping", GCFPVBool(true), logline.timestamp, false);
        adder->flush();
        return;
      }

      // IONProc@23 07-01-11 20:58:27.848 WARN  [obs 1002069 output 6 index L1002069_B139_S0_P000_bf.raw] Dropped 9 blocks this time and 15 blocks since start
      if ((result = strstr(logline.msg, "Dropped "))) {
        int dropped = 0, total = 0;

        LOG_DEBUG(formatString("[%d] Dropping data ended ",adderIndex));

        if (sscanf(result, "Dropped %d blocks this time and %d blocks since start", &dropped, &total) == 2) {
          LOG_DEBUG(formatString("[%d] Dropped %d for a total of %d", processNr, dropped, total));
          adder->setValue("dropped", GCFPVInteger(total), logline.timestamp, false);
        }
        adder->setValue("dropping", GCFPVBool(false), logline.timestamp, false);
        adder->flush();
        return;
      }
    }

    ParameterSet *feedback = 0;

    if (logline.obsID >= 0 && observationRegistered(logline.obsID)) {
      feedback = &itsFeedback[logline.obsID];
    }

    if (feedback && (result = strstr(logline.msg, "LTA FEEDBACK: "))) {
      vector<char> key(strlen(logline.msg)+1);
      vector<char> value(strlen(logline.msg)+1);

      if (sscanf(result, "LTA FEEDBACK: %s = %[^\n]s", &key[0], &value[0]) == 2) {
        feedback->replace(&key[0], &value[0]);

        LOG_DEBUG_STR("Observation " << logline.obsID << ": Added LTA feedback parameter " << &key[0] << " = " << &value[0]);
      }
    }  
}

void CEPlogProcessor::_processCNProcLine(const struct logline &logline)
{ 
  char *result;

  // CNProc@0000 13-02-12 12:13:44.823 WARN  [obs 1003431 phases 111] Station S17 subband 0 consists of only zeros.
  if ((result = strstr(logline.msg, "consists of only zeros"))) {
    int subband = 0;
    vector<char> stationName(strlen(logline.msg));
    if (sscanf(logline.msg, "Station %[^ ]s subband %d consists of only zeros", &stationName[0], &subband) == 2) {
      LOG_DEBUG(formatString("[%s] Subband %d is zeros", &stationName[0], subband));
    }
    return;
  }
}

void CEPlogProcessor::_processStorageLine(const struct logline &logline)
{
    unsigned hostNr;

    if (sscanf(logline.host, "%u", &hostNr) == 1) {
        // Storage_main@00 will yield 00, the index of the first storage node, which is output by Log4Cout
        LOG_FATAL_STR("Need a host name, not a number, for Storage (don't use Log4Cout?): " << logline.host );
        return;
    } else if (sscanf(logline.host, "%*[^0-9]%u", &hostNr) != 1) {
        LOG_WARN_STR("Could not extract host number from name: " << logline.host );
        return;
    }

    if (hostNr < 1 || hostNr > itsNrStorage) {
        LOG_WARN_STR("Storage range = 1.." << itsNrStorage << ". Index " << hostNr << " is invalid");
        return;
    }

    hostNr--; // use 0-based indexing in our arrays

    char*   result;

    int writerNr = _getParam(logline.target, "writer ");

    if (writerNr >= 0) {
      int writerIndex = hostNr * itsNrWriters + writerNr;
      RTDBPropertySet *writer = itsWriters[writerIndex];

      if (_recordLogMsg(logline)) {
        writer->setValue("process.logMsg", GCFPVString(logline.fullmsg), logline.timestamp, true);
      }

      ParameterSet *feedback = 0;

      if (logline.obsID >= 0 && observationRegistered(logline.obsID)) {
        feedback = &itsFeedback[logline.obsID];
      }

      if (feedback && (result = strstr(logline.msg, "LTA FEEDBACK: "))) {
        vector<char> key(strlen(logline.msg)+1);
        vector<char> value(strlen(logline.msg)+1);

        if (sscanf(result, "LTA FEEDBACK: %s = %[^\n]s", &key[0], &value[0]) == 2) {
          feedback->replace(&key[0], &value[0]);

          LOG_DEBUG_STR("Observation " << logline.obsID << ": Added LTA feedback parameter " << &key[0] << " = " << &value[0]);
        }
      }

      // Storage_main@locus088 10-02-12 13:20:01.056 INFO  [obs 45784 type 2 stream  12 writer   0] [OutputThread] Written block with seqno = 479, 480 blocks written, 0 blocks dropped
      if ((result = strstr(logline.msg, "Written block"))) {
        int seqno = 0, written = 0, dropped = 0, perc_written = 0;
        if (sscanf(result, "Written block with seqno = %d, %d blocks written (%d%%), %d blocks dropped", &seqno, &written, &perc_written, &dropped) == 4) {
          LOG_DEBUG(formatString("[%d] Written %d, dropped %d", writerNr, written, dropped));
          writer->setValue("written", GCFPVInteger(written), logline.timestamp, false);
          writer->setValue("dropped", GCFPVInteger(dropped), logline.timestamp, false);
          writer->flush();

          if (feedback) {
            feedback->setSubbandKey(streamNr, "percentageWritten", formatString("%d", perc_written));
          }
        }
        return;
      }

      // Storage_main@locus088 10-02-12 13:20:01.057 INFO  [obs 45784 type 2 stream  12 writer   0] [OutputThread] Finished writing: 480 blocks written, 0 blocks dropped: 0% lost
      if ((result = strstr(logline.msg, "Finished writing"))) {
        int written = 0, dropped = 0, perc_written = 0;
        if (sscanf(result, "Finished writing: %d blocks written (%d%%), %d blocks dropped", &written, &perc_written, &dropped) == 3) {
          LOG_DEBUG(formatString("[%d] Written %d, dropped %d", writerNr, written, dropped));
          writer->setValue("written", GCFPVInteger(written), logline.timestamp, false);
          writer->setValue("dropped", GCFPVInteger(dropped), logline.timestamp, false);
          writer->flush();

          if (feedback) {
            feedback->setSubbandKey(streamNr, "percentageWritten", formatString("%d", perc_written));
          }
        }
        return;
      }
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
            itsWritersBuf[processNr].timeStr[rank]  = tim;
            itsWritersBuf[processNr].count[rank] = count;
        }
        return;
    }
#endif

#if 0
    // IONProc already reports dropped blocks, and knows more (for example, blocks dropped at the end of an obs)

    // Storage_main@locus001 25-05-11 19:36:38.862 WARN  [obs 27304 output 1 index 224] OutputThread dropped 3 blocks 
    {
      int blocks, index, output;
        
      if (sscanf(result, "[obs %*d output %d index %d] OutputThread dropped %d blocks", &output, &index, &blocks) == 3) {
      {
          LOG_DEBUG(formatString("Dropped %d blocks: %d, subband: %d, output: %d", blocks, subband, output));

  // dropped has no rank in yet 
  // itsWritersBuf[processNr].dropped[rank] = result;
      }
        return;
    }
#endif
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

        itsTimerPort->cancelAllTimers();
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
