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
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

#include <Common/StringUtil.h>
#include <Common/SystemCallException.h>
#include <Common/Thread/Mutex.h>
#include <Common/Thread/Cancellation.h>
#include <ApplCommon/PVSSDatapointDefs.h>

#include <CoInterface/OutputTypes.h>
#include <CoInterface/Exceptions.h>
#include <CoInterface/LTAFeedback.h>

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
    using boost::lexical_cast;

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


    template<typename T> OutputThread<T>::OutputThread(const Parset &parset,
          unsigned streamNr, Pool<T> &outputPool,
          RTmetadata &mdLogger, const std::string &mdKeyPrefix,
          const std::string &logPrefix, const std::string &targetDirectory,
          const std::string &LTAfeedbackPrefix)
      :
      itsParset(parset),
      itsStreamNr(streamNr),
      itsMdLogger(mdLogger),
      itsMdKeyPrefix(mdKeyPrefix),
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


    template<typename T> OutputThread<T>::~OutputThread()
    {
    }


    template<typename T> void OutputThread<T>::checkForDroppedData(StreamableData *data)
    {
      // TODO: check for dropped data at end of observation

      size_t droppedBlocks = data->sequenceNumber() - itsNextSequenceNumber;

      ASSERTSTR(data->sequenceNumber() >= itsNextSequenceNumber, "Received block nr " << data->sequenceNumber() << " out of order! I expected nothing before " << itsNextSequenceNumber);

      if (droppedBlocks > 0) {
        itsBlocksDropped += droppedBlocks;

        LOG_WARN_STR(itsLogPrefix << "Just dropped " << droppedBlocks << " blocks. Dropped " << itsBlocksDropped << " blocks and written " << itsBlocksWritten << " blocks so far.");

        itsMdLogger.log(itsMdKeyPrefix + PN_COP_DROPPED  + '[' + lexical_cast<string>(itsStreamNr) + ']',
                        itsBlocksDropped * static_cast<float>(itsParset.settings.blockDuration()));
      }

      itsNextSequenceNumber = data->sequenceNumber() + 1;
      itsBlocksWritten++;

      itsMdLogger.log(itsMdKeyPrefix + PN_COP_DROPPING + '[' + lexical_cast<string>(itsStreamNr) + ']',
                      droppedBlocks > 0); // logged too late if dropping: not anymore...
      itsMdLogger.log(itsMdKeyPrefix + PN_COP_WRITTEN  + '[' + lexical_cast<string>(itsStreamNr) + ']',
                      itsBlocksWritten * static_cast<float>(itsParset.settings.blockDuration()));
    }


    template<typename T> void OutputThread<T>::doWork()
    {
      for (SmartPtr<T> data; (data = itsOutputPool.filled.remove()) != 0; itsOutputPool.free.append(data)) {
        if (itsParset.settings.realTime) {
          try {
            itsWriter->write(data);
          } catch (SystemCallException &ex) {
            LOG_WARN_STR(itsLogPrefix << "OutputThread caught non-fatal exception: " << ex.what());
            continue;
          }
        } else { // no try/catch: any loss (e.g. disk full) is fatal in non-real-time mode
          itsWriter->write(data);
        }

        checkForDroppedData(data);

        // print debug info for the other blocks
        LOG_DEBUG_STR(itsLogPrefix << "Written block with seqno = " << data->sequenceNumber() << ", " << itsBlocksWritten << " blocks written (" << itsWriter->percentageWritten() << "%), " << itsBlocksDropped << " blocks dropped");
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
    template class OutputThread<TABTranspose::BeamformedData>;


    SubbandOutputThread::SubbandOutputThread(const Parset &parset,
          unsigned streamNr, Pool<StreamableData> &outputPool,
          RTmetadata &mdLogger, const std::string &mdKeyPrefix,
          const std::string &logPrefix, const std::string &targetDirectory)
      :
      OutputThread<StreamableData>(
          parset,
          streamNr,
          outputPool,
          mdLogger,
          mdKeyPrefix,
          logPrefix + "[SubbandOutputThread] ",
          targetDirectory,
          LTAFeedback::correlatedPrefix(streamNr))
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

   try
      {
        recursiveMakeDir(directoryName, itsLogPrefix);
        LOG_INFO_STR(itsLogPrefix << "Writing to " << path);

        itsWriter = new MSWriterCorrelated(itsLogPrefix, path, itsParset, itsStreamNr);

        // Write data points wrt correlated output file for monitoring (PVSS)
        // once we know the file could at least be created.
        itsMdLogger.log(itsMdKeyPrefix + PN_COP_DATA_PRODUCT_TYPE + '[' + lexical_cast<string>(itsStreamNr) + ']', "Correlated");
        itsMdLogger.log(itsMdKeyPrefix + PN_COP_FILE_NAME         + '[' + lexical_cast<string>(itsStreamNr) + ']', fileName);
        itsMdLogger.log(itsMdKeyPrefix + PN_COP_DIRECTORY         + '[' + lexical_cast<string>(itsStreamNr) + ']', directoryName);
      } 
      catch (Exception &ex) 
      {
        LOG_ERROR_STR(itsLogPrefix << "Cannot open " << path << ": " << ex);
        if ( !itsParset.realTime())   
          THROW(StorageException, ex); 

        itsWriter = new MSWriterNull(itsParset);
#if defined HAVE_AIPSPP
      } 
      catch (casa::AipsError &ex)
      {
        LOG_ERROR_STR(itsLogPrefix << "Caught AipsError: " << ex.what());

        if (!itsParset.realTime())    
          THROW(StorageException, ex.what()); 

        itsWriter = new MSWriterNull(itsParset);
#endif
      }

      itsNrExpectedBlocks = itsParset.settings.nrBlocks() * itsParset.settings.correlator.nrIntegrationsPerBlock;
    }


    TABOutputThread::TABOutputThread(const Parset &parset,
        unsigned streamNr, Pool<TABTranspose::BeamformedData> &outputPool,
        RTmetadata &mdLogger, const std::string &mdKeyPrefix,
        const std::string &logPrefix,
        const std::string &targetDirectory)
      :
      OutputThread<TABTranspose::BeamformedData>(
          parset,
          streamNr,
          outputPool,
          mdLogger,
          mdKeyPrefix,
          logPrefix + "[TABOutputThread] ",
          targetDirectory,
          LTAFeedback::beamFormedPrefix(streamNr))
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

      try
      {
        recursiveMakeDir(directoryName, itsLogPrefix);
        LOG_INFO_STR(itsLogPrefix << "Writing to " << path);

#ifdef HAVE_DAL
        itsWriter = new MSWriterDAL<float,3>(path, itsParset, itsStreamNr);
#else
        itsWriter = new MSWriterFile(path);
#endif

        // Write data points for beamformed output file for monitoring (PVSS)
        // once we know the file could at least be created.
        itsMdLogger.log(itsMdKeyPrefix + PN_COP_DATA_PRODUCT_TYPE + '[' + lexical_cast<string>(itsStreamNr) + ']', "Beamformed");
        itsMdLogger.log(itsMdKeyPrefix + PN_COP_FILE_NAME         + '[' + lexical_cast<string>(itsStreamNr) + ']', fileName);
        itsMdLogger.log(itsMdKeyPrefix + PN_COP_DIRECTORY         + '[' + lexical_cast<string>(itsStreamNr) + ']', directoryName);
      }
      catch (Exception &ex)
      {
        LOG_ERROR_STR(itsLogPrefix << "Cannot open " << path << ": " << ex);
        if (!itsParset.realTime())
          THROW(StorageException, ex);

        itsWriter = new MSWriterNull(itsParset);
#if defined HAVE_AIPSPP
      } 
      catch (casa::AipsError &ex) 
      {
        LOG_ERROR_STR(itsLogPrefix << "Caught AipsError: " << ex.what());
        if ( !itsParset.realTime())       
          THROW(StorageException, ex.what());  

        itsWriter = new MSWriterNull(itsParset);
#endif
      }

      itsNrExpectedBlocks = itsParset.settings.nrBlocks();
    }
  } // namespace Cobalt
} // namespace LOFAR

