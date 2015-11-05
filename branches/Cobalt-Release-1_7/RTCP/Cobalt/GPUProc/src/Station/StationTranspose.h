//# StationTranspose.h: Manages the transpose of station data, which exchanges
//#                     [station][subband] to [subband][station] over MPI.
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


#ifndef LOFAR_GPUPROC_STATIONTRANSPOSE_H
#define LOFAR_GPUPROC_STATIONTRANSPOSE_H

#include <map>
#include <vector>

#include <CoInterface/Parset.h>
#include <CoInterface/Queue.h>
#include <CoInterface/SubbandMetaData.h>
#include <InputProc/RSPTimeStamp.h>
#include <InputProc/Station/RSP.h>

#ifdef HAVE_MPI
#include <InputProc/Transpose/MPIProtocol.h>
#include <InputProc/Transpose/MPIUtil.h>
#endif

namespace LOFAR {
  namespace Cobalt {

    // Which MPI rank receives which subbands?
    typedef std::map<int, std::vector<size_t> > SubbandDistribution;

    // Data meant to be sent over MPI to the receivers
    template<typename SampleT>
    struct MPIData {

      /*
       * The order of the subbands in the arrays below is
       * those of the subbands processed by all receiving
       * ranks concatenated (values(subbandDistribution).
       *
       * For example, with 2 ranks and 4 subbands, the
       * order will likely be:
       *
       *   0, 2, 1, 3
       *
       * because rank 0 will process [0, 2] and rank 1
       * will process [1, 3].
       *
       */

      // mpi_samples: the sample buffers as they will be sent over MPI
      MultiDimArray<SampleT, 2> mpi_samples; // [subband][sample]

      // mpi_metaData: the meta data as they will be sent over MPI
      // NOTE: the flags indicate which samples ARE NOT present
      MultiDimArray<MPIProtocol::MetaData, 1> mpi_metaData; // [subband]

      // metaData: the meta data as being maintained and updated.
      // NOTE: the flags indicate which samples ARE present
      std::vector<struct SubbandMetaData> metaData; // [subband]

      // read_offsets: the offsets for which the reader expects
      // to have compensated. Implements the coarse delay compensation.
      std::vector<ssize_t> read_offsets; // [subband]

      MPIData(TimeStamp obsStartTime, size_t nrSubbands, size_t nrSamples);

      /* Update block annotation for a given block number,
       * and clear all flags.
       *
       * Note that the data is not cleared for performance reasons.
       */
      void reset(ssize_t block);

      /*
       * Serialises metaData into mpi_metaData.
       *
       * NOTE: The flags are inverted from metaData to mpi_metaData.
       */
      void serialiseMetaData();

      /*
       * Write a certain RSP packet into mpi_samples, and update
       * metaData.
       *
       * The beamletIndices array contains a list of offsets
       * at which to write each beamlet of the packet into the mpi_samples
       * and metaData structures.
       *
       * Returns true if the packet contains data that (might) have to be written
       * to the next MPIData block as well.
       */
      bool write(const struct RSP &packet, const ssize_t *beamletIndices, size_t nrBeamletIndices);

      /*
       * Block annotation, set by reset().
       */
      uint64_t from;
      uint64_t to;
      ssize_t block;

      /* Observation start time, used to calculate the time span
       * of each block.
       */
      const TimeStamp obsStartTime;

      const size_t nrSamples;
      const size_t nrSubbands;
    };

    class MPISender {
    public:
      MPISender( const std::string &logPrefix, size_t stationIdx, const SubbandDistribution &subbandDistribution );

      /*
       * Sends one block over MPI.
       */
      template <typename SampleT>
      void sendBlock( MPIData<SampleT> &mpiData );

      /*
       * Sends blocks over MPI until a NULL is read from the inputQueue.
       *
       * For each block, serialiseMetaData() is called before sendBlock()
       * is used to send the block over MPI.
       */
      template <typename SampleT>
      void sendBlocks( Queue< SmartPtr< MPIData<SampleT> > > &inputQueue, Queue< SmartPtr< MPIData<SampleT> > > &outputQueue );

    private:
      const std::string logPrefix;
      const size_t stationIdx;
      const SubbandDistribution subbandDistribution;
      const std::vector<int> targetRanks;
      std::vector<size_t> subbandOffsets;
      const size_t nrSubbands;
    };
  }
}

#endif

