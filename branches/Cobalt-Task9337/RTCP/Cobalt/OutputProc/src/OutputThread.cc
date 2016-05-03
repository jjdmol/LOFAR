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

#include <unistd.h>
#include <iomanip>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

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

    static Mutex casacoreMutex;

    using namespace std;
    using boost::lexical_cast;

    template<typename T> OutputThread<T>::OutputThread(const Parset &parset,
          unsigned streamNr, Pool<T> &outputPool,
          RTmetadata &mdLogger, const std::string &mdKeyPrefix,
          const std::string &logPrefix, const std::string &targetDirectory)
      :
      itsParset(parset),
      itsStreamNr(streamNr),
      itsMdLogger(mdLogger),
      itsMdKeyPrefix(mdKeyPrefix),
      itsLogPrefix(logPrefix),
      itsTargetDirectory(targetDirectory),
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

      const string streamNrStr = '[' + lexical_cast<string>(itsStreamNr) + ']';

      if (droppedBlocks > 0) {
        itsBlocksDropped += droppedBlocks;

        LOG_WARN_STR(itsLogPrefix << "Just dropped " << droppedBlocks << " blocks. Dropped " << itsBlocksDropped << " blocks and written " << itsBlocksWritten << " blocks so far.");

        itsMdLogger.log(itsMdKeyPrefix + PN_COP_DROPPED + streamNrStr,
                        itsBlocksDropped * static_cast<float>(itsParset.settings.blockDuration()));
      }

      itsNextSequenceNumber = data->sequenceNumber() + 1;
      itsBlocksWritten++;

      itsMdLogger.log(itsMdKeyPrefix + PN_COP_DROPPING + streamNrStr,
                      droppedBlocks > 0); // logged too late if dropping: not anymore...
      itsMdLogger.log(itsMdKeyPrefix + PN_COP_WRITTEN  + streamNrStr,
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


    template<typename T>
    void OutputThread<T>::logInitialStreamMetadataEvents(const string& dataProductType,
                                                         const string& fileName,
                                                         const string& directoryName)
    {
      // Write data points wrt @dataProductType output file for monitoring (PVSS).
      const string streamNrStr = '[' + lexical_cast<string>(itsStreamNr) + ']';

      itsMdLogger.log(itsMdKeyPrefix + PN_COP_DATA_PRODUCT_TYPE + streamNrStr, dataProductType);
      itsMdLogger.log(itsMdKeyPrefix + PN_COP_FILE_NAME         + streamNrStr, fileName);
      itsMdLogger.log(itsMdKeyPrefix + PN_COP_DIRECTORY         + streamNrStr, directoryName);

      // After obs start these dynarray data points are written conditionally, so init.
      // While we only have to write the last index (PVSSGateway will zero the rest),
      // we'd have to find out who has the last subband. Don't bother, just init all.
      itsMdLogger.log(itsMdKeyPrefix + PN_COP_DROPPING + streamNrStr, 0);
      itsMdLogger.log(itsMdKeyPrefix + PN_COP_WRITTEN  + streamNrStr, 0.0f);
      itsMdLogger.log(itsMdKeyPrefix + PN_COP_DROPPED  + streamNrStr, 0.0f);
    }


    template<typename T> void OutputThread<T>::cleanUp() const
    {
      float dropPercent = itsBlocksWritten + itsBlocksDropped == 0 ? 0.0 : (100.0 * itsBlocksDropped) / (itsBlocksWritten + itsBlocksDropped);

      LOG_INFO_STR(itsLogPrefix << "Finished writing: " << itsBlocksWritten << " blocks written (" << itsWriter->percentageWritten() << "%), " << itsBlocksDropped << " blocks dropped: " << std::setprecision(3) << dropPercent << "% lost" );
    }


    template<typename T> void OutputThread<T>::init()
    {
      try {
        ASSERT(itsWriter.get());

        itsWriter->init();
      } catch (Exception &ex) {
        LOG_ERROR_STR(itsLogPrefix << "Could not create meta data: " << ex);

        if (!itsParset.settings.realTime)   
          THROW(StorageException, ex); 
#if defined HAVE_AIPSPP
      } 
      catch (casa::AipsError &ex)
      {
        LOG_ERROR_STR(itsLogPrefix << "Could not create meta data (AipsError): " << ex.what());

        if (!itsParset.settings.realTime)    
          THROW(StorageException, ex.what()); 
#endif
      }
    }


    template<typename T> void OutputThread<T>::fini( const FinalMetaData &finalMetaData )
    {
      try {
        // fini the data product
        ASSERT(itsWriter.get());

        itsWriter->fini(finalMetaData);
      } catch (Exception &ex) {
        LOG_ERROR_STR(itsLogPrefix << "Could not add final meta data: " << ex);

        if (!itsParset.settings.realTime)   
          THROW(StorageException, ex); 
      }
    }


    template<typename T> ParameterSet OutputThread<T>::feedbackLTA() const
    {
      ParameterSet result;

      try {
        result.adoptCollection(itsWriter->configuration());
      } catch (Exception &ex) {
        LOG_ERROR_STR(itsLogPrefix << "Could not obtain feedback for LTA: " << ex);
      }

      return result;
    }


    template<typename T> void OutputThread<T>::process()
    {
      LOG_DEBUG_STR(itsLogPrefix << "process() entered");

      createMS();

#     pragma omp parallel sections num_threads(2)
      {
#       pragma omp section
        {
          doWork();
          cleanUp();
        }

#       pragma omp section
        init();
      }
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
          targetDirectory)
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
        LOG_INFO_STR(itsLogPrefix << "Writing to " << path);

        itsWriter = new MSWriterCorrelated(itsLogPrefix, path, itsParset, itsStreamNr);

        logInitialStreamMetadataEvents("Correlated", fileName, directoryName);
      } 
      catch (Exception &ex) 
      {
        LOG_ERROR_STR(itsLogPrefix << "Cannot open " << path << ": " << ex);
        if (!itsParset.settings.realTime)
          THROW(StorageException, ex); 

        itsWriter = new MSWriterNull(itsParset);
#if defined HAVE_AIPSPP
      } 
      catch (casa::AipsError &ex)
      {
        LOG_ERROR_STR(itsLogPrefix << "Caught AipsError: " << ex.what());

        if (!itsParset.settings.realTime)    
          THROW(StorageException, ex.what()); 

        itsWriter = new MSWriterNull(itsParset);
#endif
      }

      itsNrExpectedBlocks = itsParset.settings.correlator.nrIntegrations;
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
          targetDirectory)
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
        LOG_INFO_STR(itsLogPrefix << "Writing to " << path);

#ifdef HAVE_DAL
        itsWriter = new MSWriterDAL<float,3>(path, itsParset, itsStreamNr);
#else
        itsWriter = new MSWriterFile(path);
#endif

        logInitialStreamMetadataEvents("Beamformed", fileName, directoryName);
      }
      catch (Exception &ex)
      {
        LOG_ERROR_STR(itsLogPrefix << "Cannot open " << path << ": " << ex);
        if (!itsParset.settings.realTime)
          THROW(StorageException, ex);

        itsWriter = new MSWriterNull(itsParset);
#if defined HAVE_AIPSPP
      } 
      catch (casa::AipsError &ex) 
      {
        LOG_ERROR_STR(itsLogPrefix << "Caught AipsError: " << ex.what());
        if ( !itsParset.settings.realTime)       
          THROW(StorageException, ex.what());  

        itsWriter = new MSWriterNull(itsParset);
#endif
      }

      itsNrExpectedBlocks = itsParset.settings.nrBlocks();
    }
  } // namespace Cobalt
} // namespace LOFAR

