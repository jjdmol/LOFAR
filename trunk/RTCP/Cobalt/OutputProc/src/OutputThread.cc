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
#include <Common/Thread/Cancellation.h>

#include <CoInterface/OutputTypes.h>

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


    template<typename T> OutputThread<T>::OutputThread(const Parset &parset, unsigned streamNr, Pool<T> &outputPool, const std::string &logPrefix, const std::string &targetDirectory, const std::string &LTAfeedbackPrefix)
      :
      itsParset(parset),
      itsStreamNr(streamNr),
      itsLogPrefix(logPrefix),
      itsTargetDirectory(targetDirectory),
      itsLTAfeedbackPrefix(LTAfeedbackPrefix),
      itsBlocksWritten(0),
      itsBlocksDropped(0),
      itsNrExpectedBlocks(0),
      itsNextSequenceNumber(0),
      itsOutputPool(outputPool)
    {
    }


    template<typename T> void OutputThread<T>::checkForDroppedData(StreamableData *data)
    {
      // TODO: check for dropped data at end of observation

      unsigned droppedBlocks = data->sequenceNumber() - itsNextSequenceNumber;

      ASSERTSTR(data->sequenceNumber() >= itsNextSequenceNumber, "Received block nr " << data->sequenceNumber() << " out of order! I expected nothing before " << itsNextSequenceNumber);

      if (droppedBlocks > 0) {
        itsBlocksDropped += droppedBlocks;

        LOG_WARN_STR(itsLogPrefix << "Dropped " << droppedBlocks << (droppedBlocks == 1 ? " block" : " blocks"));
      }

      itsNextSequenceNumber = data->sequenceNumber() + 1;
      itsBlocksWritten++;
    }


    template<typename T> void OutputThread<T>::doWork()
    {
      time_t prevlog = 0;

      for (SmartPtr<T> data; (data = itsOutputPool.filled.remove()) != 0; itsOutputPool.free.append(data)) {
        try {
          itsWriter->write(data);
          checkForDroppedData(data);
        } catch (SystemCallException &ex) {
          LOG_WARN_STR(itsLogPrefix << "OutputThread caught non-fatal exception: " << ex.what());
        }

        const time_t now = time(0L);

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


    template<typename T> void OutputThread<T>::cleanUp() const
    {
      float dropPercent = itsBlocksWritten + itsBlocksDropped == 0 ? 0.0 : (100.0 * itsBlocksDropped) / (itsBlocksWritten + itsBlocksDropped);

      LOG_INFO_STR(itsLogPrefix << "Finished writing: " << itsBlocksWritten << " blocks written (" << itsWriter->percentageWritten() << "%), " << itsBlocksDropped << " blocks dropped: " << std::setprecision(3) << dropPercent << "% lost" );
    }


    template<typename T> void OutputThread<T>::augment( const FinalMetaData &finalMetaData )
    {
      try {
        // augment the data product
        ASSERT(itsWriter.get());

        itsWriter->augment(finalMetaData);
      } catch (Exception &ex) {
        LOG_ERROR_STR(itsLogPrefix << "Could not add final meta data: " << ex);
      }
    }


    template<typename T> ParameterSet OutputThread<T>::feedbackLTA() const
    {
      ParameterSet result;

      try {
        result.adoptCollection(itsWriter->configuration(), itsLTAfeedbackPrefix);
      } catch (Exception &ex) {
        LOG_ERROR_STR(itsLogPrefix << "Could not obtain feedback for LTA: " << ex);
      }

      return result;
    }


    template<typename T> void OutputThread<T>::process()
    {
      LOG_DEBUG_STR(itsLogPrefix << "process() entered");

      createMS();
      doWork();
      cleanUp();
    }

    // Make required instantiations
    template class OutputThread<StreamableData>;
    template class OutputThread<TABTranspose::Block>;


    SubbandOutputThread::SubbandOutputThread(const Parset &parset, unsigned streamNr, Pool<StreamableData> &outputPool, const std::string &logPrefix, const std::string &targetDirectory)
      :
      OutputThread<StreamableData>(
          parset,
          streamNr,
          outputPool,
          logPrefix + "[SubbandOutputThread] ",
          targetDirectory,
          formatString("Observation.DataProducts.Output_Correlated_[%u].", itsStreamNr))
    {
    }


    void SubbandOutputThread::createMS()
    {
      ScopedLock sl(casacoreMutex);
      ScopedDelayCancellation dc; // don't cancel casacore calls

      const std::string directoryName =
        itsTargetDirectory == ""
        ? itsParset.getDirectoryName(CORRELATED_DATA, itsStreamNr)
        : itsTargetDirectory;
      const std::string fileName = itsParset.getFileName(CORRELATED_DATA, itsStreamNr);

      const std::string path = directoryName + "/" + fileName;

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


    TABOutputThread::TABOutputThread(const Parset &parset, unsigned streamNr, Pool<TABTranspose::Block> &outputPool, const std::string &logPrefix, const std::string &targetDirectory)
      :
      OutputThread<TABTranspose::Block>(
          parset,
          streamNr,
          outputPool,
          logPrefix + "[TABOutputThread] ",
          targetDirectory,
          formatString("Observation.DataProducts.Output_Beamformed_[%u].", itsStreamNr)
          )
    {
    }


    void TABOutputThread::createMS()
    {
      // even the HDF5 writer accesses casacore, to perform conversions
      ScopedLock sl(casacoreMutex);
      ScopedDelayCancellation dc; // don't cancel casacore calls

      const std::string directoryName =
        itsTargetDirectory == ""
        ? itsParset.getDirectoryName(BEAM_FORMED_DATA, itsStreamNr)
        : itsTargetDirectory;
      const std::string fileName = itsParset.getFileName(BEAM_FORMED_DATA, itsStreamNr);

      const std::string path = directoryName + "/" + fileName;

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
  } // namespace Cobalt
} // namespace LOFAR

