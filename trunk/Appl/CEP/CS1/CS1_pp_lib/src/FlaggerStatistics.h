/***************************************************************************
 *   Copyright (C) 2006 by ASTRON, Adriaan Renting                         *
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
#ifndef __CS1_PP_FLAGGER_STATISTICS_H__
#define __CS1_PP_FLAGGER_STATISTICS_H__

#include <casa/Arrays.h>
#include <iostream>
#include <vector>
#include "MsInfo.h"

namespace LOFAR
{
  namespace CS1
  {
    class FlaggerStatistics
    {
      public:
        FlaggerStatistics(MsInfo& info);
        ~FlaggerStatistics();
        void PrintStatistics(std::ostream& output);
        int& operator()(int x, int y, int z);

      protected:
      private:
        int                       NumAntennae;
        int                       NumBands;
        casa::Cube< int >         Statistics;
        int                       Normalizer;
        std::vector<casa::String> AntennaNames;
    }; // FlaggerStatistics
  }; // CS1
}; // namespace LOFAR

#endif //  __CS1_PP_FLAGGER_STATISTICS_H__
