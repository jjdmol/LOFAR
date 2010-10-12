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
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

using boost::format;

namespace LOFAR {
namespace RTCP {

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

static void makeDir( const char *dirname, const string &logPrefix )
{
  struct stat s;

  if (stat( dirname, &s ) == 0) {
    // path already exists
    if ((s.st_mode & S_IFMT) != S_IFDIR) {
      LOG_WARN_STR(logPrefix << "Not a directory: " << dirname );
    }
  } else if (errno == ENOENT) {
    // create directory
    LOG_INFO_STR(logPrefix << "Creating directory " << dirname );

    if (mkdir(dirname, 0777) != 0 && errno != EEXIST) {
      unsigned savedErrno = errno; // first argument below clears errno
      throw SystemCallException(string("mkdir ") + dirname, savedErrno, THROW_ARGS);
    }
  } else {
    // something else went wrong
    unsigned savedErrno = errno; // first argument below clears errno
    throw SystemCallException(string("stat ") + dirname, savedErrno, THROW_ARGS);
  }
}


OutputThread::OutputThread(const Parset &parset, unsigned subbandNumber, const ProcessingPlan::planlet &outputConfig, Queue<StreamableData *> &freeQueue, Queue<StreamableData *> &receiveQueue, bool isBigEndian)
:
  itsLogPrefix(str(format("[obs %u output %u subband %3u] ") % parset.observationID() % outputConfig.outputNr % subbandNumber)),
  itsParset(parset),
  itsOutputConfig(outputConfig),
  itsSubbandNumber(subbandNumber),
  itsOutputNumber(outputConfig.outputNr),
  itsObservationID(parset.observationID()),
  itsNextSequenceNumber(0),
  itsFreeQueue(freeQueue),
  itsReceiveQueue(receiveQueue),
  itsSequenceNumbersFile(0), 
  itsHaveCaughtException(false)
{
  std::string filename = getMSname();

  makeDir( dirName(filename).c_str(), itsLogPrefix );

  if (dynamic_cast<CorrelatedData *>(outputConfig.source)) {
    filename = str(format("%s/table.f0data") % getMSname());

    if (parset.getLofarStManVersion() == 2) {
      string seqfilename = str(format("%s/table.f0seqnr") % getMSname());
      
      try {
	itsSequenceNumbersFile = new FileStream(seqfilename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR |  S_IWUSR | S_IRGRP | S_IROTH);
      } catch (...) {
	LOG_WARN_STR(itsLogPrefix << "Could not open sequence numbers file " << seqfilename);
	itsSequenceNumbersFile = 0;
      }
    }

#if defined HAVE_AIPSPP
    MeasurementSetFormat myFormat(&parset, 512);
            
    /// Make MeasurementSet filestructures and required tables
    myFormat.addSubband(getMSname(), subbandNumber, isBigEndian);

    LOG_INFO_STR(itsLogPrefix << "MeasurementSet created");
#endif // defined HAVE_AIPSPP
  }

  LOG_DEBUG_STR(itsLogPrefix << "Writing to " << filename);

  try {
    itsWriter = new MSWriterFile(filename.c_str());
  
  } catch (SystemCallException &ex) {
    LOG_ERROR_STR(itsLogPrefix << "Cannot open " << filename << ": " << ex);
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
}


string OutputThread::getMSname() const
{
  using namespace boost;

  const char pols[] = "XY";
  const char stokes[] = "IQUV";

  const int beam = itsSubbandNumber / itsOutputConfig.nrFilesPerBeam;
  const int subbeam = itsSubbandNumber % itsOutputConfig.nrFilesPerBeam;

  string         name = dirName( itsParset.getString("Observation.MSNameMask") ) + itsOutputConfig.filename;
  string	 startTime = itsParset.getString("Observation.startTime");
  vector<string> splitStartTime;
  split(splitStartTime, startTime, is_any_of("- :"));

  replace_all(name, "${YEAR}", splitStartTime[0]);
  replace_all(name, "${MONTH}", splitStartTime[1]);
  replace_all(name, "${DAY}", splitStartTime[2]);
  replace_all(name, "${HOURS}", splitStartTime[3]);
  replace_all(name, "${MINUTES}", splitStartTime[4]);
  replace_all(name, "${SECONDS}", splitStartTime[5]);

  replace_all(name, "${MSNUMBER}", str(format("%05u") % itsParset.observationID()));
  replace_all(name, "${BEAM}", str(format("%02u") % itsParset.subbandToSAPmapping()[itsSubbandNumber]));
  replace_all(name, "${SUBBAND}", str(format("%03u") % itsSubbandNumber));
  replace_all(name, "${PBEAM}", str(format("%03u") % beam));
  replace_all(name, "${POL}", str(format("%c") % pols[subbeam]));
  replace_all(name, "${STOKES}", str(format("%c") % stokes[subbeam]));

  string raidlistkey = itsOutputConfig.distribution == ProcessingPlan::DIST_SUBBAND
    ? "OLAP.storageRaidList" : "OLAP.PencilInfo.storageRaidList";

  if (itsParset.isDefined(raidlistkey))
    replace_all(name, "${RAID}", str(format("%s") % itsParset.getStringVector(raidlistkey, true)[itsSubbandNumber]));

  return name;
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
  unsigned expectedSequenceNumber = itsNextSequenceNumber;
  unsigned droppedBlocks	  = data->sequenceNumber - expectedSequenceNumber;

  if (droppedBlocks > 0)
    LOG_WARN_STR(itsLogPrefix << "OutputThread dropped " << droppedBlocks << (droppedBlocks == 1 ? " block" : " blocks"));

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

    checkForDroppedData(data.get());

    //writeTimer.start();
    writeSemaphore.down();

    try {
      itsWriter->write(data.get());

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

  // CB -- non reachable? 
  flushSequenceNumbers();
}

} // namespace RTCP
} // namespace LOFAR
