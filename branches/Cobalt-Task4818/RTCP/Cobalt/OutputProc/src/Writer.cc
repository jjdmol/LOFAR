//# Writer.cc: Writes visibilities and beam-formed data
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

#include "Writer.h"

#include <CoInterface/DataFactory.h>

namespace LOFAR
{
  namespace Cobalt
  {
    Writer::~Writer()
    {
    }


    SubbandWriter::SubbandWriter(const Parset &parset, unsigned streamNr, const std::string &logPrefix)
    :
      itsInputThread(parset, streamNr, itsFreeQueue, itsReceiveQueue, logPrefix),
      itsOutputThread(parset, CORRELATED_DATA, streamNr, itsFreeQueue, itsReceiveQueue, logPrefix)
    {
      for (unsigned i = 0; i < maxReceiveQueueSize; i++)
        itsFreeQueue.append(newStreamableData(parset, CORRELATED_DATA, streamNr));
    }

    
    void SubbandWriter::process()
    {
#     pragma omp parallel sections num_threads(2)
      {
#       pragma omp section
        {
          itsInputThread.process();
        }

#       pragma omp section
        {
          itsOutputThread.process();
        }
      }
    }


    void SubbandWriter::augment( const FinalMetaData &finalMetaData )
    {
      itsOutputThread.augment(finalMetaData);
    }


    ParameterSet SubbandWriter::feedbackLTA() const
    {
      return itsOutputThread.feedbackLTA();
    }


    TABWriter::TABWriter(const Parset &parset, unsigned streamNr, const std::string &logPrefix)
    :
      itsInputThread(parset, streamNr, itsFreeQueue, itsReceiveQueue, logPrefix),
      itsOutputThread(parset, BEAM_FORMED_DATA, streamNr, itsFreeQueue, itsReceiveQueue, logPrefix)
    {
      for (unsigned i = 0; i < maxReceiveQueueSize; i++)
        itsFreeQueue.append(newStreamableData(parset, BEAM_FORMED_DATA, streamNr));
    }

    
    void TABWriter::process()
    {
#     pragma omp parallel sections num_threads(2)
      {
#       pragma omp section
        {
          itsInputThread.process();
        }

#       pragma omp section
        {
          itsOutputThread.process();
        }
      }
    }


    void TABWriter::augment( const FinalMetaData &finalMetaData )
    {
      itsOutputThread.augment(finalMetaData);
    }


    ParameterSet TABWriter::feedbackLTA() const
    {
      return itsOutputThread.feedbackLTA();
    }


  } // namespace Cobalt
} // namespace LOFAR

