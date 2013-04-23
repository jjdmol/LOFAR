/* Generator.h
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

#ifndef LOFAR_INPUT_PROC_GENERATOR_H
#define LOFAR_INPUT_PROC_GENERATOR_H

#include <string>
#include <vector>

#include <CoInterface/RSPTimeStamp.h>

#include <RSPBoards.h>
#include <Buffer/BufferSettings.h>

#include "RSP.h"

namespace LOFAR
{
  namespace Cobalt
  {

    /* Generate station input data */

    class Generator : public RSPBoards
    {
    public:
      Generator( const BufferSettings &settings, const std::vector<std::string> &streamDescriptors );

    protected:
      const BufferSettings settings;
      const std::vector<std::string> streamDescriptors;

      std::vector<size_t> nrSent;

      virtual void processBoard( size_t nr );
      virtual void logStatistics();

      virtual void makePacket( size_t boardNr, struct RSP &packet, const TimeStamp &timestamp );
    };

  }
}

#endif

