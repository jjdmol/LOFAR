//# SubbandMetaData.h
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

#ifndef LOFAR_INTERFACE_SUBBAND_META_DATA_H
#define LOFAR_INTERFACE_SUBBAND_META_DATA_H

#include <vector>

#include <Common/LofarLogger.h>
#include <Stream/Stream.h>
#include <CoInterface/SparseSet.h>

namespace LOFAR
{
  namespace Cobalt
  {

    struct SubbandMetaData
    {
    public:
      SubbandMetaData(unsigned nrTABs);

      struct beamInfo {
        float delayAtBegin;
        float delayAfterEnd;
        double beamDirectionAtBegin[3];
        double beamDirectionAfterEnd[3];
      };

      // delays for all directions
      beamInfo stationBeam;
      std::vector<beamInfo> TABs;

      // flag set.
      SparseSet<unsigned> flags;

      void read(Stream *str);
      void write(Stream *str) const;

      static const size_t MAXFLAGSIZE = 132;
    };


    inline SubbandMetaData::SubbandMetaData(unsigned nrTABs)
      :
      TABs(nrTABs)
    {
    }


    inline void SubbandMetaData::read(Stream *str)
    {
      // read station beam
      str->read(&stationBeam, sizeof stationBeam);

      // read TABs
      size_t nrTABs;
      str->read(&nrTABs, sizeof nrTABs);
      TABs.resize(nrTABs);
      str->read(&TABs[0], TABs.size() * sizeof TABs[0]);

      // read flags
      std::vector<char> flagsBuffer(MAXFLAGSIZE);
      str->read(&flagsBuffer[0], flagsBuffer.size());
      flags.unmarshall(&flagsBuffer[0]);
    }


    inline void SubbandMetaData::write(Stream *str) const
    {
      // write station beam
      str->write(&stationBeam, sizeof stationBeam);

      // write TABs
      size_t nrTABs = TABs.size();
      str->write(&nrTABs, sizeof nrTABs);
      str->write(&TABs[0], TABs.size() * sizeof TABs[0]);

      // write flags
      std::vector<char> flagsBuffer(MAXFLAGSIZE);

      ssize_t size = flags.marshall(&flagsBuffer[0], flagsBuffer.size());
      ASSERT(size >= 0);

      str->write(&flagsBuffer[0], flagsBuffer.size());
    }

  } // namespace Cobalt
} // namespace LOFAR

#endif

