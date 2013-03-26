//# RSP: RSP data format
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

#ifndef LOFAR_GPUPROC_RSP_H
#define LOFAR_GPUPROC_RSP_H

#include <stdint.h>

namespace LOFAR
{
  namespace Cobalt
  {

    // All data is in Little Endian format!
    struct RSP {
      struct Header {
        uint8_t version;
        uint8_t sourceInfo;
        uint16_t configuration;
        uint16_t station;
        uint8_t nrBeamlets;
        uint8_t nrBlocks;
        uint32_t timestamp;
        uint32_t blockSequenceNumber;
      } header;

      char data[8130];
    };

  } // namespace Cobalt
} // namespace LOFAR

#endif

