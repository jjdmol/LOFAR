/* tRSPTimeStamp2.cc
 * Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
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
 * $Id: $
 */

#include <lofar_config.h>

#include <Common/LofarTypes.h>
#include <Common/LofarLogger.h>
#include <CoInterface/RSPTimeStamp.h>

using namespace LOFAR;
using namespace Cobalt;

int main( int, char **argv )
{
  INIT_LOGGER(argv[0]);

  unsigned clock = 200 * 1000 * 1000;

  {
    TimeStamp ts(0, 0, clock);

    for (int64 i = 0; i < clock * 3; ++i, ++ts) {
      #define REPORT "(ts == " << ts << ", i == " << i << ")"

      ASSERTSTR( (int64)ts == i, REPORT );

      ASSERTSTR( ts.getSeqId() == 1024 * i / clock, REPORT );

      ASSERTSTR( ts.getBlockId() == 1024 * i % clock / 1024, REPORT );
    }
  }
}

