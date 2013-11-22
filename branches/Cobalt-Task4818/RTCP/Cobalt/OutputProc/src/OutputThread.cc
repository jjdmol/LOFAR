//# OutputThread.cc:
//# Copyright (C) 2009-2013  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include "OutputThread.h"

#include <cerrno>
#include <ctime>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iomanip>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

#include <Common/StringUtil.h>
#include <Common/SystemCallException.h>
#include <Common/Thread/Mutex.h>
#include <Common/Thread/Semaphore.h>
#include <Common/Thread/Cancellation.h>

#if defined HAVE_AIPSPP
#include <casa/Exceptions/Error.h>
#endif

#include "MSWriterFile.h"
#include "MSWriterCorrelated.h"
#include "MSWriterDAL.h"
#include "MSWriterNull.h"

namespace LOFAR
{
  namespace Cobalt
  {

    static Mutex makeDirMutex;
    static Mutex casacoreMutex;

    using namespace std;

    static void makeDir(const string &dirname, const string &logPrefix)
    {
      ScopedLock scopedLock(makeDirMutex);
      struct stat s;

      if (stat(dirname.c_str(), &s) == 0) {
        // path already exists
        if ((s.st_mode & S_IFMT) != S_IFDIR) {
          LOG_WARN_STR(logPrefix << "Not a directory: " << dirname);
        }
      } else if (errno == ENOENT) {
        // create directory
        LOG_DEBUG_STR(logPrefix << "Creating directory " << dirname);

        if (mkdir(dirname.c_str(), 0777) != 0 && errno != EEXIST) {
          THROW_SYSCALL(string("mkdir ") + dirname);
        }
      } else {
        // something else went wrong
        THROW_SYSCALL(string("stat ") + dirname);
      }
    }


    /* create a directory as well as all its parent directories */
    static void recursiveMakeDir(const string &dirname, const string &logPrefix)
    {
      using namespace boost;

      string curdir;
      vector<string> splitName;

      boost::split(splitName, dirname, boost::is_any_of("/"));

      for (unsigned i = 0; i < splitName.size(); i++) {
        curdir += splitName[i] + '/';
        makeDir(curdir, logPrefix);
      }
    }


    SubbandOutputThread::SubbandOutputThread(const Parset &parset, unsigned streamNr, Queue<SmartPtr<StreamableData> > &freeQueue, Queue<SmartPtr<StreamableData> > &receiveQueue, const std::string &logPrefix, const std::string &targetDirectory)
      :
      itsParset(parset),
      itsStreamNr(streamNr),
      itsLogPrefix(logPrefix + "[SubbandOutputThread] "),
      itsTargetDirectory(targetDirectory),
      itsFreeQueue(freeQueue),
      itsReceiveQueue(receiveQueue),
      itsBlocksWritten(0),
      itsBlocksDropped(0),
      itsNrExpectedBlocks(0),
      itsNextSequenceNumber(0)
    {
    }


    void SubbandOutputThread::createMS()
    {
      ScopedLock sl(casacoreMutex);
      ScopedDelayCancellation dc; // don't cancel casacore calls

      std::string directoryName = itsTargetDirectory == "" ? itsParset.getDirectoryName(CORRELATED_DATA, itsStreamNr) : itsTargetDirectory;
      std::string fileName = itsParset.getFileName(CORRELATED_DATA, itsStreamNr);
      std::string path = directoryName + "/" + fileName;

      recursiveMakeDir(directoryName, itsLogPrefix);
      LOG_INFO_STR(itsLogPrefix << "Writing to " << path);

      try {
        itsWriter = new MSWriterCorrelated(itsLogPrefix, path, itsParset, itsStreamNr);
      } catch (Exception &ex) {
        LOG_ERROR_STR(itsLogPrefix << "Cannot open " << path << ": " << ex);
        itsWriter = new MSWriterNull;
#if defined HAVE_AIPSPP
      } catch (casa::AipsError &ex) {
        LOG_ERROR_STR(itsLogPrefix << "Caught AipsError: " << ex.what());
        cleanUp();
#endif
      }

      itsNrExpectedBlocks = itsParset.nrCorrelatedBlocks();
    }


    void SubbandOutputThread::checkForDroppedData(StreamableData *data)
    {
      // TODO: check for dropped data at end of observation

      unsigned droppedBlocks = data->sequenceNumber() - itsNextSequenceNumber;

      if (droppedBlocks > 0) {
        itsBlocksDropped += droppedBlocks;

        LOG_WARN_STR(itsLogPrefix << "SubbandOutputThread dropped " << droppedBlocks << (droppedBlocks == 1 ? " block" : " blocks"));
      }

      itsNextSequenceNumber = data->sequenceNumber() + 1;
      itsBlocksWritten++;
    }


    void SubbandOutputThread::doWork()
    {
      time_t prevlog = 0;

      for (SmartPtr<StreamableData> data; (data = itsReceiveQueue.remove()) != 0; itsFreeQueue.append(data.release())) {
        try {
          itsWriter->write(data);
          checkForDroppedData(data);
        } catch (SystemCallException &ex) {
          LOG_WARN_STR(itsLogPrefix << "SubbandOutputThread caught non-fatal exception: " << ex.what());
        }

        time_t now = time(0L);

        if (now > prevlog + 5) {
          // print info every 5 seconds
          LOG_INFO_STR(itsLogPrefix << "Written block with seqno = " << data->sequenceNumber() << ", " << itsBlocksWritten << " blocks written (" << itsWriter->percentageWritten() << "%), " << itsBlocksDropped << " blocks dropped");

          prevlog = now;
        } else {
          // print debug info for the other blocks
          LOG_DEBUG_STR(itsLogPrefix << "Written block with seqno = " << data->sequenceNumber() << ", " << itsBlocksWritten << " blocks written (" << itsWriter->percentageWritten() << "%), " << itsBlocksDropped << " blocks dropped");
        }
      }
    }


   void SubbandOutputThread::cleanUp() const
    {
      float dropPercent = itsBlocksWritten + itsBlocksDropped == 0 ? 0.0 : (100.0 * itsBlocksDropped) / (itsBlocksWritten + itsBlocksDropped);

      LOG_INFO_STR(itsLogPrefix << "Finished writing: " << itsBlocksWritten << " blocks written (" << itsWriter->percentageWritten() << "%), " << itsBlocksDropped << " blocks dropped: " << std::setprecision(3) << dropPercent << "% lost" );
    }


    ParameterSet SubbandOutputThread::feedbackLTA() const
    {
      const string prefix = formatString("LOFAR.ObsSW.Observation.DataProducts.Output_Correlated_[%u].", itsStreamNr);

      ParameterSet result;
      result.adoptCollection(itsWriter->configuration(), prefix);

      return result;
    }


    void SubbandOutputThread::augment( const FinalMetaData &finalMetaData )
    {
      // augment the data product
      ASSERT(itsWriter.get());

      itsWriter->augment(finalMetaData);
    }


    void SubbandOutputThread::process()
    {
      LOG_DEBUG_STR(itsLogPrefix << "SubbandOutputThread::process() entered");

      createMS();
      doWork();
      cleanUp();
    }


    TABOutputThread::TABOutputThread(const Parset &parset, unsigned streamNr, Pool<TABTranspose::Block> &outputPool, const std::string &logPrefix, const std::string &targetDirectory)
      :
      itsParset(parset),
      itsStreamNr(streamNr),
      itsLogPrefix(logPrefix + "[TABOutputThread] "),
      itsTargetDirectory(targetDirectory),
      itsOutputPool(outputPool),
      itsBlocksWritten(0),
      itsBlocksDropped(0),
      itsNrExpectedBlocks(0),
      itsNextSequenceNumber(0)
    {
    }


    void TABOutputThread::createMS()
    {
      // even the HDF5 writer accesses casacore, to perform conversions
      ScopedLock sl(casacoreMutex);
      ScopedDelayCancellation dc; // don't cancel casacore calls

      std::string directoryName = itsTargetDirectory == "" ? itsParset.getDirectoryName(BEAM_FORMED_DATA, itsStreamNr) : itsTargetDirectory;
      std::string fileName = itsParset.getFileName(BEAM_FORMED_DATA, itsStreamNr);
      std::string path = directoryName + "/" + fileName;

      recursiveMakeDir(directoryName, itsLogPrefix);
      LOG_INFO_STR(itsLogPrefix << "Writing to " << path);

      try {
#ifdef HAVE_DAL
        itsWriter = new MSWriterDAL<float,3>(path, itsParset, itsStreamNr);
#else
        itsWriter = new MSWriterFile(path);
#endif
      } catch (Exception &ex) {
        LOG_ERROR_STR(itsLogPrefix << "Cannot open " << path << ": " << ex);
        itsWriter = new MSWriterNull;
#if defined HAVE_AIPSPP
      } catch (casa::AipsError &ex) {
        LOG_ERROR_STR(itsLogPrefix << "Caught AipsError: " << ex.what());
        cleanUp();
#endif
      }

      itsNrExpectedBlocks = itsParset.nrBeamFormedBlocks();
    }


    void TABOutputThread::checkForDroppedData(StreamableData *data)
    {
      // TODO: check for dropped data at end of observation

      unsigned droppedBlocks = data->sequenceNumber() - itsNextSequenceNumber;

      if (droppedBlocks > 0) {
        itsBlocksDropped += droppedBlocks;

        LOG_WARN_STR(itsLogPrefix << "TABOutputThread dropped " << droppedBlocks << (droppedBlocks == 1 ? " block" : " blocks"));
      }

      itsNextSequenceNumber = data->sequenceNumber() + 1;
      itsBlocksWritten++;
    }


    void TABOutputThread::doWork()
    {
      time_t prevlog = 0;

      for (SmartPtr<TABTranspose::Block> data; (data = itsOutputPool.filled.remove()) != 0; itsOutputPool.free.append(data)) {
        try {
          itsWriter->write(data);
          checkForDroppedData(data);
        } catch (SystemCallException &ex) {
          LOG_WARN_STR(itsLogPrefix << "TABOutputThread caught non-fatal exception: " << ex.what());
        }

        time_t now = time(0L);

        if (now > prevlog + 5) {
          // print info every 5 seconds
          LOG_INFO_STR(itsLogPrefix << "Written block with seqno = " << data->sequenceNumber() << ", " << itsBlocksWritten << " blocks written (" << itsWriter->percentageWritten() << "%), " << itsBlocksDropped << " blocks dropped");

          prevlog = now;
        } else {
          // print debug info for the other blocks
          LOG_DEBUG_STR(itsLogPrefix << "Written block with seqno = " << data->sequenceNumber() << ", " << itsBlocksWritten << " blocks written (" << itsWriter->percentageWritten() << "%), " << itsBlocksDropped << " blocks dropped");
        }
      }
    }


    void TABOutputThread::cleanUp() const
    {
      float dropPercent = itsBlocksWritten + itsBlocksDropped == 0 ? 0.0 : (100.0 * itsBlocksDropped) / (itsBlocksWritten + itsBlocksDropped);

      LOG_INFO_STR(itsLogPrefix << "Finished writing: " << itsBlocksWritten << " blocks written (" << itsWriter->percentageWritten() << "%), " << itsBlocksDropped << " blocks dropped: " << std::setprecision(3) << dropPercent << "% lost" );
    }


    ParameterSet TABOutputThread::feedbackLTA() const
    {
      const string prefix = formatString("LOFAR.ObsSW.Observation.DataProducts.Output_Beamformed_[%u].", itsStreamNr);

      ParameterSet result;
      result.adoptCollection(itsWriter->configuration(), prefix);

      return result;
    }


    void TABOutputThread::augment( const FinalMetaData &finalMetaData )
    {
      // augment the data product
      ASSERT(itsWriter.get());

      itsWriter->augment(finalMetaData);
    }


    void TABOutputThread::process()
    {
      LOG_DEBUG_STR(itsLogPrefix << "TABOutputThread::process() entered");

      createMS();
      doWork();
      cleanUp();
    }

  } // namespace Cobalt
} // namespace LOFAR

