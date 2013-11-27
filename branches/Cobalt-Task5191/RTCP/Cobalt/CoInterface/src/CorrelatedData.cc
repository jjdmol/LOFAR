//# CorrelatedData.cc
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

#include "CorrelatedData.h"
#include <Common/LofarLogger.h>

namespace LOFAR
{
  namespace Cobalt
  {
    namespace 
    {
      void addNrValidSamples(uint32_t * __restrict__ dst, 
                             const uint32_t * __restrict__ src,
                             unsigned count)
      {
        for (unsigned i = 0; i < count; i++)
          dst[i] += src[i];
      }

      void addNrValidSamples(uint16_t * __restrict__ dst,
                             const uint16_t * __restrict__ src,
                             unsigned count)
      {
        addNrValidSamples(reinterpret_cast<uint32_t*>(dst),
                          reinterpret_cast<const uint32_t*>(src),
                          count / 2);
        if (count & 1)
          dst[count - 1] += src[count - 1];
      }

      void addNrValidSamples(uint8_t * __restrict__ dst,
                             const uint8_t * __restrict__ src,
                             unsigned count)
      {
        addNrValidSamples(reinterpret_cast<uint16_t*>(dst),
                          reinterpret_cast<const uint16_t*>(src),
                          count / 2);
        if (count & 1)
          dst[count - 1] += src[count - 1];
      }
    }


    CorrelatedData::CorrelatedData(unsigned nrStations,
                                   unsigned nrChannels,
                                   unsigned maxNrValidSamples, 
                                   Allocator &allocator,
                                   unsigned alignment) :
      itsAlignment(alignment),
      itsNrBaselines(nrStations * (nrStations + 1) / 2),
      visibilities(boost::extents
                   [itsNrBaselines]
                   [nrChannels]
                   [NR_POLARIZATIONS]
                   [NR_POLARIZATIONS],
                   itsAlignment,
                   allocator,
                   true),
      itsNrBytesPerNrValidSamples(
        maxNrValidSamples < 256 ? 1 : maxNrValidSamples < 65536 ? 2 : 4)
    {
      init(nrChannels, allocator);
    }



    CorrelatedData::CorrelatedData(unsigned nrStations,
                                   unsigned nrChannels,
                                   unsigned maxNrValidSamples, 
                                   fcomplex *visibilities,
                                   size_t nrVisibilities,
                                   Allocator &allocator,
                                   unsigned alignment) :
      itsAlignment(alignment),
      itsNrBaselines(nrStations * (nrStations + 1) / 2),
      visibilities(boost::extents
                   [itsNrBaselines]
                   [nrChannels]
                   [NR_POLARIZATIONS]
                   [NR_POLARIZATIONS],
                   visibilities,
                   false),
      itsNrBytesPerNrValidSamples(
        maxNrValidSamples < 256 ? 1 : maxNrValidSamples < 65536 ? 2 : 4)
    {
      ASSERT(this->visibilities.num_elements() == nrVisibilities);
      init(nrChannels, allocator);
    }


    void CorrelatedData::init(unsigned nrChannels, Allocator &allocator)
    {
      switch (itsNrBytesPerNrValidSamples) {
      case 4:
        itsNrValidSamples4.resize(
          boost::extents[itsNrBaselines][nrChannels],
          itsAlignment, allocator, true);
        break;
      case 2: 
        itsNrValidSamples2.resize(
          boost::extents[itsNrBaselines][nrChannels],
          itsAlignment, allocator, true);
        break;
      case 1:
        itsNrValidSamples1.resize(
          boost::extents[itsNrBaselines][nrChannels],
          itsAlignment, allocator, true);
        break;
      }

      // zero weights
      for (size_t bl = 0; bl < itsNrBaselines; ++bl) {
        for (size_t ch = 0; ch < nrChannels; ++ch) {
          setNrValidSamples(bl, ch, 0);
        }
      }
    }


    unsigned CorrelatedData::getNrValidSamples(unsigned bl, unsigned ch)
    {
      switch (itsNrBytesPerNrValidSamples) {
      case 4:
        return nrValidSamples<uint32_t>(bl, ch);
      case 2:
        return nrValidSamples<uint16_t>(bl, ch);
      case 1: 
        return nrValidSamples<uint8_t>(bl, ch);
      default:
        return 0;
      }
    }


    void CorrelatedData::setNrValidSamples(unsigned bl, unsigned ch,
                                           unsigned value)
    {
      switch (itsNrBytesPerNrValidSamples) {
        case 4:
          nrValidSamples<uint32_t>(bl, ch) = value;
          break;
        case 2:
          nrValidSamples<uint16_t>(bl, ch) = value;
          break;
        case 1:
          nrValidSamples<uint8_t>(bl, ch) = value;
          break;
      }
    }


    void CorrelatedData::readData(Stream *str, unsigned alignment)
    {
      ASSERT(alignment <= itsAlignment);

      str->read(visibilities.origin(), 
                align(visibilities.num_elements() * sizeof(fcomplex),
                      alignment));

      switch (itsNrBytesPerNrValidSamples) {
      case 4:
        str->read(itsNrValidSamples4.origin(),
                  align(itsNrValidSamples4.num_elements() * sizeof(uint32_t),
                        alignment));
        break;
      case 2:
        str->read(itsNrValidSamples2.origin(),
                  align(itsNrValidSamples2.num_elements() * sizeof(uint16_t),
                        alignment));
        break;
      case 1:
        str->read(itsNrValidSamples1.origin(),
                  align(itsNrValidSamples1.num_elements() * sizeof(uint8_t),
                        alignment));
        break;
      }
    }


    void CorrelatedData::writeData(Stream *str, unsigned alignment)
    {
      ASSERT(alignment <= itsAlignment);

      str->write(
        visibilities.origin(),
        align(visibilities.num_elements() * sizeof *visibilities.origin(),
              alignment));

      switch (itsNrBytesPerNrValidSamples) {
      case 4: 
        str->write(itsNrValidSamples4.origin(), 
                   align(itsNrValidSamples4.num_elements() * sizeof(uint32_t),
                         alignment));
        break;
      case 2:
        str->write(itsNrValidSamples2.origin(),
                   align(itsNrValidSamples2.num_elements() * sizeof(uint16_t),
                         alignment));
        break;
      case 1: 
        str->write(itsNrValidSamples1.origin(), 
                   align(itsNrValidSamples1.num_elements() * sizeof(uint8_t),
                         alignment));
        break;
      }
    }


    CorrelatedData &CorrelatedData::operator += (const CorrelatedData &other)
    {
      // Number of visibilities in \c *this and \a other must be equal.
      ASSERT(visibilities.num_elements() == other.visibilities.num_elements());

      fcomplex       *dst = visibilities.origin();
      const fcomplex *src = other.visibilities.origin();
      unsigned count = visibilities.num_elements();

      for (unsigned i = 0; i < count; i++)
        dst[i] += src[i];

      // add nr. valid samples
      switch (itsNrBytesPerNrValidSamples) {
      case 4:
        addNrValidSamples(itsNrValidSamples4.origin(), 
                          other.itsNrValidSamples4.origin(),
                          itsNrValidSamples4.num_elements());
        break;
      case 2: 
        addNrValidSamples(itsNrValidSamples2.origin(), 
                          other.itsNrValidSamples2.origin(),
                          itsNrValidSamples2.num_elements());
        break;
      case 1: 
        addNrValidSamples(itsNrValidSamples1.origin(),
                          other.itsNrValidSamples1.origin(),
                          itsNrValidSamples1.num_elements());
        break;
      }

      return *this;
    }

  } // namespace Cobalt

} // namespace LOFAR
