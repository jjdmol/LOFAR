/* CN_Mapping.h
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

#ifndef LOFAR_GPUPROC_CN_MAPPING_H
#define LOFAR_GPUPROC_CN_MAPPING_H

namespace LOFAR
{
  namespace RTCP
  {

    class CN_Mapping
    {
    public:
      // Reshuffle cores within different psets differently, to make the transpose
      // over the 3D-torus much more efficient.  Without reshuffling, transposing
      // cores often communicate in the same line or plane in the torus, causing
      // severe bottlenecks over a few links.  With reshuffling, there are more
      // redundant links, significantly improving the bandwidth.  TODO: improve
      // the reshuffling function further, to minimize transpose times.

      static unsigned mapCoreOnPset(unsigned core, unsigned pset);
      static unsigned reverseMapCoreOnPset(unsigned core, unsigned pset);
    };

  } // namespace RTCP
} // namespace LOFAR

#endif

