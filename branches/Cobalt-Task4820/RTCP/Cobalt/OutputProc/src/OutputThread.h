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
    template<typename T> class OutputThread
    {
    public:
      OutputThread(const Parset &, unsigned streamNr, Pool<T> &outputPool, const std::string &logPrefix, const std::string &targetDirectory, const std::string &LTAfeedbackPrefix);

      void           process();

      // needed in createHeaders.cc
      virtual void   createMS() = 0;
      void           cleanUp() const;

      void           augment(const FinalMetaData &finalMetaData);

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


    class SubbandOutputThread: public OutputThread<StreamableData>
    {
    public:
      SubbandOutputThread(const Parset &, unsigned streamNr, Pool<StreamableData> &outputPool, const std::string &logPrefix, const std::string &targetDirectory = "");

      void           createMS();
    };


    class TABOutputThread: public OutputThread<TABTranspose::Block>
    {
    public:
      TABOutputThread(const Parset &, unsigned streamNr, Pool<TABTranspose::Block> &outputPool, const std::string &logPrefix, const std::string &targetDirectory = "");

      void           createMS();
    };


  } // namespace Cobalt
} // namespace LOFAR

#endif

