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
#include <Interface/CN_Mode.h>

#ifdef USE_MAC_PI
#include <GCF/GCF_PVDouble.h>
#include <GCF/GCF_PVString.h>
#endif

#include <boost/lexical_cast.hpp>

#include <time.h>

namespace LOFAR {
namespace RTCP {

SubbandWriter::SubbandWriter(const Parset *ps, unsigned rank) 
:
  itsPS(ps),
  itsRank(rank),
  itsTimeCounter(0),
  itsVisibilities(0),
  itsWriteTimer ("writing-MS")
#ifdef USE_MAC_PI
,itsPropertySet(0)
#endif
{
#ifdef USE_MAC_PI
  itsWriteToMAC = itsPS.getBool("Storage.WriteToMAC");
#endif
  if (itsPS->nrTabStations() > 0)
    itsNStations = itsPS->nrTabStations();
  else
    itsNStations = itsPS->nrStations();
  
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
  for (unsigned i = 0; i < itsWriters.size(); i ++)
    delete itsWriters[i];

  itsWriters.clear();
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

  for (unsigned subband = 0; subband < itsNrSubbandsPerStorage; subband ++) {
    unsigned subbandNumber = itsRank * itsNrSubbandsPerStorage + subband;

    if (connectionType == "NULL") {
      std::cout << "subband " << subbandNumber << " read from null stream" << std::endl;
      itsInputStreams.push_back(new NullStream);
    } else if (connectionType == "TCP") {
      std::string    server = itsPS->storageHostName(prefix + "_ServerHosts", subbandNumber);
      unsigned short port   = boost::lexical_cast<unsigned short>(itsPS->getPortsOf(prefix)[subbandNumber]);

      std::cout << "subband " << subbandNumber << " read from tcp:" << server << ':' << port << std::endl;
      itsInputStreams.push_back(new SocketStream(server.c_str(), port, SocketStream::TCP, SocketStream::Server));
    } else if (connectionType == "FILE") {
      std::string filename = itsPS->getString(prefix + "_BaseFileName") + '.' +
			      boost::lexical_cast<std::string>(itsRank) + '.' +
			      boost::lexical_cast<std::string>(subbandNumber);

      std::cout << "subband " << subbandNumber << " read from file:" << filename << std::endl;
      itsInputStreams.push_back(new FileStream(filename.c_str()));
    } else {
      THROW(StorageException, "unsupported ION->Storage stream type");
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
  };
#endif

  double startTime = itsPS->startTime();
  LOG_TRACE_VAR_STR("startTime = " << startTime);
  
  vector<double> antPos = itsPS->positions();
  ASSERTSTR(antPos.size() == 3 * itsNStations,
	    antPos.size() << " == " << 3 * itsNStations);
  itsNrSubbandsPerPset	  = itsPS->nrSubbandsPerPset();
  itsNrSubbandsPerStorage = itsNrSubbandsPerPset * itsPS->nrPsetsPerStorage();
  LOG_TRACE_VAR_STR("SubbandsPerStorage = " << itsNrSubbandsPerStorage);
  vector<string> stationNames;
  if (itsPS->nrTabStations() > 0)
    stationNames = itsPS->getStringVector("OLAP.tiedArrayStationNames");
  else 
    stationNames = itsPS->getStringVector("OLAP.storageStationNames");

  itsWriters.resize(itsNrSubbandsPerStorage);
  
  vector<unsigned> subbandToBeamMapping = itsPS->subbandToBeamMapping();
  
  for (unsigned i = 0; i < itsNrSubbandsPerStorage; i ++) {
    unsigned currentSubband = itsRank * itsNrSubbandsPerStorage + i;

#if 1
    if( itsPS->mode().outputDataType() == CN_Mode::CORRELATEDDATA ) {
      // use CasaCore for CorrelatedData
      itsWriters[i] = new MSWriterCasa(
        itsPS->getMSname(currentSubband).c_str(),
        startTime, itsPS->IONintegrationTime(), itsNChannels,
        itsNPolSquared, itsNStations, antPos,
        stationNames, itsWeightFactor);
    } else {
      // write to disk otherwise
      itsWriters[i] = new MSWriterFile(
        itsPS->getMSname(currentSubband).c_str(),
        startTime, itsPS->IONintegrationTime(), itsNChannels,
        itsNPolSquared, itsNStations, antPos,
        stationNames, itsWeightFactor);
    }
#else
    itsWriters[i] = new MSWriterNull(
      itsPS->getMSname(currentSubband).c_str(),
      startTime, itsPS->IONintegrationTime(), itsNChannels,
      itsNPolSquared, itsNStations, antPos,
      stationNames, itsWeightFactor);
#endif

    unsigned       beam    = subbandToBeamMapping[currentSubband];
    vector<double> beamDir = itsPS->getBeamDirection(beam);
    itsWriters[i]->addField(beamDir[0], beamDir[1], beam); // FIXME add 1???
  }

  vector<double> refFreqs = itsPS->subbandToFrequencyMapping();

  // Now we must add \a itsNrSubbandsPerStorage to the measurement set. The
  // correct indices for the reference frequencies are in the vector of
  // subbandIDs.      
  itsBandIDs.resize(itsNrSubbandsPerStorage);
  double chanWidth = itsPS->channelWidth();
  LOG_TRACE_VAR_STR("chanWidth = " << chanWidth);

  std::vector<double> frequencies = itsPS->subbandToFrequencyMapping();

  for (unsigned sb = 0; sb < itsNrSubbandsPerStorage; ++ sb) {
    // compensate for the half-channel shift introduced by the PPF
    double refFreq = frequencies[itsRank * itsNrSubbandsPerStorage + sb] - chanWidth / 2;
    itsBandIDs[sb] = itsWriters[sb]->addBand(itsNPolSquared, itsNChannels, refFreq, chanWidth);
  }
#endif // defined HAVE_AIPSPP

  itsPreviousSequenceNumbers.resize(itsNrSubbandsPerStorage, -1);
  createInputStreams();
  
  for (unsigned sb = 0; sb < itsNrSubbandsPerStorage; sb ++)
    itsInputThreads.push_back(new InputThread(itsInputStreams[sb], itsPS));
}


void SubbandWriter::writeLogMessage()
{
  static int counter = 0;
  time_t     now     = time(0);
  char	     buf[26];

  ctime_r(&now, buf);
  buf[24] = '\0';

  cout << "time = " << buf <<
#if defined HAVE_MPI
	  ", rank = " << itsRank <<
#endif
	  ", count = " << counter ++ << endl;
}


void SubbandWriter::checkForDroppedData(StreamableData *data, unsigned sb)
{
  unsigned expectedSequenceNumber = itsPreviousSequenceNumbers[sb] + 1;

  if (itsIsNullStream[sb]) {
    data->sequenceNumber	   = expectedSequenceNumber;
    itsPreviousSequenceNumbers[sb] = expectedSequenceNumber;
  } else {
    unsigned droppedBlocks = data->sequenceNumber - expectedSequenceNumber;

    if (droppedBlocks > 0) {
      unsigned subbandNumber = itsRank * itsNrSubbandsPerStorage + sb;
      std::clog << "Warning: dropped " << droppedBlocks << " block" << (droppedBlocks == 1 ? "" : "s") << " for subband " << subbandNumber << std::endl;
    }

    itsPreviousSequenceNumbers[sb] = data->sequenceNumber;
  }
}


bool SubbandWriter::processSubband(unsigned sb)
{
  StreamableData *data = itsInputThreads[sb]->itsReceiveQueue.remove();

  if (data == 0)
    return false;

  checkForDroppedData(data, sb);

#if defined HAVE_AIPSPP
  itsWriteTimer.start();
  itsWriters[sb]->write(itsBandIDs[sb], 0, itsNChannels, data);
  itsWriteTimer.stop();
#endif

  itsInputThreads[sb]->itsFreeQueue.append(data);
  return true;
}


void SubbandWriter::process() 
{
  std::vector<bool> finishedSubbands(itsNrSubbandsPerStorage, false);
  unsigned	    finishedSubbandsCount = 0;

  while (finishedSubbandsCount < itsNrSubbandsPerStorage) {
    writeLogMessage();

    for (unsigned sb = 0; sb < itsNrSubbandsPerStorage; ++ sb)
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
  for (unsigned sb = 0; sb < itsNrSubbandsPerStorage; sb ++) {
    delete itsInputThreads[sb];
    delete itsInputStreams[sb];
  }

  itsInputThreads.clear();
  itsInputStreams.clear();
  itsIsNullStream.clear();

#if defined HAVE_AIPSPP
  for (unsigned i = 0; i < itsWriters.size(); i ++)
    delete itsWriters[i];

  itsWriters.clear();
#endif

  itsPreviousSequenceNumbers.clear();
  delete itsVisibilities;	itsVisibilities   = 0;

  cout<<itsWriteTimer<<endl;
}

} // namespace RTCP
} // namespace LOFAR
