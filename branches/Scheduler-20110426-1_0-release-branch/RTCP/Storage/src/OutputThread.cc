//#  OutputThread.cc:
//#
//#  Copyright (C) 2008
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
//#  $Id: OutputThread.cc 14194 2009-10-06 09:54:51Z romein $

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Storage/InputThread.h>
#include <Storage/OutputThread.h>
#include <Storage/MSWriterFile.h>
#include <Storage/MSWriterNull.h>
#include <Storage/MeasurementSetFormat.h>
#include <Interface/StreamableData.h>
#include <Thread/Semaphore.h>
#include <Common/DataConvert.h>
#include <stdio.h>

#include <boost/format.hpp>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

using boost::format;

namespace LOFAR {
namespace RTCP {

#if 0
static string dirName( const string filename )
{
  using namespace boost;
  
  string         basedir;
  vector<string> splitName;
  
  split(splitName, filename, is_any_of("/"));
  
  for (unsigned i = 0; i < splitName.size()-1 ; i++) {
    basedir += splitName[i] + '/';
  }
  return basedir;
}
#endif

static void makeDir( const string &dirname, const string &logPrefix )
{
  struct stat s;

  if (stat( dirname.c_str(), &s ) == 0) {
    // path already exists
    if ((s.st_mode & S_IFMT) != S_IFDIR) {
      LOG_WARN_STR(logPrefix << "Not a directory: " << dirname );
    }
  } else if (errno == ENOENT) {
    // create directory
    LOG_INFO_STR(logPrefix << "Creating directory " << dirname );

    if (mkdir(dirname.c_str(), 0777) != 0 && errno != EEXIST) {
      unsigned savedErrno = errno; // first argument below clears errno
      throw SystemCallException(string("mkdir ") + dirname, savedErrno, THROW_ARGS);
    }
  } else {
    // something else went wrong
    unsigned savedErrno = errno; // first argument below clears errno
    throw SystemCallException(string("stat ") + dirname, savedErrno, THROW_ARGS);
  }
}


/* create a directory as well as all its parent directories */
static void recursiveMakeDir( const string &dirname, const string &logPrefix )
{
  using namespace boost;
  
  string         curdir;
  vector<string> splitName;
  
  split(splitName, dirname, is_any_of("/"));
  
  for (unsigned i = 0; i < splitName.size(); i++) {
    curdir += splitName[i] + '/';

    makeDir( curdir, logPrefix );
  }
}


OutputThread::OutputThread(const Parset &parset, const ProcessingPlan::planlet &outputConfig, unsigned index, const string &dir, const string &filename, Queue<StreamableData *> &freeQueue, Queue<StreamableData *> &receiveQueue, bool isBigEndian)
:
  itsLogPrefix(str(format("[obs %u output %u index %3u] ") % parset.observationID() % outputConfig.outputNr % index)),
  itsParset(parset),
  itsOutputConfig(outputConfig),
  itsOutputNumber(outputConfig.outputNr),
  itsObservationID(parset.observationID()),
  itsNextSequenceNumber(0),
  itsFreeQueue(freeQueue),
  itsReceiveQueue(receiveQueue),
  itsSequenceNumbersFile(0), 
  itsHaveCaughtException(false),
  itsBlocksWritten(0),
  itsBlocksDropped(0)
{
  string fullfilename = dir + "/" + filename;

  recursiveMakeDir( dir, itsLogPrefix );

  if (dynamic_cast<CorrelatedData *>(outputConfig.source)) {
#if defined HAVE_AIPSPP
    MeasurementSetFormat myFormat(&parset, 512);
            
    /// Make MeasurementSet filestructures and required tables
    myFormat.addSubband(fullfilename, index, isBigEndian);

    LOG_INFO_STR(itsLogPrefix << "MeasurementSet created");
#endif // defined HAVE_AIPSPP

    if (parset.getLofarStManVersion() == 2) {
      string seqfilename = str(format("%s/table.f0seqnr") % fullfilename);
      
      try {
	itsSequenceNumbersFile = new FileStream(seqfilename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR |  S_IWUSR | S_IRGRP | S_IROTH);
      } catch (...) {
	LOG_WARN_STR(itsLogPrefix << "Could not open sequence numbers file " << seqfilename);
	itsSequenceNumbersFile = 0;
      }
    }

    fullfilename = str(format("%s/table.f0data") % fullfilename);
  }

  LOG_INFO_STR(itsLogPrefix << "Writing to " << fullfilename);

  try {
    itsWriter = new MSWriterFile(fullfilename.c_str());
  } catch (SystemCallException &ex) {
    LOG_ERROR_STR(itsLogPrefix << "Cannot open " << fullfilename << ": " << ex);
    itsWriter = new MSWriterNull();
  }

  //thread = new Thread(this, &OutputThread::mainLoop, str(format("OutputThread (obs %d sb %d output %d)") % ps->observationID() % subbandNumber % outputNumber));
  itsThread = new Thread(this, &OutputThread::mainLoop, itsLogPrefix + "[OutputThread] ");
}


OutputThread::~OutputThread()
{
  delete itsThread;
  delete itsWriter;

  flushSequenceNumbers();
  delete itsSequenceNumbersFile;

  if (itsHaveCaughtException)
    LOG_WARN_STR(itsLogPrefix << "OutputThread caught non-fatal exception(s).") ;

  float dropPercent = itsBlocksWritten + itsBlocksDropped == 0 ? 0.0 : (100.0 * itsBlocksDropped) / (itsBlocksWritten + itsBlocksDropped);

  LOG_INFO_STR(itsLogPrefix << itsBlocksWritten << " blocks written, " << itsBlocksDropped << " blocks dropped: " << std::setprecision(3) << dropPercent << "% lost" );
}

void OutputThread::writeLogMessage(unsigned sequenceNumber)
{
  LOG_INFO_STR(itsLogPrefix << "Written block with seqno = " << sequenceNumber);
}


void OutputThread::flushSequenceNumbers()
{
  if (itsSequenceNumbersFile != 0) {
    LOG_INFO_STR(itsLogPrefix << "Flushing sequence numbers");
    itsSequenceNumbersFile->write(itsSequenceNumbers.data(), itsSequenceNumbers.size()*sizeof(unsigned));
    itsSequenceNumbers.clear();
  }
}


void OutputThread::checkForDroppedData(StreamableData *data)
{
  // TODO: check for dropped data at end of observation

  unsigned expectedSequenceNumber = itsNextSequenceNumber;
  unsigned droppedBlocks	  = data->sequenceNumber - expectedSequenceNumber;

  if (droppedBlocks > 0) {
    itsBlocksDropped++;

    LOG_WARN_STR(itsLogPrefix << "OutputThread dropped " << droppedBlocks << (droppedBlocks == 1 ? " block" : " blocks"));
  }

  if (itsSequenceNumbersFile != 0) {
    itsSequenceNumbers.push_back(data->sequenceNumber);
    
    if (itsSequenceNumbers.size() > 64)
      flushSequenceNumbers();
  }

  itsNextSequenceNumber = data->sequenceNumber + 1;
}

Semaphore writeSemaphore(3);

void OutputThread::mainLoop()
{
  while (true) {
    //NSTimer			  writeTimer("write data", false, false);
    std::auto_ptr<StreamableData> data(itsReceiveQueue.remove());

    if (data.get() == 0)
      break;

    //writeTimer.start();
    writeSemaphore.down();

    try {
      itsWriter->write(data.get());

      itsBlocksWritten++;

      checkForDroppedData(data.get());
    } catch (SystemCallException &ex) {
      itsHaveCaughtException = true;
      LOG_WARN_STR(itsLogPrefix << "OutputThread caught non-fatal exception: " << ex.what()) ;
    }

    writeSemaphore.up();
    //writeTimer.stop();

    writeLogMessage(data.get()->sequenceNumber);
    //LOG_INFO_STR(itsLogPrefix << writeTimer);

    itsFreeQueue.append(data.release());
  }
}

} // namespace RTCP
} // namespace LOFAR
