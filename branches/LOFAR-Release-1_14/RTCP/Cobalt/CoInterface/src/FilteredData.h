//# FilteredData.h
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

#ifndef LOFAR_INTERFACE_FILTERED_DATA_H
#define LOFAR_INTERFACE_FILTERED_DATA_H

#include <Common/lofar_complex.h>
#include <Stream/Stream.h>
#include <CoInterface/Align.h>
#include <CoInterface/Config.h>
#include <CoInterface/MultiDimArray.h>
#include <CoInterface/SparseSet.h>
#include <CoInterface/StreamableData.h>

namespace LOFAR
{
  namespace Cobalt
  {

    class FilteredData : public SampleData<fcomplex, 4, 2>
    {
    public:
      typedef SampleData<fcomplex, 4, 2> SuperType;

      FilteredData(unsigned nrStations, unsigned nrChannels, unsigned nrSamplesPerIntegration, Allocator & = heapAllocator);

      void resetFlags(void);
    };


    inline FilteredData::FilteredData(unsigned nrStations, unsigned nrChannels, unsigned nrSamplesPerIntegration, Allocator &allocator)
      :
      // The "| 2" significantly improves transpose speeds for particular
      // numbers of stations due to cache conflict effects.  The extra memory
      // is not used.
      SuperType::SampleData(boost::extents[nrChannels][nrStations][nrSamplesPerIntegration | 2][NR_POLARIZATIONS], boost::extents[nrChannels][nrStations], allocator)
    {
    }


    inline void FilteredData::resetFlags(void)
    {
      for(unsigned c = 0; c < flags.shape()[0]; c++) {
        for(unsigned s = 0; s < flags.shape()[1]; s++) {
          flags[c][s].reset();
        }
      }
    }

  } // namespace Cobalt
} // namespace LOFAR

#endif

