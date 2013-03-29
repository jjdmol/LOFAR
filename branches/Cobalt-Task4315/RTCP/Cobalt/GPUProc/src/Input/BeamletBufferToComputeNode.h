//# BeamletBufferToComputeNode.h: Catch RSP ethernet frames and synchronize RSP inputs
//# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
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

#ifndef LOFAR_GPUPROC_BEAMLET_BUFFER_TO_COMPUTE_NODE_H
#define LOFAR_GPUPROC_BEAMLET_BUFFER_TO_COMPUTE_NODE_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <string>
#include <vector>
#include <pthread.h>
#include <boost/multi_array.hpp>

#include <Stream/Stream.h>
#include <CoInterface/MultiDimArray.h>
#include <CoInterface/Parset.h>
#include <CoInterface/RSPTimeStamp.h>
#include <CoInterface/SmartPtr.h>

#include <casa/Quanta/MVDirection.h>

#include "BeamletBuffer.h"
#include "Delays.h"

namespace LOFAR
{
  namespace Cobalt
  {

    template <typename SAMPLE_TYPE>
    class BeamletBufferToComputeNode
    {
    public:
      BeamletBufferToComputeNode(const Parset &ps, const std::string &stationName, const std::vector<SmartPtr<BeamletBuffer<SAMPLE_TYPE> > > &beamletBuffers, unsigned firstBlockNumber);
      ~BeamletBufferToComputeNode();

      struct header {
        unsigned subband;
        size_t nrSamples;
        size_t sampleSize;
        size_t nrTABs;
      };

      void                         process( Stream *stream );

      TimeStamp                    getCurrentTimeStamp() const
      {
        return itsCurrentTimeStamp;
      }

    private:
      static void                  limitFlagsLength(SparseSet<unsigned> &flags);

      void                         computeDelays(), computeNextDelays();

      void                         sendSubband( Stream *stream, unsigned subband );


      void                         startTransaction();
      void                         writeLogMessage() const;
      void                         toStream( Stream *stream );
      void                         stopTransaction();

      std::string itsLogPrefix;

      bool itsDelayCompensation;
      bool itsCorrectClocks;
      bool itsNeedDelays;
      bool itsIsRealTime;
      std::vector<unsigned>        itsSubbandToSAPmapping;
      std::vector<unsigned>        itsSubbandToRSPboardMapping;
      std::vector<unsigned>        itsSubbandToRSPslotMapping;

      const Parset                 &itsPS;

      TimeStamp itsCurrentTimeStamp;

      Matrix<double>               itsDelaysAtBegin;
      Matrix<double>               itsDelaysAfterEnd;
      Matrix<casa::MVDirection>    itsBeamDirectionsAtBegin;
      Matrix<casa::MVDirection>    itsBeamDirectionsAfterEnd;

      unsigned itsMaxNetworkDelay;                   // in samples
      unsigned itsNrSubbands;
      unsigned itsNrSamplesPerSubband;
      unsigned itsNrHistorySamples;
      unsigned itsNrRSPboards;
      unsigned itsNrBeams;
      unsigned itsMaxNrTABs;
      std::vector<unsigned>        itsNrTABs;

      const std::vector<SmartPtr<BeamletBuffer<SAMPLE_TYPE> > > &itsBeamletBuffers;
      SmartPtr<Delays>             itsDelays;
      double itsSubbandBandwidth, itsSampleDuration;
      double itsClockCorrectionTime;

      std::vector<TimeStamp>       itsDelayedStamps;
      std::vector<signed int>      itsSamplesDelay;
      boost::multi_array<SparseSet<unsigned>, 2> itsFlags;

      Matrix<float>                itsFineDelaysAtBegin, itsFineDelaysAfterEnd;

      static const unsigned itsMaximumDelay = 1000;      // samples; roughly 1500 km
      TimeStamp itsCorrelationStartTime;
      WallClockTime itsWallClock;

      NSTimer itsDelayTimer;
    };

  } // namespace Cobalt
} // namespace LOFAR

#endif

