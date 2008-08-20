/***************************************************************************
 *   Copyright (C) 2006-8 by ASTRON, Adriaan Renting                       *
 *   renting@astron.nl                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef __FLAGGER_MADFLAGGER_H__
#define __FLAGGER_MADFLAGGER_H__

#include <casa/Arrays.h>
#include <utility>
#include <vector>
#include <list>
#include <map>
#include "Flagger.h"

namespace LOFAR
{
  namespace CS1
  {
    //Foreward declarations
    class DataBuffer;
    class MsInfo;
    class RunDetails;
    class FlaggerStatistics;

    class MADFlagger: public Flagger
    {
      public:
        MADFlagger();
        ~MADFlagger();

        void ProcessTimeslot(DataBuffer& data,
                             MsInfo& info,
                             RunDetails& details,
                             FlaggerStatistics& stats);

      protected:
      private:
        void ComputeThreshold(const casa::Cube<casa::Complex>& Values,
                              int TWindowSize, int FWindowSize,
                              int TimePos, int ChanPos, int PolPos,
                              float& Z1, float& Z2, casa::Matrix<casa::Float>& Medians);
        int FlagBaselineBand(casa::Matrix<casa::Bool>& Flags,
                             const casa::Cube<casa::Complex>& Data,
                             int flagCounter,
                             double Level,
                             int Position, bool Existing,
                             int TWindowSize, int FWindowSize);
        int NumChannels;
        int NumPolarizations;
    }; // MADFlagger
  }; // CS1
}; // namespace LOFAR

#endif //  __FLAGGER_MADFLAGGER_H__
