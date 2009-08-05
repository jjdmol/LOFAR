//#  RTStorage.cc: Raw file writer for LOFAR
//#
//#  Copyright (C) 2009
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#

#include <lofar_config.h>

#include <Common/ParameterSet.h>
#include <Common/LofarLogger.h>

#include <Stream/FileStream.h>
#include <Stream/NullStream.h>
#include <Stream/SocketStream.h>

#ifdef USE_MAC_PI
#include <GCF/GCF_PVDouble.h>
#include <GCF/GCF_PVString.h>
#endif

#include <Storage/RTStorage.h>
#include <Storage/InputThread.h>
#include <Storage/MeasurementSetFormat.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>

namespace LOFAR {
namespace RTCP {

RTStorage::RTStorage(const Parset *ps, unsigned rank, unsigned size)
  :
  itsPS(ps),
  itsRank(rank), 
  itsSize(size),
  itsPipelineOutputSet(*ps),
  itsNrOutputs(itsPipelineOutputSet.size()),
  itsAlignment(512),
  itsWriteTimer("WriteTimer", false),
  bytesWritten(0)
{
  myFormat = new MeasurementSetFormat(itsPS, itsAlignment);
  ASSERTSTR( itsNrOutputs == 1, "Multiple outputs not (yet) sufficiently tested, unavailable for now" );
}

RTStorage::~RTStorage()
{
  LOG_INFO_STR(itsWriteTimer);

  LOG_INFO_STR("Wrote a total of approximately " << bytesWritten/1024/1024 << " MB in " << itsWriteTimer.getElapsed() << " seconds.");
  LOG_INFO_STR("Average write speed was " << bytesWritten/1024/1024/itsWriteTimer.getElapsed() << " MB/s.");

}
  
void RTStorage::preprocess() 
{
  /// Get various observation parameters
  itsNrSubbands = itsPS->nrSubbands();
  if (itsNrSubbands % itsSize == 0) {
    itsNrSubbandsPerStorage = itsNrSubbands / itsSize;
  } else {
    itsNrSubbandsPerStorage = (itsNrSubbands / itsSize) + 1;
  }
  
  itsMyNrSubbands = 0;
  for (unsigned i = 0; i < itsNrSubbandsPerStorage; i ++) {
    unsigned currentSubband = itsRank * itsNrSubbandsPerStorage + i;
    if(currentSubband < itsNrSubbands) {
      itsMyNrSubbands++;
    }
  }

  // create root directory of the observation tree
  if ( (mkdir(itsPS->getMSBaseDir().c_str(), 0770) != 0) && (errno != EEXIST) ) {
    LOG_FATAL_STR("failed to create directory " << itsPS->getMSBaseDir());
    perror("mkdir");
    exit(1);
  }
        
  myFDs.resize(itsMyNrSubbands, itsNrOutputs);
  
  for (unsigned sb = 0; sb < itsMyNrSubbands; sb++) {
    /// Make MeasurementSet filestructures and required tables
    myFormat->addSubband(itsRank * itsNrSubbandsPerStorage + sb);
  }

  // clean up the format object, so we don't carry this overhead into
  // the online part of the application.
  delete myFormat;
  LOG_INFO_STR("MeasuremetSet created");

  for (unsigned sb = 0; sb < itsMyNrSubbands; sb++) {
    /// create filedescriptors for raw data files
    for (unsigned o = 0; o < itsNrOutputs; o++) {
      
      std::stringstream out;
      out << o;
      
      myFDs[sb][o] = new FileStream((itsPS->getMSname( itsNrSubbandsPerStorage * itsRank + sb )+"/table.f"+out.str()+"data").c_str(), 
				    O_SYNC | O_RDWR | O_CREAT | O_TRUNC | O_DIRECT,
				    S_IRUSR |  S_IWUSR | S_IRGRP | S_IROTH);
    }
    /// create input stream and thread to handle the inputstream
    createInputStream(sb);
    itsInputThreads.push_back(new InputThread(itsInputStreams[sb], itsPS));
  }  

  itsPreviousSequenceNumbers.resize(itsMyNrSubbands, itsNrOutputs);
  for (unsigned i = 0; i < itsMyNrSubbands; i++) {
    for (unsigned j = 0; j < itsNrOutputs; j++) {

      itsPreviousSequenceNumbers[i][j] = -1;
    }
  }    
}


void RTStorage::process()
{
  std::vector<bool> finishedSubbands(itsMyNrSubbands, false);
  unsigned finishedSubbandsCount = 0;

  while (finishedSubbandsCount < itsMyNrSubbands) {
    writeLogMessage();
   
    for (unsigned sb = 0; sb < itsMyNrSubbands; sb++) {
      if (!finishedSubbands[sb]) {
	if(!processSubband(sb)) {
	  finishedSubbands[sb] = true;
	  ++finishedSubbandsCount;
	}
      }
    }
  }
}

void RTStorage::postprocess()
{

  /// close filedescriptors
  for (unsigned i = 0; i < itsMyNrSubbands; i++) {
    for (unsigned j = 0; j < itsNrOutputs; j++) {
      
      delete myFDs[i][j];
      
    }

    delete itsInputThreads[i];
    delete itsInputStreams[i];
  }
  itsInputStreams.clear();
  itsInputThreads.clear();
  itsIsNullStream.clear();
  myFDs.resize(0, 0);  

  itsPreviousSequenceNumbers.resize(0, 0);
}



void RTStorage::writeLogMessage()
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
	       ", count = " << counter ++);
}


void RTStorage::checkForDroppedData(StreamableData *data, unsigned sb, unsigned output)
{
  unsigned expectedSequenceNumber = itsPreviousSequenceNumbers[sb][output] + 1;
    
  if (itsIsNullStream[sb]) {
    data->sequenceNumber	           = expectedSequenceNumber;
    itsPreviousSequenceNumbers[sb][output] = expectedSequenceNumber;
  } else {
    unsigned droppedBlocks = data->sequenceNumber - expectedSequenceNumber;

    if (droppedBlocks > 0) {
      unsigned subbandNumber = itsRank * itsNrSubbandsPerStorage + sb;
      LOG_WARN_STR("MPI rank  " << itsRank << " dropped " << droppedBlocks << " block" << (droppedBlocks == 1 ? "" : "s") << " for subband " << subbandNumber << " and output " << output);
    }

    itsPreviousSequenceNumbers[sb][output] = data->sequenceNumber;
  }
}


bool RTStorage::processSubband(unsigned sb)
{
  unsigned o = itsInputThreads[sb]->itsReceiveQueueActivity.remove();
  struct InputThread::SingleInput &input = itsInputThreads[sb]->itsInputs[o];

  StreamableData *data = input.receiveQueue.remove();
  if (data == 0) 
    return false;

  checkForDroppedData(data, sb, o);

  itsWriteTimer.start();
  /// write data to correct fd
  for (unsigned i = 0; i < itsNrOutputs; i++) {
    data->write( myFDs[sb][i], true, itsAlignment);
  }
  itsWriteTimer.stop();

  bytesWritten += data->requiredSize();
  input.freeQueue.append(data);

  return true;
}


void RTStorage::createInputStream(unsigned subband)
{
  string   prefix            = "OLAP.OLAP_Conn.IONProc_Storage";
  string   connectionType    = itsPS->getString(prefix + "_Transport");
  unsigned subbandNumber     = itsRank * itsNrSubbandsPerStorage + subband;
      
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
} // namespace RTCP
} // namespace LOFAR
