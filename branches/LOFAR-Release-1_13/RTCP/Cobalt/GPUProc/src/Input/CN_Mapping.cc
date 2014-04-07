/* CN_Mapping.cc: map work to cores on BG/L psets
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
 * $Id$
 */

#include <lofar_config.h>

#include "CN_Mapping.h"

namespace LOFAR
{
  namespace RTCP
  {

    unsigned CN_Mapping::mapCoreOnPset(unsigned core, unsigned pset)
    {
#if defined HAVE_BGP
      //return core ^ ((pset & 0x1) << 2) ^ ((pset & 0x02) >> 1) ^ ((pset & 0x04) >> 1) ^ ((pset & 0x08)) ^ ((pset & 0x10) >> 1) ^ ((pset & 0x20) >> 3);

      // TODO: there may be better mappings for partitions larger than one rack
      static unsigned char mapX[] = { 0, 12 };
      static unsigned char mapY[] = { 0,  2, 10,  8 };
      static unsigned char mapZ[] = { 0,  1,  3,  2,  6,  7,  5,  4 };

      return core ^
             mapX[((pset & 0x08) >> 3)] ^
             mapY[((pset & 0x01) >> 0) | ((pset & 0x10) >> 3)] ^
             mapZ[((pset & 0x03) >> 1) | ((pset & 0x20) >> 3)];

#else
      (void)pset;

      return core;
#endif
    }

    unsigned CN_Mapping::reverseMapCoreOnPset(unsigned core, unsigned pset)
    {
      // just the same function
      return mapCoreOnPset(core, pset);
    }


  } // namespace RTCP
} // namespace LOFAR

