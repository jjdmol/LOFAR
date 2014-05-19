//# MPI_utils.cc
//# Copyright (C) 2013  ASTRON (Netherlands Institute for Radio Astronomy)
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

#include "MPI_utils.h"
#include <InputProc/Transpose/MPIProtocol.h>



namespace LOFAR
{
  namespace Cobalt
  {
    template<typename SampleT> void MPIRecvData::allocate(
        size_t nrStations, size_t nrBeamlets, size_t nrSamples)
    {
      data = (char*)mpiAllocator.allocate(
              nrStations * nrBeamlets * nrSamples * sizeof(SampleT));
      metaData = (char*)mpiAllocator.allocate(
            nrStations * nrBeamlets * sizeof(struct MPIProtocol::MetaData));
    }

    template void MPIRecvData::allocate< SampleType<i16complex> >(
        size_t nrStations, size_t nrBeamlets, size_t nrSamples);
    template void MPIRecvData::allocate< SampleType<i8complex> >(
        size_t nrStations, size_t nrBeamlets, size_t nrSamples);
    template void MPIRecvData::allocate< SampleType<i4complex> >(
      size_t nrStations, size_t nrBeamlets, size_t nrSamples);

  }
}