//# OutputThread.h
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

#ifndef LOFAR_RTCP_STORAGE_OUTPUT_THREAD_H
#define LOFAR_RTCP_STORAGE_OUTPUT_THREAD_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

#include <string>
#include <vector>

#include <Stream/FileStream.h>
#include <CoInterface/SmartPtr.h>
#include <CoInterface/StreamableData.h>
#include <CoInterface/TABTranspose.h>
#include <CoInterface/FinalMetaData.h>
#include <CoInterface/Pool.h>

#include "MSWriter.h"

namespace LOFAR
{
  namespace Cobalt
  {
    /*
     * OutputThread<T> manages the writing of data blocks to disk. It is
     * responsible for:
     *   1. Creating the data container (MS, HDF5, etc), including the
     *      required meta data.
     *   2. Processing data blocks from a Pool<T> pool, writing them to disk.
     *   3. Producing LTA feedback.
     *   4. Augmenting the data container with the FinalMetaData.
     */
    template<typename T> class OutputThread
    {
    public:
      OutputThread(const Parset &, unsigned streamNr, Pool<T> &outputPool, const std::string &logPrefix, const std::string &targetDirectory, const std::string &LTAfeedbackPrefix);

      virtual ~OutputThread();

      // Create the data container, and process blocks from outputPool.
      void           process();

      // Creates the data container. Needed in createHeaders.cc
      virtual void   createMS() = 0;

      // Wrap-up the writing.
      void           cleanUp() const;

      // Add FinalMetaData to the data container.
      void           augment(const FinalMetaData &finalMetaData);

      // Return the LTA feedback produced by this writer.
      ParameterSet feedbackLTA() const;

    protected:
      void checkForDroppedData(StreamableData *);
      void doWork();

      const Parset &itsParset;
      const unsigned itsStreamNr;
      const std::string itsLogPrefix;
      const std::string itsTargetDirectory;
      const std::string itsLTAfeedbackPrefix;

      size_t itsBlocksWritten, itsBlocksDropped;
      size_t itsNrExpectedBlocks;
      size_t itsNextSequenceNumber;

      Pool<T> &itsOutputPool;

      SmartPtr<MSWriter> itsWriter;
    };


    /*
     * SubbandOutputThread specialises in creating LOFAR MeasurementSets (MS).
     */
    class SubbandOutputThread: public OutputThread<StreamableData>
    {
    public:
      SubbandOutputThread(const Parset &, unsigned streamNr, Pool<StreamableData> &outputPool, const std::string &logPrefix, const std::string &targetDirectory = "");

      void           createMS();
    };



    /*
     * TABOutputThread specialises in creating LOFAR HDF5 files corresponding
     * to ICD003.
     */
    class TABOutputThread: public OutputThread<TABTranspose::Block>
    {
    public:
      TABOutputThread(const Parset &, unsigned streamNr, Pool<TABTranspose::Block> &outputPool, const std::string &logPrefix, const std::string &targetDirectory = "");

      void           createMS();
    };


  } // namespace Cobalt
} // namespace LOFAR

#endif

