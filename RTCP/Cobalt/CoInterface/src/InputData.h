//# InputData.h
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

#ifndef LOFAR_CNPROC_INPUT_DATA_H
#define LOFAR_CNPROC_INPUT_DATA_H

#include <vector>

#include <Common/DataConvert.h>
#include <CoInterface/Allocator.h>
#include <CoInterface/MultiDimArray.h>
#include <CoInterface/Config.h>
#include <CoInterface/StreamableData.h>
#include <Stream/Stream.h>


namespace LOFAR
{
  namespace Cobalt
  {

    template <typename SAMPLE_TYPE>
    class InputData : public SampleData<SAMPLE_TYPE,3,1>
    {
    public:
      typedef SampleData<SAMPLE_TYPE,3,1> SuperType;

      InputData(unsigned nrSubbands, unsigned nrSamplesToCNProc, Allocator &allocator = heapAllocator);

      // used for asynchronous transpose
      void readOne(Stream *str, unsigned subbandPosition);

    protected:
      virtual void checkEndianness();
    };


    template <typename SAMPLE_TYPE>
    inline InputData<SAMPLE_TYPE>::InputData(unsigned nrSubbands, unsigned nrSamplesToCNProc, Allocator &allocator)
      :
      SuperType(boost::extents[nrSubbands][nrSamplesToCNProc][NR_POLARIZATIONS], boost::extents[0], allocator)
    {
    }

    // used for asynchronous transpose
    template <typename SAMPLE_TYPE>
    inline void InputData<SAMPLE_TYPE>::readOne(Stream *str, unsigned subbandPosition)
    {
      str->read(SuperType::samples[subbandPosition].origin(), SuperType::samples[subbandPosition].num_elements() * sizeof(SAMPLE_TYPE));

#if defined C_IMPLEMENTATION && defined WORDS_BIGENDIAN
      dataConvert(LittleEndian, SuperType::samples[subbandPosition].origin(), SuperType::samples[subbandPosition].num_elements());
#endif
    }

    template <typename SAMPLE_TYPE>
    inline void InputData<SAMPLE_TYPE>::checkEndianness()
    {
#if defined C_IMPLEMENTATION && defined WORDS_BIGENDIAN
      dataConvert(LittleEndian, SuperType::samples.origin(), SuperType::samples.num_elements());
#endif
    }

  } // namespace Cobalt
} // namespace LOFAR

#endif

