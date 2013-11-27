//# CorrelatedData.h
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

#ifndef LOFAR_INTERFACE_CORRELATED_DATA_H
#define LOFAR_INTERFACE_CORRELATED_DATA_H

#include <Common/lofar_complex.h>
#include <Common/DataConvert.h>
#include <Stream/Stream.h>
#include <CoInterface/Align.h>
#include <CoInterface/Allocator.h>
#include <CoInterface/Config.h>
#include <CoInterface/StreamableData.h>
#include <CoInterface/MultiDimArray.h>
#include <CoInterface/OutputTypes.h>


namespace LOFAR
{
  namespace Cobalt
  {

    class CorrelatedData : public StreamableData
    {
    public:
      CorrelatedData(unsigned nrStations, unsigned nrChannels,
                     unsigned maxNrValidSamples, Allocator & = heapAllocator,
                     unsigned alignment = 1);

      CorrelatedData(unsigned nrStations, unsigned nrChannels,
                     unsigned maxNrValidSamples, fcomplex *visibilities,
                     size_t nrVisibilities, Allocator & = heapAllocator, 
                     unsigned alignment = 1);

      CorrelatedData &operator += (const CorrelatedData &);

      // Fast access to weights; T = uint32_t, uint16_t, or uint8_t,
      // based on itsNrBytesPerNrValidSamples.
      template<typename T> T &nrValidSamples(unsigned bl, unsigned ch);

      // Slow short-cut functions. Use for testing only!
      unsigned getNrValidSamples(unsigned bl, unsigned ch);
      void setNrValidSamples(unsigned bl, unsigned ch, unsigned value);

      const unsigned itsAlignment;
      const unsigned itsNrBaselines;

      // 4-D array of
      // <tt>[nrBaselines][nrChannels][NR_POLARIZATIONS][NR_POLARIZATIONS]</tt>
      MultiDimArray<fcomplex, 4>  visibilities;

      // The size of the nrValidSamples is determined by the maximum value that
      // has to be stored, which fits either in 1, 2, or 4 bytes.
      const unsigned itsNrBytesPerNrValidSamples;     // 1, 2, or 4

    protected:
      virtual void                readData(Stream *, unsigned);
      virtual void                writeData(Stream *, unsigned);

    private:
      void init(unsigned nrChannels, Allocator &allocator);

      Matrix<uint32_t>  itsNrValidSamples4; // [nrBaselines][nrChannels]
      Matrix<uint16_t>  itsNrValidSamples2; // [nrBaselines][nrChannels]
      Matrix<uint8_t>   itsNrValidSamples1; // [nrBaselines][nrChannels]
    };


    //## ----------------  Template specializations  ---------------- ##//

    template<> inline uint32_t&
    CorrelatedData::nrValidSamples<uint32_t>(unsigned bl, unsigned ch)
    {
      return itsNrValidSamples4[bl][ch];
    }

    template<> inline uint16_t&
    CorrelatedData::nrValidSamples<uint16_t>(unsigned bl, unsigned ch)
    {
      return itsNrValidSamples2[bl][ch];
    }

    template<> inline uint8_t&
    CorrelatedData::nrValidSamples<uint8_t>(unsigned bl, unsigned ch)
    {
      return itsNrValidSamples1[bl][ch];
    }


  } // namespace Cobalt

} // namespace LOFAR

#endif

