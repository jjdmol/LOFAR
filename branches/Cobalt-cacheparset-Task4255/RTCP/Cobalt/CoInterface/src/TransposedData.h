/* TransposedData.h
 * Copyright (C) 2008-2013  ASTRON (Netherlands Institute for Radio Astronomy)
 * P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
 *
 * This file is part of the LOFAR software suite.
 * The LOFAR software suite is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The LOFAR software suite is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
 *
 * $Id$
 */

#ifndef LOFAR_CNPROC_TRANSPOSED_DATA_H
#define LOFAR_CNPROC_TRANSPOSED_DATA_H

#include <CoInterface/Config.h>
#include <CoInterface/Allocator.h>
#include <CoInterface/StreamableData.h>


namespace LOFAR
{
  namespace Cobalt
  {

    template <typename SAMPLE_TYPE>
    class TransposedData : public SampleData<SAMPLE_TYPE,3,1>
    {
    public:
      typedef SampleData<SAMPLE_TYPE,3,1> SuperType;

      TransposedData(const unsigned nrStations, const unsigned nrSamplesToCNProc, Allocator &allocator = heapAllocator);
    };


    template <typename SAMPLE_TYPE>
    inline TransposedData<SAMPLE_TYPE>::TransposedData(const unsigned nrStations, const unsigned nrSamplesToCNProc, Allocator &allocator)
      :
      SuperType(boost::extents[nrStations][nrSamplesToCNProc][NR_POLARIZATIONS], boost::extents[0], allocator)
    {
    }

  } // namespace Cobalt
} // namespace LOFAR

#endif

