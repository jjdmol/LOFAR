

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

#include <APS/ParameterSet.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_iomanip.h>
#include <CS1_Storage/SubbandWriter.h>
#include <CS1_Storage/MSWriter.h>
#include <Stream/FileStream.h>
#include <Stream/NullStream.h>
#include <Stream/SocketStream.h>

#ifdef USE_MAC_PI
#include <GCF/GCF_PVDouble.h>
#include <GCF/GCF_PVString.h>
#endif

#include <boost/lexical_cast.hpp>

#include <time.h>

namespace LOFAR {
namespace CS1 {

SubbandWriter::SubbandWriter(const CS1_Parset *ps, unsigned rank) 
:
  itsCS1PS(ps),
  itsRank(rank),
  itsTimeCounter(0),
  itsTimesToIntegrate(ps->storageIntegrationSteps()),
  itsFlagsBuffers(0),
  itsWeightsBuffers(0),
  itsVisibilities(0),
  itsWriteTimer ("writing-MS")
#ifdef USE_MAC_PI
,itsPropertySet(0)
#endif
{
#ifdef USE_MAC_PI
  itsWriteToMAC = itsPS.getBool("Storage.WriteToMAC");
#endif
  itsNStations = itsCS1PS->nrStations();
  itsNBaselines = itsCS1PS->nrBaselines();
  itsNChannels = itsCS1PS->nrChannelsPerSubband();
  itsNBeams = itsCS1PS->nrBeams();
  unsigned pols = itsCS1PS->getUint32("Observation.nrPolarisations");
  itsNPolSquared = pols*pols;

  // itsWeightFactor = the inverse of maximum number of valid samples
  itsWeightFactor = 1.0 / (ps->BGLintegrationSteps() * ps->IONintegrationSteps() * ps->storageIntegrationSteps());
  
  itsNVisibilities = itsNBaselines * itsNChannels * itsNPolSquared;
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
  string   prefix            = "OLAP.OLAP_Conn.BGLProc_Storage";
  string   connectionType    = itsCS1PS->getString(prefix + "_Transport");

  itsInputStreams.resize(itsCS1PS->nrPsetsPerStorage());

  for (unsigned i = 0; i < itsCS1PS->nrPsetsPerStorage(); i ++)
    if (connectionType == "NULL") {
      std::cout << "input " << i << ": null stream" << std::endl;
      itsInputStreams[i] = new NullStream;
    } else if (connectionType == "TCP") {
      std::string    server = itsCS1PS->getStringVector(prefix + "_ServerHosts")[itsRank];
      unsigned short port   = boost::lexical_cast<unsigned short>(itsCS1PS->getPortsOf(prefix)[i]);

      std::cout << "input " << i << ": tcp:" << server << ':' << port << std::endl;
      itsInputStreams[i] = new SocketStream(server.c_str(), port, SocketStream::TCP, SocketStream::Server);
    } else if (connectionType == "FILE") {
      std::string filename = itsCS1PS->getString(prefix + "_BaseFileName") + '.' +
			      boost::lexical_cast<std::string>(itsRank) + '.' +
			      boost::lexical_cast<std::string>(i);

      std::cout << "input " << i << ": file:" << filename << std::endl;
      itsInputStreams[i] = new FileStream(filename.c_str());
    } else {
      throw std::runtime_error("unsupported ION->Storage stream type");
    }
}


void *SubbandWriter::inputThreadStub(void *arg)
{
  try {
    static_cast<SubbandWriter *>(arg)->inputThread();
  } catch (Exception &ex) {
    std::cerr << "caught Exception: " << ex.what() << std::endl;
  } catch (std::exception &ex) {
    std::cerr << "caught std::exception: " << ex.what() << std::endl;
  } catch (...) {
    std::cerr << "caught non-std::exception" << std::endl;
  } 

  return 0;
}


void SubbandWriter::inputThread()
{
  bool nullInput = dynamic_cast<NullStream *>(itsInputStreams[0]) != 0;

  do {
    for (unsigned sb = 0; sb < itsNrSubbandsPerStorage; ++ sb) {
      // find out from which input channel we should read
      unsigned	     pset  = sb / itsNrSubbandsPerPset;
      CorrelatedData *data = itsFreeQueue.remove();

      try {
	data->read(itsInputStreams[pset]);
      } catch (Stream::EndOfStreamException &) {
	itsReceiveQueue.append(0); // signal main thread that this was the last
	itsFreeQueue.append(data);
	return;
      }

      itsReceiveQueue.append(data);
    }
  } while (!nullInput);  // prevent infinite loop when using NullStream
}


void SubbandWriter::createInputThread()
{
  if (pthread_create(&itsInputThread, 0, inputThreadStub, this) != 0) {
    std::cerr << "could not create input thread" << std::endl;
    exit(1);
  }
}


void SubbandWriter::stopInputThread()
{
  if (pthread_join(itsInputThread, 0) != 0)
    std::cerr << "could not join input thread";
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

  for (unsigned i = 0; i < nrInputBuffers; i ++) {
    itsArenas.push_back(new MallocedArena(CorrelatedData::requiredSize(itsNBaselines), 32));
    itsFreeQueue.append(new CorrelatedData(*itsArenas.back(), itsNBaselines));
  }

  double startTime = itsCS1PS->startTime();
  LOG_TRACE_VAR_STR("startTime = " << startTime);
  
  vector<double> antPos = itsCS1PS->positions();
  ASSERTSTR(antPos.size() == 3 * itsNStations,
	    antPos.size() << " == " << 3 * itsNStations);
  itsNrSubbandsPerPset	  = itsCS1PS->nrSubbandsPerPset();
  itsNrSubbandsPerStorage = itsNrSubbandsPerPset * itsCS1PS->nrPsetsPerStorage();
  LOG_TRACE_VAR_STR("SubbandsPerStorage = " << itsNrSubbandsPerStorage);
  vector<string> storageStationNames = itsCS1PS->getStringVector("OLAP.storageStationNames");

  itsWriters.resize(itsNrSubbandsPerStorage);
  
  vector<unsigned> subbandToBeamMapping = itsCS1PS->subbandToBeamMapping();
  
  for (unsigned i = 0; i < itsNrSubbandsPerStorage; i ++) {
    unsigned currentSubband = itsRank * itsNrSubbandsPerStorage + i;

    itsWriters[i] = new MSWriter(
      itsCS1PS->getMSname(currentSubband).c_str(),
      startTime, itsCS1PS->storageIntegrationTime(), itsNChannels,
      itsNPolSquared, itsNStations, antPos,
      storageStationNames, itsTimesToIntegrate);

    unsigned       beam    = subbandToBeamMapping[currentSubband];
    vector<double> beamDir = itsCS1PS->getBeamDirection(beam);
    itsWriters[i]->addField(beamDir[0], beamDir[1], beam); // FIXME add 1???
  }

  vector<double> refFreqs = itsCS1PS->subbandToFrequencyMapping();

  // Now we must add \a itsNrSubbandsPerStorage to the measurement set. The
  // correct indices for the reference frequencies are in the vector of
  // subbandIDs.      
  itsBandIDs.resize(itsNrSubbandsPerStorage);
  double chanWidth = itsCS1PS->channelWidth();
  LOG_TRACE_VAR_STR("chanWidth = " << chanWidth);

  std::vector<double> frequencies = itsCS1PS->subbandToFrequencyMapping();

  for (unsigned sb = 0; sb < itsNrSubbandsPerStorage; ++ sb) {
    // compensate for the half-channel shift introduced by the PPF
    double refFreq = frequencies[itsRank * itsNrSubbandsPerStorage + sb] - chanWidth / 2;
    itsBandIDs[sb] = itsWriters[sb]->addBand(itsNPolSquared, itsNChannels, refFreq, chanWidth);
  }
  
  // Allocate buffers
  if (itsTimesToIntegrate > 1) {
    itsFlagsBuffers   = new bool[itsNrSubbandsPerStorage * itsNVisibilities];
    itsWeightsBuffers = new float[itsNrSubbandsPerStorage * itsNBaselines * itsNChannels];
    itsVisibilities   = new fcomplex[itsNrSubbandsPerStorage * itsNVisibilities];
    clearAllSums();
  } else {
    itsFlagsBuffers   = new bool[itsNVisibilities];
    itsWeightsBuffers = new float[itsNBaselines * itsNChannels];
  }
#endif // defined HAVE_AIPSPP

  createInputStreams();
  createInputThread();
}


void SubbandWriter::clearAllSums()
{
  assert(itsTimesToIntegrate > 1);
  memset(itsWeightsBuffers, 0, itsNrSubbandsPerStorage * itsNBaselines * itsNChannels * sizeof(float));
  memset(itsVisibilities, 0, itsNrSubbandsPerStorage * itsNVisibilities * sizeof(fcomplex));
  for (unsigned i = 0; i < itsNrSubbandsPerStorage * itsNVisibilities; i++) {
    itsFlagsBuffers[i] = true;
  }
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


bool SubbandWriter::processSubband(unsigned sb)
{
  CorrelatedData *data = itsReceiveQueue.remove();

  if (data == 0)
    return false;

#if defined HAVE_AIPSPP
  // Write one set of visibilities of size
  // fcomplex[itsNBaselines][itsNChannels][npol][npol]

  unsigned short *valSamples = data->nrValidSamples.origin();
  fcomplex	 *newVis     = data->visibilities.origin();
 
  if (itsTimesToIntegrate > 1) {
    for (unsigned i = 0; i < itsNBaselines * itsNChannels; i ++) {
      itsWeightsBuffers[sb * itsNBaselines * itsNChannels + i] += itsWeightFactor * valSamples[i];
      bool flagged = valSamples[i] == 0;
      itsFlagsBuffers[sb * itsNVisibilities + 4 * i    ] &= flagged;
      itsFlagsBuffers[sb * itsNVisibilities + 4 * i + 1] &= flagged;
      itsFlagsBuffers[sb * itsNVisibilities + 4 * i + 2] &= flagged;
      itsFlagsBuffers[sb * itsNVisibilities + 4 * i + 3] &= flagged;
      // Currently we just add the samples, this way the time centroid stays in place
      // We could also divide by the weight and multiple the sum by the total weight.
      itsVisibilities[sb * itsNVisibilities + 4 * i    ] += newVis[4 * i    ];
      itsVisibilities[sb * itsNVisibilities + 4 * i + 1] += newVis[4 * i + 1];
      itsVisibilities[sb * itsNVisibilities + 4 * i + 2] += newVis[4 * i + 2];
      itsVisibilities[sb * itsNVisibilities + 4 * i + 3] += newVis[4 * i + 3];
    }

    if ((itsTimeCounter + 1) % itsTimesToIntegrate == 0) {
      itsWriteTimer.start();
      itsWriters[sb]->write(itsBandIDs[sb], 0, itsNChannels, itsTimeCounter,
			    itsNVisibilities,
			    &itsVisibilities[sb * itsNVisibilities],
			    &itsFlagsBuffers[sb * itsNVisibilities], 
	&itsWeightsBuffers[sb * itsNBaselines * itsNChannels]);
      itsWriteTimer.stop();
    }
  } else {
    for (unsigned i = 0; i < itsNBaselines * itsNChannels; i ++) {
      itsWeightsBuffers[i] = itsWeightFactor * valSamples[i];
      bool flagged = valSamples[i] == 0;
      itsFlagsBuffers[4 * i    ] = flagged;
      itsFlagsBuffers[4 * i + 1] = flagged;
      itsFlagsBuffers[4 * i + 2] = flagged;
      itsFlagsBuffers[4 * i + 3] = flagged;
    }

    itsWriteTimer.start();
    itsWriters[sb]->write(itsBandIDs[sb], 0, itsNChannels, itsTimeCounter,
			  itsNVisibilities, newVis, itsFlagsBuffers,
			  itsWeightsBuffers);
    itsWriteTimer.stop();
  }
#endif

  itsFreeQueue.append(data);
  return true;
}


void SubbandWriter::process() 
{
  while (true) {
    writeLogMessage();

#if defined HAVE_AIPSPP
    if (itsTimesToIntegrate > 1 && itsTimeCounter % itsTimesToIntegrate == 0)
      clearAllSums();
#endif

    for (unsigned sb = 0; sb < itsNrSubbandsPerStorage; ++ sb)
      if (!processSubband(sb))
	return;

    ++ itsTimeCounter;
  }
}


void SubbandWriter::postprocess() 
{
  stopInputThread();

  for (unsigned i = 0; i < itsInputStreams.size(); i ++)
    delete itsInputStreams[i];

  itsInputStreams.clear();

  delete [] itsFlagsBuffers;	itsFlagsBuffers   = 0;
  delete [] itsWeightsBuffers;	itsWeightsBuffers = 0;

#if defined HAVE_AIPSPP
  for (unsigned i = 0; i < itsWriters.size(); i ++)
    delete itsWriters[i];

  itsWriters.clear();
#endif

  for (unsigned i = 0; i < nrInputBuffers; i ++) {
    delete itsFreeQueue.remove();
    delete itsArenas[i];
  }

  itsArenas.clear();
  delete itsVisibilities;	itsVisibilities   = 0;

  cout<<itsWriteTimer<<endl;
}

} // namespace CS1
} // namespace LOFAR
