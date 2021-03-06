//# SubbandWriter.cc: Writes visibilities and beam-formed data
//# Copyright (C) 2008-2013  ASTRON (Netherlands Institute for Radio Astronomy)
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

#include <lofar_config.h>

#include "SubbandWriter.h"

#include <CoInterface/CorrelatedData.h>
#include <CoInterface/OMPThread.h>

#include <boost/format.hpp>
using boost::format;

namespace LOFAR
{
  namespace Cobalt
  {
    SubbandWriter::SubbandWriter(const Parset &parset, unsigned streamNr,
        RTmetadata &mdLogger, const std::string &mdKeyPrefix,
        const std::string &logPrefix)
    :
      itsStreamNr(streamNr),
      itsOutputPool(str(format("SubbandWriter::itsOutputPool [stream %u]") % streamNr), parset.settings.realTime),
      itsInputThread(parset, streamNr, itsOutputPool, logPrefix),
      itsOutputThread(parset, streamNr, itsOutputPool, mdLogger, mdKeyPrefix, logPrefix)
    {
      for (unsigned i = 0; i < maxReceiveQueueSize; i++)
        itsOutputPool.free.append(new CorrelatedData(parset.settings.correlator.stations.size(), parset.settings.correlator.nrChannels, parset.settings.correlator.nrSamplesPerIntegration(), heapAllocator, 512));
    }

    
    void SubbandWriter::process()
    {
#     pragma omp parallel sections num_threads(2)
      {
#       pragma omp section
        {
          OMPThread::ScopedName sn(str(format("input %u") % itsStreamNr));

          itsInputThread.process();
        }

#       pragma omp section
        {
          OMPThread::ScopedName sn(str(format("output %u") % itsStreamNr));

          itsOutputThread.process();
        }
      }
    }


    void SubbandWriter::fini( const FinalMetaData &finalMetaData )
    {
      itsOutputThread.fini(finalMetaData);
    }


    ParameterSet SubbandWriter::feedbackLTA() const
    {
      return itsOutputThread.feedbackLTA();
    }
  } // namespace Cobalt
} // namespace LOFAR

