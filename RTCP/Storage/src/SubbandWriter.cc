//#  SubbandWriter.cc: Writes visibilities in an AIPS++ measurement set
//#
//#  Copyright (C) 2002-2005
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

#include <Common/ParameterSet.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_iomanip.h>
#include <Storage/SubbandWriter.h>
#include <Storage/MSWriter.h>
#include <Storage/MSWriterNull.h>
#include <Storage/MSWriterCasa.h>
#include <Storage/MSWriterFile.h>
#include <Stream/FileStream.h>
#include <Stream/NullStream.h>
#include <Stream/SocketStream.h>
#include <Interface/Exceptions.h>

#ifdef USE_MAC_PI
#include <GCF/GCF_PVDouble.h>
#include <GCF/GCF_PVString.h>
#endif

#include <boost/lexical_cast.hpp>

#include <time.h>

namespace LOFAR {
namespace RTCP {

SubbandWriter::SubbandWriter(const Parset *ps, unsigned rank, unsigned size) 
:
  itsPS(ps),
  itsRank(rank),
  itsSize(size),
  itsPipelineOutputSet(*ps),
  itsNrOutputs(itsPipelineOutputSet.size()),
  itsTimeCounter(0),
  itsVisibilities(0),
  itsWriteTimer ("writing-MS", false, true)
#ifdef USE_MAC_PI
,itsPropertySet(0)
#endif
{
#ifdef USE_MAC_PI
  itsWriteToMAC = itsPS.getBool("Storage.WriteToMAC");
#endif
  if (itsPS->nrTabStations() > 0) {
    itsNStations = itsPS->nrTabStations();
  } else {
    itsNStations = itsPS->nrStations();
  }
  
  itsNBaselines = itsPS->nrBaselines();
  itsNChannels = itsPS->nrChannelsPerSubband();
  itsNBeams = itsPS->nrBeams();
  unsigned pols = itsPS->getUint32("Observation.nrPolarisations");
  itsNPolSquared = pols*pols;

  // itsWeightFactor = the inverse of maximum number of valid samples
  itsWeightFactor = 1.0 / (ps->CNintegrationSteps() * ps->IONintegrationSteps());
}


SubbandWriter::~SubbandWriter() 
{
#if defined HAVE_AIPSPP
  for (unsigned i = 0; i < itsWriters.size(); i ++) {
    for (unsigned output = 0; output < itsNrOutputs; output ++) {
      delete itsWriters[i][output];
    }
  }
#endif

#ifdef USE_MAC_PI
  delete itsPropertySet;

  GCF::Common::GCFPValueArray::iterator it;
  for (it = itsVArray.begin(); it != itsVArray.end(); it++){
    delete *it;
  }
  itsVArray.clear();
#endif
}


void SubbandWriter::createInputStreams()
{
  string   prefix            = "OLAP.OLAP_Conn.IONProc_Storage";
  string   connectionType    = itsPS->getString(prefix + "_Transport");

  for (unsigned subband = 0; subband < itsMyNrSubbands; subband ++) {
    unsigned subbandNumber = itsRank * itsNrSubbandsPerStorage + subband;

    if (connectionType == "NULL") {
      LOG_DEBUG_STR(itsRank << ": subband " << subbandNumber << " read from null stream");
      itsInputStreams.push_back(new NullStream);
    } else if (connectionType == "TCP") {
      std::string    server = itsPS->storageHostName(prefix + "_ServerHosts", subbandNumber);
      unsigned short port   = boost::lexical_cast<unsigned short>(itsPS->getPortsOf(prefix)[subbandNumber]);
      
      LOG_DEBUG_STR(itsRank << ": subband " << subbandNumber << " read from tcp:" << server << ':' << port);
      itsInputStreams.push_back(new SocketStream(server.c_str(), port, SocketStream::TCP, SocketStream::Server));
    } else if (connectionType == "FILE") {
      std::string filename = itsPS->getString(prefix + "_BaseFileName") + '.' +
	boost::lexical_cast<std::string>(itsRank) + '.' +
	boost::lexical_cast<std::string>(subbandNumber);
      
      LOG_DEBUG_STR(itsRank << ": subband " << subbandNumber << " read from file:" << filename);
      itsInputStreams.push_back(new FileStream(filename.c_str()));
    } else {
      THROW(StorageException, itsRank << ": unsupported ION->Storage stream type");
    }
    
    itsIsNullStream.push_back(dynamic_cast<NullStream *>(itsInputStreams.back()) != 0);
  }
}


void SubbandWriter::preprocess() 
{
#if defined HAVE_AIPSPP
  LOG_TRACE_FLOW("SubbandWriter enabling PropertySet");
#ifdef USE_MAC_PI
  if (itsWriteToMAC) {
    itsPropertySet = new GCF::CEPPMLlight::CEPPropertySet("CEP_TFCD", "TTeraFlopCorrelator", GCF::Common::PS_CAT_PERMANENT);
    itsPropertySet->enable();
    LOG_TRACE_FLOW("SubbandWriter PropertySet enabled");
  } else {
    LOG_TRACE_FLOW("SubbandWriter PropertySet not enabled");
  }
#endif

  double startTime = itsPS->startTime();
  LOG_TRACE_VAR_STR("startTime = " << startTime);
  
  vector<double> antPos = itsPS->positions();
  ASSERTSTR(antPos.size() == 3 * itsNStations,
	    antPos.size() << " == " << 3 * itsNStations);
  itsNrSubbands           = itsPS->nrSubbands();
  itsNrSubbandsPerPset	  = itsPS->nrSubbandsPerPset();
  if(itsNrSubbands % itsSize == 0) {
    itsNrSubbandsPerStorage = itsNrSubbands / itsSize;
  } else {
    itsNrSubbandsPerStorage = (itsNrSubbands / itsSize) + 1;
  }


  const double sampleFreq   = itsPS->sampleRate();
  const unsigned seconds    = static_cast<unsigned>(floor(startTime));
  const unsigned samples    = static_cast<unsigned>((startTime - floor(startTime)) * sampleFreq);

  itsSyncedStamp.setStationClockSpeed(static_cast<unsigned>(sampleFreq * 1024));
  itsSyncedStamp = TimeStamp(seconds, samples);

  LOG_TRACE_VAR_STR("SubbandsPerStorage = " << itsNrSubbandsPerStorage);

  itsMyNrSubbands = 0;
  for (unsigned i = 0; i < itsNrSubbandsPerStorage; i ++) {
    unsigned currentSubband = itsRank * itsNrSubbandsPerStorage + i;
    if(currentSubband < itsNrSubbands) {
      itsMyNrSubbands++;
    }
  }

  LOG_TRACE_VAR_STR("Subbands per storage = " << itsNrSubbandsPerStorage << ", I will store " 
		    << itsMyNrSubbands << " subbands, nrOutputs = " << itsNrOutputs);

  vector<string> stationNames;
  if (itsPS->nrTabStations() > 0)
    stationNames = itsPS->getStringVector("OLAP.tiedArrayStationNames");
  else 
    stationNames = itsPS->getStringVector("OLAP.storageStationNames");

  itsWriters.resize(itsMyNrSubbands, itsNrOutputs);
  
  vector<unsigned> subbandToBeamMapping = itsPS->subbandToBeamMapping();

  for (unsigned i = 0; i < itsMyNrSubbands; i ++) {
    unsigned currentSubband = itsRank * itsNrSubbandsPerStorage + i;
    
    for (unsigned output = 0; output < itsNrOutputs; output++ ) {
      
      string filename = itsPS->getMSname(currentSubband) + itsPipelineOutputSet[output].filenameSuffix();
      
      switch( itsPipelineOutputSet[output].writerType() ) {
      case PipelineOutput::CASAWRITER:
	
	itsWriters[i][output] = new MSWriterCasa(
	  filename.c_str(),
	  startTime, itsPS->IONintegrationTime(), itsNChannels,
	  itsNPolSquared, itsNStations, antPos,
	  stationNames, itsWeightFactor);
	
	break;
	
      case PipelineOutput::RAWWRITER:
	itsWriters[i][output] = new MSWriterFile(
	  filename.c_str(),
	  startTime, itsPS->IONintegrationTime(), itsNChannels,
	  itsNPolSquared, itsNStations, antPos,
	  stationNames, itsWeightFactor);
	break;
	
      case PipelineOutput::NULLWRITER:
	itsWriters[i][output] = new MSWriterNull(
	  filename.c_str(),
	  startTime, itsPS->IONintegrationTime(), itsNChannels,
	  itsNPolSquared, itsNStations, antPos,
	  stationNames, itsWeightFactor);
	break;
	  
      default:
	LOG_WARN_STR("unknown writer type!");
	break;
      }

      unsigned       beam    = subbandToBeamMapping[currentSubband];
      vector<double> beamDir = itsPS->getBeamDirection(beam);
      itsWriters[i][output]->addField(beamDir[0], beamDir[1], beam); // FIXME add 1???
    }
  }
  
  vector<double> refFreqs = itsPS->subbandToFrequencyMapping();

  // Now we must add \a itsNrSubbandsPerStorage to the measurement set. The
  // correct indices for the reference frequencies are in the vector of
  // subbandIDs.      
  itsBandIDs.resize(itsMyNrSubbands, itsNrOutputs);
  double chanWidth = itsPS->channelWidth();
  LOG_TRACE_VAR_STR("chanWidth = " << chanWidth);

  std::vector<double> frequencies = itsPS->subbandToFrequencyMapping();

  for (unsigned sb = 0; sb < itsMyNrSubbands; ++ sb) {
    unsigned currentSubband = itsRank * itsNrSubbandsPerStorage + sb;
    if(currentSubband < itsNrSubbands) {
      // compensate for the half-channel shift introduced by the PPF
      double refFreq = frequencies[currentSubband] - chanWidth / 2;
      for (unsigned output = 0; output < itsNrOutputs; output++ ) {
	itsBandIDs[sb][output] = itsWriters[sb][output]->addBand(itsNPolSquared, itsNChannels, refFreq, chanWidth);
      }
    }
  }
#endif // defined HAVE_AIPSPP

  itsPreviousSequenceNumbers.resize(itsMyNrSubbands, itsNrOutputs);
  for (unsigned i = 0; i < itsMyNrSubbands; ++ i) {
    for (unsigned output = 0; output < itsNrOutputs; output++ ) {
      itsPreviousSequenceNumbers[i][output] = -1;
    }
  }
  
  createInputStreams();
  
  for (unsigned sb = 0; sb < itsMyNrSubbands; sb ++) {
    itsInputThreads.push_back(new InputThread(itsInputStreams[sb], itsPS));
  }
}


void SubbandWriter::writeLogMessage()
{
  static int counter = 0;
  time_t     now     = time(0);
  char	     buf[26];

  ctime_r(&now, buf);
  buf[24] = '\0';

  LOG_INFO_STR("time = " << buf <<
#if defined HAVE_MPI
	       ", rank = " << itsRank <<
#endif
	       ", count = " << counter ++ <<
	       ", timestamp = " << itsSyncedStamp) ;
  
  itsSyncedStamp += itsPS->nrSubbandSamples() * itsPS->IONintegrationSteps();
}


void SubbandWriter::checkForDroppedData(StreamableData *data, unsigned sb, unsigned output)
{
  unsigned expectedSequenceNumber = itsPreviousSequenceNumbers[sb][output] + 1;

  if (itsIsNullStream[sb]) {
    data->sequenceNumber	           = expectedSequenceNumber;
    itsPreviousSequenceNumbers[sb][output] = expectedSequenceNumber;
  } else {
    unsigned droppedBlocks = data->sequenceNumber - expectedSequenceNumber;

    if (droppedBlocks > 0) {
      unsigned subbandNumber = itsRank * itsNrSubbandsPerStorage + sb;
      LOG_WARN_STR("dropped " << droppedBlocks << " block" << (droppedBlocks == 1 ? "" : "s") << " for subband " << subbandNumber << " and output " << output);
    }

    itsPreviousSequenceNumbers[sb][output] = data->sequenceNumber;
  }
}


bool SubbandWriter::processSubband(unsigned sb)
{
  do {
    unsigned o = itsInputThreads[sb]->itsReceiveQueueActivity.remove();
    struct InputThread::SingleInput &input = itsInputThreads[sb]->itsInputs[o];

    StreamableData *data = input.receiveQueue.remove();

    if (data == 0)
      return false;

    checkForDroppedData(data, sb, o);

#if defined HAVE_AIPSPP
    itsWriteTimer.start();
    itsWriters[sb][o]->write(itsBandIDs[sb][o], 0, itsNChannels, data);
    itsWriteTimer.stop();
#endif

    input.freeQueue.append(data);
  } while( !itsInputThreads[sb]->itsReceiveQueueActivity.empty() );

  return true;
}


void SubbandWriter::process() 
{
  std::vector<bool> finishedSubbands(itsMyNrSubbands, false);
  unsigned	    finishedSubbandsCount = 0;

  while (finishedSubbandsCount < itsMyNrSubbands) {
    writeLogMessage();

    for (unsigned sb = 0; sb < itsMyNrSubbands; ++ sb)
      if (!finishedSubbands[sb])
	if (!processSubband(sb)) {
	  finishedSubbands[sb] = true;
	  ++ finishedSubbandsCount;
	}

    ++ itsTimeCounter;
  }
}


void SubbandWriter::postprocess() 
{
  for (unsigned sb = 0; sb < itsMyNrSubbands; sb ++) {
    delete itsInputThreads[sb];
    delete itsInputStreams[sb];
  }

  itsInputThreads.clear();
  itsInputStreams.clear();
  itsIsNullStream.clear();

#if defined HAVE_AIPSPP
  for (unsigned i = 0; i < itsWriters.size(); i ++) {
    for (unsigned output = 0; output < itsNrOutputs; output ++) {
      delete itsWriters[i][output];
    }
  }
  itsWriters.resize(0,0);
#endif
  itsPreviousSequenceNumbers.resize(0,0);
  itsBandIDs.resize(0,0);

  delete itsVisibilities;	itsVisibilities   = 0;

  LOG_DEBUG_STR(itsWriteTimer);
}

} // namespace RTCP
} // namespace LOFAR
